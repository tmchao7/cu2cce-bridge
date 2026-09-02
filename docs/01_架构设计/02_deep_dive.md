# 架构深度解析（Runtime Deep Dive）

> 面向需要维护/扩展 `src/bridge/runtime/` 的工程师。
> 所有机制均来自真机排障实证；可复现的诊断资产见 `tools/real_test/diag/`，具体证据在文中标注。

## 1. 初始化：BRIDGE_INIT 与 107002 教训

```c
BRIDGE_INIT();   // std::call_once 幂等：aclInit(nullptr) → aclrtGetDeviceCount → aclrtSetDevice(0) → CreateContext
```

- ACL 要求首次 runtime API 前完成 `aclInit` + 设备/上下文绑定，CUDA 无此要求
  ——这是 CUDA→ACL 桥的第一性差异。
- **107002 事故定案**：`wcudaMalloc` 曾把 `aclrtMalloc` 写在 `WCUDA_RETURN`
  （内含 BRIDGE_INIT）之前，导致"首个 API 就是 cudaMalloc"的进程稳定 107002。
  规则：**任何直接调用 aclrt* 的函数体，第一行必须是 `BRIDGE_INIT()`**，
  不能依赖后续宏补初始化。
- 错误模型：`WCUDA_RETURN(x)` = 记录 `g_last_error`（thread_local）→ 返回；
  `aclGetRecentErrMsg()` 取 ACL 侧最近错误文本。

## 2. 默认流：三种 CUDA 拼写 → 一条桥内自有流

```c
static aclError wcuda_resolve_stream(aclrtStream request, aclrtStream *resolved);
```

- CUDA 的 `0 / cudaStreamLegacy / cudaStreamPerThread` 在 ACL 无对应句柄，
  且 aclrt* 家族对 nullptr 流一律 `ACL_ERROR_INVALID_PARAM(100000)`。
- 桥内惰性 `aclrtCreateStream` 一条自有流作为三种默认流拼写的后端替身；
  显式创建的流原样透传；创建失败持续返回同一错误（不静默伪造）。
- **legacy NULL-stream 语义（消费点实现）**：CUDA 语义中 stream-0 与所有
  阻塞流隐式同步。桥的实现放在**消费点**：
  - `wcudaEventSynchronize / wcudaEventQuery(COMPLETE) / wcudaEventElapsedTime`
    涉及"记录在默认流上的事件"时，先 `aclrtSynchronizeDevice()` 再返回。
  - 事件是否记录在默认流，由 `wcudaEventRecord` 写入注册表
    （mutex + unordered_map）登记。
  - **为什么不能在记录时同步**：asyncAPI 这类样本会在流水线中途向默认流
    记事件，记录时全设备同步会把它的等待节奏从数千次 spin 压成 33 次、
    且数据从 25/26 恶化到 0（真机回归实证，2026-08-28）。

## 3. 事件与跨流计时

- `wcudaEventRecord(event, stream)`：解析流 → 登记"是否默认流" →
  `aclrtRecordEvent`。
- `wcudaEventQuery`：`aclrtQueryEventStatus`（9.x 新 API；注意
  `aclrtEventRecordedStatus` 的 COMPLETE=1 与旧枚举相反）→ COMPLETE 且
  默认流事件 → 先设备同步 → 返回 success；未完成 →
  `ACL_ERROR_RT_EVENT_NOT_COMPLETE`（映射 `cudaErrorNotReady`）。
- `wcudaEventElapsedTime`：涉及默认流事件 → 先设备同步 → 读毫秒。
  （simpleStreams 依赖跨流计时，未修前 elapsed≈0 且数据大面积丢失。）

## 4. 同步拷贝的隐式排序

CUDA 语义中同步 `cudaMemcpy/cudaMemset` 与默认流工作有序。桥内：

- `wcudaMemcpy`（D2H/D2D）与 `wcudaMemset2DAsync` 等在调用前
  `aclrtSynchronizeDevice()`（D2H/D2D 路径，vectorAdd 教训）；
- 异步路径（memcpyAsync/memsetAsync/2D/peer）走解析流，无额外同步——
  流内顺序由 ACL 保证。

## 5. launch 路径

```c
wcudaLaunchKernel → aclrtLaunchSIMTKernelWithArgsArray(func, grid, block,
                    dynUbuf=sharedMemBytes, stream, cfg=nullptr, args)
```

- 仅 Ascend 950PR/950DT 支持（官方文档口径）。
- `include/bridge/kernel/simt_adaptor.h` 负责 CUDA SIMT 拼写映射：
  `__syncthreads/__threadfence/__shared__(=__ubuf__)/shfl 族/atomic 族`；
  以及 **`__device__ → __SIMT_DEVICE_FUNCTIONS_DECL__` 执行空间映射**
  （前端不识别裸 `__device__`；该选择器自动跟随 aicore/simt_callee 双模式；
  逃生开关 `CU2CCE_NO_DEVICE_KEYWORD_MAP`）。

## 6. 平台事实清单（写代码前必读，均为真机实证）

1. bisheng **原生注入全套 CUDA 向量类型**（ext_vector_type typedef）——
   桥接层禁止再定义（曾冲突击穿全部门禁）。
2. `__host__/__device__/__forceinline__` 是**语言关键字**，绝不可
   `#define`（曾剥离双空间声明，致误导性报错）。
3. **device 侧没有 double**：无 double 数学、连 doubleN 向量类型都不存在。
4. `.cu` 的 host 代码**禁裸 malloc/free**——bisheng 注入
   `__aicore__ void *malloc(uintptr_t)` 劫持该名字；宿主内存用
   `cudaMallocHost/cudaFreeHost` 或 `std::`/`new[]`。
5. 链式自动包含的头必须**全符号可证**（inert 原则）——可选兼容面
   独立成头、按证据逐个接入。
6. CANN 计算接口（aclblas*）：①要求 device scalar（host 标量需 staging）；
   ②`aclblasGemmEx` 的 lda/ldb/ldc、`aclblasGemvEx` 的 (lda,incx,incy)
   是保留参数，**必须传 -1**；③返回 100024 = 算子/模型缺失（查
   ASCEND_OPP_PATH 或 aclopSetModelDir）。

## 7. 调试开关

| 开关 | 产物 |
|---|---|
| `WCUDA_DEBUG=1` | 每个 wcuda* 调用的进入/返回码逐行 stderr（harness 诊断运行默认开启） |
| `CU2CCE_DEBUG=1` + `CU2CCE_TRACE_FILE=f` | cucc 驱动的规则决策 JSON（参数级，无源码内容） |
| `bridge_init` 行 | aclInit/枚举设备/SetDevice/建上下文逐步返回码 |
