# CUDA Build System Compatibility Matrix

本矩阵面向大型 CUDA 项目迁移，和 kernel/operator compatibility matrix 分开维护。
状态必须以测试证据为准。

| 能力 | 状态 | 测试/证据 | 限制 |
|---|---|---|---|
| modern CMake CUDA project | `SUPPORTED` for validated core | `examples/cmake_demo` 真机通过 | 大型项目高级 CUDA property 仍需扩展 |
| multiple `.cu` CMake target | `SUPPORTED` for validated core | `examples/cmake_compat` | 复杂 device symbol resolution 未覆盖 |
| static/shared CUDA library | `EXPERIMENTAL` | `examples/cmake_compat`, `examples/build_compat` | 每个 linked library 当前限制为一个 `.cu` translation unit |
| object/interface library | `EXPERIMENTAL` | build compatibility suite | 多 `.cu` 静态归档会触发 ccec registration symbol 冲突 |
| host C++ + CUDA | `SUPPORTED` for validated core | `examples/cmake_compat` | 其他 host toolchain 未覆盖 |
| `find_package(CUDAToolkit)` | `EXPERIMENTAL` | minimal module | 不是完整 NVIDIA Toolkit discovery |
| `CUDA::cudart` | `EXPERIMENTAL` | bridge-backed runtime target | 不代表完整 CUDA Runtime 等价 |
| legacy `FindCUDA` | `EXPERIMENTAL` | `examples/cmake_compat_legacy` | 仅基础宏 |
| install/export/package | `EXPERIMENTAL` | native `cu2cce::runtime` package | 需安装树真机回归 |
| Unix Makefiles | `EXPERIMENTAL` | `examples/build_compat/make` | 通过 `make_cce` |
| Ninja | `EXPERIMENTAL` | CMake Ninja smoke + `ninja_cce` | 需服务器安装 Ninja |
| Bazel/TF cu-bridge environment mode | `UNSUPPORTED` with explicit diagnostic | `WCUDA_BAZEL_TF_BUILD=1` driver probe | 无已验证 Ascend puresimt 等价实现，不伪造 MACA 参数/链接 |
| multi-directory CMake project | `SUPPORTED` for validated core | `examples/large_project` 真机通过（configure/build/run/install-export，`LARGE_PROJECT_REGRESSION_PASS`） | 每 linked library 仍为一个 `.cu` |
| generated input + cross-directory target dependency | `SUPPORTED` for validated core | `examples/large_project/util` custom command 真机通过 | 生成的数据文件仅供 host 侧读取，不进入 device 编译 |
| response file | `EXPERIMENTAL` | `tests/test_cucc_rules.py` | 构建系统组合仍需扩大 |
| dependency generation `-M/-MM/-MD/-MF/-MT` | `DIAGNOSTIC_ONLY` until device proof | driver phase tests | ccec 真实依赖输出需真机确认 |
| query `--version/-V/--help` | `SUPPORTED` for driver | local unit tests | 输出是 cu2cce compatibility identity |
| toolchain probes `-dumpversion/-dumpmachine/-print-*` | `SUPPORTED` (passthrough) | `tests/test_cucc_rules.py` query bypass | cu-bridge skipstring parity：命中即整条命令原样直通 ccec，无注入 |
| preprocess `-E` | `SUPPORTED` for passthrough mechanism | driver query tests | ccec receives original `-E` command; actual output remains diagnostic-only until real proof |
| CUDA `-gencode` / `-arch` | `DIAGNOSTIC_ONLY`（识别并记录） | rule trace + `extract_cuda_arch` | flag 仍丢弃；架构值进 trace；`CU2CCE_ARCH_MACRO=auto`（opt-in）时注入 `__CUDA_ARCH__`（无 `-Xdevice` 式 device-pass 限定，host 分支语义见 PARITY_AUDIT） |
| `-std` 用户值 | `SUPPORTED`（去重） | `tests/test_cucc_rules.py` | 用户 `-std=` 存在时不注入 `-std=gnu++17` |
| `-Werror=implicit-fallthrough*` | `SUPPORTED`（规范化） | `tests/test_cucc_rules.py` | nvcc/gcc 的 =N 拼写重写为 clang 兼容 `-Werror=implicit-fallthrough`（cu-bridge bin/cucc:250-280 对齐）；`-Xcompiler` 值中的裸 `-Werror` 丢弃（conf_regex.yaml C500S-11780 对齐） |
| `-Xcompiler` 宿主旗标过滤 | `SUPPORTED`（逐 flag） | `tests/test_cucc_rules.py` | 逗号列表逐 flag 处理；`-Xcompiler=...` 等号形式同样过滤（回归锁定）；gcc-only 拼写（-fno-canonical-system-headers、var-tracking 族等）由 `regex_cleanup` 段剔除 |
| `__CUDACC__`/`__CUDACC_VER_*` 宏族 | `SUPPORTED`（nvcc 语义） | `tests/test_cucc_rules.py` | 仅 `.cu` TU 注入，值由 `CUCC_CUDA_VERSION` 推导（11060→11/6/0/110600）；mock identity 路径同源不重复 |
| `-Xlinker` 链接选项 | `SUPPORTED`（成对透传） | `tests/test_cucc_rules.py` | clang 原生；compile 阶段仅 unused 告警（真机确认项） |
| `-lnvrtc`/`-lOpenCL` | `UNSUPPORTED`（提前 drop） | `tests/test_cucc_rules.py` | cu-bridge conf.json 对齐：带警告丢弃而非链接期失败；strict 模式报错 |
| `--use_fast_math` 族 | `SUPPORTED`（映射） | `tests/test_cucc_rules.py`；真机 dryrun 探针 | 四种拼写规范化为 clang/ccec 的 `-ffast-math`（FP 语义变化即调用方所请；cu-bridge 同样映射到自家拼写） |
| OpenMP `-lgomp`/`-l:libgomp*` | `SUPPORTED`（映射） | `tests/test_cucc_rules.py` | clang 系驱动经 libomp 链接 OpenMP，非 gcc libgomp（cu-bridge 同样映射到自家运行时）；`-fopenmp` 经 `-Xcompiler` 白名单放行 |
| `-static` | `UNSUPPORTED`（诚实 drop） | `tests/test_cucc_rules.py` | 全静态链接无法解析桥运行时（.so）；带警告丢弃而非在 ld 内报混淆错误（cu-bridge 同为后端限制做了改写） |
| `--version/-V` banner | `SUPPORTED`（可解析） | `tests/test_cucc_rules.py` | 携带 cu-bridge 同款 `compatible with release 11.6` 词组 + 诚实的 cu2cce 身份 |
| 杂项 filter 补齐（`g++*`/`/usr/bin/cc`/`--threads=*`/`--W*` 双横线族/`-diag-suppress`/`-keep-dir`） | `SUPPORTED`（drop） | `tests/test_cucc_rules.py` | 第八批 conf.json 差距收口；合法的单词号 clang 警告（如 `-Wreorder`）仍放行 |
| `.cu` 自动预包含 `cuda_runtime.h` | `EXPERIMENTAL`（对齐 nvcc 语义） | `tests/test_cucc_rules.py`；真机回归待复验 | 桥内 `__cuda_runtime.h` 自含 simt_adaptor.h 且有 include guard |
| shim lib64 CUDA 库名族 | `EXPERIMENTAL`（libcublas/libnvToolsExt 新增） | `tests/test_cmake_compat.py` | cublas 走桥内实验面（三用例真机绿）；cufft/cusparse/cudnn 等仍无替身、不造假 |
| plain host-compiler link（`make_gxx_link` 载体 + hostcc 规则化注入） | `EXPERIMENTAL` | `examples/build_compat/make_gxx_link`；rules `hostcc.link.inject` | 链接器保持 g++，注入改为规则驱动 |
| PTX/CUBIN/FATBIN | `UNSUPPORTED` | explicit rejection | puresimt 不消费 NVIDIA binary artifacts |
| device-link/RDC | `UNSUPPORTED` for current scope | explicit rejection | 后续需单独验证 ccec 能力 |
| `CUDA::cublas` | `EXPERIMENTAL` secondary | separate BLAS regression | 不属于默认构建兼容主线 |
| `CUDA::cufft/cusparse/curand/cudnn` | `UNSUPPORTED` | no validated target | 不创建虚假 target |

