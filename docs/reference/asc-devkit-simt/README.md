# asc-devkit SIMT 头文件参考

来源：cann/asc-devkit（gitcode.com）`include/simt_api/`，Copyright 2026 Huawei，CANN 9.x。

用途：cu2cce `simt_adaptor.h` 的权威签名依据（asc_syncthreads / asc_shfl / asc_atomic_* 等）。

| 文件 | 内容 |
|---|---|
| asc_simt.h | SIMT 入口（include common/math/device/vector） |
| device_types.h | `__SIMT_DEVICE_FUNCTIONS_DECL__`（__aicore__/__simt_callee__） |
| device_sync_functions.h | asc_syncthreads / asc_threadfence / asc_threadfence_block |
| device_warp_functions.h | asc_shfl(_up/down/xor)、asc_all/any/ballot、asc_reduce_* |
| device_atomic_functions.h | asc_atomic_add/sub/exch/max/min（__ubuf__/__gm__ 地址空间） |

获取方式（如需刷新）：
```
curl -L "https://gitcode.com/api/v5/repos/cann/asc-devkit/contents/include/simt_api/<file>"
```

| device_functions.h | asc_ldcg/asc_ldg 等全局内存直读（__gm__ 地址空间） |
| hello_world.asc | SIMT 完整示例：`kernel<<<blocks, threads, dyn_ubuf_size, stream>>>` 4 参启动 |

## 关键事实（2026-08-07 核对）

- **kernel 启动是 4 参**：`kernel<<<blocks_per_grid, threads_per_block, dyn_ubuf_size, stream>>>`（与 CUDA `<<<grid, block, sharedMem, stream>>>` 一致）。**不是 3 参！** 我此前在 vectorAdd 里写成 3 参是错的。
- warp 函数（asc_shfl 等）在 `#if (__NPU_ARCH__ == 3510) || (__NPU_ARCH__ == 5102)` 下启用。
- `warpSize`/`threadIdx`/`blockIdx`/`blockDim`/`gridDim` 是内置变量。
- host 侧：`aclInit → aclrtSetDevice → aclrtCreateStream → <<<>>> → aclrtSynchronizeStream → aclrtResetDevice → aclFinalize`。
