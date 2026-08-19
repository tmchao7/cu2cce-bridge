# 昇腾 CANN / AI Core SIMT 调研笔记

> 状态：2026-08-07 调研记录（Web 检索，未真机验证）
> 用途：cu2cce-bridge 移植 cu-bridge 到昇腾的参考资料库。与 `CUDA_TO_CCE_PORTING_PLAN.md`（决策型）配套，本文件是**事实型**存档。
> 说明：本机无昇腾硬件、无 CANN，以下事实来自 CANN 官方文档检索与开源工程，标注 ✅=官方文档/源码确认，⚠️=需真机二次核对。
>
> **CANN 版本基准（2026-08-07 用户确认）：按 CANN 9.x 系列开发（发布版 9.1.0，用户服务器实际为 9.2.0）**。
> 因此优先采用 9.x 新增 API：`aclrtLaunchKernelV2`（9.0+）、`aclrtCreateEventWithFlag`（新版命名）、AI Core SIMT（9.0+ 支持）。
> 8.x 及更早的旧命名（如 `aclrtCreateEventWithFlags`）不作为基准。

---

## 1. Host 侧 Runtime = AscendCL（ACL）✅

### 1.1 核心 API 面（对标 CUDA Runtime）

| CUDA | AscendCL | 说明 |
|---|---|---|
| `cudaMalloc` | `aclrtMalloc` | 设备内存，注意页表大小/对齐规则 |
| `cudaFree` | `aclrtFree` | |
| `cudaMemcpy` | `aclrtMemcpy` | 同步；另有 `aclrtMemcpyAsync` 异步 |
| `cudaMemcpyKind` | `aclrtMemcpyKind` | 方向枚举 |
| `cudaSetDevice` | `aclrtSetDevice` | |
| `cudaGetDeviceCount` | `aclrtGetDeviceCount` | |
| `cudaGetDeviceProperties` | `aclrtGetDeviceProperties` | |
| `cudaStreamCreate` | `aclrtCreateStream` | 另有 `aclrtDestroyStream` |
| `cudaEventCreate` | `aclrtCreateEvent` | 另有 `aclrtRecordEvent` |
| `cudaStreamSynchronize` | `aclrtSynchronizeStream` | |
| `cudaError_t` | `aclError` | `ACL_SUCCESS` 等 |

**头文件**：`acl/acl_rt.h`、`acl/acl_base.h`。

**运行时 API 官方参考文档**（毕昇/CANN 侧，含函数原型与参数）：
- [昇腾社区 - CANN 运行时 API 参考（用户指定）](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/runtimeapi/aclcppdevg_03_1952.html) — 属 `API/runtimeapi/` 系列（与 `aclrtLaunchKernelV2`=`aclcppdevg_03_1960` 同文档系列）；⚠️ 具体函数名因网络拦截未能抓取确认，需真机/手工核对。

### 1.2 头文件与库文件对应表（CANN 9.1.0，`${INSTALL_DIR}/include/` + `${INSTALL_DIR}/lib64/`）

| 头文件 | 用途 | 库 |
|---|---|---|
| `acl/acl_rt.h` | 初始化/去初始化、Device/Context/Stream 管理、同步等待、内存管理 | `libacl_rt.so`（旧版 `libascendcl.so` 将废弃） |
| `acl/acl_rt_api.h` | C++ 扩展接口（函数重载+模板封装，仅 C++，依赖 acl_rt.h） | `libacl_rt.so` |
| `acl/acl_base.h` | 基础类型、错误码 | `libacl_rt.so` |
| `acl/acl_dump.h` | 模型和算子 Dump | `libascend_dump.so` |
| `acl/acl_prof.h` | Profiling 采集 | `libmsprofiler.so`（旧版 libascendcl.so 将废弃） |
| `acl/acl_tdt.h` | Tensor 数据传输 | `libacl_tdt_channel.so` |
| `acl/acl_tdt_queue.h` | 共享队列/Buffer 管理 | `libacl_tdt_queue.so` |

### 1.3 Runtime API(C) 全目录地图（2026-08 curl 实测编号，最新版文档）

> 编号= `API/runtimeapi/aclcppdevg_03_XXXX.html`。未列编号为该编号无独立页或为占位。**含"❄️"为开发 cucc wrapper 最关键项。**

- **初始化和去初始化**：`0039 aclrtSetDevice`❄️、`0023 aclFinalize`、`0001-0022` 部分为说明页
- **Device 管理**：`0039 aclrtSetDevice`❄️、`0040 aclrtResetDevice`、`0041 aclrtResetDeviceForce`、`0042 aclrtGetDevice`、`0043 aclrtGetRunMode`、`0044 aclrtSetTsDevice`、`0045 aclrtGetDeviceCount`❄️、`0046 aclrtGetDeviceUtilizationRate`、`0047 aclrtQueryDeviceStatus`、`0048 aclrtGetSocName`、`0049 aclrtSetDeviceSatMode`、`0050 aclrtGetDeviceSatMode`、`0051 aclrtDeviceCanAccessPeer`、`0052 aclrtDeviceEnablePeerAccess`、`0053 aclrtDeviceDisablePeerAccess`、`0054 aclrtGetOverflowStatus`、`0055 aclrtResetOverflowStatus`、`0056 aclrtSynchronizeDevice`、`0057 aclrtSynchronizeDeviceWithTimeout`
- **Context 管理**：`0058/0059 aclrtCreateContext`、`0060 aclrtDestroyContext`、`0061 aclrtSetCurrentContext`、`0062 aclrtGetCurrentContext`、`0063 aclrtCtxSetSysParamOpt`、`0064 aclrtCtxGetSysParamOpt`
- **Stream 管理**：`0065 aclrtCreateStream`❄️、`0066 aclrtDestroyStream`、`0071 aclrtDestroyStreamForce`、`0073 aclrtGetStreamOverflowSwitch`、`0074 aclrtSetStreamFailureMode`、`0075 aclrtStreamQuery`、`0076 aclrtSynchronizeStream`❄️、`0077 aclrtSynchronizeStreamWithTimeout`
- **Event 管理**：`0078/0079 aclrtCreateEvent`❄️、`0080 aclrtCreateEventWithFlag`、`0081 aclrtCreateEventExWithFlag`、`0082 aclrtDestroyEvent`、`0083 aclrtRecordEvent`、`0084 aclrtResetEvent`、`0085 aclrtQueryEvent`（废弃）、`0090 aclrtEventElapsedTime`、`0091 aclrtStreamWaitEvent`、`0092 aclrtSetOpWaitTimeout`
- **内存管理**：
  - 设备内存分配与释放：`0093/0094 内存使用说明`、`0095 aclrtMalloc`❄️、`0100 aclrtFree`、`0107 aclrtGetMemInfo`
  - 主机内存管理：`0101 aclrtMallocHost`、`0102 aclrtFreeHost`
  - 内存复制与设置：`0103 aclrtMemset`、`0104 aclrtMemsetAsync`、`0105 aclrtMemcpy`❄️、`0106 aclrtMemcpyAsync`❄️、`0108 aclrtMemcpy2d`、`0109 aclrtMemcpy2dAsync`
  - 自定义内存分配器：`0110 aclrtAllocatorRegister`、`0111 aclrtAllocatorUnregister`
  - 虚拟内存管理：`0112 aclrtMallocPhysical`、`0113 aclrtFreePhysical`、`0114 aclrtReserveMemAddress`、`0115 aclrtReleaseMemAddress`、`0120 aclrtMemSetPidToShareableHandle`、`0121 aclrtMemImportFromShareableHandle`、`0122 aclrtMemGetAllocationGranularity`
  - CMO 缓存操作：`0123 aclrtCmoAsync`
