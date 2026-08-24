# cu-bridge 对照与 main 当前映射

完整证据矩阵：[`PURESIMT_COMPATIBILITY_MATRIX.md`](PURESIMT_COMPATIBILITY_MATRIX.md)。

## 正式路线：puresimt

| 用户 CUDA | cu2cce-bridge main | 说明 |
|---|---|---|
| `#include <cuda_runtime.h>` | `include/cuda_runtime.h` | CUDA-compatible header |
| `cudaMalloc` | `wcudaMalloc` → `aclrtMalloc` | host runtime bridge |
| `cudaMemcpy` | `wcudaMemcpy` → `aclrtMemcpy` | 同步语义适配 |
| `cudaStream*` | `wcudaStream*` → `aclrtStream*` | stream bridge |
| `__global__`, `threadIdx`, `blockIdx`, `<<<>>>` | ccec `--enable-simt` 原生解析 | 不做源码转换 |
| `__shared__`, `__syncthreads`, CUDA atomics | `simt_adaptor.h` 原语别名 | 仅符号级兼容 |
| NVIDIA helper headers | `third_party/nvidia_cuda_samples/Common` | 只作为 include 依赖 |

## 组件状态

- Runtime bridge：main 正式能力，已在真机验证。
- SIMT primitive aliases：main 正式能力，已由 vector/matrix/atomic/reduction 验证。
- `bin/cucc`：main 正式驱动，`.cu` 直接调用 ccec。
- cublas→aclblas：保留在 `src/bridge/blas`，experimental，不阻塞 puresimt。
- Python kernel converter / hidden block lowering：不属于 main，历史在
  `exp/new-homogeneous-v1` 与 Git 历史。

## 验收

正式回归只认：

```text
PURESIMT_REGRESSION_PASS
```
