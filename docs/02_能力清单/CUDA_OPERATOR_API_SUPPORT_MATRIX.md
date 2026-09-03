# CUDA 算子、Kernel 写法、API 映射与构建能力支持矩阵

**更新时间：** 2026-09-02
**分支：** `main` / puresimt
**目标后端：** Ascend NPU / CANN / `ccec` SIMT
**对标：** `cu-bridge-muxi` `6ab1076` / `3.8.0`
**真机入口：** `tools/real_test/server_cmd.txt`

本文是当前项目的**正面能力与反面边界总表**。它回答：

1. 哪些 CUDA Kernel 写法和算子模式已经验证；
2. 哪些 CUDA Runtime/库 API 映射到什么 Ascend/bridge 路径；
3. cucc、CMake、Make、Ninja 现在能构建什么产物；
4. 哪些能力明确不支持，不能从“头文件有声明”推断为支持。

## 1. 状态定义和证据规则

| 状态 | 含义 |
|---|---|
| `SUPPORTED` | 当前声明的形态有 Ascend 真机编译/运行证据，并有结果校验；这里通常表示“验证过的形状”，不是整个 CUDA API 家族无限制兼容。 |
| `EXPERIMENTAL` | 有实现或有限真机证据，但数据类型、尺寸、性能、语义或环境依赖仍有限。 |
| `UNSUPPORTED` | 当前明确拒绝、返回 `ACL_ERROR_API_NOT_SUPPORT`，或者没有可验证的 Ascend 等价实现。 |
| `DIAGNOSTIC_ONLY` | 用于记录后端限制或问题证据，不计入主线 blocking PASS。 |
| `NOT_VERIFIED` | 代码可能有声明或转发，但当前没有足够证据，不能对用户承诺。 |

**证据优先级：** 当前 `server_cmd.txt` 的真机结果 > 对应示例的结果校验 > 单元/语法测试 > 头文件声明。
**重要：** “能 configure”“能编译”“头文件中存在函数声明”都不等于 `SUPPORTED`。

主链保持：

```text
CUDA .cu
  → cucc / cmake_cce / make_cce / ninja_cce
  → tools/cu2cce_driver.py
  → ccec -x asc --enable-simt --cce-aicore-arch=dav-*
  → libcce_bridge_runtime.so + CANN runtime
  → Ascend NPU
```

## 2. 已验证的 CUDA Kernel 写法和算子模式

以下是当前 puresimt full/fast 主线中有结果校验的载体。支持范围是表中对应的 CUDA 写法和测试形状，不代表任意规模、任意数据类型或任意 CUDA 库实现都自动支持。

### 2.1 真机 blocking operator/kernel 载体

| 示例 | 已验证的算子/Kernel 写法 | CUDA 关键语法/API | 类型与线程形态 | 状态 |
|---|---|---|---|---|
| `examples/puresimt/vectorAdd/vectorAdd.cu` | 向量加法 | `__global__`、`<<<>>>`、`cudaMalloc`、`cudaMemcpy` | FP32；1D grid/block；默认 stream | `SUPPORTED` |
| `examples/puresimt/matrixMul/matrixMul.cu` | tiled 矩阵乘法 | shared-memory tile、`__syncthreads`、event | FP32；2D grid/block | `SUPPORTED` |
| `examples/puresimt/vectorAdd_minimal.cu` | 多 block 向量加法 | global memory、同步 D2H | FP32；1D 多 block | `SUPPORTED` |
| `examples/puresimt/matrixMul_minimal.cu` | 简化 tiled GEMM | `__shared__`、`__syncthreads` | FP32；2D grid/block | `SUPPORTED` |
| `examples/puresimt/atomicAdd_minimal.cu` | 全局原子加/最小/最大 | `atomicAdd`、`atomicMin`、`atomicMax` | INT32；多 block 1D | `SUPPORTED` |
| `examples/puresimt/reduction_minimal.cu` | block tree reduction + 跨 block 累加 | shared reduction、`atomicAdd` | INT32；global + shared | `SUPPORTED` |
| `examples/puresimt/warpShuffle_minimal.cu` | warp 内 shuffle reduction | `__shfl_down_sync` | INT32；warp groups | `SUPPORTED` |
| `examples/puresimt/scan_minimal.cu` | block inclusive prefix scan | shared array、barrier | INT32；单 block 1D | `SUPPORTED` |
| `examples/puresimt/transpose_minimal.cu` | tiled matrix transpose | shared tile、barrier | FP32；2D grid/block | `SUPPORTED` |
| `examples/puresimt/vote_minimal.cu` | warp vote | `__all_sync`、`__any_sync`、`__ballot_sync` | INT32 predicate；多 warp | `SUPPORTED` |
| `examples/puresimt/dot_minimal.cu` | 点积 + 原子累加 | shared reduction、`atomicAdd` | FP32；多 block 1D | `SUPPORTED` |
| `examples/puresimt/stencil_minimal.cu` | 1D halo stencil | shared halo tile、邻域访问 | FP32；多 block 1D | `SUPPORTED` |
| `examples/puresimt/core_ops_minimal.cu` | 基础标量算子 | 四则、位逻辑、比较、选择、转换、load/store | INT32/FP32；local/shared/global | `SUPPORTED` |
| `examples/puresimt/math_minimal.cu` | 单精度数学 | `fmaf`、`sqrtf`、`sinf`、`cosf`、`expf`、`logf`、`floorf`、`ceilf` | FP32；单 block | `SUPPORTED` |
| `examples/puresimt/device_math_minimal.cu` | device helper 数学链 | 独立 `__device__` helper + kernel 调用 | FP32；与 host `libm` 对照 | `SUPPORTED`（验证形状） |
| `examples/puresimt/event_minimal.cu` | event 时序控制 | `cudaEventCreate/Record/Query/Synchronize/ElapsedTime` | 显式 stream | `SUPPORTED` |
| `examples/puresimt/pitchedMemcpy_minimal.cu` | pitch 分配与 2D copy | `cudaMallocPitch`、`cudaMemcpy2D` | FP32；2D index | `SUPPORTED`（验证形状） |
| `examples/puresimt/multiKernel_minimal.cu` | 同 stream 多 kernel pipeline | `cudaMemcpyAsync`、ordered kernel launches | INT32；显式 stream | `SUPPORTED`（验证形状） |
| `examples/puresimt/atomicVariants_minimal.cu` | 原子扩展 | `atomicSub`、`atomicExch`、shared `atomicAdd` | INT32；单 block | `SUPPORTED` |
| `examples/puresimt/dynamicShared_minimal.cu` | 动态 shared memory | `extern __shared__`、动态 launch bytes | INT32；单 block | `SUPPORTED`（验证形状） |
| `examples/puresimt/index3D_minimal.cu` | 3D 索引展开 | `threadIdx/blockIdx/blockDim/gridDim` 的 x/y/z | INT32；3D grid/block | `SUPPORTED` |
| `examples/puresimt/threadFence_minimal.cu` | 全局发布与 fence | `__threadfence`、原子发布标志 | INT32；global memory | `SUPPORTED` |
| `examples/puresimt/half_minimal.cu` | FP16/half2 转换 | `__half`、`__half2`、`__float2half`、half2 packing | FP16/half2 | `SUPPORTED`（验证形状） |
| `examples/puresimt/bf16x2_minimal.cu` | BF16/BF16x2 转换 | CUDA BF16 aliases/conversion | BF16/BF16x2 | `SUPPORTED`（验证形状） |