- **执行控制**：`0124 aclrtLaunchCallback`、`0125 aclrtSubscribeReport`
- **异常处理**：`0128 aclGetRecentErrMsg`、`0135 aclrtGetTaskIdFromExceptionInfo`、`0140 aclrtGetMemUceInfo`、`0141 aclrtDeviceTaskAbort`、`0142 aclrtMemUceRepair`、`0143 aclrtPeekAtLastError`、`0144 aclrtGetLastError`❄️
- **Kernel 加载与执行**：`0145 概念及使用说明`、`0146 aclrtCreateBinary`、`0147 aclrtDestroyBinary`、`0148 aclrtBinaryLoad`、`0149 aclrtBinaryUnLoad`、`0150 aclrtBinaryGetFunction`、`0151 aclrtLaunchKernel`❄️、`1960 aclrtLaunchKernelV2`❄️
- **算力 Group 查询与设置**：`0152/0153 aclrtSetGroup`、`0154 aclrtGetGroupCount`、`0155 aclrtGetAllGroupInfo`、`0156 aclrtGetGroupInfoDetail`、`0158 aclrtDestroyGroupInfo`
- **其他接口**：`0030 aclDataTypeSize`、`0034 aclGetCannAttributeList`、`0035 aclGetCannAttribute`、`0036 aclGetDeviceCapability`、`0024 aclrtGetVersion`、`0130 aclrtProcessHostFunc`、`0019 废弃接口和返回码列表`、`0020 同步和异步API说明`

### 1.4 关键函数签名（curl 实测，2026-08）

```c
// 设备/流/事件
aclError aclrtSetDevice(int32_t deviceId);
aclError aclrtGetDeviceCount(uint32_t *count);      // 推断，编号 0045
aclError aclrtCreateStream(aclrtStream *stream);
aclError aclrtSynchronizeStream(aclrtStream stream);
aclError aclrtCreateEvent(aclrtEvent *event);

// 内存
aclError aclrtMalloc(void **devPtr, size_t size, aclrtMemMallocPolicy policy);  // policy: 大页/普通页; 不初始化内容, 需自行 aclrtMemset
aclError aclrtMemcpy(void *dst, size_t destMax, const void *src, size_t count, aclrtMemcpyKind kind);
aclError aclrtMemcpyAsync(void *dst, size_t destMax, const void *src, size_t count, aclrtMemcpyKind kind, aclrtStream stream);

// Kernel 加载与执行（三方式之一：显式加载）
aclError aclrtLaunchKernel(aclrtFuncHandle funcHandle, uint32_t numBlocks,
                           const void *argsData, size_t argsSize, aclrtStream stream);
aclError aclrtLaunchKernelV2(aclrtFuncHandle funcHandle, uint32_t numBlocks,
                             const void *argsData, size_t argsSize,
                             aclrtLaunchKernelCfg *cfg, aclrtStream stream);  // cfg 可传 NULL
```

### 1.5 错误码（来自 MegEngine 镜像 acl_base.h / error_codes/rt_error_codes.h）

- **成功码：`ACL_ERROR_NONE = 0`**（⚠️ 部分文档/版本写作 `ACL_SUCCESS`，真机核对）。
- 基础错误码（acl_base.h）：
  | 码 | 值 | 码 | 值 |
  |---|---|---|---|
  | ACL_ERROR_INVALID_PARAM | 100000 | ACL_ERROR_BAD_ALLOC | 200000 |
  | ACL_ERROR_UNINITIALIZE | 100001 | ACL_ERROR_API_NOT_SUPPORT | 200001 |
  | ACL_ERROR_REPEAT_INITIALIZE | 100002 | ACL_ERROR_INVALID_DEVICE | 200002 |
  | ACL_ERROR_INVALID_FILE | 100003 | ACL_ERROR_MEMORY_ADDRESS_UNALIGNED | 200003 |
  | ACL_ERROR_INVALID_FILE_SIZE | 100005 | ACL_ERROR_RESOURCE_NOT_MATCH | 200004 |
  | ACL_ERROR_PARSE_FILE | 100006 | ACL_ERROR_INVALID_RESOURCE_HANDLE | 200005 |
  | ACL_ERROR_INVALID_MODEL_ID | 100011 | ACL_ERROR_FEATURE_UNSUPPORTED | 200006 |
  | ACL_ERROR_STORAGE_OVER_LIMIT | 300000 | ACL_ERROR_INTERNAL_ERROR | 500000 |
  | ACL_ERROR_FAILURE | 500001 | ACL_ERROR_RT_FAILURE | 500003 |
- RT 错误码（error_codes/rt_error_codes.h，107xxx=参数类、207xxx=资源类、507xxx=内部/超时类）：
  | 码 | 值 | 码 | 值 |
  |---|---|---|---|
  | ACL_ERROR_RT_PARAM_INVALID | 107000 | ACL_ERROR_RT_NO_DEVICE | 207004 |
  | ACL_ERROR_RT_INVALID_DEVICEID | 107001 | ACL_ERROR_RT_RESOURCE_ALLOC_FAIL | 207005 |
  | ACL_ERROR_RT_CONTEXT_NULL | 107002 | ACL_ERROR_RT_NO_EVENT_RESOURCE | 207007 |
  | ACL_ERROR_RT_STREAM_CONTEXT | 107003 | ACL_ERROR_RT_NO_STREAM_RESOURCE | 207008 |
  | ACL_ERROR_RT_EVENT_TIMESTAMP_INVALID | 107006 | ACL_ERROR_RT_OVER_LIMIT | 207012 |
  | ACL_ERROR_RT_ADDR_UNALIGNED | 107008 | ACL_ERROR_RT_INTERNAL_ERROR | 507000 |
  | ACL_ERROR_RT_INVALID_HANDLE | 107017 | ACL_ERROR_RT_STREAM_NOT_COMPLETE | 507004 |
  | ACL_ERROR_RT_INVALID_MALLOC_TYPE | 107018 | ACL_ERROR_RT_EVENT_NOT_COMPLETE | 507006 |
  | ACL_ERROR_RT_WAIT_TIMEOUT | 107019 | ACL_ERROR_RT_AICORE_EXCEPTION | 507015 |
  | ACL_ERROR_RT_FEATURE_NOT_SUPPORT | 207000 | ACL_ERROR_RT_AICPU_TIMEOUT | 507017 |
  | ACL_ERROR_RT_MEMORY_ALLOCATION | 207001 | ACL_ERROR_RT_KERNEL_LOOKUP | 507027 |
  | ACL_ERROR_RT_MEMORY_FREE | 207002 | ACL_ERROR_RT_KERNEL_DUPLICATE | 507028 |

