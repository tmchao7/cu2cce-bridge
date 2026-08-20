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
    def test_wrapper_uses_runtime_block_dimensions(self):
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
            "cce::async_invoke<add_VF>("
            "cce::dim3{blockDim.x, blockDim.y, blockDim.z}, x, n);",
            converted,
        )
        self.assertNotIn("cce::dim3{256, 1, 1}", converted)

    def test_four_argument_launch_with_scalar_variable_becomes_three_arguments(self):
        source = """
__global__ void k(int *x) { x[threadIdx.x] = 1; }
void launch(int *x, cudaStream_t stream) {
    int threads = 512;
    k<<<2, threads, 0, stream>>>(x);
}
"""

        converted, _ = kernel_convert.convert(source)

        self.assertIn("k<<<2, threads, stream>>>(x);", converted)

    def test_literal_512_launch_is_preserved(self):
        source = """
__global__ void k(int *x) { x[threadIdx.x] = 1; }
void launch(int *x, cudaStream_t stream) {
    k<<<2, 512, 0, stream>>>(x);
}
"""

        converted, _ = kernel_convert.convert(source)

        self.assertIn("k<<<2, 512, stream>>>(x);", converted)

    def test_dim3_variable_launch_is_preserved(self):
        source = """
__global__ void k(int *x) { x[threadIdx.x] = 1; }
void launch(int *x, cudaStream_t stream) {
    dim3 grid(2, 3, 1);
    dim3 block(32, 8, 1);
    k<<<grid, block, 0, stream>>>(x);
}
"""

        converted, _ = kernel_convert.convert(source)

        self.assertIn("k<<<grid, block, stream>>>(x);", converted)

    def test_multiple_kernels_and_launch_configs_do_not_freeze_first_block(self):
        source = """
__global__ void first(int *x) { x[threadIdx.x] = 1; }
__global__ void second(int *x) { x[threadIdx.x] = 2; }
void launch(int *x, cudaStream_t stream) {
    first<<<1, 128, 0, stream>>>(x);
    first<<<1, 512, 0, stream>>>(x);
    second<<<1, 256, 0, stream>>>(x);
}
"""

        converted, count = kernel_convert.convert(source)

        self.assertEqual(count, 2)
        runtime_dims = "cce::dim3{blockDim.x, blockDim.y, blockDim.z}"
        self.assertEqual(converted.count(runtime_dims), 2)
        self.assertIn("first<<<1, 128, stream>>>(x);", converted)
        self.assertIn("first<<<1, 512, stream>>>(x);", converted)
        self.assertIn("second<<<1, 256, stream>>>(x);", converted)

    def test_host_only_input_is_unchanged(self):
        source = "int host_only(int x) { return x + 1; }\n"

        converted, count = kernel_convert.convert(source)

        self.assertEqual(count, 0)
        self.assertEqual(converted, source)

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
        self.assertIn("k<<<1, 512, stream>>>(x);", converted)


if __name__ == "__main__":
    unittest.main()