## 维护规则

新增构建场景必须同步：

1. 测试工程；
2. `tools/real_test/server_cmd.txt`；
3. 本矩阵；
4. `README.md`/`QUICKSTART.md` 中的使用边界。

## 当前已知后端限制

在 Ascend ccec puresimt 输出中，每个 `.cu` translation unit 会生成相同名称的
运行时注册辅助符号，例如 `BishengLaunchKernelWithHostArgs` 和
`__cce_rtLinkedDevBinaryAscPureSimtEagerRegister`。因此，将多个 `.cu` 对象直接放入
同一个静态 archive 后再由 host C++ 链接，可能出现 duplicate symbol；这不是普通
CMake 参数过滤问题，也不能用 `--allow-multiple-definition` 伪装修复。当前测试将每个
linked static/shared library 限制为一个 `.cu`，并把该场景标记为未完成的 backend
translation-unit aggregation 能力。

### 真机证据（multi_cu_diagnostic，DIAGNOSTIC_ONLY）

`examples/multi_cu_diagnostic`（2 个 `.cu` + 1 个 static library，separable OFF）
在服务器回归中如实复现该边界：

- configure：`[PASS] multi_cu_diagnostic configure` —— CMake 配置面完全通过；
- 归档：`Linking CUDA static library libmulti_cu_lib.a` + `Built target multi_cu_lib`（100%）；
- 链接：host 链接在 `ld` 层报
  `multiple definition of 'BishengLaunchKernelWithHostArgs'` 与
  `multiple definition of '__cce_rtLinkedDevBinaryAscPureSimtEagerRegister'`
  （kernel_b.cu.o vs kernel_a.cu.o），collect2 error → `DIAG-ONLY`，不阻塞 required。

