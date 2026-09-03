# CUDA → CCE puresimt 方案（main）

## 1. 目标

用户保留原始 NVIDIA CUDA `.cu`，不手工加入 ACL/Ascend C，不经 Python kernel
转换，在 Ascend 950 上通过 ccec SIMT 直接编译运行。

## 2. 主链路

```text
CUDA .cu
  → bin/cucc 参数/头/库注入
  → ccec -x asc --enable-simt --cce-aicore-arch=dav-c310-vec
  → cuda runtime symbols → wcuda* → aclrt*
  → Ascend NPU
```

`cucc` 不生成中间 `.cce`，不调用 Python converter，不生成 kernel wrapper。

## 3. Host bridge

- CUDA runtime headers 在 `include/`。
- `cuda_to_cce_mcr_adaptor.h` 做 CUDA 符号、类型和枚举兼容。
- `cuda_runtime_wrapper.cpp` 将 runtime 调用映射到 ACL runtime。
- CUDA 阻塞式 D2H copy、stream、device reset 等语义在 wrapper 中适配。

## 4. Device SIMT

ccec 原生识别 CUDA SIMT kernel。`include/bridge/kernel/simt_adaptor.h` 只做：

- `__shared__ → __ubuf__`；
- `__syncthreads → asc_syncthreads`；
- 已验证的 CUDA warp collectives → Ascend SIMT collectives；
- 已验证的基础 CUDA atomics（Add/Sub/Exch/Min/Max）→ Ascend SIMT atomics；
- `atomicCAS/Inc/Dec/And/Or/Xor`、`__activemask` 和 reduce max/min 仅在
  `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 下暴露，不能当作主线支持。

用户代码不得出现 `__simt_vf__`、`__global__[aicore]`、`cce::async_invoke` 或
hidden block 参数。

## 5. 目标与限制

- 正式目标硬件：Ascend 950PR/950DT，`dav-c310-vec`。
- 老型号无 SIMT 时需要另一条路线，但不属于 main。
- cublas→aclblas 保留为 experimental secondary capability。

## 6. 验收

唯一入口：`tools/real_test/server_cmd.txt`。

必须通过官方 NVIDIA vectorAdd/matrixMul 与最小 vector/matrix/atomic/reduction
用例，并输出 `PURESIMT_REGRESSION_PASS`。