### 1.6 已核对的关键签名修正（2026-08-07，对照 MegEngine + torch_npu 真实头）

| 我此前写错 | 真实签名 |
|---|---|
| `aclrtResetDevice()` | `aclrtResetDevice(int32_t deviceId)` |
| `aclrtGetSocName(&soc)` 返回 aclError | `const char *aclrtGetSocName()`（无错误返回，失败返回 nullptr） |
| `aclrtMemset(devPtr,count,value,0,count)` 5 参 | `aclrtMemset(devPtr, maxCount, value, count)` 4 参 |
| `aclrtMemsetAsync(+offset)` 6 参 | `aclrtMemsetAsync(devPtr, maxCount, value, count, stream)` 5 参 |
| `aclrtQueryEvent(event)` | `aclrtQueryEvent(event, aclrtEventStatus*)`（**已废弃**，9.x 用 aclrtQueryEventStatus） |
| ~~`aclrtStreamQuery` 不存在~~ | **存在！** `aclrtStreamQuery(stream, aclrtStreamStatus*)`（我此前用 SynchronizeStream 模拟是错的） |
| `aclrtGetMemInfo(ACL_HBM_MEM, ...)` | `aclrtGetMemInfo(aclrtMemAttr, size_t*, size_t*)`，ACL_HBM_MEM 有效 |
| `aclrtCreateEventWithFlags` | 真实为 `aclrtCreateEventWithFlag`（无 s） |
| `aclrtGetVersion(&ver)` 单参 | **torch_npu CANN9 头中不存在**；8.5 文档签名是 3 参 (major/minor/patch)。改为返回模拟 CUDA 版本宏 |
| **`aclblasCreate/Destroy/SetStream/GetStream` 存在** | **不存在！** aclblasGemmEx 无 handle、直接传 aclrtStream；handle 需 cu2cce 自定义结构 |
| aclTransType 含 `ACL_TRANS_C` | **无 C**：N/T/NZ/NZ_T |
| aclDataType 含 double | **无 ACL_DT_DOUBLE**（FP16/FP32/int8） |
| `aclrtQueryEventStatus` 用 aclrtEventStatus | 用 **`aclrtEventRecordedStatus`**：NOT_READY=0/COMPLETE=1（与 EventStatus 相反！） |

**关键差异点（写 wrapper 时注意）**：
1. `aclrtMemcpy`/`aclrtMemcpyAsync` 多一个 `destMax` 参数（CUDA `cudaMemcpy(dst,src,count,kind)` 无此参数）——wrapper 需补 `destMax`。
2. `aclrtMalloc` 的第三个参数是 `aclrtMemMallocPolicy`（大页/普通页），不是 flags。
3. `aclrtMemcpyKind` 的 kind 在同步版是"预留"（系统按指针判断方向），异步版才用枚举。
4. Kernel 启动参数区 `argsData` 必须放 **Device 内存**（不是 Host），需先在 Device 上 malloc 一块参数区打包。
5. 支持同步 API：`aclrtSynchronizeDevice`（重同步）、`aclrtSynchronizeStream`、`aclrtSynchronizeEvent`。

### 1.2 Kernel 启动三种方式（C3 ✅）

1. **`<<<>>>` 语法糖**：`kernel_name<<<blockDim, l2ctrl, stream>>>(args...)`，`l2ctrl` 保留参数固定 `nullptr`。
2. **`ACLRT_LAUNCH_KERNEL` 宏**：`ACLRT_LAUNCH_KERNEL(kernel_name)(blockDim, stream, arg1, arg2, ...)`。
3. **显式加载**：`aclrtLoadBinary` → `aclrtBinaryGetFunction` → `aclrtLaunchKernel`：

```c
aclError aclrtLaunchKernel(aclrtFuncHandle funcHandle, uint32_t blockDim,
                           const void *argsData, size_t argsSize, aclrtStream stream);
```
- `argsData` 为 **Device 内存地址**的参数区指针（与下发任务同 Device）。
- 异步接口，需 `aclrtSynchronizeStream` 等待完成。

编译时框架自动生成 `aclrtlaunch_{kernel_name}.h` 提供启动 API。blockDim 按核类型设置：仅 Vector 设 AIV 核数（如 40）、仅 Cube 设 AIC 核数（如 20）、融合按配比（1,0)/(0,1)/(1,1)/(1,2)。

示例（ACLRT_LAUNCH_KERNEL，add_custom 算子，pybind11）：
```cpp
#include "aclrtlaunch_add_custom.h"
uint32_t blockDim = 8;
ACLRT_LAUNCH_KERNEL(add_custom)
(blockDim, acl_stream, x, y, z, totalLength);
```

示例（`<<<>>>` 语法，HCCL AI CPU 内核）：
```cpp
extern __global__ __aicpu__ uint32_t HcclP2PAicpuKernelAsc(void *args);
constexpr uint32_t blockDim = 1;
HcclP2PAicpuKernelAsc<<<blockDim, nullptr, stream>>>(&param, sizeof(OpParam));
aclError aclRet = aclrtGetLastError(ACL_RT_THREAD_LEVEL);
```

---

## 2. 设备侧 kernel = AI Core SIMT ✅（重大发现）

### 2.1 硬件支持范围

- **Ascend 950PR / 950DT 架构**的 AIV 向量核同时支持 SIMD 与 SIMT。
- **Atlas A2/A3 系列（含 910B/910C）不支持 SIMT** —— 这是 cu2cce 路线分叉的关键。
- 适用场景：离散访问、复杂分支控制、稀疏计算（Gather/Scatter 类）。
- **限制：仅向量计算，不支持 Cube 矩阵运算 / SIMD+SIMT 融合**。

### 2.2 线程架构（与 CUDA 同构）