当前这些载体在服务器 full profile 中由 `server_cmd.txt` 编译、运行并匹配预期输出；具体日志位于：

```text
build/real_test/puresimt/
```

### 2.2 Kernel 语法支持矩阵

| CUDA 写法/模式 | Ascend 处理 | 当前状态 | 边界 |
|---|---|---|---|
| `__global__ void kernel(...)` | ccec puresimt 原生解析 | `SUPPORTED` | 以已验证参数/类型为准 |
| `kernel<<<grid, block>>>(...)` | ccec puresimt 原生 launch 语法 | `SUPPORTED` | 支持当前测试中的 1D/2D/3D 和可选 shared/stream 形态 |
| `threadIdx` / `blockIdx` / `blockDim` / `gridDim` | ccec/Ascend SIMT 内建 | `SUPPORTED` | 3D 展开已有测试 |
| `__shared__ T x[...]` | `simt_adaptor.h` 中 `__shared__ → __ubuf__` | `SUPPORTED` | 受 Ascend UB/shared 资源约束 |
| `extern __shared__ T x[]` | 动态 shared/UB 适配 | `SUPPORTED`（验证形状） | 不是 CUDA GPU shared-memory 容量承诺 |
| `__syncthreads()` | `asc_syncthreads()` | `SUPPORTED` | 需要满足 barrier 的线程收敛要求 |
| `__threadfence()` / `__threadfence_block()` | Ascend SIMT fence | `SUPPORTED`（验证形状） | 内存模型不宣称与 NVIDIA 全部细节字节级相同 |
| `atomicAdd/Sub/Min/Max/Exch` | `asc_atomic_*` | `SUPPORTED` | 当前结果校验覆盖 INT32 和部分 FP32 累加 |
| `__shfl_*_sync` | `asc_shfl*` | `SUPPORTED`（当前覆盖 `down`） | mask 在 Ascend 侧为隐式全 warp，复杂 mask 语义 `EXPERIMENTAL` |
| `__all_sync` / `__any_sync` / `__ballot_sync` | `asc_all/any/ballot` | `SUPPORTED`（验证形状） | mask 非 NVIDIA 全部语义等价 |
| 独立 `__device__` helper | 映射到 CANN execution-space selector | `SUPPORTED`（当前数学载体） | 只保证已验证的 helper 形状，不扩展为完整 device ABI 承诺 |
| `__host__` / host C++ | host 编译器/ccec 语言关键字 | `SUPPORTED` for tested mixed code | 不要用宏重定义覆盖 ccec 关键字 |
| CUDA `.cu.cc` 输入 | cucc 按 CUDA source 处理 | `SUPPORTED`（驱动规则） | 仍需使用 cucc，不应交给普通 host compiler |

### 2.3 当前明确不支持的 Kernel/Device 能力

