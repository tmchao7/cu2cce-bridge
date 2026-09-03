# CUDA → Ascend puresimt 兼容性矩阵

更新时间：2026-09-02
分支：`main`
目标设备：Ascend 950 / CANN 9.2.0 / `Ascend950DT_9582`
唯一真机入口：`tools/real_test/server_cmd.txt`
算子、Kernel 写法、API 映射和 CMake/Make/Ninja 总表：
`CUDA_OPERATOR_API_SUPPORT_MATRIX.md`

## 1. 评审结论

| 项目 | main 当前结论 |
|---|---|
| 用户源码 | 原始 CUDA `.cu`，不加入 ACL、Ascend C 或 bridge 专用 kernel 包装 |
| 编译前端 | CANN `ccec` 直接解析 CUDA SIMT |
| 编译参数 | `-x asc --enable-simt --cce-aicore-arch=dav-c310-vec` |
| Host runtime | `cuda* → wcuda* → aclrt*` |
| Kernel lowering | main 不使用 Python converter，不生成中间 `.cce`/`__simt_vf__` wrapper |
| 真机验收 | 24 个 kernel/operator 用例、`cucc` 直通检查、`PURESIMT_REGRESSION_PASS` |
| BLAS | `cublas→aclblas` 保留为 experimental secondary capability，不阻塞 puresimt |

### 状态定义

#### `SUPPORTED`

同时满足：

- CUDA 源码无需修改；
- 通过 main puresimt 编译链；
- Ascend 真机运行通过；
- 有明确结果校验；
- 不是只返回成功的 no-op。

#### `EXPERIMENTAL`

表示实现存在但证据或语义仍有限，例如：

- 只有编译/声明证据，没有当前真机回归；
- 依赖特定 OPP、ATC 模型或数据类型；
- 语义与 CUDA 有明确差异；
- 属于 cublas/aclblas secondary capability；
- 是成功返回的 fallback/no-op。

#### `UNSUPPORTED`

表示当前明确无对应后端能力、实现返回 `ACL_ERROR_API_NOT_SUPPORT`，或仅有占位
声明，不能承诺 CUDA 语义。

> 编译可通过 ≠ `SUPPORTED`；头文件有声明 ≠ `SUPPORTED`；返回 `ACL_SUCCESS` 的
> no-op ≠ `SUPPORTED`。

## 2. 24 个真机验证用例

证据来源统一为：

```text
bash tools/real_test/server_cmd.txt
```

当前服务器 full profile 由维护者报告 `FAIL=0`；本表的历史逐用例基线为 2026-08-24。