结论：多 `.cu` 静态库 = `DIAGNOSTIC_ONLY`（构建面能过，链接面受后端限制）；不作为
required profile，`server_cmd.txt` 有专门的非阻塞段记录。

## CMake CUDA 语言模块 override（真机验证通过）

`cmake_cce` 默认注入 `-DCMAKE_USER_MAKE_RULES_OVERRIDE[_CUDA]` →
`cmake_modules/override/CUDAOverrides.cmake`，在 CMake 内置探测开始前固定：

- `CMAKE_CUDA_COMPILER` / `CMAKE_CUDA_DEVICE_LINKER` / `CMAKE_CUDA_FATBINARY` =
  shim nvcc（现代 CMake 不再去探测缺失的 nvlink/fatbinary）；
- toolkit root 指向 shim 的 CUDA 形状目录；vendor 正则接受 shim 的
  NVIDIA-compat 身份；
- `CUDA_RUNTIME_LIBRARY=None`、`CUDA_ARCHITECTURES=OFF`（Ascend arch 由
  `CU2CCE_NPU_ARCH` 决定，零硬编码）。

真机回归下 `cmake_demo` / `cmake_compat` / `cmake_compat_legacy` /
`cmake_large_project` / `cmake_build_compat` / `cmake_ninja` 全部 PASS，无回归。
回退开关：`CU2CCE_CMAKE_NO_OVERRIDE=1`（或用旧纯 `-DCMAKE_CUDA_*` 路径）。

## External upstream project

The curated external profile is pinned to NVIDIA/cuda-samples `v12.8` revision
`db3eea23946bca2e90a75eca2b5b3e07158a9e11`. The source is fetched into
`build/real_test/external/` and remains unmodified. Because this revision uses
CUDA-era CMake features newer than the CANN compatibility version, its status is
`EXPERIMENTAL`; the profile validates the build graph and selected core samples,
not complete CUDA 12.8 API compatibility.


## External CUDA Samples reduction limitation

The external required profile intentionally excludes NVIDIA's `reduction` sample. The
source instantiates device-side `double` kernels and uses cross-warp
`thread_block_tile<64/128/256/512>`. Stable CANN 9.2 SIMT does not provide that combination.
This is recorded as `UNSUPPORTED` for the current backend; it does not affect the
validated external CMake profile (`vectorAdd`, `matrixMul`, `simpleStreams`,
`simpleMultiCopy`).


## Current external profile

The required external CMake profile now uses NVIDIA CUDA Samples `vectorAdd`, `matrixMul`,
`simpleStreams`, and `simpleMultiCopy`. The upstream `reduction` sample is excluded and
recorded as a known limitation because stable CANN 9.2 does not support its cross-warp
`thread_block_tile<64..512>` and device-side `double` instantiations.


The external required profile is intentionally limited to `vectorAdd` and `matrixMul`.
`simpleStreams` and `simpleMultiCopy` remain diagnostic-only until their CUDA runtime
flags, `cudaDeviceProp` fields, host-memory mapping, and host-side overload behavior are
validated on CANN.


Diagnostic samples are NOT part of the required `--target vectorAdd matrixMul`
invocation: each one is compiled separately right after the required profile and then run,
with a non-blocking `[DIAG-ONLY]`/`[DIAG-PASS]` outcome. Evidence lives in
`build/real_test/puresimt/cuda_samples_<sample>.build.log` and
`build/real_test/puresimt/cuda_samples_diag_<sample>.log`. Earlier runs printed empty
diagnostic tails because the harness never compiled these targets before trying to run
them; with the per-sample build phase any outcome reflects real compile/runtime behavior
on CANN.

### 真机证据（诊断样本首轮逐个编译）

- `simpleStreams` / `simpleMultiCopy` 在**编译期**失败：CANN
  `simt_api/math_functions.h` 将整数 `min/max` 仅声明为 `__device__`，host 调用报
  "candidate function not viable: call to __device__ function from __host__
  function"（simpleStreams 1 处、simpleMultiCopy 7 处）。修复：
  `include/bridge/kernel/simt_adaptor.h` 追加全局命名空间值语义 `min/max`
  模板——模板与非模板静态声明永不合并，设备语境仍精确匹配 CANN 原生候选，
  既有通过样本的重载决议不变。
