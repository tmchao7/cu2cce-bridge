# CUDA BLAS → CANN ACL CBLAS experimental matrix

Branch: `main`（experimental capability）
Backend: CANN `acl/ops/acl_cblas.h` / `libacl_cblas.so`
Main puresimt regression: independent

## Status policy

- `SUPPORTED`: CUDA API and numerical semantics have a real-device test with no known semantic loss.
- `EXPERIMENTAL`: implementation or real-device evidence exists, but dtype/model/precision/semantic limits remain.
- `UNSUPPORTED`: current backend has no usable equivalent or wrapper returns not-support.

## Current coverage

Latest real-device evidence: 2026-08-24, CANN 9.2.0, `Ascend950DT_9582`,
`BLAS_EXPERIMENTAL_REGRESSION_PASS`. The `cublasSgemm` row below has a passing
32×32×32 end-to-end test, but remains `EXPERIMENTAL` because the wrapper performs
FP32→FP16 input staging and requires an ATC/OPP GEMM model.

| CUDA API | CANN path | Data types | Current status | Limitation |
|---|---|---|---|---|
| `cublasCreate/Destroy` | custom handle allocation | n/a | `EXPERIMENTAL` | no native CANN cublas handle |
| `cublasSetStream/GetStream` | handle-held `aclrtStream` | n/a | `EXPERIMENTAL` | CANN BLAS is stream-based, not handle-based |
| `cublasSetPointerMode/GetPointerMode` | handle-held mode | n/a | `EXPERIMENTAL` | HOST mode stages scalars to device (real-device lesson); DEVICE mode passes pointers through unchanged |
| `cublasSetMathMode/GetMathMode` | handle-held mode | n/a | `EXPERIMENTAL` | performance-hint semantics only: stored and returned, never changes numerics |
| `cublasSetAtomicsMode/GetAtomicsMode` | handle-held mode | n/a | `EXPERIMENTAL` | stored and returned; Ascend exposes no equivalent scheduling switch |
| `cublasSgemm` | `aclblasGemmEx` | FP32 input staged to FP16, FP32 C | `EXPERIMENTAL` | **PASS:** 32×32×32 identity-B test; requires ATC/OPP model and has FP32→FP16 precision loss |
| `cublasGemmEx` / `cublasSgemmEx` | `aclblasGemmEx` | (16F,16F,16F) / (16F,16F,32F) / (8I,8I,32I); compute 32F family | `EXPERIMENTAL` until device proof | dtype-explicit entry added; no bridge-side dtype restaging; scalars staged per PointerMode; computeType limited to 32F/16F family (others honestly NOT_SUPPORT); real-device case `cublas_gemmex_experimental OK` pending |
| `cublasSgemv` | `aclblasGemvEx` | FP32 | `EXPERIMENTAL` | **契约实证（round-4 ErrMsg）**：(lda, incx, incy) 三元组为保留参数、必须全 -1（后端仅支持连续向量）；步长非 1 宿主侧明确拒绝；scalars staging + aclopSetModelDir 已对齐；真机 `cublas_gemv_experimental OK` 待本轮确认 |
| `cublasDgemm` | none | FP64 | `UNSUPPORTED` | CANN 9.2 public `aclDataType` has no double GEMM path |
| `cublasSaxpy/Scopy/Sdot/Sscal/Sswap/Snrm2` | placeholder | FP32 | `UNSUPPORTED` | return API-not-support **after** full parameter validation (unified `blas1_validate`), so error attribution stays consistent |

## Current passing test

```text
examples/blas/cublas_sgemm.cu
→ cublasCreate/SetStream/Sgemm/Destroy
→ C = A × I numerical verification
→ cublas_sgemm_experimental OK (FP32 CUDA -> FP16 GemmEx, 32x32x32)
```

Pending real-device cases (added to the regression):

```text
examples/blas/cublas_gemmex.cu → cublas_gemmex_experimental OK (FP16xFP16->FP32)
examples/blas/cublas_gemv.cu   → cublas_gemv_experimental OK (FP32 identity Sgemv)
```

## Real-device entrypoint

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git pull --ff-only gitcode main
CU2CCE_BLAS_TEST=1 bash tools/real_test/server_cmd.txt
```

Expected success marker:

```text
BLAS_EXPERIMENTAL_REGRESSION_PASS
```