| # | 测试用例 | 类别 | CUDA API / 语义 | 数据类型 | 内存模型 | 线程模型 | 预期输出 | 状态 |
|---:|---|---|---|---|---|---|---|---|
| 1 | `vectorAdd/vectorAdd.cu` | 官方样例/向量 | `__global__`, `<<<>>>`, `cudaMalloc`, `cudaMemcpy` | FP32 | global | 1D grid/block、默认 stream | `Test PASSED` | `SUPPORTED` |
| 2 | `matrixMul/matrixMul.cu` | 官方样例/矩阵 | tiled GEMM kernel、shared memory、event | FP32 | global + shared | 2D grid/block | `Result = PASS` | `SUPPORTED` |
| 3 | `vectorAdd_minimal.cu` | 向量/大 grid | kernel、同步 D2H copy | FP32 | global | 多 block 1D | `vectorAdd_minimal OK` | `SUPPORTED` |
| 4 | `matrixMul_minimal.cu` | 矩阵/shared | tiled multiply、`__syncthreads` | FP32 | global + shared | 2D grid/block | `matrixMul_minimal OK` | `SUPPORTED` |
| 5 | `atomicAdd_minimal.cu` | 原子 | `atomicAdd`, `atomicMin`, `atomicMax` | INT32 | global | 多 block 1D | `atomicAdd_minimal OK` | `SUPPORTED` |
| 6 | `reduction_minimal.cu` | 归约 | tree reduction + cross-block `atomicAdd` | INT32 | global + shared | 多 block 1D | `reduction_minimal OK` | `SUPPORTED` |
| 7 | `warpShuffle_minimal.cu` | Warp | `__shfl_down_sync` | INT32 | global | 1D warp groups | `warpShuffle_minimal OK` | `SUPPORTED` |
| 8 | `scan_minimal.cu` | Scan | inclusive prefix scan + barriers | INT32 | global + shared | 单 block 1D | `scan_minimal OK` | `SUPPORTED` |
| 9 | `transpose_minimal.cu` | 转置 | tiled transpose + barrier | FP32 | global + shared | 2D grid/block | `transpose_minimal OK` | `SUPPORTED` |
| 10 | `vote_minimal.cu` | Warp vote | `__all_sync`, `__any_sync`, `__ballot_sync` | INT32 predicate | global | 单 block、多 warp | `vote_minimal OK` | `SUPPORTED` |
| 11 | `dot_minimal.cu` | 归约/线代 | dot product + atomic accumulation | FP32 | global + shared | 多 block 1D | `dot_minimal OK` | `SUPPORTED` |
| 12 | `stencil_minimal.cu` | 邻域算子 | halo load + shared tile | FP32 | global + shared | 多 block 1D | `stencil_minimal OK` | `SUPPORTED` |
| 13 | `core_ops_minimal.cu` | 基础完备性 | 四则、位逻辑、比较、选择、转换、load/store | INT32/FP32 | global + shared + local | 单 block 1D | `core_ops_minimal OK` | `SUPPORTED` |
| 14 | `math_minimal.cu` | 数学 | `fmaf`, `sqrtf`, `sinf`, `cosf`, `expf`, `logf`, `floorf`, `ceilf` | FP32 | global | 单 block 1D | `math_minimal OK` | `SUPPORTED` |
| 15 | `device_math_minimal.cu` | device 数学 | 独立 `__device__` helper + `expf/logf/sqrtf/sinf/cosf/tanhf/powf/fmin/fmax/fabsf` | FP32 | global | 单 block 1D | `device_math_minimal OK` | `SUPPORTED`（验证形状） |
| 16 | `event_minimal.cu` | Runtime/event | event create/record/query/sync/elapsed | FP32 | global | 显式 stream | `event_minimal OK` | `SUPPORTED` |
| 17 | `pitchedMemcpy_minimal.cu` | Pitch/2D copy | `cudaMallocPitch`, `cudaMemcpy2D` | FP32 | pitched global | 2D grid/block | `pitchedMemcpy_minimal OK` | `SUPPORTED` |
| 18 | `multiKernel_minimal.cu` | Pipeline | async H2D/D2H + ordered kernels | INT32 | global | 显式 stream | `multiKernel_minimal OK` | `SUPPORTED` |
| 19 | `atomicVariants_minimal.cu` | 原子扩展 | `atomicSub`, `atomicExch`, shared `atomicAdd` | INT32 | global + shared | 单 block 1D | `atomicVariants_minimal OK` | `SUPPORTED` |
| 20 | `dynamicShared_minimal.cu` | 动态 shared | `extern __shared__`, dynamic shared bytes | INT32 | global + dynamic shared | 单 block 1D | `dynamicShared_minimal OK` | `SUPPORTED` |
| 21 | `index3D_minimal.cu` | 索引 | x/y/z flattening | INT32 | global | 3D grid/block | `index3D_minimal OK` | `SUPPORTED` |
| 22 | `threadFence_minimal.cu` | 内存顺序 | `__threadfence` + atomic publication | INT32 | global | 单 block 1D | `threadFence_minimal OK` | `SUPPORTED` |
| 23 | `half_minimal.cu` | 精度类型 | `__half`, `__half2`, conversion round-trip | FP16 | register/local + global output | 单 block 1D | `half_minimal OK` | `SUPPORTED` |
| 24 | `bf16x2_minimal.cu` | 精度类型 | `__nv_bfloat16`, BF16x2 conversion | BF16/BF16x2 | register/local + global output | 单 block 1D | `bf16x2_minimal OK` | `SUPPORTED` |

## 3. 按语义维度汇总

