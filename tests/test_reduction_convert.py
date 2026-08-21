import importlib.util
import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
CONVERTER_PATH = ROOT / "tools" / "kernel_convert.py"
SPEC = importlib.util.spec_from_file_location("kernel_convert", CONVERTER_PATH)
kernel_convert = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(kernel_convert)

EXAMPLE = (
    ROOT
    / "examples"
    / "simtPrimitives"
    / "reduction.cu"
)


class ReductionConvertTests(unittest.TestCase):
    def test_example_exists(self):
        self.assertTrue(EXAMPLE.is_file())
        source = EXAMPLE.read_text()
        # 纯 CUDA 写法：kernel 体不使用昇腾专属符号。
        self.assertIn("__global__ void blockReduceKernel", source)
        self.assertIn("__shared__ float sdata", source)
        self.assertIn("__syncthreads()", source)
        # 真机教训：非指针标量不能带 __ubuf__，计数器用长度 1 的数组承载。
        self.assertIn("__shared__ unsigned temp[1]", source)
        self.assertIn("atomicAdd(&temp[0], 1u)", source)

    def test_conversion_produces_simt_vf_wrappers_with_hidden_dims(self):
        source = EXAMPLE.read_text()
        converted, count = kernel_convert.convert(source)

        self.assertEqual(count, 2)
        # 两个 kernel 都被拆成 __simt_vf__ 计算体 + __global__[aicore] wrapper
        self.assertEqual(converted.count("__global__[aicore] void blockReduceKernel("), 1)
        self.assertEqual(converted.count("__global__[aicore] void smemAtomicCounterKernel("), 1)
        self.assertEqual(converted.count("__simt_vf__"), 2)
        # 隐藏 block 形状参数被附加到 wrapper 与 launch
        self.assertIn("unsigned __cu2cce_bx", converted)
        self.assertIn("cce::async_invoke<blockReduceKernel_VF>(", converted)
        self.assertIn("cce::async_invoke<smemAtomicCounterKernel_VF>(", converted)
        # 共享内存/同步/原子写法原样保留在 VF 体内
        self.assertIn("__shared__ float sdata", converted)
        self.assertIn("__syncthreads()", converted)
        self.assertIn("atomicAdd(&temp[0], 1u)", converted)

    def test_launches_are_four_to_three_arg_with_hidden_blocks(self):
        source = EXAMPLE.read_text()
        converted, _ = kernel_convert.convert(source)

        # <<<blocks, THREADS, 0, stream>>> -> <<<blocks, THREADS, stream>>>(…, 256, 1, 1)
        self.assertIn(
            "blockReduceKernel<<<blocks, THREADS, stream>>>("
            "in, partial, N, THREADS, 1, 1);",
            converted,
        )
        self.assertIn(
            "smemAtomicCounterKernel<<<blocks, THREADS, stream>>>("
            "counter, N, THREADS, 1, 1);",
            converted,
        )


if __name__ == "__main__":
    unittest.main()