- Grid-Block-Thread 三级；warp = 32 线程；每块最多 2048 线程；`gridDim.x*gridDim.y*gridDim.z ≤ 65535`。
- 内置变量：`threadIdx` / `blockIdx` / `blockDim` / `gridDim`（三维 `dim3`）。
- 线程 ID：`blockIdx.x * blockDim.x + threadIdx.x`。
- SIMT 下 Global Memory **可直接读写**（不需要 tile 搬移）。

### 2.3 SIMT ↔ CUDA 语法映射表

| CUDA | 昇腾 SIMT | 说明 |
|---|---|---|
| `__global__` | `__global__` | 同名；返回 void、不能递归、异步调用、只能 Host 调用 |
| `__device__` | `__aicore__` | 只在 Device 侧执行 |
| `__host__` | `__host__` | |
| `__shared__` | `__ubuf__` | 静态数组或动态申请，块内共享，高带宽低延迟 |
| 全局内存 | `__gm__` | 所有线程可访问 |
| `__syncthreads()` | `asc_syncthreads()` | 头文件 `simt_api/device_sync_functions.h` |
| `__threadfence()` | `asc_threadfence()` | 另有 `asc_threadfence_block()` |
| `__shfl_sync(mask,var,srcLane)` | `asc_shfl(var, src_lane, width=warpSize)` | **无 mask 参数**（warp 内全同步）；有 up/down/xor 变体 |
| `__all_sync`/`__any_sync`/`__ballot_sync` | `asc_all`/`asc_any`/`asc_ballot` | 无 mask |
| `atomicAdd` 等 | `asc_atomic_add`/`asc_atomic_sub`/`asc_atomic_exch`/`max`/`min` | 参数带 `__ubuf__`/`__gm__` 地址空间修饰 |
| `__launch_bounds__` | `__launch_bounds__` | 同名 |
| 线程归约 | `asc_reduce_add` | |
| `sin/cos/exp/half/half2/bfloat16` | 支持 | `half`/`bfloat16_t`/`float2` 等 |
| `<<<grid,block,sharedMem,stream>>>` | `<<<blocks_per_grid, threads_per_block, dyn_ubuf_size, stream>>>` | **4 参**（与 CUDA 一致），第 3 参是动态 UB 大小 |

**⚠️ 2026-08-07 权威核对（asc-devkit hello_world_simt 示例）**：kernel 启动是 **4 参**
`kernel<<<blocks_per_grid, threads_per_block, dyn_ubuf_size, stream>>>`，**不是 3 参**。
warp/原子函数在 `#if (__NPU_ARCH__ == 3510) || (__NPU_ARCH__ == 5102)` 下启用。

**⚠️ 显式启动 API（cann/runtime 官方 14_kerne_loading_and_execution.md 确认，2026-08-07）**：
- **`aclrtLaunchSIMTKernelWithArgsArray(void *func, dim3 gridDim, dim3 blockDim, size_t dynUbufSize, aclrtStream stream, aclrtLaunchKernelCfg *cfg, void **args)`** —— SIMT 核函数专用启动，**Ascend 950PR/950DT 支持**（A2/A3/910B 不支持）。`cudaLaunchKernel` 的真实映射目标。
- 另有 `aclrtLaunchSIMTKernelWithHostArgs`（Host 内存参数区）。
- `aclrtLaunchKernelV2`（Ascend 950PR/950DT 支持）：argsData 需 Device 内存。
- 完整流程：aclrtCreateBinary → aclrtBinaryGetFunction → aclrtLaunchKernelV2/SIMT 启动 → aclrtBinaryUnLoad。
- `dim3` 是官方类型（bisheng 提供，25-04_Structs.md#dim3）。

另有函数形式：`AscendC::Simt::GetThreadIdx<<0>>()`、`GetBlockIdx()`、`GetThreadNum<<0>>()`、`GetBlockNum()`。

### 2.4 SIMT kernel 代码示例（官方/社区）

**同步 + 归约（`__launch_bounds__` 与 `asc_syncthreads`）**：
```cpp
__global__ __launch_bounds__(1024) void KernelSyncThreads(float* dst, int count)
{
     int idx = threadIdx.x;
     if (idx > 0 && idx < count) dst[idx] = 1;
     asc_syncthreads();                       // 块内所有线程同步
     if (idx == 0) {
         dst[0] = 0;
         for(int i = 1023; i > 0; i--) dst[0] += dst[i];
     }
}
```

**矩阵转置（`__ubuf__` 共享内存 + 避免 Bank 冲突）**：
```cpp
__global__ void transpose_coalesced_kernel(float *output, const float *input, int width, int height)
{
    __ubuf__ float tile[TILE_DIM][TILE_DIM];
    int x_index = blockIdx.x * TILE_DIM + threadIdx.x;
    int y_index = blockIdx.y * TILE_DIM + threadIdx.y;
    tile[threadIdx.y][threadIdx.x] = input[x_index + y_index * width];
    asc_syncthreads();
    x_index = blockIdx.y * TILE_DIM + threadIdx.x;
    y_index = blockIdx.x * TILE_DIM + threadIdx.y;
    output[x_index + y_index * height] = tile[threadIdx.x][threadIdx.y];
}
// 加 padding 版：__ubuf__ float tile[TILE_DIM][TILE_DIM + 1]; → 实测性能 +25.7%
```

**线程分配模式（真实算子）**：
```cpp
int32_t threadIdx = AscendC::Simt::GetThreadIdx<<0>>();
int32_t blockIdx  = AscendC::Simt::GetBlockIdx();
int32_t threadNum = AscendC::Simt::GetThreadNum<<0>>();
int32_t totalThreadNum = AscendC::Simt::GetBlockNum() * threadNum;
int64_t start = (int64_t)blockIdx * threadNum + threadIdx;
int64_t kDataPerThread = (totalLength + totalThreadNum - 1) / totalThreadNum;
for (int i = 0; i < kDataPerThread; ++i) {
    int64_t idx = start + i * totalThreadNum;
    if (idx >= totalLength) return;
    // ...
}
```

### 2.5 坑（来自社区实践）

- `asc_syncthreads()` 必须块内所有线程都到；放分支/提前 return 会导致块级死锁（报错 `ACL stream synchronize failed, error code: 507035`）。
- UB 256KB、16 bank（8 组）；集中访问会 bank 冲突，加 padding 缓解。
- Host 侧负责 InferShape/InferDtype/Tiling，设对 blockDim/gridDim 防超限。

---

## 3. 编译器 = bisheng（clang 封装）✅

### 3.1 位置与本质

