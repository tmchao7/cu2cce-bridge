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
| multi-directory CMake project | `SUPPORTED` for validated core | `examples/large_project` 真机通过（configure/build/run/install-export，`LARGE_PROJECT_REGRESSION_PASS`） | 每 linked library 仍为一个 `.cu` |
| generated input + cross-directory target dependency | `SUPPORTED` for validated core | `examples/large_project/util` custom command 真机通过 | 生成的数据文件仅供 host 侧读取，不进入 device 编译 |
| response file | `EXPERIMENTAL` | `tests/test_cucc_rules.py` | 构建系统组合仍需扩大 |
| dependency generation `-M/-MM/-MD/-MF/-MT` | `DIAGNOSTIC_ONLY` until device proof | driver phase tests | ccec 真实依赖输出需真机确认 |
| query `--version/-V/--help` | `SUPPORTED` for driver | local unit tests | 输出是 cu2cce compatibility identity |
| preprocess `-E` | `EXPERIMENTAL` | driver phase tests | ccec 真实预处理输出需真机确认 |
| CUDA `-gencode` / `-arch` | `DIAGNOSTIC_ONLY` | rule trace | 不映射 fake Ascend architecture |
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