| 维度 | 已验证内容 | 状态 |
|---|---|---|
| Kernel 入口 | `__global__`, CUDA `<<<>>>` | `SUPPORTED` |
| 线程索引 | 1D/2D/3D `threadIdx/blockIdx/blockDim/gridDim` | `SUPPORTED` |
| 内存 | global、local、静态 shared、dynamic shared、pitched | `SUPPORTED` |
| 同步 | `__syncthreads`, `__threadfence`, stream/event sync | `SUPPORTED` |
| 原子 | add/sub/min/max/exch | `SUPPORTED` |
| Warp | shuffle、all/any/ballot | `SUPPORTED` |
| 算术 | int/float 基础算术、FMA、数学函数 | `SUPPORTED` |
| 类型 | INT32、FP32、FP16/half2、BF16/BF16x2 | `SUPPORTED` |
| Pipeline | async copy、同 stream 多 kernel 顺序 | `SUPPORTED` |

## 4. Host Runtime API 映射矩阵

以下状态按“当前真机证据”判定，不等同于头文件声明数量。

### 4.1 `SUPPORTED`：已有真机语义证据

| CUDA API/组 | Bridge 路径 | 证据 |
|---|---|---|
| `cudaGetDeviceCount`, `cudaSetDevice`, `cudaGetDevice` | `wcuda* → aclrt*` | 官方 helper、初始化和全量回归 |
| `cudaMalloc`, `cudaFree` | `wcudaMalloc/Free → aclrtMalloc/Free` | 23 个用例 |
| `cudaMemcpy`, `cudaMemcpyAsync` | `wcudaMemcpy* → aclrtMemcpy*` | vector/pipeline 回归 |
| `cudaMemcpy2D` | row-wise `aclrtMemcpy` fallback | pitched 回归 |
| `cudaMallocPitch` | pitch-compatible `aclrtMalloc` | pitched 回归 |
| `cudaMemset` | `aclrtMemset` | matrix/reduction/runtime 回归 |
| `cudaStreamCreate`, `cudaStreamCreateWithFlags` | `aclrtCreateStream` | 官方 matrix、全量回归 |
| `cudaStreamDestroy`, `cudaStreamSynchronize` | `aclrtDestroy/SynchronizeStream` | 全量回归 |
| `cudaEventCreate/Record/Query/Synchronize/ElapsedTime` | ACL event API | event 回归 |
| `cudaDeviceSynchronize`, `cudaDeviceReset` | ACL device API | 全量回归 |
| `cudaGetLastError` | bridge error state | kernel launch checks |
| `cudaDeviceGetAttribute/Properties` | compatibility values | NVIDIA helper 编译/运行 |

### 4.2 `EXPERIMENTAL`：有实现但证据或语义有限

| CUDA API/组 | 当前情况 | 限制 |
|---|---|---|
| `cudaGetErrorString` | 从 ACL recent error/fallback code string 返回 | 不是 CUDA 完整错误名表 |
| `cudaProfilerStart/Stop` | no-op success | 不等同于 CUDA profiler 语义 |
| `cudaMallocHost/FreeHost` | ACL pinned host memory | 仅覆盖当前样例使用方式 |
| `cudaMemcpy2DAsync` | row-wise async fallback | 语义/性能与 CUDA 可能不同 |
| `cudaMallocPitch` | pitch 等于 width 的简化实现 | 未承诺 CUDA 设备的对齐规则 |
| `cudaLaunchHostFunc` | ACL callback 映射 | callback 语义需独立验证 |
| `cudaMemAdvise/PrefetchAsync` | no-op/fallback | Ascend 无 CUDA unified-memory 迁移语义 |
| cublas `Sgemm` | fp32 输入 staging 到 fp16 + GemmEx | 依赖 device scalar、OPP/ATC、精度有损 |
| cublas `Dgemm` | not supported fallback | 保留接口但无 double BLAS 证据 |
| `cudaDeviceProp` CUDA compute capability | compatibility values | 不是实际 NVIDIA SM 属性 |

### 4.3 `UNSUPPORTED`：当前明确无等价能力或仅占位

