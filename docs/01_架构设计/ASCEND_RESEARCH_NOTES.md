# Ascend / CANN 调研事实库（main / puresimt）

更新时间：2026-08-24（main puresimt 进度更新）

本文只记录 main 当前采用的 puresimt 事实和仍保留的 experimental BLAS 事实。
旧新同构 lowering 记录不再作为 main 设计依据；历史版本在 Git 历史与
`exp/new-homogeneous-v1` 分支。

## 1. 目标环境

- 硬件：Ascend 950PR/950DT，真机曾识别为 `Ascend950DT_9582`。
- CANN：9.2.0。
- 编译器：`/usr/local/Ascend/latest/bin/ccec`。
- 必要命令参数：

```text
-x asc --enable-simt --cce-aicore-arch=dav-c310-vec -std=c++17
```

`dav-c310` 不带 `-vec` 会触发目标特性错误；main 统一使用 `dav-c310-vec`。

## 2. puresimt 事实

真机已验证 ccec 直接编译普通 CUDA `.cu`，无需 Python converter 或人工生成
Ascend kernel wrapper。已覆盖：

- `__global__` kernel；
- `threadIdx` / `blockIdx` / `blockDim` / 2D grid/block；
- CUDA `<<<>>>` 启动；
- `__shared__` + `__syncthreads()`；
- `atomicAdd`、`atomicMin`、`atomicMax`；
- 默认 stream 与显式 stream；
- 官方 NVIDIA vectorAdd/matrixMul；
- shared-memory tiled matrix multiply；
- tree reduction。

main 的原则是：用户源代码保持 CUDA；Ascend 特有名字只藏在兼容头和 runtime
实现中。

## 3. SIMT 兼容头

`include/bridge/kernel/simt_adaptor.h` 只提供符号级别适配：

| CUDA | Ascend SIMT |
|---|---|
| `__shared__` | `__ubuf__` |
| `__syncthreads()` | `asc_syncthreads()` |
| `__threadfence()` | `asc_threadfence()` |
| `atomicAdd/Min/Max` | `asc_atomic_add/min/max` |
| CUDA warp collectives | `asc_shfl` / `asc_all` / `asc_ballot` 等 |

该头不负责：

- kernel wrapper 生成；
- `__simt_vf__` 插入；
- `__global__[aicore]` 插入；
- `cce::async_invoke` 插入；
- hidden block 参数 lowering；
- Python/text source conversion。

## 4. Host Runtime bridge

用户 CUDA runtime API 经过：

```text
cuda* → wcuda* → aclrt*
```

主要已验证映射：

- `cudaMalloc/cudaFree` → `aclrtMalloc/aclrtFree`；
- `cudaMemcpy` → `aclrtMemcpy`，同步 D2H copy 保证 CUDA blocking 语义；
- stream/event → ACL runtime stream/event；
- device/context 初始化 → `aclInit`、设备枚举、`aclrtSetDevice`、context 设置；
- CUDA device attributes → bridge compatibility values，支持 NVIDIA helper 头编译；
- shadow headers 解决 ccec/系统 glibc 头冲突。

## 5. 官方样例回归

唯一入口：`tools/real_test/server_cmd.txt`。

完整逐用例、API、数据类型、内存模型和状态矩阵见：
`docs/02_能力清单/PURESIMT_COMPATIBILITY_MATRIX.md`。

当前主线已建立 23 个已验证样例，包含 half/BF16x2 类型测试：

```text
vectorAdd / matrixMul（官方 NVIDIA 源码）
vectorAdd_minimal / matrixMul_minimal
atomicAdd_minimal / reduction_minimal
warpShuffle / vote / scan / transpose
dot / stencil / core_ops / math
event / pitchedMemcpy / multiKernel
atomicVariants / dynamicShared / index3D / threadFence
half / bf16x2
```

验收标志：

```text
FAIL=0 DEVICE_HEALTHY=1 PURESIMT_REGRESSION_PASS
```

## 6. Precision types

CANN SIMT API 为 half/half2 与 bfloat16_t/bfloat16x2_t 提供独立头文件：

```text
simt_api/asc_fp16.h
simt_api/asc_bf16.h
```

main 提供 CUDA-compatible headers：

```text
include/cuda_fp16.h
include/cuda_bf16.h
```

真机回归用例 `half_minimal.cu` 与 `bf16x2_minimal.cu` 已验证转换 round-trip。

## 7. Experimental BLAS

`src/bridge/blas` 保留 cublas→aclblas 的独立实验能力，不作为 puresimt 验收条件。

已确认的 CANN GemmEx 调用约束：

- alpha/beta 必须是 device scalar pointer；
- `lda/ldb/ldc` 传 `-1`；
- GemmEx 需要匹配的 OPP/ATC 单算子模型；
- `FP16 × FP16 → FP32` 在匹配模型后可执行；
- 该能力仍不等同于完整 CUDA cuBLAS 兼容，后续单独维护。

## 8. 构建与环境

```bash
export ASCEND_HOME_PATH=/usr/local/Ascend/latest
source /usr/local/Ascend/latest/set_env.sh
export ASCEND_OPP_PATH=/usr/local/Ascend/cann-9.2.0/opp
```

main 的 `cucc` 只做参数、头文件、SIMT API、库和架构注入，然后直接 exec ccec。

## 9. 参考资料

- `docs/99_归档/参考资料/asc-devkit-simt/`：Ascend SIMT API 头和官方形式参考；
- NVIDIA CUDA Samples：`third_party/nvidia_cuda_samples/Common` helper headers；
- `tools/real_test/diag/`：runtime/ACL 排障资产。