| 能力 | 状态 | 原因/行为 |
|---|---|---|
| device-side `double` / `double2` 等 FP64 device 类型 | `UNSUPPORTED` | 当前 Ascend SIMT/AICore 路径无 device FP64；cucc 可在 ccec 前以 error/warn 策略报告，不能静默降精度 |
| PTX / CUBIN / FATBIN 生成 | `UNSUPPORTED` | puresimt 直接消费 CUDA source，不产生 NVIDIA device binary |
| CUDA device-link / RDC / separable compilation | `UNSUPPORTED` | 没有经过验证的 Ascend 等价路径；相关选项由 cucc reject 或按明确规则 drop |
| 多个 `.cu` 对象直接聚合到同一 static archive 后无条件链接 | `DIAGNOSTIC_ONLY` | ccec 会生成重复 `BishengLaunchKernelWithHostArgs` / eager-registration helper；由 `multi_cu_diagnostic` 取证，不用 `--allow-multiple-definition` 伪装支持 |
| `__simt_vf__`、`cce::async_invoke`、手写 `__global__[aicore]` 包装 | `UNSUPPORTED in main` | 这是退出 main 的旧 lowering/包装路线，不应写入用户 CUDA 源码 |
| CUDA dynamic parallelism、CUDA texture/surface device object、CUDA array/mipmap、UVM | `UNSUPPORTED/NOT_VERIFIED` | 当前没有稳定 Ascend 等价物；不要从普通 pointer/shared 测试外推 |

## 3. Host Runtime API 映射

### 3.0 类型与命名映射规则

| CUDA 类型/常量 | cu2cce/Ascend 表示 | 说明 |
|---|---|---|
| `cudaError_t` / `cudaSuccess` | `aclError` / `ACL_SUCCESS` | ACL 错误码不是 CUDA 完整错误码表；未知 CUDA 错误会落到明确的 ACL/failure 映射 |
| `cudaStream_t` | `aclrtStream` | stream handle 由 CANN runtime 管理 |
| `cudaEvent_t` | `aclrtEvent` | event 由 ACL event API 管理 |
| `cudaMemcpyKind` | `aclrtMemcpyKind` | 当前常用 H2H/H2D/D2H/D2D 顺序映射 |
| `cudaDeviceProp` | `_cceDeviceProp_t` | 提供 CUDA 形状的属性结构；数值是 Ascend compatibility values，不是 NVIDIA SM 属性 |
| `cudaCtx_t` | `wcceContext_t` / `aclrtContext` | 通过 bridge wrapper 管理 context |
| `cudaHostFn_t` | `wcceHostFn_t` | 仅用于 callback bridge 方向，完整 callback 生命周期仍是 experimental |

函数名的基本规则是：

```text
cudaXxx(...) → wcudaXxx(...) → aclrtXxx(...)（或 bridge 内部实现）
cublasXxx(...) → wcublasXxx(...) → aclblasXxx(...)（experimental BLAS）
```

### 3.1 `SUPPORTED`：当前主线或明确验证形状

| CUDA API/组 | 当前 bridge 符号 | Ascend/CANN 路径 | 状态/边界 |
|---|---|---|---|
| `cudaGetDeviceCount` / `cudaSetDevice` / `cudaGetDevice` / `cudaGetDeviceFlags` | `wcudaGetDeviceCount/SetDevice/GetDevice/GetDeviceFlags` | `aclrtGetDeviceCount`、`aclrtSetDevice`、bridge 兼容 flags | `SUPPORTED` for declared surface；flags 本身是兼容状态 |
| `cudaDeviceSynchronize` / `cudaDeviceReset` | `wcudaDeviceSynchronize/Reset` | ACL device sync/reset | `SUPPORTED`（当前样例） |
| `cudaGetDeviceProperties` / 常用 `cudaDeviceGetAttribute` | `wcudaGetDeviceProperties/Attribute` | `_cceDeviceProp_t`/兼容属性映射 | `SUPPORTED` for helper/sample queries；不是 NVIDIA SM 属性承诺 |
| `cudaMalloc` / `cudaFree` | `wcudaMalloc/Free` | `aclrtMalloc/Free` | `SUPPORTED` |
| `cudaMemcpy` / `cudaMemcpyAsync` | `wcudaMemcpy/Async` | `aclrtMemcpy/Async` | `SUPPORTED` |
| `cudaMemcpyDefault` | `wcudaMemcpyDefault` | CANN 没有公开的默认方向推断接口 | `UNSUPPORTED`；明确返回 `ACL_ERROR_API_NOT_SUPPORT`，不猜测为 D2D |
| `cudaMemset` / `cudaMemsetAsync` | `wcudaMemset/MemsetAsync` | ACL memset path | `SUPPORTED`（当前载体） |
| `cudaMallocPitch` | `wcudaMallocPitch` | bridge pitch-compatible allocation | `SUPPORTED` for current width/contiguous shape；对齐规则不是 CUDA GPU 承诺 |
| `cudaMemcpy2D` | `wcudaMemcpy2D` | row-wise `aclrtMemcpy` fallback | `SUPPORTED` for current pitched shape |
| `cudaStreamCreate` / `cudaStreamCreateWithFlags` | `wcudaStreamCreate*` | `aclrtCreateStream` | `SUPPORTED` for tested flags |
| `cudaStreamCreateWithPriority` / `cudaStreamGetPriority` | `wcudaStreamCreateWithPriority/GetPriority` | CANN `aclrtCreateStream` 无 CUDA priority 语义 | `UNSUPPORTED`；保留声明但不静默伪造优先级 |
| `cudaStreamGetFlags` / `cudaStreamGetId` | `wcudaStreamGetFlags/GetId` | CANN 无稳定公开等价查询 | `UNSUPPORTED` |
| `cudaStreamGetDevice` | `wcudaStreamGetDevice` | 当前 ACL context 的 device 查询 | `EXPERIMENTAL`；不承诺外部 stream 句柄可移植 |
| `cudaStreamAttachMemAsync` | `wcudaStreamAttachMemAsync` | CANN 无 managed-memory stream association | `UNSUPPORTED` |
| `cudaStreamDestroy` / `cudaStreamSynchronize` / `cudaStreamQuery` | `wcudaStream*` | ACL stream APIs | `SUPPORTED` for tested shapes |
| `cudaStreamWaitEvent` | `wcudaStreamWaitEvent` | ACL event/stream dependency | `SUPPORTED` for tested shape |
| `cudaEventCreate/Destroy/Record/Query/Synchronize/ElapsedTime` | `wcudaEvent*` | ACL event APIs | `SUPPORTED` |
| `cudaGetLastError` / `cudaPeekAtLastError` | `wcudaGetLastError/PeekAtLastError` | bridge error state | `SUPPORTED` for launch/error checks |
| `cudaGetErrorString` / `cudaGetErrorName` | `wcudaGetErrorString/Name` | ACL error/fallback mapping | `EXPERIMENTAL`；不是完整 CUDA error name table |
| `cudaDeviceCanAccessPeer` / peer access | `wcudaDevice*Peer*` | ACL peer API where available | `NOT_VERIFIED` outside declared wrapper shape |
| CUDA launch syntax | ccec native launch + bridge runtime | puresimt launch/registration path | `SUPPORTED`; explicit `cudaLaunchKernel` 另见限制 |