| CUDA API/组 | 当前实现/原因 |
|---|---|
| CUDA Graph / stream capture | wrapper 返回 `ACL_ERROR_API_NOT_SUPPORT` |
| Texture / Surface object | Ascend 无 CUDA texture/surface 等价对象 |
| CUDA array / mipmapped array | wrapper 占位或 `not-support` |
| External memory | wrapper 占位或 `not-support` |
| 显式 `cudaLaunchKernel` | main 使用 CUDA `<<<>>>`，显式函数入口无稳定等价路径 |
| Host register/device pointer mapping | Ascend 无统一 CUDA mapped-host 语义 |
| BLAS1/BLAS2 未实现组 | wrapper 返回 `ACL_ERROR_API_NOT_SUPPORT` |
| BLAS double GEMM | `ACL_DT_DOUBLE` 不存在，保持 unsupported |

## 5. Kernel 原语边界

`include/bridge/kernel/simt_adaptor.h` 只做符号级兼容：

| CUDA | Ascend SIMT | 状态 |
|---|---|---|
| `__shared__` | `__ubuf__` | `SUPPORTED` |
| `__syncthreads` | `asc_syncthreads` | `SUPPORTED` |
| `__threadfence` | `asc_threadfence` | `SUPPORTED` |
| `atomicAdd/Sub/Min/Max/Exch` | `asc_atomic_*` | `SUPPORTED` |
| `__shfl_down_sync` | `asc_shfl_down` | `SUPPORTED` |
| `__all/any/ballot_sync` | `asc_all/any/ballot` | `SUPPORTED` |
| mask semantics | Ascend warp API 为隐式全 warp | `EXPERIMENTAL` |
| `__simt_vf__` / `cce::async_invoke` | main 不使用 | `UNSUPPORTED in main` |

## 6. 实验性 BLAS 边界

cublas/aclblas 与 puresimt kernel 路线独立：

- 保留 `src/bridge/blas`、`include/bridge/blas`、`include/cublas_v2.h`；
- 不进入 `PURESIMT_REGRESSION_PASS` 的必要条件；
- `FP16 × FP16 → FP32` GemmEx 需要 device alpha/beta、`lda/ldb/ldc=-1` 和匹配
  OPP/ATC 单算子模型；
- fp32 输入的 `cublasSgemm` fallback 会产生 fp16 输入精度损失；
- 不把当前 BLAS wrapper 描述为完整 CUDA cuBLAS 兼容。

## 7. CMake 集成

CMake 集成属于独立的 secondary compatibility capability，不改变 puresimt kernel
直通主线。当前核心范围已经完成 Ascend 真机验证，扩展 CUDA 工具链能力仍保持边界。

最新 CMake 真机证据：

```text
日期：2026-08-25
commit：6fdb330
入口：tools/real_test/server_cmd.txt
结果：CMAKE_CCE_REGRESSION_PASS
```

当前调用链为：

```text
CMake configure → cmake_cce → CUDA 虚拟工具链目录 → nvcc_entry → mock_nvcc；
推荐 build → cmake_cce --build → make_cce/ninja_cce → CUDA 虚拟工具链目录 nvcc → cucc → ccec
```