- 路径：`${INSTALL_DIR}/compiler/ccec_compiler/`
- 二进制：`bin/clang`（本质是 clang 封装）、`bin/lld`、`bin/cce-ld`（异构链接器）、`llvm-objdump/nm/size/ar` 等。
- 旧版 CCEC 编译器（TBE 算子）不建议手工调用；**毕昇编译器 `bisheng`** 是现代 Ascend C/SIMT 的编译器。

### 3.2 用法

```bash
# 环境
export PATH=${INSTALL_DIR}/compiler/ccec_compiler/bin:$PATH
# 或 source ${INSTALL_DIR}/bin/setenv.bash

# AI Core SIMT/SIMD 编译
bisheng <source>.asc -o <out> --npu-arch=dav-<arch> --std=c++17 -xcce

# 异构编译（host + device 同文件）
bisheng main.cpp example.asc -o main --npu-arch=dav-xxxx
# 或分步: bisheng -c example.asc -o example.o; bisheng -c main.cpp -o main.o; bisheng example.o main.o -o main
```

### 3.3 关键选项

| 选项 | 说明 |
|---|---|
| `--npu-arch=dav-<arch>` | 新版本指定 AI 处理器架构 |
| `--cce-soc-version=AscendXXXXY` | 旧版本指定型号（`npu-smi info` 查询） |
| `--cce-soc-core-type=VecCore/CubeCore/AICore` | 指定生成对应核二进制 |
| `--std=c++17` | 编译 Ascend C 算子**必选** |
| `-xcce` | 代码含设备侧代码但后缀非 .cce 时**必选** |
| `-I/-L/-l/-D` | 标准 clang 风格 |

### 3.4 设备侧限定符

- `__global__`：kernel 入口（返回 void、异步、仅 Host 调用）。
- `__aicore__`：AI Core 函数（`__global__` + `__aicore__` = AI Core kernel）。
- `__aicpu__`：AI CPU 内核（须与 `__global__` 同用，入参仅一个指针，不能返回 void）。

### 3.5 `--npu-arch=dav-c310` 的来历与注意事项（2026-08-19 纠正）

- **dav-c310 = 内部昇腾 950 系列架构参数（导师提供，公司内部硬件，公开文档无此值）**。公开资料只有 dav-3510（Atlas 350/351x）等，**不要把 dav-c310 当笔误改掉**。
- ⚠️ 历史教训：2026-08-19 之前文档/脚本一直误记为 **dav-310**，真机 bisheng（clang 15.0.5，cann-9.2.0）实测编译报 `Unsupported NPU architecture or soc: NPU arch is not supported!!!`，无法编译任何 kernel。导师纠正为 **dav-c310** 后已全仓库统一（grep dav-310 应为 0）。**任何文档若再见 dav-310 均为过时写法，需改 dav-c310。**
- ⚠️ 风险点：公开 asc-devkit 的 simt_api 头里 warp/atomic 函数按 `__NPU_ARCH__ == 3510 || 5102`（真机头为 2201/3510/5102）条件编译启用；dav-c310 对应的 `__NPU_ARCH__` 数字宏尚未确认。若真机头文件的门控不含该值，`asc_shfl`/`asc_atomic_*` 会被裁掉，simt_adaptor 映射静默失效。
- 验证方式：`tools/real_test/env_probe.sh` 第 7 节会自动做 ①`-E`/`-dM` 取 `__NPU_ARCH__` 宏值 ②grep simt_api 头门控值并比对 ③最小 SIMT kernel（asc_shfl+asc_syncthreads）实测编译；dav-c310 失败时自动尝试门控值对应的 dav-3510/5102/2201。

### 3.6 真机核实记录（2026-08-19，cann-9.2.0 / x86_64 / driver 7.0.t9.0.B880）

> 环境：`root@bisheng` 服务器（`/home/t50063365/workspace/repos/cu2cce-bridge`）。本机（macOS 无 CANN）无法验证的内容，以本节为准。

**真实路径布局（与公开文档不同！）**

| 组件 | 公开文档假设 | 真机实际（cann-9.2.0） |
|---|---|---|
| bisheng | `${ASCEND_TOOLKIT_HOME}/compiler/ccec_compiler/bin/bisheng` | `/usr/local/Ascend/cann-9.2.0/tools/bisheng_compiler/bin/bisheng`（独立于 ascend-toolkit） |
| **ccec（CANN 自带编译器）** | 无 | **`/usr/local/Ascend/latest/bin/ccec`**（`CANN_BASE=/usr/local/Ascend/latest`，含 set_env.sh、runtime/） |
| simt_api | `${ASCEND_TOOLKIT_HOME}/include/simt_api` | `/usr/local/Ascend/cann-9.2.0/x86_64-linux/asc/include/simt_api` |
| ACL 头/库 | `${ASCEND_TOOLKIT_HOME}/include` + `/lib64` | ✅ 与公开布局一致 |
| ascend-toolkit/compiler/ | ccec_compiler | 实为 asc/ascendc/bin/conf/python/tikcpp（无 ccec_compiler，无 bisheng） |

**两大编译器（2026-08-19 关键发现，用户实测脚本 scripts/04_build_exec.sh 证实）**：

| | **ccec**（CANN 自带，✅ 主线） | **bisheng**（独立编译器，备选） |
|---|---|---|
| 位置 | `/usr/local/Ascend/latest/bin/ccec` | `cann-9.2.0/tools/bisheng_compiler/bin/bisheng` |
| 架构参数 | `--cce-aicore-arch=dav-c310` ✅ **认识 dav-c310** | `--npu-arch=dav-xxx`，**不认识 dav-c310**（lib 里只有 dav-920r2/l210/n350/t210） |
| 与 CANN 绑定 | 是（内置当前硬件支持） | 否（通用 clang 系） |
| 实测 | **用户脚本已用它编译 .cce + 在 davinci0 上运行成功** | 编译 SIMT kernel 进行到 include 阶段 |

→ **cu2cce 主线改用 ccec + `--cce-aicore-arch=dav-c310`**（导师指定架构唯一被接受路径）；bisheng 留作对照。ccec 与 bisheng 共享同一 clang 资源（用户脚本 `-I ${CANN_BASE}/tools/bisheng_compiler/lib/clang/15.0.5/include` 含 `__clang_cce_types.h`）。

**用户实测编译命令参考（tile 模型 .cce，2026-08-19 成功）**：
```bash
CANN_BASE=/usr/local/Ascend/latest; source ${CANN_BASE}/set_env.sh
${CANN_BASE}/bin/ccec -O2 --cce-aicore-arch=dav-c310 -std=c++17 <src>.cce \
  -I${CANN_BASE}/runtime/include -I${CANN_BASE}/include \
  -I${CANN_BASE}/include/ascendc/host_api \
  -I${CANN_BASE}/compiler/ascendc/include/highlevel_api \
  -I${CANN_BASE}/compiler/tikcpp/tikcfw{-,/lib,/lib/matmul,/impl,/interface} \
  -I${CANN_BASE}/tools/bisheng_compiler/lib/clang/15.0.5/include \
  -L${CANN_BASE}/runtime/lib64 -L${CANN_BASE}/lib64 -lascendcl -lruntime \
  -lstdc++ -lpthread -lm -Wl,-rpath=${CANN_BASE}/runtime/lib64 -o build/<name>.out
# 运行：./build/<name>.out 0（device 0）
```