### 3.1.1 内存 API 的细分边界

| CUDA API/组 | 当前实现 | 状态 |
|---|---|---|
| `cudaMemcpy3D` / `cudaMemcpy3DAsync` | 明确返回 `ACL_ERROR_API_NOT_SUPPORT` | `UNSUPPORTED` | Ascend bridge 当前没有对应 CUDA 3D 参数结构路径 |
| `cudaMemcpyPeer` / `cudaMemcpyPeerAsync` | 简化为 device-to-device ACL copy | `EXPERIMENTAL` | 忽略 CUDA device id 的完整跨设备语义，假定 peer access 已准备 |
| `cudaMallocAsync` / `cudaFreeAsync` | fallback 到普通 `aclrtMalloc/Free` | `EXPERIMENTAL` | 没有 CUDA stream-ordered allocator/pool 复用语义 |
| `cudaMemPool*` / `cudaDeviceGet/SetMemPool` | 明确返回 not-support | `UNSUPPORTED` | CANN 当前没有已验证对应 mempool surface |
| `cudaMallocManaged` | fallback 到普通 device allocation | `EXPERIMENTAL` | 没有 unified-memory 自动迁移；依赖迁移的程序不能直接承诺 |
| `cudaMemAdvise` / `cudaMemPrefetchAsync` | no-op 返回 success | `EXPERIMENTAL` | 成功只表示兼容调用，不表示执行了迁移/预取 |
| `cudaPointerGetAttributes` | 明确返回 not-support | `UNSUPPORTED` | 没有当前 bridge-backed CUDA pointer attribute 结构 |
| `cudaMemGetInfo` | `aclrtGetMemInfo(ACL_HBM_MEM, ...)` | `EXPERIMENTAL` | 返回 Ascend HBM 视图，不是 NVIDIA memory model |
| `cudaMallocHost` / `cudaFreeHost` | `aclrtMallocHost/FreeHost` | `EXPERIMENTAL` | pinned host memory 形态已覆盖，flags/映射语义有限 |
| `cudaHostAlloc` | 转发到 pinned host allocation | `EXPERIMENTAL` | `Mapped`/`WriteCombined` 等 flag 不保证 CUDA 等价 |
| `cudaHostRegister` / `cudaHostUnregister` / `cudaHostGetDevicePointer` / `cudaHostGetFlags` | 返回 not-support | `UNSUPPORTED` | Ascend 没有当前统一 mapped-host 语义 |

### 3.2 `EXPERIMENTAL`：实现存在但不能扩大承诺