| Capability | Status | Evidence / boundary |
|---|---|---|
| `cmake_cce` + `project(... LANGUAGES CUDA)` core smoke | `SUPPORTED` | `examples/cmake_demo`; Ascend configure/build/run passed on 2026-08-25 |
| Multiple `.cu` translation units | `SUPPORTED` | `examples/cmake_compat`; Ascend real-device build/run passed |
| CUDA static/shared library targets | `SUPPORTED` | `examples/cmake_compat`; both targets built and ran on Ascend |
| Host C++ + CUDA library mixed target | `SUPPORTED` | `examples/cmake_compat`; host link resolved Ascend runtime and ran |
| Unix Makefiles | `SUPPORTED` | exercised by default CMake smoke |
| Ninja generator | `SUPPORTED` | `cmake_ninja` configure/build/run passed on the test server |
| CUDA Toolkit relative shim layout | `SUPPORTED` for tested Linux layout | `targets/<arch>-<os>/{include,lib}`, `lib64/stubs`; no fake unsupported libraries |
| `find_package(CUDAToolkit)` | `SUPPORTED` for bridge-backed scope | installed `cmake_modules/FindCUDAToolkit.cmake`; covered by `cmake_compat` |
| `CUDA::cudart` | `SUPPORTED` for bridge-backed scope | maps to `libcce_bridge_runtime.so`; not full NVIDIA cudart equivalence |
| `cu2cce::runtime` native CMake target | `EXPERIMENTAL` | installed bridge-owned package target; does not claim CUDA API coverage |
| legacy `find_package(CUDA)` | `SUPPORTED` for basic macro scope | installed minimal `FindCUDA.cmake`; covered by `cmake_compat_legacy` |
| `cuda_add_executable` / `cuda_add_library` | `SUPPORTED` for basic macro scope | `cuda_add_executable` real-device smoke passed; PTX helpers remain unsupported |
| `CUDA::cublas` | `EXPERIMENTAL` | only through the separate BLAS/aclblas capability; not part of puresimt CMake smoke |
| CUDA device-link/separable compilation | `UNSUPPORTED` for current scope | no semantic lowering; `mock_nvcc` diagnoses/rejects strict mode |
| PTX/CUBIN/FATBIN generation | `UNSUPPORTED` | Ascend puresimt consumes CUDA source through ccec, not NVIDIA binary artifacts |
| `CUDA::cufft`, `CUDA::cusparse`, `CUDA::curand`, `CUDA::cudnn` | `UNSUPPORTED` | no validated bridge-backed target |

`CUDA::cudart` 和 legacy `FindCUDA` 的状态必须在 Ascend 真机通过对应工程后再考虑
升级。CMake 配置成功或头文件存在不等于 CUDA API 已经 `SUPPORTED`。

## 8. cucc 驱动策略

| Capability | Status | Evidence / boundary |
|---|---|---|
| Rule-driven CUDA argument processing | `EXPERIMENTAL` | `bin/conf.json` + shared `tools/cu2cce_driver.py` |
| Shared policy for normal cucc and CMake nvcc shim | `EXPERIMENTAL` | `bin/cucc` and `tools/mock/mock_nvcc.py` use the same driver |
| NVIDIA architecture flag handling | `EXPERIMENTAL` | `-gencode`/`-arch=sm_*` are diagnosed and not mapped to fake Ascend architectures |
| PTX/CUBIN/FATBIN/device-link rejection | `UNSUPPORTED` | explicitly rejected by the puresimt driver |
| Automatic unvalidated CUDA library replacement | `UNSUPPORTED` | no blanket `-lcublas`/`-lcufft`/`-lcusparse` mapping |
| crt include-surface compat (`host_defines`, `common_functions`) | `EXPERIMENTAL` | `include/crt/*` forward-only headers shipped with `<cuda_runtime.h>` |
| Device math forwarding (`crt/math_functions.h` → `simt_api`) | `EXPERIMENTAL` until server proof | forward-only; missing single-precision symbols fail compiles naming themselves via `device_math_minimal` diagnostic carrier |
| **Double 精度（device 侧）** | **`UNSUPPORTED`（昇腾确认的架构边界）** | 毕昇 device 完全没有 double：连 `doubleN` 向量类型都不存在（真机 "unknown type name 'double2'"）。不做伪实现、不做静默降精度；reduction 样本排除与此同根 |
| 单精度设备数学链（expf/logf/sqrtf/sinf/cosf/tanhf/powf/fmin/fmax/fabsf） | `SUPPORTED`（真机多轮 [DIAG-PASS]，round-7 起进入必需门禁） | device helper + kernel + libm 对照全链通过；host 侧 `-lm` 已入通用链接行 |
| 执行空间关键字映射（`__device__`） | `EXPERIMENTAL` until server proof | 前端不识别独立 `__device__`（round-4 "unknown type name"）；simt_adaptor 复用 CANN 自身的 `__SIMT_DEVICE_FUNCTIONS_DECL__` 选择器做映射（自动跟随 __aicore__/__simt_callee__ 双模式），逃生开关 `CU2CCE_NO_DEVICE_KEYWORD_MAP=1`；仅映射有失败证据的拼写，不做投机扩展 |
| Device 不支持类型的前置诊断（FP64/double） | `SUPPORTED`（driver 层） | `cucc` 编译前只读扫描 `.cu` 的 device 区域，`[type-policy]` 输出行号并给出"手动改为 float"指引（导师方向：桥层报错/警告 + 用户自行降级）。策略 `CU2CCE_DEVICE_FP64_POLICY`：`error`=默认在 ccec 前拦截 / `warn`=提示后放行 / `allow`=跳过。载体：`examples/puresimt/double_policy_diag.cu` 真机演示 warn/error 两路径。已知限制：纯词法启发式，宏生成签名可能漏报；浮点字面量（如 device 内的 `2.0`）暂不在本检查范围 |
| CUDA vector types（char1..float4 全族，无 double 族） | `SUPPORTED`（编译器原生） | 真机证据：bisheng 以 ext_vector_type typedef 注入向量类型（`__clang_dpp_types.h`），桥接层零定义；曾试 struct 重定义即冲突，已回退为文档化空壳。`make_*` 工厂 host 可见性待清单证据后再评估 |
| CUDA `make_*` 向量工厂 host 可见性 | `EXPERIMENTAL` until server proof | 真机证据：原生工厂在 `vector_functions_impl.h` 标注仅 device；桥接层以全局模板补 host 候选（`include/crt/vector_functions_compat.h`，策略同 min/max），设备端精确匹配优先级不变 |
| 执行空间关键字安全（host_defines） | `SUPPORTED`（教训固化） | ccec 将 __host__/__device__ 作语言关键字而非宏；桥内禁止 #define 兜底（曾剥离双空间声明致误导报错，头文件已文档化并加契约测试锁定） |
| Installed-CANN symbol inventory | `SUPPORTED`（driver 层） | 只读 `tools/real_test/inventory_simt_api.sh` → `build/real_test/puresimt/simt_api_inventory.log`，替代无法抓取正文的 hiascend SPA 页面 |

