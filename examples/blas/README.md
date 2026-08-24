# Experimental CUDA BLAS bridge examples

These examples are not part of the `main / puresimt` regression. They validate the
separate host-library path:

```text
CUDA cublas API
  → cu2cce cublas compatibility wrapper
  → CANN aclblas / libacl_cblas.so
```

Run them only from the `exp/cublas-aclblas` branch with:

```bash
bash tools/real_test/server_cmd.txt
```

The current sample uses `cublasSgemm` with FP32 CUDA inputs. The wrapper stages
A/B as FP16 and calls CANN `aclblasGemmEx` with FP32 C, so this is an experimental
compatibility path with possible FP32→FP16 precision loss. It also requires an
ATC GEMM model and `aclopSetModelDir` through `CU2CCE_GEMM_MODEL_DIR`.
