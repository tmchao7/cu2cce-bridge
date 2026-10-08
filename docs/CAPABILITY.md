# cu2cce-bridge 能力声明 (Capability Statement)

> 本文档是项目能力的**权威总声明**：用最直白的方式说清楚我们**能做什么**、
> **不能做什么**、以及**为什么不能**。接手人、评估者或用户只需读这一个文件
> 就能判断本项目是否满足自己的需求。
>
> 逐项证据矩阵见 [`02_能力清单/`](02_能力清单/)；边界与限制的反面清单见
> [`02_能力清单/LIMITS_AND_BOUNDARIES.md`](02_能力清单/LIMITS_AND_BOUNDARIES.md)。

---

## 1. 一句话定位

cu2cce-bridge 是 **CUDA → 昇腾 NPU Vector Core 的 SIMT 兼容层**。

它让用户保留原始 `.cu` 源码，通过 CANN `ccec --enable-simt` 直接编译运行在
Ascend 950 的 **Vector 核**上，覆盖常见的 CUDA Runtime API、Kernel SIMT 原语
和构建系统入口。

**它不是**一个全量 CUDA → Ascend 的翻译器，不覆盖 Cube 核、不覆盖 Tensor Core
等价物、不覆盖 cutlass/cuDNN/cuFFT/cuSPARSE/cuRAND 等高级算子库。

---

## 2. 核心能力：我们擅长什么

### 2.1 CUDA SIMT Kernel → Ascend Vector Core 直通

这是项目的**核心价值**。用户写标准 CUDA kernel，ccec 直接编译到 Ascend
Vector Core 的 SIMT 模型，**不做源码转换、不生成中间文件**。

| CUDA 写法 | Ascend 映射 | 状态 |
|---|---|---|
| `__global__ void kernel(...)` | ccec `--enable-simt` 原生解析 | `SUPPORTED` |
| `kernel<<<grid, block>>>(...)` | ccec 原生 launch 语法 | `SUPPORTED` |
| `threadIdx/blockIdx/blockDim/gridDim` (1D/2D/3D) | ccec SIMT builtins | `SUPPORTED` |
| `__shared__` | `__ubuf__` (UB 资源) | `SUPPORTED` |
| `extern __shared__` (动态 shared) | 动态 UB 适配 | `SUPPORTED` |
| `__syncthreads()` | `asc_syncthreads()` | `SUPPORTED` |
| `__threadfence()` / `__threadfence_block()` | `asc_threadfence*` | `SUPPORTED` |
| `atomicAdd/Sub/Min/Max/Exch` | `asc_atomic_*` | `SUPPORTED` |
| `__shfl_down_sync` / `__shfl_up_sync` | `asc_shfl_down/up` | `SUPPORTED` |
| `__all_sync` / `__any_sync` / `__ballot_sync` | `asc_all/any/ballot` | `SUPPORTED` |
| `__reduce_add_sync` | `asc_reduce_add` | `SUPPORTED` |
| 独立 `__device__` helper | `__SIMT_DEVICE_FUNCTIONS_DECL__` | `SUPPORTED` |

映射层在 `include/bridge/kernel/simt_adaptor.h` 和 `cuda_device_api.h`，
只做**符号级别名**，不改写 kernel 入口、launch 或参数。

### 2.2 已验证的 25 个真机用例

以下用例在 Ascend 950 / CANN 9.2.0 真机 `full` profile 下全部通过（`FAIL=0`）：

| 类别 | 用例 | 关键验证点 |
|---|---|---|
| 向量 | vectorAdd (官方), vectorAdd_minimal | `__global__`、`<<<>>>`、`cudaMalloc/cudaMemcpy` |
| 矩阵 | matrixMul (官方), matrixMul_minimal | tiled GEMM、shared memory、`__syncthreads` |
| 原子 | atomicAdd_minimal, atomicIndex_minimal, atomicVariants_minimal | `atomicAdd/Min/Max/Sub/Exch`、global + shared |
| 归约 | reduction_minimal, dot_minimal | tree reduction、cross-block atomic |
| Warp | warpShuffle_minimal, vote_minimal | `__shfl_down_sync`、`__all/any/ballot_sync` |
| Scan | scan_minimal | inclusive prefix scan + barrier |
| 转置 | transpose_minimal | tiled transpose + shared tile |
| Stencil | stencil_minimal | halo load + shared tile |
| 基础算子 | core_ops_minimal | 四则、位逻辑、比较、选择、转换 |
| 数学 | math_minimal, device_math_minimal | `expf/logf/sqrtf/sinf/cosf/tanhf/powf/fmin/fmax/fabsf` |
| Runtime | event_minimal, multiKernel_minimal | event 时序、同 stream 多 kernel pipeline |
| 内存 | pitchedMemcpy_minimal, dynamicShared_minimal | `cudaMallocPitch`、`cudaMemcpy2D`、动态 shared |
| 索引 | index3D_minimal | 3D grid/block flattening |
| 内存序 | threadFence_minimal | `__threadfence` + atomic publication |
| 精度类型 | half_minimal, bf16x2_minimal | `__half`/`__half2`、`__nv_bfloat16`/BF16x2 |
| 官方样本 | vectorAdd (NVIDIA), matrixMul (NVIDIA) | 零改写编译运行 |