- `asyncAPI` 在**产物阶段**失败：其上游 CMakeLists 自设 per-target
  `CUDA_ARCHITECTURES sm_61..sm_120`，CMake 因此生成按架构的 `.cubin` 规则，
  `ld.lld: cannot open output file =asyncAPI/.../sm_120.cubin`。修复：harness 在
  `add_subdirectory` 之后对全部样本目标强制
  `CUDA_ARCHITECTURES/SEPARABLE/CUBIN/FATBIN = OFF`。

两项修复待服务器复验；pinned memory、host mapping、事件次序等运行时面仍为
diagnostic-only。

复验迭代补充：无条件给所有目标设置 `CUDA_CUBIN/FATBIN_COMPILATION` 会让外部
configure 直接失败——CMake 规定这两个属性只能用于 OBJECT 目标（即使赋值 OFF，
出现在 executable 上即报 "property can only be applied to OBJECT targets"）。
修正后仅在 `TYPE STREQUAL OBJECT_LIBRARY` 且 CMake>=3.28 时设置；普通
executable 的按架构产物由 `CUDA_ARCHITECTURES OFF` 移除。

### 真机证据（第二轮：诊断样本剥洋葱推进）

类型守卫修复后 external configure/build 恢复 PASS，必需样本照旧 PASS；三个
诊断样本进入真实结果：

- `simpleStreams`：编译通过并实际运行（打印 device prop、pinned memory 分配
  均成功），最终停在
  `simpleStreams.cu:353 code=100000(unknown) "cudaEventRecord(start_event, 0)"`。
  根因：桥内 `wcudaEventRecord` 把 nullptr 流一律判 INVALID_PARAM，而 CUDA 的
  默认流就是 0。修复：新增 `wcuda_resolve_stream`——为三种 CUDA 默认流拼写
  惰性创建一条桥内自有 ACL 流，event record、async memcpy/memset、stream
  sync/query/waitEvent、host callback 共 10 处守卫统一走它。
- `simpleMultiCopy`：仍编译失败但已是下一层缺口：
  `cudaHostAllocDefault` 未声明 + `_cceDeviceProp_t` 缺 `deviceOverlap` /
  `asyncEngineCount`。修复：adaptor 补 4 个 hostAlloc 标志常量、
  `wcudaHostAlloc` 类型化模板重载、deviceProp 结构补两个字段（如实报 0，
  ACL 无重叠拷贝引擎）。
- `asyncAPI`：编译通过，运行却无任何输出静默退出。harness 已改为记录
  `exit=$diag_status`，下一轮日志即可区分崩溃/超时/错误码。

默认流语义与 copy-engine 字段的诚实边界已同步记入 profile_core.json
known_limitations。


## CUDA runtime wrapper additions

The bridge now declares and implements the basic `simpleStreams` device/event flag surface:
`cudaDeviceBlockingSync`, `cudaDeviceMapHost`, `cudaEventDefault`,
`cudaEventBlockingSync`, and `cudaSetDeviceFlags`. Ascend has no direct CUDA scheduling
flag equivalent, so `cudaSetDeviceFlags` is a compatibility no-op and
`cudaDeviceProp::canMapHostMemory` reports `0`; generic host mapping remains diagnostic-only.
The simpleStreams profile is not required until its host/device `max` overload and pinned
memory semantics are separately validated.

## External profile and runtime-API diagnostic boundary

- Required samples: `vectorAdd`, `matrixMul` (build + run).
- Diagnostic-only samples (in configure/build graph, run failure does not block
  required PASS): `simpleStreams`, `simpleMultiCopy`, `asyncAPI`.
- Excluded sample: `reduction` (cross-warp `thread_block_tile` + device double).
- Runtime APIs still `ACL_ERROR_API_NOT_SUPPORT` on CANN: `cudaHostRegister`,
  `cudaHostGetDevicePointer`, `cudaHostUnregister`, `cudaHostGetFlags`. Declarations,
  macro mapping, and runtime diagnostics exist; no no-op fake success.
- Fetch security: external CUDA Samples URL must be http/https with a public
  remote host; `tools/real_test/validate_fetch_url.py` rejects loopback,
  private, link-local, multicast, reserved and special-use hosts (including
  DNS-resolvable ones). Non-allowlisted hosts fall back to an exact
  known-good GitHub allowlist (github.com / api.github.com / raw.githubusercontent.com /
  codeload.github.com / objects.githubusercontent.com).
