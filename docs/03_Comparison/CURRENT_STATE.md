# cu2cce-bridge 当前状态摘要（给审查 AI 的核对基线）

> 2026-08-07 生成。供 GPT-5.6 审查时对照，**避免重复发现已修正的问题**。
> 与 `REVIEW_PROMPT.md` 配套：先读本文了解基线，再执行审查。

---

## 1. 项目规模

- **124 个 `wcuda*` wrapper 声明**（`include/bridge/runtime/cuda_runtime_wrapper.h`）
- 实现全部在 `src/bridge/runtime/src/cuda_runtime_wrapper.cpp`（含 40 个真实 acl* 调用）
- cublas 桥：`include/bridge/blas/cublas_v2_wrapper.h` + `src/bridge/blas/src/cublas_v2_wrapper.cpp`
- SIMT 适配宏表：`include/bridge/kernel/simt_adaptor.h`
- mock 验证：`tests/mock_acl/` + `syntax_check.sh`（3/3 通过，**仅语法**，非运行时）

## 2. 目标栈（已定稿）

| 项 | 值 |
|---|---|
| 硬件 | Ascend 950 系列（950PR/950DT） |
| bisheng 架构参数 | `--npu-arch=dav-310` |
| CANN 版本 | 9.1.0（发布）/ 9.2.0（用户服务器） |
| 路线 | A（SIMT 直通，kernel 由 bisheng 原生编译） |

## 3. 实际调用的 40 个 acl* 函数（全部"已核实"基线）

> ⚠️ 这是**已按 CANN 9.x 核实的签名**，审查时核对它们是否仍是 9.2.0 正确的即可，
> 不要把它们当作"未核实"重新猜。

### runtime（`cuda_runtime_wrapper.cpp`）
| 函数 | 签名要点 |
|---|---|
| aclInit | `aclError aclInit(const char *config)`，bridge_init 中 nullptr |
| aclGetRecentErrMsg | `const char *aclGetRecentErrMsg()`（获取并清空最近错误） |
| aclrtSetDevice/GetDevice/GetDeviceCount | `(int32_t)/(int32_t*)/(uint32_t*)` |
| aclrtResetDevice | `aclrtResetDevice(int32_t deviceId)`（带参！） |
| aclrtSynchronizeDevice | `(void)` |
| aclrtGetSocName | `const char *aclrtGetSocName()`（无 aclError 返回，失败 nullptr） |
| aclrtGetMemInfo | `(aclrtMemAttr, size_t*, size_t*)`，用 ACL_HBM_MEM |
| aclrtCreateStream/DestroyStream/SynchronizeStream | `(aclrtStream*)` / `(aclrtStream)` / `(aclrtStream)` |
| aclrtStreamQuery | `(aclrtStream, aclrtStreamStatus*)`（**存在**，此前误判过） |
| aclrtStreamWaitEvent | `(aclrtStream, aclrtEvent)` |
| aclrtCreateEvent/DestroyEvent/RecordEvent/SynchronizeEvent | RecordEvent `(event, stream)` |
| aclrtQueryEventStatus | `(aclrtEvent, aclrtEventRecordedStatus*)`（9.x 新，取代废弃的 aclrtQueryEvent；取值 NOT_READY=0/COMPLETE=1） |
| aclrtEventElapsedTime | `(float*, aclrtEvent, aclrtEvent)` |
| aclrtMalloc | `(void**, size_t, aclrtMemMallocPolicy)` |
| aclrtFree | `(void*)` |
| aclrtMallocHost/FreeHost | `(void**, size_t)` / `(void*)` |
| aclrtMemcpy/MemcpyAsync | `(dst, destMax, src, count, kind[, stream])`（多 destMax） |
| aclrtMemcpy2d/Memcpy2dAsync | `(dst, dpitch, src, spitch, width, height, kind[, stream])` |
| aclrtMemset/MemsetAsync | `(devPtr, maxCount, value, count[, stream])`（4/5 参） |
| aclrtCreateContext/DestroyContext/SetCurrentContext/GetCurrentContext | 标准 |
| aclrtDeviceCanAccessPeer/Enable/DisablePeerAccess | `(int32_t*, int32_t, int32_t)` 等 |
| aclrtLaunchCallback | `(aclrtCallback, void*, aclrtCallbackBlockType, aclrtStream)` |
| aclrtLaunchSIMTKernelWithArgsArray | `(void*, dim3 gridDim, dim3 blockDim, size_t dynUbufSize, aclrtStream, aclrtLaunchKernelCfg*, void**)`，**950 支持** |

### blas（`cublas_v2_wrapper.cpp`）
| 函数 | 签名要点 |
|---|---|
| aclblasGemmEx | `(aclTransType A/B/C, m,n,k, alpha, A, lda, dtA, B, ldb, dtB, beta, C, ldc, dtC, aclComputeType, aclrtStream)` —— **无 handle**，直接传 stream |
| （无）aclblasCreate/Destroy/SetStream/GetStream | **不存在**，cu2cce 自定义 handle 结构承载 stream |