### 2.3 Host Runtime API Bridge

`cuda* → wcuda* → aclrt*` 三层桥接，覆盖以下已验证 API 族：

| API 族 | 覆盖范围 | 状态 |
|---|---|---|
| 设备管理 | `cudaGetDeviceCount/SetDevice/GetDevice/GetDeviceProperties/GetAttribute` | `SUPPORTED` |
| 内存分配 | `cudaMalloc/Free/Memset` | `SUPPORTED` |
| 内存拷贝 | `cudaMemcpy/Async`、`cudaMemcpy2D`、`cudaMallocPitch` | `SUPPORTED` |
| 流管理 | `cudaStreamCreate/CreateWithFlags/Destroy/Synchronize/Query/WaitEvent` | `SUPPORTED` |
| 事件管理 | `cudaEventCreate/Destroy/Record/Query/Synchronize/ElapsedTime` | `SUPPORTED` |
| 设备同步 | `cudaDeviceSynchronize/Reset` | `SUPPORTED` |
| 错误处理 | `cudaGetLastError/PeekAtLastError` | `SUPPORTED` |
| 默认流 | 三种拼写（0、`cudaStreamPerThread`→桥内流）的 legacy 语义 | `SUPPORTED` |

### 2.4 构建系统适配

| 构建系统 | 覆盖范围 | 状态 |
|---|---|---|
| cucc 驱动 | nvcc 风格参数 → ccec puresimt，规则数据驱动 | `SUPPORTED` |
| CMake (modern) | `project(... LANGUAGES CUDA)`、多 .cu、static/shared lib、install/export | `SUPPORTED` |
| CMake (legacy) | `find_package(CUDA)` + `cuda_add_executable/library` 基础宏 | `SUPPORTED` |
| Make | `make_cce` wrapper + virtual toolkit | `SUPPORTED` |
| Ninja | `ninja_cce` wrapper + CMake Ninja generator | `SUPPORTED` |
| 产物契约 | `.o/.d/.a/.so/ELF executable`，无 `.cce/.ptx/.cubin/.fatbin` | `SUPPORTED` |
| 官方 NVIDIA Samples | vectorAdd/matrixMul 零改写通过 | `SUPPORTED` |

### 2.5 数据类型支持

| 类型 | Device 侧 | Host 侧 |
|---|---|---|
| INT32 | `SUPPORTED` | `SUPPORTED` |
| FP32 | `SUPPORTED` | `SUPPORTED` |
| FP16 / `__half` / `__half2` | `SUPPORTED` | `SUPPORTED` |
| BF16 / `__nv_bfloat16` / BF16x2 | `SUPPORTED` | `SUPPORTED` |
| INT8 | 编译通过 (GemmEx 8I,8I,32I) | `EXPERIMENTAL` |
| INT64 | 未单独验证 | `SUPPORTED` (host) |
| FP64 / double | **`UNSUPPORTED`** (见下文) | host 可用 |

---

## 3. 明确不能做的：能力边界

### 3.1 只覆盖 Vector Core，不覆盖 Cube Core

**这是最核心的架构边界。**

Ascend NPU 有两类 AI Core 计算单元：
- **Vector Core**（向量核）：SIMT 模型，逐元素并行，适合 element-wise、reduction、scan 等通用计算
- **Cube Core**（矩阵核）：矩阵乘加速单元，适合大规模 GEMM/Conv

cu2cce-bridge 的 puresimt 路线 **只走 Vector Core**：

```text
ccec --enable-simt --cce-aicore-arch=dav-c310-vec
                                         ^^^^
                                    Vector Core 架构
```

