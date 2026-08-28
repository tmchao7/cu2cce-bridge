# 支持范围总览（Supported Scope）

> 一页看懂"现在什么能用"。逐项真机证据见 `../03_Comparison/` 各矩阵；
> 反面清单（明确不支持/已知限制）见同目录 `LIMITS_AND_BOUNDARIES.md`。
> 环境：Ascend 950DT · CANN 9.2.0 · 真机回归 `main@51f0382+`。

## 1. 编译驱动（cucc）

| 能力 | 状态 |
|---|---|
| nvcc 风格参数 → ccec puresimt（`-x asc --enable-simt`） | `SUPPORTED` |
| 规则数据驱动（JSON 五动作）+ strict 模式 | `SUPPORTED` |
| `-c` 编译 / compile_and_link / link 三形态 | `SUPPORTED` |
| FP64 device 使用前置策略（error/warn/allow） | `SUPPORTED` |
| dependency（-M/-MD）/ preprocess（-E） | `DIAGNOSTIC_ONLY` |
| PTX/CUBIN/FATBIN/S/设备链 | `UNSUPPORTED`（明确拒绝） |

## 2. 语言/头文件面（.cu 源码所见）

| 能力 | 状态 |
|---|---|
| CUDA SIMT 语法（`__global__/__shared__/warp 原语/atomic 族） | `SUPPORTED` |
| `__device__` 独立拼写（经执行空间映射） | `SUPPORTED`（真机验证） |
| 单精度设备数学（expf/logf/sqrtf/sinf/cosf/tanhf/powf/fmin/fmax/fabsf 等） | `SUPPORTED` |
| CUDA 向量类型 char1..float4（编译器原生） | `SUPPORTED` |
| crt 前向兼容层（math/device/common_functions + host_defines） | `SUPPORTED`（前向） |
| host 侧整数 min/max（CUDA 双向语义） | `SUPPORTED`（模板桥） |
| device double（数学/向量类型） | **`UNSUPPORTED`**（架构边界） |
| thrust / cutlass / CCCL / nvtx3 | `UNSUPPORTED`（未覆盖） |

## 3. 运行时（wcuda → ACL）

| 能力族 | 状态 |
|---|---|
| 内存（malloc/free/MallocHost/HostAlloc/managed 部分/pitch） | `SUPPORTED`（核心） |
| 拷贝/填充（同步+异步、1D/2D/peer） | `SUPPORTED`（核心） |
| 流（create/destroy/sync/query/waitEvent）、事件（record/query/sync/elapsed） | `SUPPORTED`（含默认流语义消费点实现） |
| 默认流三种拼写的 legacy 语义 | `SUPPORTED`（消费点实现，simpleStreams 复验） |
| device 管理/属性（GetDeviceProperties 常用字段） | `SUPPORTED`（诚实填充） |
| Graph / HostFunc / Pointer 属性等长尾 | `EXPERIMENTAL`~`UNSUPPORTED`（逐项见 PURESIMT 矩阵） |

## 4. 构建系统

| 能力 | 状态 |
|---|---|
| 现代 CMake CUDA 工程（探测/规则/链接） | `SUPPORTED`（6 案例） |
| 多目录 + install/export + 生成文件依赖 | `SUPPORTED`（large_project） |
| Makefiles / Ninja | `EXPERIMENTAL` |
| `find_package(CUDAToolkit)` + `CUDA::*` 别名族 | `EXPERIMENTAL` |
| NVIDIA CUDA Samples 零改写 | vectorAdd/matrixMul 必需；simpleStreams/simpleMultiCopy DIAG-PASS 复验 |
| multi `.cu` 静态库 | `DIAGNOSTIC_ONLY`（后端注册符号聚合缺口） |

## 5. 库

| 库 | 状态 |
|---|---|
| cuBLAS：Sgemm（FP32→FP16 staging）/ GemmEx（16F,16F,16F·(16F,16F,32F)·(8I,8I,32I)）/ Sgemv | `EXPERIMENTAL`/`SUPPORTED`(gemv 限定形状)，真机三绿 |
| cublas 模式 API（PointerMode 真实落实；Math/Atomics 提示透传） | `EXPERIMENTAL` |
| Dgemm / BLAS1（axpy/copy/dot/scal/swap/nrm2） | `UNSUPPORTED`（参数校验先行） |
| cufft/cusparse/curand/cudnn | `UNSUPPORTED`（不建虚假 target） |

## 6. 悬置（非桥侧）

| 议题 | 状态 |
|---|---|
| asyncAPI 单元素丢失（26 轮自增后 1 元素少 1，索引漂移） | `PENDING`——三变体最小复现未触发，昇腾侧执行层嫌疑（档案含触发清单） |
