import contextlib
import importlib.util
import io
import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
CONVERTER_PATH = ROOT / "tools" / "kernel_convert.py"
SPEC = importlib.util.spec_from_file_location("kernel_convert", CONVERTER_PATH)
kernel_convert = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(kernel_convert)


class KernelConvertTests(unittest.TestCase):
    def test_scalar_literal_is_forwarded_as_hidden_dimensions(self):
        source = """
__global__ void add(float *x, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) x[i] += 1.0f;
}
void launch(float *x, int n, cudaStream_t stream) {
    add<<<1, 512, 0, stream>>>(x, n);
}
"""

        converted, count = kernel_convert.convert(source)

        self.assertEqual(count, 1)
        self.assertIn(
            "void add(__gm__ float * x, int n, unsigned __cu2cce_bx, "
            "unsigned __cu2cce_by, unsigned __cu2cce_bz)",
            converted,
        )
        self.assertIn(
            "cce::async_invoke<add_VF>("
            "cce::dim3{__cu2cce_bx, __cu2cce_by, __cu2cce_bz}, x, n);",
            converted,
        )
        self.assertIn(
            "add<<<1, 512, stream>>>(x, n, 512, 1, 1);",
            converted,
        )
        self.assertNotIn("blockDim.x, blockDim.y, blockDim.z}, x, n", converted)

    def test_scalar_variable_is_forwarded_at_each_launch(self):
        source = """
__global__ void k(int *x) { x[threadIdx.x] = 1; }
void launch(int *x, cudaStream_t stream) {
    unsigned threads256 = 256;
    unsigned threads512 = 512;
    k<<<1, threads256, 0, stream>>>(x);
    k<<<1, threads512, 0, stream>>>(x);
}
"""

        converted, _ = kernel_convert.convert(source)

        self.assertIn(
            "k<<<1, threads256, stream>>>(x, threads256, 1, 1);",
            converted,
        )
        self.assertIn(
            "k<<<1, threads512, stream>>>(x, threads512, 1, 1);",
            converted,
        )
        self.assertEqual(converted.count("__global__[aicore] void k("), 1)

    def test_runtime_scalar_expression_is_preserved_not_statically_resolved(self):
        source = """
__global__ void k(int *x) { x[threadIdx.x] = 1; }
void launch(int *x, int runtime_threads, cudaStream_t stream) {
    k<<<1, runtime_threads, 0, stream>>>(x);
}
"""

        converted, _ = kernel_convert.convert(source)

        self.assertIn(
            "k<<<1, runtime_threads, stream>>>(x, runtime_threads, 1, 1);",
            converted,
        )

    def test_dim3_variable_uses_scalar_outer_thread_count_and_hidden_shape(self):
        source = """
__global__ void k(int *x) {
    int i = threadIdx.y * blockDim.x + threadIdx.x;
    x[i] = 1;
}
void launch(int *x, cudaStream_t stream) {
    dim3 block(32, 8, 1);
    k<<<1, block, 0, stream>>>(x);
}
"""

        converted, _ = kernel_convert.convert(source)

        self.assertIn(
            "k<<<1, (block.x * block.y * block.z), stream>>>("
            "x, block.x, block.y, block.z);",
            converted,
        )

    def test_multiple_kernels_are_lowered_independently(self):
        source = """
__global__ void first(int *x) { x[threadIdx.x] = 1; }
__global__ void second(int *x) { x[threadIdx.x] = 2; }
void launch(int *x, cudaStream_t stream) {
    first<<<1, 128, 0, stream>>>(x);
    second<<<1, 512, 0, stream>>>(x);
}
"""

        converted, count = kernel_convert.convert(source)

        self.assertEqual(count, 2)
        self.assertIn("first<<<1, 128, stream>>>(x, 128, 1, 1);", converted)
        self.assertIn("second<<<1, 512, stream>>>(x, 512, 1, 1);", converted)
        self.assertEqual(converted.count("cce::dim3{__cu2cce_bx"), 2)

    def test_zero_argument_kernel_gets_only_hidden_arguments(self):
        source = """
__global__ void tick() { }
void launch(cudaStream_t stream) {
    tick<<<1, 64, 0, stream>>>();
}
"""

        converted, _ = kernel_convert.convert(source)

        self.assertIn(
            "void tick(unsigned __cu2cce_bx, unsigned __cu2cce_by, "
            "unsigned __cu2cce_bz)",
            converted,
        )
        self.assertIn("tick<<<1, 64, stream>>>(64, 1, 1);", converted)
        self.assertIn(
            "cce::async_invoke<tick_VF>("
            "cce::dim3{__cu2cce_bx, __cu2cce_by, __cu2cce_bz});",
            converted,
        )

    def test_handwritten_aicore_wrapper_only_gets_four_to_three_arg_rewrite(self):
        source = """
__global__[aicore] void wrapper(__gm__ int *x, unsigned bx) {
    cce::async_invoke<vf>(cce::dim3{bx, 1, 1}, x);
}
void launch(int *x, unsigned threads, cudaStream_t stream) {
    wrapper<<<1, threads, 0, stream>>>(x, threads);
}
"""

        converted, count = kernel_convert.convert(source)

        self.assertEqual(count, 0)
        self.assertIn("wrapper<<<1, threads, stream>>>(x, threads);", converted)
        self.assertNotIn("x, threads, threads, 1, 1", converted)

    def test_launch_text_in_comments_and_strings_is_ignored(self):
        source = r"""
// fake<<<grid, block, shared, stream>>>(x);
const char *text = "fake<<<grid, block, shared, stream>>>(x)";
__global__ void k(int *x) { x[threadIdx.x] = 1; }
void launch(int *x, cudaStream_t stream) {
    k<<<1, 512, 0, stream>>>(x);
}
"""
        stderr = io.StringIO()

        with contextlib.redirect_stderr(stderr):
            converted, _ = kernel_convert.convert(source)

        self.assertEqual(stderr.getvalue(), "")
        self.assertIn("fake<<<grid, block, shared, stream>>>", converted)
        self.assertIn("k<<<1, 512, stream>>>(x, 512, 1, 1);", converted)

    def test_nonzero_shared_memory_still_warns_and_is_dropped(self):
        source = """
__global__ void k(int *x) { x[threadIdx.x] = 1; }
void launch(int *x, cudaStream_t stream) {
    k<<<1, 512, 64, stream>>>(x);
}
"""
        stderr = io.StringIO()

        with contextlib.redirect_stderr(stderr):
            converted, _ = kernel_convert.convert(source)

        self.assertIn("sharedMem=64", stderr.getvalue())
        self.assertIn("k<<<1, 512, stream>>>(x, 512, 1, 1);", converted)

    def test_pure_cuda_dynamic_block_example_exercises_1d_and_2d_lowering(self):
        example = (
            ROOT
            / "examples"
            / "experimental"
            / "dynamicBlockDim"
            / "dynamic_blockdim.cu"
        )
        source = example.read_text()

        converted, count = kernel_convert.convert(source)

        self.assertEqual(count, 2)
        self.assertIn(
            "flexibleAdd<<<1, threads256, stream>>>("
            "d256, n256, threads256, 1, 1);",
            converted,
        )
        self.assertIn(
            "flexibleAdd<<<1, threads512, stream>>>("
            "d512, n512, threads512, 1, 1);",
            converted,
        )
        self.assertIn(
            "fill2D<<<1, (block2d.x * block2d.y * block2d.z), stream>>>("
            "d2d, width, height, block2d.x, block2d.y, block2d.z);",
            converted,
        )
        self.assertIn("dynamic blockdim OK (256,512,32x8x1)", source)

    def test_host_only_input_is_unchanged(self):
        source = "int host_only(int x) { return x + 1; }\n"

        converted, count = kernel_convert.convert(source)

        self.assertEqual(count, 0)
        self.assertEqual(converted, source)


if __name__ == "__main__":
    unittest.main()