**编译器**：bisheng = clang 15.0.5（2026-08-13 构建，x86_64-linux）。`--npu-arch`、`--cce-soc-version`、`--offload-arch` 均支持；`--help` 未列出合法架构值。

**架构参数（2026-08-19 实测，bisheng clang 15.0.5 / cann-9.2.0）**：

| 参数 | 结果 | 说明 |
|---|---|---|
| `--npu-arch=dav-c310` | ❌ `Unsupported NPU architecture` | 导师提供的 950 系列参数，bisheng 15.0.5 **不认识** |
| **bisheng lib 内置架构（grep 文本资源发现）** | **`dav-920r2 / dav-l210 / dav-n350 / dav-t210`** | 内部架构命名体系（与公开 dav-3510 等不同），**950 系列最可能在其中**（如 dav-n350），env_probe 已自动加入候选并实测 |
| `--npu-arch=dav-2201` | ✅ 被接受，`__NPU_ARCH__=2201` | 编译进行到 include 阶段 |
| `--npu-arch=dav-3510` | ✅ 被接受，`__NPU_ARCH__=3510` | 编译进行到 include 阶段（公开文档 Atlas 350 = dav-3510） |
| `--npu-arch=dav-5102` | ❌ Unsupported | 不被支持（但门控里有 5102） |

→ **待与导师确认 950 系列的正确参数**（候选：dav-n350 / dav-920r2 / dav-l210 / dav-t210 / dav-3510 / dav-2201；env_probe 第 7 节自动逐个实测）。

**设备可用性（2026-08-19 实测）**：`/dev/davinci0 /dev/davinci_manager` 设备节点存在；`probe_device`（aclInit→aclrtGetDeviceCount→aclrtGetSocName）**运行成功** → 本机**直连可用 NPU**（此前 npu-smi 缺失仅是无查询工具，不影响运行）。`DEVICE_QUERY=1`。

**SIMT include 依赖（编译要点）**：simt_api 头依赖链很长（asc_simt.h → common_functions.h → device_types.h → utils/base/sys_macros.h → **impl/utils/sys_macros.h**，跨目录），单靠 `-I simt_api 父目录` 不够。env_probe 第 7 节已实现**缺头自动修复**：解析 `fatal error: 'xxx'` → 在 ASCEND_HOME 下 find → 补 `-I` 重试（最多 3 轮）。

**API 差异（真机头文件，修正此前基于公开版的判断）**

| API | 公开版判断 | 真机 9.2.0 | 影响 |
|---|---|---|---|
| aclrtGetVersion | 不存在（wrapper 用模拟版本宏） | **存在但已废弃**：`aclError aclrtGetVersion(int32_t* major, int32_t* minor, int32_t* patch)`，带 `ACL_DEPRECATED_MESSAGE`，官方建议 **aclSysGetVersion** | wrapper 应改用 aclSysGetVersion（签名待核） |
| aclblasCreate | 不存在（自定义 handle 结构） | **仍无独立 aclblasCreate**；存在 `aclblasCreateHandleForGemvEx/Hgemv/...` 系列 | cublas 桥维持自定义 handle 结构 ✅ 原判断正确 |
| aclrtGetSocName | 在 acl_rt.h | acl_rt.h 未匹配，但 **include/acl 全目录可搜到** | wrapper 依赖它，可编译（所在头待定位） |
| aclblasAxpy/Copy/Dot/Scal/Swap/Nrm2 | 待核 | **不存在** | BLAS1 维持 not-support 占位 ✅ 结论确认 |
| aclrtGetDeviceProperties | 不存在 | 不存在 ✅ | 维持 aclrtGetDeviceInfo |
| ACL_DT_DOUBLE | 无 | 无 ✅ | Dgemm 维持 not-support |

**SIMT 头非自包含（编译依赖）**：`asc_simt.h` 内部 `#include "simt_api/common_functions.h"`（还依赖 device_types.h/device_functions.h 等），因此编译时必须把 **simt_api 的父目录**（cann-9.2.0/x86_64-linux/asc/include）加入 `-I`，只加 simt_api 目录本身会报 `fatal error: 'simt_api/common_functions.h' file not found`。env_probe/run_tests 已处理（`SIMT_API_ROOT`）。

**SIMT 函数签名**：asc_syncthreads/threadfence/block、asc_shfl(_up/down/xor)、asc_all/any/ballot、asc_atomic_add/sub/exch/max/min、asc_reduce_add **全部存在** ✅（simt_adaptor.h 映射目标确认）。

**driver**：Version=7.0.t9.0.B880（2026-08-12 构建）；npu-smi 未安装（纯编译验证可忽略）。

**后续动作**：①确认 dav-c310 可编译（或确认正确参数）；②确认 aclrtGetVersion/aclblasCreate 签名后改造 wrapper；③定位 aclrtGetSocName。

---

## 4. 库

### 4.1 BLAS（C4 ✅）

- 头文件：`acl/ops/acl_cblas.h`；库：`libacl_cblas.so`（`${INSTALL_DIR}/lib64/`）。
- 函数：`aclblasSgemm`、`aclblasS8gemm`、`aclblasGemmEx`、`aclblasHgemm`、`aclblasCgemm`、`aclblasCreateHandleForGemmEx` 等。
- 两种执行风格：非 handle（每次匹配模型）与 handle（`CreateHandleFor*` + `acl.op.execute_with_handle` + `destroy_handle`）。
- GemmEx 数据类型组合受限：`aclFloat16×aclFloat16×aclFloat16`、`aclFloat16×aclFloat16×float`、`int8×int8×float`、`int8×int8×int32`。

原型示例：
```c
aclError aclblasGemmEx(aclTransType transA, aclTransType transB, aclTransType transC,
    int m, int n, int k, const void *alpha,
    const void *matrixA, int lda, aclDataType dataTypeA,
    const void *matrixB, int ldb, aclDataType dataTypeB,
    const void *beta, void *matrixC, int ldc, aclDataType dataTypeC,
    aclComputeType type, aclrtStream stream);
```

### 4.2 cudnn/curand/cufft（C5）

- 无同名 API 级对应。910B 兼容层方案提到"内置 cuDNN 兼容算子库，覆盖卷积/归一化/注意力等 2000+ 通用算子"（算子级，非 cudnn API 级）。
- 需逐个定制或后置。