**后果**：
- CUDA kernel 中的所有计算都映射到 Vector Core 的 SIMT 执行模型
- **不自动将 CUDA GEMM kernel 调度到 Cube Core**
- 不自动将 CUDA Tensor Core 操作（`wmma::*`、`mma::*`）映射到 Cube Core
- 用户如果在 CUDA kernel 中手写 tiled GEMM（如 `matrixMul`），走的是 Vector Core 的
  乘加指令，**不是 Cube Core 的矩阵加速引擎**
- 性能：Vector Core 跑 GEMM 远慢于 Cube Core，本项目不承诺性能等价

### 3.2 不覆盖 Tensor Core / WMMA / MMA

| CUDA 能力 | 状态 | 原因 |
|---|---|---|
| `nvcuda::wmma::*` (Warp Matrix Multiply Accumulate) | `UNSUPPORTED` | Ascend SIMT 无对应 warp-level matrix fragment API |
| `mma_sync` / `ldmatrix` / PTX `mma` 指令 | `UNSUPPORTED` | PTX 整体被 reject；Ascend 无 PTX 等价物 |
| Tensor Core 数据类型 (`__nv_fp8_*`, TF32) | `UNSUPPORTED` | 无 Tensor Core 等价物 |
| `__dp4a` / `__dp2a` (dot product accumulate) | `NOT_VERIFIED` | 无 Ascend SIMT 等价探针 |

### 3.3 不覆盖高级 CUDA 算子库

| CUDA 库 | 状态 | 说明 |
|---|---|---|
| **cutlass** | `UNSUPPORTED` | 高性能 GEMM/Epilogue 模板库；依赖 Tensor Core、PTX、device-link，本项目均不支持 |
| **cuDNN** | `UNSUPPORTED` | 深度学习算子库；无 Ascend 等价 bridge，不伪造 target |
| **cuFFT** | `UNSUPPORTED` | FFT 库；无 bridge |
| **cuSPARSE** | `UNSUPPORTED` | 稀疏矩阵库；无 bridge |
| **cuRAND** | `UNSUPPORTED` | 随机数库；无 bridge（链接期 drop `-lcurand`，符号由 bridge runtime stub 提供） |
| **NCCL** | `UNSUPPORTED` | 通信库；无 HCCL 自动 alias |
| **Thrust** | `UNSUPPORTED` | CUDA STL 级算法库；依赖 device lambda/functor，未覆盖 |
| **CCCL** | `UNSUPPORTED` | CUDA C++ Core Libraries；同上 |
| **nvtx3** | `UNSUPPORTED` | Profiling 标注库；未覆盖 |

### 3.4 cuBLAS 只有极有限的实验性覆盖

cuBLAS 不是主线能力，只有 3 个用例在 `EXPERIMENTAL` 状态：

| CUDA API | 状态 | 限制 |
|---|---|---|
| `cublasSgemm` | `EXPERIMENTAL` | FP32 输入 staging 到 FP16 + `aclblasGemmEx`，**有精度损失**；32x32x32 identity-B 测试；需要 ATC/OPP 单算子模型 |
| `cublasGemmEx` | `EXPERIMENTAL` | 仅 (16F,16F,16F)/(16F,16F,32F)/(8I,8I,32I) 组合；32x32x32 小规模测试 |
| `cublasSgemv` | `SUPPORTED` (限定形状) | 8x8 identity 测试；(lda,incx,incy) 必须 (-1,-1,-1)，仅连续向量 |
| `cublasDgemm` | `UNSUPPORTED` | Ascend 无 FP64 GEMM |
| BLAS1 (axpy/copy/dot/scal/swap/nrm2) | `UNSUPPORTED` | 参数校验后返回 not-support |

**这不等于 cuBLAS 兼容**，只是极窄的实验性探针。

### 3.5 Device 侧 FP64 / double 完全不支持

| 能力 | 状态 | 原因 |
|---|---|---|
| device `double` 数学运算 | `UNSUPPORTED` | Ascend AICore 无 FP64 计算能力 |
| `double2` 等向量类型 | `UNSUPPORTED` | 编译器无 `double2` 类型定义 |
| `cublasDgemm` | `UNSUPPORTED` | 同上 |

**桥的行为**：cucc 在调用 ccec 前做词法扫描，发现 device 区域的 double 使用时
以 `error`（默认）/`warn`/`allow` 策略报告，**不做静默降精度**。

### 3.6 PTX / CUBIN / FATBIN / 设备二进制