`CU2CCE_DEBUG=1` prints rule decisions. `CU2CCE_TRACE_FILE` writes a parameter-only JSON
trace. `CU2CCE_STRICT_FLAGS=1` turns unvalidated CUDA library use and unknown long CUDA
options into errors.

## 9. 维护规则

每次新增测试必须同步：

1. `tools/real_test/server_cmd.txt`；
2. `examples/puresimt/README.md`；
3. 本矩阵文档。

每次 API 状态变化必须同步：

1. 本矩阵；
2. `docs/04_对比与验证/CURRENT_STATE.md` 摘要；
3. `docs/01_架构设计/ASCEND_RESEARCH_NOTES.md` 事实记录。

## 10. 官方 SIMT API 与当前 adaptor 的边界（2026-09-03）

官方 Ascend C 的 SIMT 入口为 `simt/asc_simt.h`，通常属于 Device 编译期接口，不对应单独的 Host 动态库。当前 CUDA 兼容头通过 `include/bridge/kernel/simt_adaptor.h` 提供有限映射：

| CUDA 写法 | 当前 Ascend 入口 | 当前状态 | 限制 |
|---|---|---|---|
| `__shared__` | `__ubuf__` | `SUPPORTED` for tested kernels | 地址空间不是 CUDA shared memory 的字节级等价物 |
| `__syncthreads()` | `asc_syncthreads()` | `SUPPORTED` for tested kernels | 仅限 ccec puresimt 语境 |
| `__threadfence*` | `asc_threadfence*` | `SUPPORTED` for tested forms | 需要继续覆盖内存序语义 |
| `atomicAdd/Sub/Exch/Min/Max` | `asc_atomic_*` | `SUPPORTED` for tested types | 类型和地址空间需逐项验证 |
| `__shfl*_sync` | `asc_shfl*` | `SUPPORTED` for tested forms | mask/width 参数不是完整 CUDA 语义复制 |
| `__all_sync`/`__any_sync`/`__ballot_sync` | `asc_all/asc_any/asc_ballot` | `SUPPORTED` for tested forms | 需要保持 lane/warp 行为证据 |
| `__reduce_add_sync` | `asc_reduce_add` | `SUPPORTED` for tested form | 不是完整 warp reduction 覆盖 |