---

## 5. 构建与集成（C6 ✅）

- 昇腾算子工程：CMakePresets.json + build.sh，产物 `custom_opp_<os>__<arch>.run`、kernel `.o`、`libcust_opsproto_*.so`、`liboptiling.so`、`libcust_opapi.so`。
- asc-devkit 用 `CMAKE_ASC_ARCHITECTURES` 配置 NPU 架构。
- 环境变量：`source ${INSTALL_DIR}/bin/setenv.bash`；`ASCENDC_BUILD_LOG_DIR`（日志）。
- 头文件 `${INSTALL_DIR}/include/`、库 `${INSTALL_DIR}/lib64/`。

---

## 6. 社区参考资产（新发现）

| 资产 | 说明 | 与 cu2cce-bridge 关系 |
|---|---|---|
| **caas-cli** | 开源命令行：`caas analyze`（PyTorch 兼容性分析）/ `caas convert`（`.cu`→`.cpp`，映射 `cudaMalloc→aclrtMalloc`、`cudaFree→aclrtFree`、`cudaMemcpy→aclrtMemcpy` 等；含 ascend/cambricon/hygon 多平台映射表）；架构 analyzer.py/converter.py/cuda_parser.py | ⚠️ 借鉴其 API 映射表（license 待核实） |
| **cann/asc-devkit** | CANN 官方开源算子开发工程，`examples/03_simt_api/`（00_introduction HelloWorld、01_gather 等） | 真机验证的现成测试基线 |
| **cann/ops-blas** | 开源昇腾 BLAS（Validate+Launch 架构、dlog 日志、workspace 管理、NN/NT/TN/TT 测试） | aclblas 实现参照 |
| **Triton→Ascend**（MindSpore/akg PR） | Triton-CUDA 算子→Triton-Ascend 自动转换，`cuda_to_ascend_utils.py` + 参考数据验证 | 思路参考，不直接复用 |
| 910B CUDA 兼容层方案 | AST 解析 torch.cuda→torch_npu；自定义核函数转换率 86.5%；性能损耗未改造 38-45%、完整兼容栈 14-19% | 背景/方法论参考 |

---

## 7. 传统 Ascend C（tile 模型）与 SIMT 的差异（路线 B 需要）

- 传统 Ascend C：SPMD + 静态流水线，**无 threadIdx 逐元素模型**；用 `GetBlockIdx()/GetBlockNum()` 做多核分片；黄金法则"计算必须在 UB 中进行，GM 仅输入/输出，不能 random access"；典型 `CopyIn → Compute → CopyOut` tile 循环；`AscendC::Add(zLocal, xLocal, yLocal, tileLength)` 一条指令操作 256 个数据（SIMD）。
- **只有当目标硬件无 SIMT（910B/910C）时，才需要把 CUDA SIMT kernel 转成这种 tile 模型**（路线 B 的第 4 条线）。

---

## 8. cu2cce-bridge wrapper API 覆盖清单（2026-08-07，124 个函数）

> 来源：`include/bridge/runtime/cuda_runtime_wrapper.h` + `src/bridge/runtime/src/cuda_runtime_wrapper.cpp`。
> 状态：`✅`=真实映射昇腾 API，`🔧`=占位/简化/not-support，`⚠️`=需真机核对签名。

### 8.1 真实映射（昇腾有对应）✅

| CUDA 函数 | 昇腾调用 | 备注 |
|---|---|---|
| cudaMalloc/cudaFree | aclrtMalloc(HUGE_FIRST)/aclrtFree | policy 差异 |
| cudaMallocHost/cudaFreeHost | aclrtMallocHost/aclrtFreeHost | |
| cudaMallocPitch | aclrtMalloc 模拟 | ⚠️ pitch 对齐规则待真机 |
| cudaMemcpy/cudaMemcpyAsync | aclrtMemcpy/Async | 补 destMax |
| cudaMemcpy2D/2DAsync | aclrtMemcpy2d/2dAsync | 签名一致 |
| cudaMemcpyPeer/PeerAsync | aclrtMemcpy(D2D) | 需先 EnablePeerAccess |
| cudaMemset/MemsetAsync | aclrtMemset/Async | 4/5 参 |
| cudaMemset2D/2DAsync | 逐行 aclrtMemset | 模拟 |
| cudaMemGetInfo | aclrtGetMemInfo(HBM) | |
| cudaGetDeviceCount/SetDevice/GetDevice | aclrtGetDeviceCount/SetDevice/GetDevice | |
| cudaDeviceSynchronize | aclrtSynchronizeDevice | |
| cudaDeviceReset | aclrtGetDevice+ResetDevice | ResetDevice 带参 |
| cudaDeviceGetAttribute | 从 _cceDeviceProp_t 映射 | 13 个属性 |
| cudaGetDeviceProperties | soc name + mem info 组装 | ⚠️ aclrtGetDeviceProperties 签名待核 |
| cudaDeviceCanAccessPeer/Enable/Disable | aclrtDeviceCanAccessPeer/... | |
| cudaCtxCreate/Destroy/SetCurrent/GetCurrent | aclrtCreateContext/... | |
| cudaStreamCreate/Destroy/Synchronize/WaitEvent | aclrtCreateStream/... | |
| cudaEventCreate/Destroy/Record/Synchronize/Query/ElapsedTime | aclrtCreateEvent/... | QueryEvent 带 status |
| cudaLaunchHostFunc | aclrtLaunchCallback | blockType=NO_BLOCK |
| cudaMallocManaged | fallback aclrtMalloc | ⚠️ 无自动迁移 |
| cudaDeviceGetPCIBusId/GetByPCIBusId | soc name 近似 | |

### 8.2 占位（昇腾无对应，返回 not-support / no-op）🔧

- **返回 not-support**：HostGetDevicePointer、HostRegister/Unregister/HostGetFlags、LaunchKernel（显式 API）、Dgemm（无 double）、BLAS1/2（Sgemv/Saxpy/Scopy/Sdot/Sscal/Sswap/Snrm2）、Stream capture 全组、FuncGetAttributes/SetAttribute、Symbol 组、PointerGetAttributes、DeviceGet/SetCacheConfig、DeviceGet/SetSharedMemConfig、DeviceGet/SetLimit、ThreadGetCacheConfig
- **no-op 返回成功**：MemAdvise、MemPrefetchAsync、ProfilerStart/Stop
- **废弃旧名映射**：ThreadSynchronize→DeviceSynchronize、ThreadExit→DeviceReset

### 8.3 已补的占位组（2026-08-07 后续轮次，全部 not-support 或 fallback）