| 能力 | 状态 | 原因 |
|---|---|---|
| PTX 生成 (`--ptx`) | `UNSUPPORTED` | cucc 明确 reject |
| CUBIN / FATBIN | `UNSUPPORTED` | puresimt 消费 CUDA source，不产生 NVIDIA binary |
| device-link / RDC / separable compilation | `UNSUPPORTED` | 无 Ascend 等价路径 |
| 内联 PTX (`asm("...")`) | `UNSUPPORTED` | Ascend 无 PTX 指令集 |

### 3.7 不覆盖的 CUDA 高级特性

| 特性 | 状态 | 原因 |
|---|---|---|
| CUDA Graph / stream capture | `UNSUPPORTED` | 返回 `ACL_ERROR_API_NOT_SUPPORT` |
| Texture / Surface object | `UNSUPPORTED` | Ascend 无 CUDA texture/surface 等价对象 |
| CUDA array / mipmapped array | `UNSUPPORTED` | 无等价物 |
| CUDA dynamic parallelism | `UNSUPPORTED` | 无 device-side launch 等价物 |
| Unified Memory (UVM) 自动迁移 | `EXPERIMENTAL` (fallback) | `cudaMallocManaged` fallback 到普通分配；无自动迁移语义 |
| Cooperative kernel launch | `UNSUPPORTED` | 无 cooperative-grid 语义 |
| `cudaLaunchKernel` 显式入口 | `UNSUPPORTED` | main 使用 `<<<>>>`，显式函数入口无稳定等价路径 |
| `cudaHostRegister` / mapped host memory | `UNSUPPORTED` | Ascend 无统一 mapped-host 语义 |
| `__threadfence_system()` | `UNSUPPORTED` | 无系统级 fence 等价物；编译报错而非静默降级 |
| Stream priority / ID | `UNSUPPORTED` | CANN 无对应语义 |

### 3.8 扩展 Device API（opt-in，未验证）

以下 API 的宏映射存在，但需显式设置 `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1`
才暴露，且**当前未纳入主线数值回归**：

| API | 状态 | 说明 |
|---|---|---|
| `atomicInc/Dec/CAS/And/Or/Xor` | `NOT_VERIFIED` | 映射存在，未验证 |
| `__activemask()` | `NOT_VERIFIED` | 官方接口存在，lane 行为未验证 |
| `__reduce_max_sync/min_sync` | `NOT_VERIFIED` | 依赖 CANN Warp header 和编译模型 |

### 3.9 多 .cu 聚合限制

ccec 对每个 `.cu` 编译单元生成相同的运行时注册符号
（`BishengLaunchKernelWithHostArgs`、`__cce_rtLinkedDevBinaryAscPureSimtEagerRegister`），
导致多个 `.cu` 对象放入同一 static archive 后链接时出现 duplicate symbol。

**当前限制**：每个 linked library 最多一个 `.cu` translation unit。
这是 Ascend 后端限制，不是桥侧可修复的。

### 3.10 Warp mask 语义不完全等价

CUDA 的 `*_sync` 系列 API 接受 `mask` 参数表示参与操作的 lane 集合。
Ascend SIMT 的 warp 操作为**隐式全 warp**，mask 参数被求值后忽略。

**后果**：
- 简单的 `__shfl_down_sync(0xffffffff, ...)` 可正常工作
- 依赖动态 mask 的复杂 warp 协作模式**不保证语义正确**
- `__activemask()` 返回值的位图宽度由 CANN 定义，不保证与 NVIDIA 一致

---

## 4. 能力总结表

### 能做 (SUPPORTED)

| 领域 | 具体能力 |
|---|---|
| Kernel 语言 | `__global__`、`<<<>>>`、1D/2D/3D 索引、`__shared__`(静态+动态)、`__syncthreads`、`__threadfence` |
| Device 原语 | `atomicAdd/Sub/Min/Max/Exch`、`__shfl_*_sync`(down/up)、`__all/any/ballot_sync`、`__reduce_add_sync` |
| Device 数学 | 单精度 `expf/logf/sqrtf/sinf/cosf/tanhf/powf/fmin/fmax/fabsf/fmaf/floorf/ceilf` |
| Device 类型 | INT32、FP32、FP16/`__half`/`__half2`、BF16/`__nv_bfloat16`/BF16x2 |
| Host Runtime | 设备管理、内存分配/释放/拷贝/填充、流、事件、同步、错误处理 |
| 构建系统 | cucc 驱动、CMake(modern+legacy)、Make、Ninja、install/export |
| 官方样本 | NVIDIA vectorAdd/matrixMul 零改写 |
| 产物 | `.o/.d/.a/.so/ELF executable` |