以下内容不属于当前 adaptor 合约：

```text
__simt_vf__
cce::async_invoke
PTX intrinsic
CUDA device-link/RDC
```

## 11. CUDA device-side API → Ascend C SIMT 逐项映射（2026-09-03）

官方 Ascend C API 列表将 SIMT 作为独立编程模型，入口头文件为 `simt/asc_simt.h`；底层同步、Warp 和原子接口由 CANN 的 `simt_api` 头文件提供。cu2cce 的 CUDA 头文件入口通过 `include/bridge/kernel/cuda_device_api.h` 集中维护以下映射。扩展 API 默认关闭；只有显式设置 `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 才会暴露依赖产品/编译模型的扩展接口。

### 11.1 线程、同步和执行空间

| CUDA 写法 | CANN/Ascend 写法 | 当前处理 | 状态 | 说明 |
|---|---|---|---|---|
| `__global__` | ccec 原生 CUDA SIMT kernel 语法 | 不改写 | `SUPPORTED` for tested kernels | ccec `--enable-simt` 直接解析 |
| `__device__` | `__SIMT_DEVICE_FUNCTIONS_DECL__` | 由 `simt_adaptor.h` 映射 | `SUPPORTED` for tested helper | 由 CANN `device_types.h` 选择执行空间 |
| `__shared__` | `__ubuf__` | 宏映射 | `SUPPORTED` for tested kernels | UB 资源不是 NVIDIA shared memory 的字节级承诺 |
| `__syncthreads()` | `asc_syncthreads()` | 宏映射 | `SUPPORTED` for tested kernels | 依赖 CANN SIMT barrier |
| `__threadfence()` | `asc_threadfence()` | 宏映射 | `SUPPORTED` for tested forms | 只保证已验证的 Ascend SIMT 内存序 |
| `__threadfence_block()` | `asc_threadfence_block()` | 宏映射 | `SUPPORTED` for tested forms | block/warp 语义需按 CANN 约束使用 |
| `__threadfence_system()` | 无公开等价映射 | 不定义 alias | `UNSUPPORTED` | 禁止静默降级成普通 fence |

### 11.2 Warp、Vote 和归约

| CUDA 写法 | CANN/Ascend 写法 | 当前处理 | 状态 | 关键差异 |
|---|---|---|---|---|
| `__shfl_sync(mask,v,lane)` | `asc_shfl(v,lane,warpSize)` | 保留 3 参数形式 | `SUPPORTED` for tested forms | mask 在 Ascend 侧为隐式执行组 |
| `__shfl_sync(mask,v,lane,width)` | `asc_shfl(v,lane,width)` | 保留 width | `EXPERIMENTAL` | width 受 CANN SIMT 支持范围约束 |
| `__shfl_up_sync` | `asc_shfl_up` | 3/4 参数分派 | `SUPPORTED` for tested forms | mask 不可表达任意 CUDA 子组 |
| `__shfl_down_sync` | `asc_shfl_down` | 3/4 参数分派 | `SUPPORTED` for tested forms | 当前回归重点覆盖 down |
| `__shfl_xor_sync` | `asc_shfl_xor` | 3/4 参数分派 | `EXPERIMENTAL` | 需要按 lane mask 和 width 约束使用 |
| 旧 CUDA `__shfl/__shfl_up/__shfl_down/__shfl_xor` | `asc_shfl*` | `CU2CCE_ENABLE_LEGACY_DEVICE_APIS=1` 后映射 | `EXPERIMENTAL` | 默认关闭，避免污染 CANN FP16 头文件内部同名 `__shfl` 声明 |
| `__all_sync` | `asc_all` | mask 被显式求值后忽略 | `SUPPORTED` for tested forms | 不是任意 CUDA active mask 等价 |
| `__any_sync` | `asc_any` | mask 被显式求值后忽略 | `SUPPORTED` for tested forms | 同上 |
| `__ballot_sync` | `asc_ballot` | mask 被显式求值后忽略 | `SUPPORTED` for tested forms | 返回位图宽度由 CANN 定义 |
| `__activemask()` | `asc_activemask()` | `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 后映射 | `EXPERIMENTAL` | 官方接口存在，但当前目标需单独验证 active lane 行为 |
| `__reduce_add_sync` | `asc_reduce_add` | mask 被显式求值后忽略 | `SUPPORTED` for tested form | 当前只承诺已验证类型 |
| `__reduce_max_sync` | `asc_reduce_max` | `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 后映射 | `EXPERIMENTAL` | 依赖 CANN Warp header、类型和目标编译模型 |
| `__reduce_min_sync` | `asc_reduce_min` | `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 后映射 | `EXPERIMENTAL` | 同上 |