- **SOMA**：MallocAsync/FreeAsync fallback 到 aclrtMalloc/Free；MemPoolCreate/Destroy/DeviceGet/SetMemPool not-support
- **Array**：MallocArray/Malloc3DArray/MallocMipmappedArray/FreeArray/FreeMipmappedArray/MemcpyToArray/FromArray/2DToArray/2DFromArray（9 个，not-support）
- **Texture/Surface**：CreateChannelDesc（4×16bit 编码）/Create-DestroyTextureObject/Create-DestroySurfaceObject/BindTexture（6 个）
- **Graph 核心**：Create/Destroy/Instantiate/Launch/ExecDestroy/ExecUpdate/AddKernelNode/MemcpyNodeSetParams1D（8 个）
- **3D 拷贝**：Memcpy3D/Memcpy3DAsync
- **ExternalMemory**：Import/Destroy

### 8.4 仍未声明（cu-bridge 有但昇腾无，应用极少用）

- EGL/GL 互操作（cudaEGL*）、ExternalSemaphore、cudaCG 协作组、Occupancy 其余变体、Graph 其余 90+ 节点/属性函数、SOMA 其余（MemPool 属性/导入导出）、cudaMemcpy2DToArrayAsync 等 Async 变体。

> 真机验证建议：对照 §8.1 逐函数跑 smoke test；§8.2 验证 not-support 返回码正确；§8.3 已声明占位保证应用可编译；§8.4 评估应用是否依赖再按需补。

---

## 9. 来源链接

- [昇腾社区 - AI Core SIMT 编译/算子编译简介](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/910beta1/programug/Ascendcopdevg/docs/guide/%E7%BC%96%E7%A8%8B%E6%8C%87%E5%8D%97/%E7%BC%96%E8%AF%91%E4%B8%8E%E8%BF%90%E8%A1%8C/AI-Core-SIMT%E7%BC%96%E8%AF%91/%E7%AE%97%E5%AD%90%E7%BC%96%E8%AF%91%E7%AE%80%E4%BB%8B.md)
- [昇腾社区 - SIMT BuiltIn 关键字](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/910beta3/programug/Ascendcopdevg/docs/guide/%E7%BC%96%E7%A8%8B%E6%8C%87%E5%8D%97/%E8%AF%AD%E8%A8%80%E6%89%A9%E5%B1%95%E5%B1%82/SIMT-BuiltIn%E5%85%B3%E9%94%AE%E5%AD%97.md)
- [昇腾社区 - asc_syncthreads](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/910beta1/API/ascendcopapi/atlasascendc_api_07_10306.html)
- [昇腾社区 - SIMT 算子实践参考/基础知识](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/910beta1/programug/Ascendcopdevg/docs/guide/%E7%AE%97%E5%AD%90%E5%AE%9E%E8%B7%B5%E5%8F%82%E8%80%83/SIMT%E7%AE%97%E5%AD%90%E5%AE%9E%E7%8E%B0/%E5%9F%BA%E7%A1%80%E7%9F%A5%E8%AF%86.md)
- [昇腾社区 - Runtime API(C) 系列首页：头文件和库文件说明（CANN 9.1.0）](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/runtimeapi/aclcppdevg_03_1952.html)
- [昇腾社区 - 同步和异步 API 说明](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/runtimeapi/aclcppdevg_03_0020.html)
- [昇腾社区 - aclrtLaunchKernel（03_0151）](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/runtimeapi/aclcppdevg_03_0151.html)
- [昇腾社区 - aclrtLaunchKernelV2（03_1960）](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/runtimeapi/aclcppdevg_03_1960.html)
- [昇腾社区 - aclrtMalloc（03_0095）](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/runtimeapi/aclcppdevg_03_0095.html)
- [昇腾社区 - aclrtMemcpy（03_0105）](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/runtimeapi/aclcppdevg_03_0105.html)
- [昇腾社区 - aclrtMemcpyAsync（03_0106）](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/runtimeapi/aclcppdevg_03_0106.html)
- [昇腾社区 - aclrtCreateStream（03_0065）](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/runtimeapi/aclcppdevg_03_0065.html)
- [昇腾社区 - aclrtCreateEvent（03_0078）](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/runtimeapi/aclcppdevg_03_0078.html)
- [昇腾社区 - aclrtSetDevice（03_0039）](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/runtimeapi/aclcppdevg_03_0039.html)
- [昇腾社区 - aclrtLaunchKernel](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900beta2/API/appdevgapi/aclcppdevg_03_0151.html)
- [昇腾社区 - 毕昇编译器概述](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/compiler/BishengCompiler/atlas_bisheng_10_0001.html)
- [昇腾社区 - CCEC 编译器说明](https://www.hiascend.com/document/detail/zh/canncommercial/5046/operatordev/tbedevg/tbedevg_000147.html)
- [昇腾社区 - 矩阵-矩阵乘运算（BLAS）](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/850alpha002/appdevg/acldevg/aclcppdevg_04_0001.html)
- [asc-devkit - AI Core SIMT 编程模型概述](https://asc.gitcode.com/guide/%E7%BC%96%E7%A8%8B%E6%8C%87%E5%8D%97/%E7%BC%96%E7%A8%8B%E6%A8%A1%E5%9E%8B/AI-Core-SIMT%E7%BC%96%E7%A8%8B/%E6%A6%82%E8%BF%B0.html)
- [asc-devkit - examples/03_simt_api](https://gitcode.com/cann/asc-devkit/tree/25cbbdbbc0dfed820dc1aabf93d431934fee7576/examples/03_simt_api/00_introduction)
- [asc-devkit - 避免 UB Bank 冲突](https://gitcode.com/cann/asc-devkit/blob/d314b5d4e5820e1283590ec4ec12eeb6e235ed9f/docs/guide/%E7%AE%97%E5%AD%90%E5%AE%9E%E8%B7%B5%E5%8F%82%E8%80%83/SIMT%E7%AE%97%E5%AD%90%E6%80%A7%E8%83%BD%E4%BC%98%E5%8C%96/%E5%86%85%E5%AD%98%E8%AE%BF%E9%97%AE/%E9%81%BF%E5%85%8DUB%E7%9A%84Bank%E5%86%B2%E7%AA%81.md)
- [ops-blas 昇腾 BLAS](https://gitcode.com/cann/ops-blas/pull/292)
- [昇腾 CUDA 迁移工具综述（知乎）](https://zhuanlan.zhihu.com/p/2062880742728791164)
- [910B CUDA 兼容层实操（腾讯云）](https://cloud.tencent.cn/developer/article/2690422?policyId=1003)
- [Triton-CUDA→Triton-Ascend（MindSpore/akg PR）](https://gitee.com/mindspore/akg/pulls/1259)