### 勉强能做 (EXPERIMENTAL)

| 领域 | 具体能力 | 限制 |
|---|---|---|
| cuBLAS | `Sgemm`/`GemmEx`/`Sgemv` | 极小规模、有精度损失、需 ATC/OPP 模型 |
| `cudaMallocManaged` | fallback 到普通分配 | 无自动迁移 |
| `cudaMemAdvise/PrefetchAsync` | no-op success | 不执行迁移/预取 |
| `cudaMallocHost/HostAlloc` | pinned host memory | flag 语义有限 |
| `cudaMemcpy2DAsync` | row-wise async fallback | 语义/性能可能不同 |
| `cudaLaunchHostFunc` | ACL callback | 生命周期未验证 |
| `find_package(CUDAToolkit)` | bridge-backed | 不是完整 NVIDIA Toolkit |
| `CUDA::cudart` | bridge-backed target | 不代表完整 CUDA Runtime |
| 扩展 atomic | `Inc/Dec/CAS/And/Or/Xor` | opt-in，未验证 |
| 旧 `__shfl*` (无 _sync) | opt-in 映射 | 默认关闭 |

### 不能做 (UNSUPPORTED)

| 领域 | 具体能力 | 根本原因 |
|---|---|---|
| **Cube Core** | 矩阵加速引擎 | 只走 `dav-c310-vec` (Vector Core) |
| **Tensor Core** | `wmma::*`、`mma::*` | Ascend SIMT 无 warp-level matrix fragment |
| **cutlass** | 高性能 GEMM 模板库 | 依赖 Tensor Core + PTX + device-link |
| **cuDNN** | 深度学习算子库 | 无 bridge |
| **cuFFT** | FFT 库 | 无 bridge |
| **cuSPARSE** | 稀疏矩阵库 | 无 bridge |
| **cuRAND** | 随机数库 | 链接期 drop，符号仅 stub |
| **NCCL** | 通信库 | 无 HCCL alias |
| **Thrust/CCCL** | CUDA STL 算法库 | 未覆盖 |
| **device double** | FP64 数学/向量类型 | Ascend AICore 无 FP64 |
| **PTX/CUBIN/FATBIN** | 设备二进制 | puresimt 消费 source |
| **device-link/RDC** | separable compilation | 无 Ascend 等价 |
| **CUDA Graph** | 流捕获 | 返回 not-support |
| **Texture/Surface** | 纹理/表面对象 | 无 Ascend 等价 |
| **CUDA array** | 数组对象 | 无 Ascend 等价 |
| **Dynamic parallelism** | device-side launch | 无等价 |
| **Cooperative launch** | cooperative-grid | 无等价 |
| **`__threadfence_system`** | 系统级 fence | 无等价，编译报错 |
| **BLAS1** | axpy/copy/dot/scal/swap/nrm2 | 返回 not-support |
| **`cublasDgemm`** | FP64 GEMM | 无 FP64 |
| **多 .cu 聚合** | 多 TU static archive | ccec duplicate symbol |
| **Stream priority/ID** | 优先级/标识 | CANN 无对应语义 |
| **`cudaHostRegister`** | mapped host memory | 无统一 mapped-host 语义 |
| **Bazel/TF** | 构建系统集成 | 显式拒绝 |

---

## 5. 为什么有这些限制

### 5.1 架构决定（Ascend 硬件 / CANN 决定，桥不可修）

| 限制 | 根因 |
|---|---|
| 无 device FP64 | Ascend AICore 无 FP64 计算单元 |
| 只走 Vector Core | `--cce-aicore-arch=dav-c310-vec` 是 SIMT 唯一可用架构 |
| 多 .cu duplicate symbol | ccec 对每个 TU 生成相同注册符号 |
| PTX/CUBIN 不可用 | Ascend 不是 NVIDIA GPU，无 PTX 指令集 |
| 无 Tensor Core | Ascend 的矩阵加速在 Cube Core，SIMT 模型不覆盖 |

### 5.2 设计决定（桥主动选择不做）