| CUDA API/组 | 当前映射/行为 | 限制 |
|---|---|---|
| `cudaMallocHost` / `cudaFreeHost` | pinned host allocation bridge | 仅当前样例和常见 typed overload 形状有证据 |
| `cudaHostAlloc` | host allocation bridge | flag 语义不是完整 CUDA mapped/pinned 等价 |
| `cudaMemcpy2DAsync` | row-wise async fallback | 语义/性能需按具体场景确认 |
| `cudaLaunchHostFunc` | `aclrtLaunchCallback` 方向 | callback 生命周期和并发语义需单独验证 |
| `cudaMemAdvise` / `cudaMemPrefetchAsync` | no-op/fallback | Ascend 没有 CUDA unified-memory 迁移语义 |
| `cudaProfilerStart/Stop` | no-op compatibility | 实际 profiling 应使用 CANN `aclprof` |
| 设备 cache/shared-memory config 查询/设置 | compatibility/default/not-support | Ascend 没有完全对应的 CUDA GPU 调度配置 |
| `cudaRuntimeGetVersion` / `cudaDriverGetVersion` | compatibility values | 版本值不能理解为 NVIDIA driver ABI |

### 3.3 `UNSUPPORTED`：明确不承诺

| CUDA API/组 | 当前行为/原因 |
|---|---|
| CUDA Graph API 与 stream capture | wrapper 返回 `ACL_ERROR_API_NOT_SUPPORT`；Ascend 没有已验证 CUDA Graph 等价物 |
| Texture/Surface object 与 texture reference | wrapper 保留声明但返回 not-support/无等价对象 |
| CUDA array / 3D array / mipmapped array | 当前 wrapper 不提供可承诺的 Ascend array 语义 |
| External memory import/export | Ascend 侧无当前桥接等价物 |
| `cudaLaunchKernel` 显式函数入口 | main 主线使用 `<<<>>>`；显式入口当前返回 not-support |
| `cudaHostRegister` / `cudaHostUnregister` / `cudaHostGetDevicePointer` / `cudaHostGetFlags` | 返回 `ACL_ERROR_API_NOT_SUPPORT`，不做 no-op success |
| 完整 CUDA Driver API/module loading | 当前项目是 Runtime + puresimt bridge，不是完整 CUDA driver replacement |

## 4. CUDA 库与 BLAS 映射

### 4.1 Runtime 和已验证 bridge 库

| CUDA 名称 | cu2cce 处理 | 状态 |
|---|---|---|
| `cudart` / `cuda` 常规 runtime 依赖 | `-lcudart` 被规则处理，注入 `libcce_bridge_runtime.so`；shim 提供 CUDA-shaped alias 供兼容构建发现 | `SUPPORTED` for tested bridge path |
| `CUDA::cudart` | `FindCUDAToolkit.cmake` 的 imported target 指向 bridge runtime，并按存在性传播 CANN libs | `SUPPORTED` for tested scope |
| `cu2cce::runtime` | 原生 CMake package target | `EXPERIMENTAL` package surface；只表示 bridge runtime |
| `nvToolsExt` / `nvtx` header surface | 提供有限 stub/header | `EXPERIMENTAL`；不等同完整 NVIDIA profiler |

### 4.2 Experimental BLAS

| CUDA API | Ascend 路径 | 数据类型/形状 | 状态 |
|---|---|---|---|
| `cublasCreate/Destroy` | custom handle | n/a | `EXPERIMENTAL` |
| `cublasSet/GetStream` | handle 保存 `aclrtStream` | n/a | `EXPERIMENTAL` |
| `cublasSet/GetPointerMode` | handle mode；HOST scalar 可 staging 到 device | HOST/DEVICE | `EXPERIMENTAL` |
| `cublasSet/GetMathMode`、AtomicsMode | 保存/返回 compatibility mode | n/a | `EXPERIMENTAL`，不改变数值路径 |
| `cublasSgemm` | `aclblasGemmEx` | FP32 输入 staging 到 FP16，32×32×32 验证 | `EXPERIMENTAL`；有 FP32→FP16 精度损失 |
| `cublasGemmEx` / `cublasSgemmEx` | `aclblasGemmEx` | FP16×FP16→FP32；另有 INT8 家族规则 | `EXPERIMENTAL`；形状/dtype 窄 |
| `cublasSgemv` | `aclblasGemvEx` | FP32，8×8 identity，`lda/incx/incy=-1` 保留参数约束 | `SUPPORTED` for validated shape |
| `cublasDgemm` | 无可用 FP64 CANN 路径 | FP64 | `UNSUPPORTED` |
| BLAS1/BLAS2：`Saxpy/Scopy/Sdot/Sscal/Sswap/Snrm2` | placeholder + full parameter validation | FP32 | `UNSUPPORTED` |

详细 BLAS 证据以 [`BLAS_COMPATIBILITY_MATRIX.md`](BLAS_COMPATIBILITY_MATRIX.md) 为准；
运行 optional BLAS 回归：

```bash
CU2CCE_BLAS_TEST=1 bash tools/real_test/server_cmd.txt
```

### 4.3 未验证/不自动伪造的 CUDA 库

