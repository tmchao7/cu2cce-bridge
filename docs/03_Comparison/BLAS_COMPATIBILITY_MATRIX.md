# CUDA BLAS → CANN ACL CBLAS experimental matrix

Branch: `exp/cublas-aclblas`
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
| `cublasSgemm` | `aclblasGemmEx` | FP32 input staged to FP16, FP32 C | `EXPERIMENTAL` | **PASS:** 32×32×32 identity-B test; requires ATC/OPP model and has FP32→FP16 precision loss |
| `cublasSgemv` | `aclblasGemvEx` | FP32 | `EXPERIMENTAL` | standalone real-device coverage pending |
| `cublasDgemm` | none | FP64 | `UNSUPPORTED` | CANN 9.2 public `aclDataType` has no double GEMM path |
| `cublasSaxpy/Scopy/Sdot/Sscal/Sswap/Snrm2` | placeholder | FP32 | `UNSUPPORTED` | current wrapper returns API-not-support |

## Current passing test

```text
examples/blas/cublas_sgemm.cu
→ cublasCreate/SetStream/Sgemm/Destroy
→ C = A × I numerical verification
→ cublas_sgemm_experimental OK (FP32 CUDA -> FP16 GemmEx, 32x32x32)
```

## Real-device entrypoint

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git pull --ff-only gitcode exp/cublas-aclblas
bash tools/real_test/server_cmd.txt
```

Expected success marker:

```text
BLAS_EXPERIMENTAL_REGRESSION_PASS
```