| 限制 | 理由 |
|---|---|
| 不伪造 cuDNN/cuFFT/cuSPARSE | 没有 Ascend 等价 bridge，不建虚假 target |
| 不静默降 double 为 float | 精度损失必须由用户知情决定 |
| 不用 `--allow-multiple-definition` 修复多 .cu | 掩盖问题不如诚实记录 |
| 不把 mask 静默降为全 warp | 编译时保留 mask 求值，文档化差异 |
| 不做 Python kernel converter | 旧路线已退出 main；puresimt 直通更可靠 |

### 5.3 未覆盖（当前无人力/无需求，非技术不可能）

| 限制 | 说明 |
|---|---|
| Thrust/CCCL | 需要大量 device lambda/functor 适配，未排期 |
| cuDNN → CANN 算子库 | 需要逐算子映射，工作量巨大 |
| cutlass → Cube Core | 需要完全不同的编译路径（非 SIMT），超出项目定位 |
| NCCL → HCCL | 需要独立通信层 bridge |

---

## 6. 如果你的项目需要...

| 你的需求 | 能否满足 | 建议 |
|---|---|---|
| 把现有 `.cu` kernel 跑到 Ascend NPU 上 | **能** | puresimt 直通，零改写 |
| 用 `cudaMalloc/cudaMemcpy/cudaStream` 等 Runtime API | **能** | bridge 已覆盖核心 API |
| 用 CMake/Make 构建 CUDA 工程 | **能** | cmake_cce/make_cce/ninja_cce 已覆盖 |
| 跑 NVIDIA CUDA Samples | **部分能** | vectorAdd/matrixMul 零改写通过；依赖 Texture/Graph 的样本不支持 |
| 用 FP16/BF16 kernel 计算 | **能** | device 侧已验证 |
| 用 FP64/double kernel 计算 | **不能** | 硬件限制；需手动改为 float |
| 用 Tensor Core (`wmma`) 加速 GEMM | **不能** | 无 Tensor Core 等价；走 Vector Core 的手写 GEMM |
| 用 cutlass 做高性能 GEMM | **不能** | 需要重写为 Ascend Cube Core 算子或手写 Vector GEMM |
| 用 cuDNN 做卷积/池化 | **不能** | 需改用 CANN ACL 算子 API 或 Ascend C kernel |
| 用 cuFFT 做 FFT | **不能** | 需寻找 Ascend 侧 FFT 实现 |
| 用 Thrust 做排序/扫描 | **不能** | 需改写为手写 CUDA kernel（scan kernel 已有验证） |
| 用 NCCL 做多卡通信 | **不能** | 需改用 HCCL |
| 把多个 `.cu` 打包成一个静态库 | **不能** | ccec duplicate symbol；每库一个 `.cu` |
| 生成 PTX 或 device binary | **不能** | puresimt 消费 source，不产生 NVIDIA binary |
| 用 CUDA Graph 做流捕获 | **不能** | 返回 not-support |

---

## 7. 性能预期

本项目的首要目标是**功能兼容**（让 CUDA 代码跑起来），不是**性能等价**。

| 场景 | 性能预期 |
|---|---|
| Element-wise kernel (vectorAdd) | 接近 Ascend Vector Core 原生 SIMT 性能 |
| Reduction/Scan | 合理；走 Vector Core 的 atomic + shared memory |
| 手写 tiled GEMM (matrixMul) | **远慢于** Cube Core 原生 GEMM；只是功能验证 |
| 大规模 GEMM | **不建议**用本项目跑；应走 CANN ACL 的 `aclblasGemmEx` 或 Ascend C Cube 算子 |
| Memory-bound kernel | 取决于 Ascend HBM 带宽和 bridge 拷贝开销 |
| Warp shuffle/vote | 走 Ascend SIMT warp API，性能待基准测试 |

---

## 8. 术语速查

| 术语 | 含义 |
|---|---|
| puresimt | 当前正式路线：ccec 直接解析 CUDA SIMT，不做源码转换 |
| Vector Core | Ascend AI Core 的向量计算单元，SIMT 模型 |
| Cube Core | Ascend AI Core 的矩阵乘加速单元，本项目不覆盖 |
| SIMT | Single Instruction Multiple Thread，Ascend 的线程级并行模型 |
| ccec | CANN 的毕昇 C 编译器，`--enable-simt` 模式可解析 CUDA |
| dav-c310-vec | Ascend 950 的 Vector Core 架构标识 |
| bridge runtime | `libcce_bridge_runtime.so`，封装 `cuda* → wcuda* → aclrt*` |
| cucc | 本项目的编译驱动，参数解析后调用 ccec |
| UB | Unified Buffer，Ascend 的片上存储，`__shared__` 映射到此 |
