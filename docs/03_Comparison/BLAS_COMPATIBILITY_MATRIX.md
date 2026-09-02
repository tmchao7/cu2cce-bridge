# CUDA BLAS → CANN ACL CBLAS experimental matrix

Branch: `main`（experimental capability）
Backend: CANN `acl/ops/acl_cblas.h` / `libacl_cblas.so`
Main puresimt regression: independent

## Status policy

- `SUPPORTED`: CUDA API and numerical semantics have a real-device test with no known semantic loss.
- `EXPERIMENTAL`: implementation or real-device evidence exists, but dtype/model/precision/semantic limits remain.
- `UNSUPPORTED`: current backend has no usable equivalent or wrapper returns not-support.

## Current coverage

Latest real-device evidence: 2026-08-28, CANN 9.2.0, `Ascend950DT_9582`,
`BLAS_EXPERIMENTAL_REGRESSION_PASS` with `FAIL=0`: all three cases
(`cublas_sgemm`, `cublas_gemmex`, `cublas_gemv`) pass end-to-end.

| CUDA API | CANN path | Data types | Current status | Limitation |
|---|---|---|---|---|
| `cublasCreate/Destroy` | custom handle allocation | n/a | `EXPERIMENTAL` | no native CANN cublas handle |
| `cublasSetStream/GetStream` | handle-held `aclrtStream` | n/a | `EXPERIMENTAL` | CANN BLAS is stream-based, not handle-based |
| `cublasSetPointerMode/GetPointerMode` | handle-held mode | n/a | `EXPERIMENTAL` | HOST mode stages scalars to device (real-device lesson); DEVICE mode passes pointers through unchanged |
| `cublasSetMathMode/GetMathMode` | handle-held mode | n/a | `EXPERIMENTAL` | performance-hint semantics only: stored and returned, never changes numerics |
| `cublasSetAtomicsMode/GetAtomicsMode` | handle-held mode | n/a | `EXPERIMENTAL` | stored and returned; Ascend exposes no equivalent scheduling switch |
| `cublasSgemm` | `aclblasGemmEx` | FP32 input staged to FP16, FP32 C | `EXPERIMENTAL` | **PASS:** 32×32×32 identity-B test; requires ATC/OPP model and has FP32→FP16 precision loss |
| `cublasGemmEx` / `cublasSgemmEx` | `aclblasGemmEx` | (16F,16F,16F) / (16F,16F,32F) / (8I,8I,32I); compute 32F family | `EXPERIMENTAL` | **PASS:** 32×32×32 FP16×FP16→FP32 case; fp64 combos honestly rejected; shape coverage narrow |
| `cublasSgemv` | `aclblasGemvEx` | FP32 | `SUPPORTED` for validated shape | **PASS:** 8×8 identity test, no semantic loss (native FP32 path). Contract discovered via server ErrMsg: (lda, incx, incy) is a reserved triple that must be (-1,-1,-1) — contiguous vectors only; host-side rejects stride≠1 with guidance. Larger shapes/transposes pending |
| `cublasDgemm` | none | FP64 | `UNSUPPORTED` | CANN 9.2 public `aclDataType` has no double GEMM path |
| `cublasSaxpy/Scopy/Sdot/Sscal/Sswap/Snrm2` | placeholder | FP32 | `UNSUPPORTED` | return API-not-support **after** full parameter validation (unified `blas1_validate`), so error attribution stays consistent |
| `aclblasHgemv/S8gemv` (native variants) | direct ACL, unmapped | FP16 / INT8 | `UNSUPPORTED` (not exposed) | available for future dtype extensions; same reserved-triple contract applies |

## Current passing tests

```text
examples/blas/cublas_sgemm.cu  → cublas_sgemm_experimental OK  (FP32→FP16 GemmEx, 32x32x32)
examples/blas/cublas_gemmex.cu → cublas_gemmex_experimental OK (FP16xFP16->FP32, 32x32x32)
examples/blas/cublas_gemv.cu   → cublas_gemv_experimental OK   (FP32 identity Sgemv, 8x8)
```

Pending real-device cases (added to the regression):

```text
examples/blas/cublas_gemmex.cu → cublas_gemmex_experimental OK (FP16xFP16->FP32)
examples/blas/cublas_gemv.cu   → cublas_gemv_experimental OK (FP32 identity Sgemv)
```

## Real-device entrypoint

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
REMOTE="${CU2CCE_REGRESSION_REMOTE:-origin}"
git fetch "$REMOTE" main
git pull --ff-only "$REMOTE" main
bash tools/real_test/clean.sh
CU2CCE_REGRESSION_REMOTE="$REMOTE" CU2CCE_BLAS_TEST=1 \
  bash tools/real_test/server_cmd.txt
```

Expected success marker:

```text
BLAS_EXPERIMENTAL_REGRESSION_PASS
```