| CUDA 库/target | 状态 | 处理原则 |
|---|---|---|
| `cublasLt` | `EXPERIMENTAL`/`NOT_VERIFIED` | 只有已验证 wrapper 才可使用；不因 CUDA target 名称存在就自动链接 |
| `cufft/cufftw` | `UNSUPPORTED` | 可给出 CANN FFT `map_to` 诊断，但不创建虚假 target |
| `cusparse` | `UNSUPPORTED` | map_to CANN sparse operators；未实现 1:1 shim |
| `curand` | `UNSUPPORTED` | 未实现 CUDA device random API 等价层 |
| `cudnn` | `UNSUPPORTED` | 不伪造 ATB/CANN target |
| `nccl` | `UNSUPPORTED`/map available | 记录 HCCL 路线，但未实现即 warning/reject，不假称 NCCL 等价 |
| `nvrtc`、OpenCL、PTX 工具链 | `UNSUPPORTED` | cucc 明确 drop/reject |
| NPP、nvJPEG、cuFile、CUPTI、NVML、nvperf | `UNSUPPORTED`/diagnostic map | 只记录 CANN 映射方向，不自动替换 CUDA 库名 |

## 5. cucc 命令行与产物

### 5.1 当前支持的直接命令形态

```bash
# CUDA source -> ELF relocatable object
cucc -c kernel.cu -o build/kernel.o

# CUDA source -> object + Make dependency file
cucc -c kernel.cu -MD -MF build/kernel.d -o build/kernel.o

# object -> ELF executable
cucc build/kernel.o -o build/app

# CUDA source -> ELF executable（推荐大型工程使用两步法）
cucc kernel.cu -o build/app

# CUDA source -> ELF shared object
cucc -shared -fPIC kernel.cu -o build/libkernel.so
```

支持的产物契约：

```text
.o       标准 ELF relocatable object
.d       Make 风格依赖规则
.a       标准 archive（由 CMake/Make 归档）
.so      ELF shared object
app      ELF executable
```

链接阶段由规则注入 bridge/CANN runtime 和 RPATH；不生成 `.cce`、`.ptx`、`.cubin`、`.fatbin`、`.maca`。

### 5.2 cucc 参数/策略边界

| 形态 | 状态 | 说明 |
|---|---|---|
| `-I/-D/-U/-O/-std/-fPIC` | `SUPPORTED` for tested forms | 普通编译参数透传 |
| `-MD/-MF/-MT` | `SUPPORTED` for tested dependency shape | `.d` 由真实 ccec 编译链生成 |
| `--options-file` / `@response-file` | `SUPPORTED` | 使用安全 `shlex` 展开，支持递归深度限制 |
| `-E`、toolchain query | `SUPPORTED` for mechanism | query 直通 ccec，不注入 SIMT/runtime；输出与 NVIDIA query 不宣称完全一致 |
| `-arch/-gencode/-code=sm_*` | `DIAGNOSTIC_ONLY` | drop + warning；可选 `CU2CCE_ARCH_MACRO=auto` 只生成兼容宏 |
| `CU2CCE_NPU_ARCH` / `CUCC_NPU_ARCH` / 单一 `CUCC_TARGETS=dav-*` | `SUPPORTED` | 选择 Ascend `dav-*`，不把 `sm_*` 转为 ccec target |
| `--use_fast_math` | `SUPPORTED` for mapped spelling | 映射为 ccec/clang `-ffast-math`，这是语义变更请求 |
| `--ptx/-cubin/-fatbin` | `UNSUPPORTED` | 明确 reject |
| `-rdc/-dc/-dlink` | `UNSUPPORTED`/diagnostic drop | 当前无 Ascend device-link 等价物；CMake 强制 separable off |
| `WCUDA_BAZEL_TF_BUILD=1` | `UNSUPPORTED` | Ascend 路径明确拒绝 |

完整规则以 [`CUCC_OPTION_COMPATIBILITY_MATRIX.md`](CUCC_OPTION_COMPATIBILITY_MATRIX.md) 为准。

## 6. CMake / Make / Ninja 支持能力

### 6.1 CMake

| 能力 | 状态 | 证据/产物 |
|---|---|---|
| `project(... LANGUAGES CXX CUDA)` | `SUPPORTED` | `examples/cmake_demo`；`.o` + ELF executable |
| CMake Unix Makefiles generator | `SUPPORTED` for tested Linux scope | `cmake_cce --build` → `make_cce` |
| CMake Ninja generator | `SUPPORTED` for tested Linux scope | `build.ninja` + `CMakeFiles/rules.ninja` + 同类 ELF/archives |
| 多 `.cu` 分属不同 static/shared target | `SUPPORTED` for tested graph | `examples/cmake_compat`；每个 linked library 一个 `.cu` |
| static/shared/executable 混合 target | `SUPPORTED` | `.a`、`.so`、ELF executable |
| `find_package(CUDAToolkit)` + `CUDA::cudart` | `SUPPORTED` for bridge scope | bridge runtime target；不是完整 toolkit |
| legacy `find_package(CUDA)` + `cuda_add_executable/library` | `SUPPORTED` for basic macro scope | `examples/cmake_compat_legacy` |
| `cuda_compile` / `cuda_wrap_srcs` common forms | `SUPPORTED` for compatibility subset | 普通 CUDA object path；PTX/CUBIN/FATBIN 明确失败 |
| install/export target graph | `SUPPORTED` for tested examples | `largeProjectTargets.cmake`、`buildCompatTargets.cmake` |
| `CUDA::cublas/cufft/...` 全量 target | `UNSUPPORTED`/`EXPERIMENTAL` | 只有已验证能力才创建 target |
| CUDA separable compilation/device-link | `UNSUPPORTED` | 不生成 NVIDIA device artifacts |