## 4. 已修正的"坑"（勿重复报告）

| # | 问题 | 修正 |
|---|---|---|
| 1 | `aclrtGetVersion(&ver)` 单参 | 9.x 头中无此函数；改为返回模拟 CUDA 版本宏 `WCUDA_VERSION` |
| 2 | `aclrtStreamQuery` 判为不存在 | 存在，`(stream, status*)`；已真实调用 |
| 3 | `aclrtQueryEvent` | 已废弃，改用 `aclrtQueryEventStatus` + `aclrtEventRecordedStatus` |
| 4 | `aclblasCreate/Destroy/SetStream/GetStream` | 不存在；cublas 桥用内部 handle 存 stream |
| 5 | `aclTransType` 有 C | 实为 N/T/NZ/NZ_T；CUBLAS_OP_C → ACL_TRANS_NZ |
| 6 | `aclDataType` 有 double | 无；Dgemm 返回 not-support |
| 7 | kernel 启动 3 参 `<<<b,t,0>>>` | 4 参 `<<<blocks,threads,dyn_ubuf_size,stream>>>`；vectorAdd 已改 |
| 8 | `asc_shfl` 有 mask | 无 mask：`asc_shfl(var, src_lane, width=warpSize)` |
| 9 | `aclrtMemset` 5 参 | 4 参 `(devPtr, maxCount, value, count)` |
| 10 | 成功码用 `ACL_ERROR_NONE` | 官方 aclError 文档：**`ACL_SUCCESS` 是当前正确，`ACL_ERROR_NONE` 将废弃**；48 处已改回 |
| 11 | `wcublasSgemv` 占位 not-support | `aclblasGemvEx` 存在（FP32 gemv）；已真实映射 |
| 12 | `aclrtGetDeviceProperties`（注释提到） | **不存在**；正确 API 是 `aclrtGetDeviceInfo(deviceId, attr, int64_t*)`；wrapper 用 SocName+MemInfo 组装，未调用它 |

## 5. 待实机确认项（真机 /usr/local/Ascend/.../include/acl/ 核实）

| # | 项 | 疑点 | 建议验证 |
|---|---|---|---|
| 1 | aclrtGetDeviceProperties / aclrtDeviceProperties | 9.x 新增，torch_npu stub 无；wrapper 未调用它（用 SocName+MemInfo 组装） | 真机 grep acl_rt.h |
| 2 | aclrtGetVersion | 9.2.0 完整头是否含（wrapper 已不依赖） | 真机 grep acl_rt.h |
| 3 | 成功码 | 代码用 `ACL_ERROR_NONE`；部分 9.x 文档写 `ACL_SUCCESS` | 真机 grep acl_base.h |
| 4 | aclblas 的 FP32 gemv/axpy/dot 等 | 是否真实存在（wrapper 当前占位 not-support） | 真机 grep acl_cblas.h |
| 5 | pitch 对齐规则 | wcudaMallocPitch 简化（width 无对齐） | 真机测对齐 |
| 6 | 原子操作地址空间 | asc_atomic_add 参数带 __ubuf__/__gm__，CUDA 指针能否隐式匹配 | 真机编译 SIMT kernel |
| 7 | aclrtRecordEvent stream 传 null | CUDA 默认 stream 语义 vs 昇腾 | 真机测 |

## 6. 已知的 fallback/占位（设计如此，非错误）

- `wcudaMallocManaged` → fallback aclrtMalloc（无自动迁移）
- `wcudaMallocAsync/FreeAsync` → fallback aclrtMalloc/Free
- `wcudaMemAdvise/MemPrefetchAsync` → no-op
- `wcudaLaunchKernel` → 真实映射 aclrtLaunchSIMTKernelWithArgsArray（✅ 非占位）
- Dgemm / BLAS1-2 / Stream capture / Graph / Texture / Array / Symbol → not-support 占位
- `wcudaGetErrorString` → aclGetRecentErrMsg + 错误码表

## 7. 文件树索引（审查范围）

见仓库根。关键：`src/bridge/runtime/src/cuda_runtime_wrapper.cpp`（核心，~1200 行）、
`src/bridge/blas/src/cublas_v2_wrapper.cpp`、`include/bridge/runtime/cuda_to_cce_mcr_adaptor.h`、
`include/bridge/kernel/simt_adaptor.h`、`tests/mock_acl/`。

## 8. 审查后的下一步

1. GPT 产出错误清单 + 待确认清单（按 REVIEW_PROMPT.md 格式）
2. 人工/Claude 按清单改代码
3. 真机验证（用户服务器 9.2.0）
