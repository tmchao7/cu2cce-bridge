import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
EXAMPLE = ROOT / "examples" / "cublasMatmul" / "matmul.cu"


class CublasMatmulExampleTests(unittest.TestCase):
    def test_example_uses_pure_cuda_cublas_api(self):
        source = EXAMPLE.read_text()
        self.assertIn("#include <cublas_v2.h>", source)
        self.assertIn("cublasCreate", source)
        self.assertIn("cublasSgemm", source)
        self.assertIn("CUBLAS_OP_N", source)
        self.assertIn("cudaMemset", source)  # 昇腾不清零：gemm 前手动初始化 C
        self.assertIn("cublas matmul OK", source)

    def test_example_does_not_need_kernel_conversion(self):
        import importlib.util

        spec = importlib.util.spec_from_file_location(
            "kernel_convert", ROOT / "tools" / "kernel_convert.py"
        )
        kernel_convert = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(kernel_convert)
        converted, count = kernel_convert.convert(EXAMPLE.read_text())
        # 本示例只有 host 侧 BLAS 调用，没有 CUDA kernel；应完全不被改写。
        self.assertEqual(count, 0)
        self.assertEqual(converted, EXAMPLE.read_text())


if __name__ == "__main__":
    unittest.main()