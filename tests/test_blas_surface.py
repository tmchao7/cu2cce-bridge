#!/usr/bin/env python3
"""Contract tests for the cuBLAS compatibility surface (batch B)."""
from __future__ import annotations

import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class BlasSurfaceContractTest(unittest.TestCase):
    def test_gemmex_and_mode_surface_is_declared(self) -> None:
        header = (ROOT / "include/bridge/blas/cublas_v2_wrapper.h").read_text(encoding="utf-8")
        # dtype/compute constants mirror NVIDIA values
        for token in ("CUDA_R_16F = 150", "CUDA_R_32F = 151", "CUDA_R_8I = 160",
                      "CUDA_R_32I = 170", "CUBLAS_COMPUTE_32F 68ULL",
                      "CUBLAS_COMPUTE_16F 64ULL"):
            self.assertIn(token, header)
        # mode surface
        for token in ("cublasPointerMode_t", "cublasMath_t", "cublasAtomicsMode_t",
                      "CUBLAS_POINTER_MODE_HOST", "CUBLAS_DEFAULT_MATH",
                      "CUBLAS_ATOMICS_NOT_ALLOWED"):
            self.assertIn(token, header)
        for macro in ("#define cublasGemmEx wcublasGemmEx",
                      "#define cublasSgemmEx wcublasSgemmEx",
                      "#define cublasSetMathMode wcublasSetMathMode",
                      "#define cublasSetPointerMode wcublasSetPointerMode",
                      "#define cublasSetAtomicsMode wcublasSetAtomicsMode"):
            self.assertIn(macro, header)
        # handle carries the modes
        self.assertIn("cublasPointerMode_t pointerMode;", header)

    def test_gemmex_impl_maps_supported_combos_and_rejects_the_rest(self) -> None:
        impl = (ROOT / "src/bridge/blas/src/cublas_v2_wrapper.cpp").read_text(encoding="utf-8")
        # supported combos are exactly the CANN-aclblas ones
        self.assertIn("(a == CUDA_R_16F && b == CUDA_R_16F && c == CUDA_R_16F)", impl)
        self.assertIn("(a == CUDA_R_8I && b == CUDA_R_8I && c == CUDA_R_32I)", impl)
        # honest rejection + staging lessons honored
        self.assertIn("gemmex_dtype_supported", impl)
        self.assertIn("ACL_COMPUTE_HIGH_PRECISION", impl)
        self.assertIn("scalar_bytes", impl)
        self.assertIn("CUBLAS_POINTER_MODE_DEVICE", impl)
        # BLAS1 placeholders validate before not-support
        self.assertIn("blas1_validate", impl)
        self.assertNotIn("not_supported_placeholder()", impl)
        # GemvEx reserved-parameter contract (server ErrMsg round-4): the
        # (lda, incx, incy) triple must be (-1, -1, -1); strides other than 1
        # are rejected on the host side with a clear message.
        self.assertIn("A, -1, ACL_FLOAT, x, -1, ACL_FLOAT", impl)
        self.assertIn("y, -1, ACL_FLOAT", impl)
        self.assertIn("unsupported stride incx", impl)

    def test_blas_regression_has_three_cases(self) -> None:
        script = (ROOT / "tools/real_test/blas_regression.sh").read_text(encoding="utf-8")
        self.assertIn("run_blas_case cublas_sgemm examples/blas/cublas_sgemm.cu", script)
        self.assertIn("run_blas_case cublas_gemmex examples/blas/cublas_gemmex.cu", script)
        self.assertIn("run_blas_case cublas_gemv examples/blas/cublas_gemv.cu", script)
        for section in ("[1/8]", "[2/8]", "[3/8]", "[4/8]", "[5/8]", "[6/8]", "[7/8]", "[8/8]"):
            self.assertIn(section, script)
        # the GemvEx contract probe is diagnostic-only and must not gate
        self.assertIn("[7/8] aclblasGemvEx parameter-matrix probe", script)
        self.assertIn("[PROBE-ONLY] gemv_param_probe compile failed (non-blocking)", script)
        for example in ("examples/blas/cublas_gemmex.cu", "examples/blas/cublas_gemv.cu"):
            self.assertTrue((ROOT / example).is_file(), example)

    def test_blas_matrix_documents_new_surface(self) -> None:
        matrix = (ROOT / "docs/02_能力清单/BLAS_COMPATIBILITY_MATRIX.md").read_text(encoding="utf-8")
        self.assertIn("cublasGemmEx", matrix)
        self.assertIn("(16F,16F,32F)", matrix)
        self.assertIn("cublas_gemmex_experimental OK", matrix)
        self.assertIn("cublas_gemv_experimental OK", matrix)
        self.assertIn("after** full parameter validation", matrix)


if __name__ == "__main__":
    unittest.main()