推荐流程：

```bash
cmake_cce -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CUDA_ARCHITECTURES=OFF
cmake_cce --build build --parallel
./build/<app>
```

Ninja 工程：

```bash
cmake_cce -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CUDA_ARCHITECTURES=OFF
cmake_cce --build build --parallel
# 或直接使用
ninja_cce -C build
```

### 6.2 Make

| 能力 | 状态 | 证据/边界 |
|---|---|---|
| Makefile 中写 `nvcc` | `SUPPORTED` for tested forms | `make_cce` virtual CUDA toolkit 将 `nvcc → cucc → ccec` |
| `.cu → .o`、`.d` | `SUPPORTED` | `examples/build_compat/make` |
| `ar` static archive `.a` | `SUPPORTED` | `build/libkernel_a.a` |
| shared `.so` | `SUPPORTED` for tested one-`.cu` library | `build/libkernel_b.so` |
| host C++ + CUDA object link | `SUPPORTED` for tested graph | `build/build_compat_app` |
| plain g++ + `-lcudart` carrier | `SUPPORTED` for tested bridge alias | `examples/build_compat/make_gxx_link` |
| `make clean` | `SUPPORTED` | 清理用户产物和 shim |
| `make cleanpack` | `SUPPORTED` | 不因 token 子串误删 shim |
| 多 `.cu` 放进同一 static archive 后无条件链接 | `DIAGNOSTIC_ONLY` | 受 ccec registration helper 重复限制 |

```bash
make_cce -C . -j2
./build/app
make_cce -C . clean
```

### 6.3 Ninja

| 能力 | 状态 | 证据/边界 |
|---|---|---|
| 直接 `ninja_cce -C build` | `SUPPORTED` for tested CMake tree | 恢复/重建同一 CUDA 虚拟工具链目录 后执行系统 Ninja |
| `-jN` / `-v` / `-t clean` | `SUPPORTED` for tested forms | argv 原样传给 Ninja |
| `cmake_cce --build` 自动选择 Ninja | `SUPPORTED` | 检测 `build.ninja` 后 dispatch `ninja_cce` |
| 旧 CMake build tree 增量构建 | `SUPPORTED` for recorded shim | 读取 `CMakeCache.txt` 中 `CUDA_DIR/bin/nvcc` |
| Ninja Multi-Config `--config` | `SUPPORTED` for `build-<config>.ninja` shape | wrapper 选择对应 build file |
| 生成/消费 PTX/CUBIN/FATBIN | `UNSUPPORTED` | Ascend puresimt 不生成 NVIDIA device artifact |

## 7. 构建产物与证据位置

| 构建层 | 典型产物 | 当前检查 |
|---|---|---|
| 直接 cucc | `.o/.d/.so/ELF` | `config/artifact_contract.json` 的 `cucc_direct` |
| CMake Makefiles | `.o/.a/.so/ELF`、`build.make`、`link.txt` | `cmake_*` contract |
| CMake Ninja | `.o/.a/.so/ELF`、`build.ninja`、`rules.ninja` | `cmake_ninja` contract |
| Make | `.o/.d/.a/.so/ELF` | `make_compat` / `make_gxx_link` |
| install/export | `lib/cmake/*/*Targets.cmake`、bridge runtime、headers、入口脚本 | install contract |

真机 artifact 日志：

```text
build/real_test/puresimt/artifacts/<case>.artifact.log
```

标准检查命令：

```bash
python3 tools/real_test/verify_artifacts.py \
    --contract config/artifact_contract.json \
    --case cucc_direct \
    --root build/real_test/puresimt \
    --log /tmp/cucc_direct.artifact.log
```

## 8. 不要误判的项目边界

- 本项目是 CUDA Runtime + SIMT source compatibility bridge，不是完整 NVIDIA CUDA Toolkit，也不是完整 CUDA Driver replacement。
- `CUDA::cudart` 只表示 `libcce_bridge_runtime.so` 和存在的 CANN 依赖；不能据此推断 `CUDA::cublas/cufft/nccl` 都可用。
- 当前 kernel 支持来自明确的真机载体，不能由 `vectorAdd` 通过外推到任意复杂 CUDA template、cooperative group、dynamic parallelism 或库调用。
- Ascend 的 device architecture 是 `dav-*`；`sm_*` 只作兼容宏/诊断，不能传给 ccec。
- device-side double、NVIDIA device artifacts、RDC/device-link 和多 `.cu` archive 聚合限制是设备/编译器事实，不通过 wrapper “绕过”。
- `DIAGNOSTIC_ONLY` 结果用于帮助定位昇腾后端问题，不计入 blocking PASS，也不应写成支持承诺。

## 9. 维护与更新规则

新增一个“支持”条目必须同时具备：

1. 用户源码形态和最小示例；
2. 明确输入/输出结果校验；
3. `server_cmd.txt` 真机证据或明确的 `DIAGNOSTIC_ONLY` 证据；
4. 对应代码/测试/矩阵同步更新。

修改 API 映射时，至少同步：