### 11.3 原子操作

| CUDA 写法 | CANN/Ascend 写法 | 当前处理 | 状态 | 限制 |
|---|---|---|---|---|
| `atomicAdd/Sub/Exch/Max/Min` | `asc_atomic_add/sub/exch/max/min` | 直接宏映射 | `SUPPORTED` for tested types | 需匹配 CANN 支持的地址空间和数据类型 |
| `atomicInc` | `asc_atomic_inc` | 设置 `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 后映射 | `NOT_VERIFIED` | 当前未纳入主线数值回归 |
| `atomicDec` | `asc_atomic_dec` | 设置 `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 后映射 | `NOT_VERIFIED` | 同上 |
| `atomicCAS` | `asc_atomic_cas` | 设置 `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 后映射 | `NOT_VERIFIED` | 返回旧值和支持类型需真机探针确认 |
| `atomicAnd` | `asc_atomic_and` | 设置 `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 后映射 | `NOT_VERIFIED` | 需确认 global/UB 地址空间 |
| `atomicOr` | `asc_atomic_or` | 设置 `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 后映射 | `NOT_VERIFIED` | 同上 |
| `atomicXor` | `asc_atomic_xor` | 设置 `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 后映射 | `NOT_VERIFIED` | 同上 |

### 11.4 数学、类型和内建变量

| CUDA 面 | Ascend C 面 | 当前状态 |
|---|---|---|
| `__device__` 内的单精度数学函数 | CANN `simt_api/math_functions.h` | `SUPPORTED` for tested functions |
| `__float_as_int` 等位级转换 | CANN `simt_api/device_functions.h` | `EXPERIMENTAL`，需按安装版本探针确认 |
| `__half` / `__half2` | `half` / `half2`、`simt_api/asc_fp16.h` | `SUPPORTED` for tested forms |
| `__nv_bfloat16` / `__nv_bfloat162` | `bfloat16_t` / `bfloat16x2_t`、`simt_api/asc_bf16.h` | `SUPPORTED` for tested forms |
| `threadIdx/blockIdx/blockDim/gridDim` | ccec 原生 SIMT builtins | `SUPPORTED` for tested kernels |
| CUDA vector factory 全量 | CANN vector API | `NOT_VERIFIED` | 未经逐项 host/device 可见性验证，不在兼容头中猜测补齐 |
| `__CUDA_ARCH__` | 兼容宏或 trace | `DIAGNOSTIC_ONLY` | 不转成 Ascend `dav-*` target |

### 11.5 头文件责任边界

```text
include/cuda_runtime.h
  ├─ host Runtime：__cuda_runtime.h → wcuda* → aclrt*
  ├─ CRT surface：crt/*.h、device_launch_parameters.h、math_constants.h
  └─ device surface：bridge/kernel/simt_adaptor.h
                       └─ bridge/kernel/cuda_device_api.h
```

`cuda_runtime.h` 的头文件可见性、ccec 编译成功和真机运行结果必须分别记录；任何未提供可靠 Ascend 等价物的 CUDA device API 都不能用一个同名宏伪造。

官方核对入口（以服务器实际安装的 CANN 版本为最终依据）：

```text
https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/ascendcopapi/docs/zh/api/api_list.md
https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/headerliblist/hfandlf_09_0001.html
```

其中，CANN 文档中列出的 SIMT/原子接口仍需结合具体产品和编译模型判断；本项目的扩展 API 编译结果只作为 `DIAG-ONLY` 证据，不能替代当前 Ascend950 目标的数值执行回归。