```text
include/bridge/runtime/cuda_to_cce_mcr_adaptor.h
include/bridge/runtime/cuda_runtime_wrapper.h
src/bridge/runtime/src/cuda_runtime_wrapper.cpp
docs/02_能力清单/PURESIMT_COMPATIBILITY_MATRIX.md
docs/02_能力清单/BLAS_COMPATIBILITY_MATRIX.md
```

相关入口：

- [`PURESIMT_COMPATIBILITY_MATRIX.md`](PURESIMT_COMPATIBILITY_MATRIX.md)：逐用例与 Runtime 证据；
- [`BLAS_COMPATIBILITY_MATRIX.md`](BLAS_COMPATIBILITY_MATRIX.md)：experimental BLAS 细节；
- [`CUCC_OPTION_COMPATIBILITY_MATRIX.md`](CUCC_OPTION_COMPATIBILITY_MATRIX.md)：cucc 参数规则；
- [`BUILD_SYSTEM_COMPATIBILITY_MATRIX.md`](BUILD_SYSTEM_COMPATIBILITY_MATRIX.md)：CMake/Make/Ninja 机制；
- [`docs/03_文件目录/REPOSITORY_FILE_GUIDE.md`](../03_文件目录/REPOSITORY_FILE_GUIDE.md)：文件定位和维护地图。

## 8. CUDA → Ascend/CANN API 映射校准（2026-09-03）

### 8.1 Runtime 映射

| CUDA API 类别 | CANN 头文件/API | CANN 库 | bridge 入口 | 映射类型 | 状态 |
|---|---|---|---|---|---|
| Device 初始化/枚举 | `acl/acl_rt.h`：`aclInit`、`aclrtGetDeviceCount` | `libacl_rt.so` | `cudaGetDeviceCount`、`cudaSetDevice` | `ADAPTED` | `SUPPORTED` |
| Context | `acl/acl_rt.h`：`aclrtCreateContext`、`aclrtSetCurrentContext` | `libacl_rt.so` | `cudaCtx*`/内部上下文 | `ADAPTED` | `EXPERIMENTAL` |
| Stream | `acl/acl_rt.h`：`aclrtCreateStream`、`aclrtSynchronizeStream` | `libacl_rt.so` | `cudaStream*` | `ADAPTED` | `SUPPORTED` |
| Event | `acl/acl_rt.h`：`aclrtCreateEvent`、`aclrtRecordEvent` | `libacl_rt.so` | `cudaEvent*` | `ADAPTED` | `SUPPORTED` |
| Device 内存 | `acl/acl_rt.h`：`aclrtMalloc`、`aclrtFree` | `libacl_rt.so` | `cudaMalloc`、`cudaFree` | `ADAPTED` | `SUPPORTED` |
| 同步拷贝 | `acl/acl_rt.h`：`aclrtMemcpy` | `libacl_rt.so` | `cudaMemcpy` | `ADAPTED` | `SUPPORTED` |
| 异步拷贝 | `acl/acl_rt.h`：`aclrtMemcpyAsync` | `libacl_rt.so` | `cudaMemcpyAsync` | `ADAPTED` | `SUPPORTED` |
| 2D/pitch 拷贝 | `acl/acl_rt.h` 或逐行 fallback | `libacl_rt.so` | `cudaMemcpy2D*` | `ADAPTED` | `SUPPORTED` for tested shape |
| Host 注册/映射 | 无稳定一一对应 | - | `cudaHostRegister` 等 | `REJECT` | `UNSUPPORTED` |
| CUDA Graph | 无 CUDA Graph 一一对应 | - | `cudaGraph*` | `REJECT` | `UNSUPPORTED` |
| Texture/Surface | 无稳定一一对应 | - | `cudaTexture*`/`cudaSurface*` | `REJECT` | `UNSUPPORTED` |

### 8.2 官方库路线与当前承诺

| CUDA 库 | Ascend/CANN 路线 | 当前状态 | 说明 |
|---|---|---|---|
| `cudart` | bridge runtime + `libacl_rt.so` | `SUPPORTED` for tested scope | 不是完整 NVIDIA cudart |
| `cublas` | `acl_cblas.h` + `libacl_cblas.so` | `EXPERIMENTAL` | 仅已验证数据类型/形状 |
| `nccl` | `hccl/hccl.h` + `libhccl.so` | `ROUTE_ONLY` | 需要独立 HCCL wrapper 和多卡验证 |
| `cufft` | SiP/FFT/aclnn | `ROUTE_ONLY` | 没有当前 1:1 shim |
| `cusparse` | CANN sparse/aclnn | `NOT_VERIFIED` | 不自动链接 |
| `curand` | Ascend C/应用侧随机数 | `UNSUPPORTED` | 没有当前安全等价层 |
| `cudnn` | aclnn/ATB | `ROUTE_ONLY` | 不生成虚假 CUDA target |
| `nvrtc`、PTX、CUBIN、FATBIN | 无 puresimt 等价物 | `UNSUPPORTED` | cucc 明确拒绝 |

### 8.3 判定要求

头文件存在、CMake configure 成功、函数返回成功或 no-op 均不能单独升级为 `SUPPORTED`。每项必须关联服务器探针或 `tools/real_test/server_cmd.txt` 中的真实结果。
