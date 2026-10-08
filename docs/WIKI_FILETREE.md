# cu2cce-bridge 仓库文件树导航

**适用分支：** `main` (puresimt)
**更新日期：** 2026-10-08
**目标后端：** Ascend 950 / CANN 9.x / ccec `--enable-simt --cce-aicore-arch=dav-c310-vec`

本文档描述仓库中**每个目录和每个 Git 跟踪文件**的职责，帮助新成员快速定位代码。
配套文档：[架构总览](WIKI_ARCHITECTURE.md) · [快速上手](WIKI_QUICKSTART.md) · [能力声明](CAPABILITY.md)

---

## 状态标记

| 标记 | 含义 |
|---|---|
| **ACTIVE** | 有真实 bridge 适配或运行时实现 |
| **STUB** | 编译时占位：API 声明存在，运行时返回 NOT_SUPPORTED |
| **RESERVED_NPU** | `#if 0` 保留 CUDA 名称和 future unlock point，不激活 |
| **EXPERIMENTAL** | 有限形状或独立能力，不阻塞主线 |
| **DIAGNOSTIC_ONLY** | 用于取证，不阻塞主线回归 |

---

## 目录树总览

```
cu2cce-bridge/
├── bin/                    # 用户入口（cucc, nvcc, gnu, gfortran, conf.json）
├── cmake/                  # CMake package config（find_package 入口）
├── cmake_modules/          # CMake CUDA 兼容模块（FindCUDA, FindCUDAToolkit, override）
├── config/                 # 产物契约 + CUDA API parity inventory 数据
├── include/                # CUDA 兼容头、bridge wrapper、SIMT adaptor、shadow libc
│   ├── bridge/
│   │   ├── blas/           # cuBLAS wrapper（1 ACTIVE + 5 RESERVED_NPU）
│   │   ├── dnn/            # cuDNN wrapper（7 RESERVED_NPU）
│   │   ├── fft/            # cuFFT wrapper（4 RESERVED_NPU）
│   │   ├── jpeg/           # nvJPEG wrapper（1 RESERVED_NPU）
│   │   ├── kernel/         # SIMT 原语适配（2 ACTIVE）
│   │   ├── mckl/           # device runtime wrapper（1 RESERVED_NPU）
│   │   ├── mpp/            # NPP→MPP wrapper（7 RESERVED_NPU）
│   │   ├── pti/            # CUPTI/NVPerf wrapper（5 RESERVED_NPU）
│   │   ├── rand/           # cuRAND wrapper（3 RESERVED_NPU）
│   │   ├── runtime/        # CUDA runtime/driver wrapper（~10 ACTIVE + ~11 RESERVED_NPU）
│   │   ├── sml/            # NVML wrapper（1 RESERVED_NPU）
│   │   ├── solver/         # cuSOLVER wrapper（1 STUB + 5 RESERVED_NPU）
│   │   ├── sparse/         # cuSPARSE wrapper（1 RESERVED_NPU）
│   │   ├── tools_ext/      # NVTX wrapper（1 ACTIVE + 2 RESERVED_NPU）
│   │   └── video/          # VDPAU interop wrapper（2 RESERVED_NPU）
│   ├── cooperative_groups/ # cooperative groups 兼容头
│   ├── crt/                # CUDA CRT 兼容层（types, math, device functions）
│   ├── nvtx3/              # NVTX3 兼容转发
│   └── shadow/             # shadow libc（math.h, stdio.h, stdlib.h, string.h）
├── src/                    # bridge 实现源码
│   └── bridge/
│       ├── blas/src/       # cuBLAS→ACL BLAS experimental 实现
│       ├── runtime/src/    # cuda*→wcuda*→aclrt* 正式 runtime 实现
│       └── solver/src/     # cusolverDn compile-time stub 实现
├── tests/                  # 本地测试套件 + mock ACL 语法检查
├── tools/                  # 构建wrapper、driver、validator、真机回归脚本
│   ├── mock/               # CMake nvcc mock（configure 阶段探针应答）
│   └── real_test/          # 真机回归入口 + 诊断脚本
│       └── diag/           # 非阻塞 ACL/runtime 诊断资产
├── examples/               # 示例工程
│   ├── puresimt/           # 25+ 真机验证 CUDA 测试用例（主线）
│   ├── blas/               # cuBLAS experimental 示例
│   ├── build_compat/       # CMake/Make 构建兼容载体
│   ├── cmake_compat/       # 多 .cu + static/shared library CMake 兼容
│   ├── cmake_compat_legacy/# legacy FindCUDA 示例
│   ├── cmake_demo/         # 最小 CMake CUDA smoke
│   ├── external/           # 外部 NVIDIA CUDA Samples CMake profile
│   ├── large_project/      # 多目录 CMake 项目（install/export）
│   └── multi_cu_diagnostic/# 多 .cu 链接诊断载体
├── third_party/            # NVIDIA CUDA Samples helper 头（include-only）
├── docs/                   # 项目文档
│   ├── 01_架构设计/         # 架构设计文档
│   ├── 02_能力清单/         # 能力矩阵与边界
│   ├── 03_文件目录/         # 仓库文件指南
│   ├── 04_对比与验证/       # cu-bridge 对比与真机验证
│   └── 99_归档/            # 历史归档与参考资料
├── CMakeLists.txt          # 顶层 CMake 构建（bridge runtime + 安装树）
├── CLAUDE.md               # 维护 agent 工作纪律
├── QUICKSTART.md           # 用户操作手册
├── README.md               # 项目定位与能力边界
├── LICENSE.txt             # 许可证
├── NOTICE.txt              # 第三方版权声明
├── opencode.json           # opencode CLI 权限配置
├── .gitattributes          # 跨平台文本属性
└── .gitignore              # 构建产物排除规则
```

---

## 顶层文件

| 文件 | 职责 |
|---|---|
| `CMakeLists.txt` | 顶层 CMake 构建：编译安装 `libcce_bridge_runtime.so`、头文件、工具脚本和 CMake package |
| `CLAUDE.md` | 维护 agent 工作纪律：puresimt 路线、对标基线、真机回归纪律、git 推送规则 |
| `QUICKSTART.md` | 用户从 CANN 环境到编译运行 CUDA 的完整操作手册 |
| `README.md` | 项目定位、能力边界和二进制安装树使用入口 |
| `LICENSE.txt` | 项目许可证正文 |
| `NOTICE.txt` | 第三方版权与归属声明 |
| `opencode.json` | opencode CLI 工具权限配置 |
| `.gitattributes` | 跨平台文本换行与脚本文件属性 |
| `.gitignore` | 排除构建输出、日志、缓存和本地工具产物 |

---

## `bin/` — 用户入口

| 文件 | 职责 |
|---|---|
| `bin/cucc` | **主入口**：薄 shell 脚本，解析安装根并启动 `tools/cu2cce_driver.py`；不转换源码 |
| `bin/nvcc` | 安装树中的 CUDA 兼容入口，转发到 `bin/cucc`；供 CMake/Make 虚拟工具链使用 |
| `bin/conf.json` | cucc 规则配置：5 种 action（reject/drop/replace/forward/inject）、frontend、capabilities、hostcc 注入 |
| `bin/gnu` | host compiler carrier 入口，实际调用 `tools/hostcc_cce` |
| `bin/gfortran` | host Fortran carrier 入口，实际调用 `tools/gfortran_cce` |

---

## `cmake/` — CMake Package Config

| 文件 | 职责 |
|---|---|
| `cmake/cu2cce-bridgeConfig.cmake` | `find_package(cu2cce-bridge CONFIG)` 的原生 package 入口 |
| `cmake/cu2cce-bridgeConfigVersion.cmake` | package version 兼容性检查 |

---

## `cmake_modules/` — CMake CUDA 兼容模块

| 文件 | 职责 |
|---|---|
| `cmake_modules/FindCUDA.cmake` | legacy `find_package(CUDA)` 兼容：`cuda_add_*`、`cuda_compile`、`cuda_wrap_srcs`；PTX/CUBIN/FATBIN/RDC 不伪造 |
| `cmake_modules/FindCUDAToolkit.cmake` | bridge-backed `CUDA::cudart`/toolkit alias；传播 CANN include/lib；不自动造未验证 math library target |
| `cmake_modules/README.md` | CMake 兼容模块使用边界说明 |
| `cmake_modules/override/CUDAOverrides.cmake` | configure 阶段固定 compiler/toolkit identity，关闭 NVIDIA architecture/runtime injection |
| `cmake_modules/override/README.md` | override 模块使用边界说明 |

---

## `config/` — 契约与 Parity 数据

| 文件 | 职责 |
|---|---|
| `config/artifact_contract.json` | 构建产物契约：每个 case 的 expected files、类型、禁止后缀、NEEDED/RPATH 和 metadata |
| `config/cuda_api_parity_inventory.json` | CUDA API parity inventory：`headers`/`wrappers`/`api_entries`/`libraries` 四维度，是生成式文件族的逐项维护唯一来源 |

### Parity Inventory 说明

`cuda_api_parity_inventory.json` 是自动生成 header/wrapper 的**唯一数据源**：

- `headers` → `include/<CUDA public header>` 的实现路径
- `wrappers` → `include/bridge/<wrapper header>` 的实现路径
- `api_entries` → CUDA API、reference wrapper、CANN 路线、状态与未来实现位置
- `libraries` → CUDA 库到 CANN 路线及是否有真实 wrapper 证据

状态含义：`ACTIVE_ADAPTED`（有真实适配）、`UNSUPPORTED_TYPED`（可调用 stub 返回 NOT_SUPPORT）、`RESERVED_NPU`（`#if 0` 保留）、`DIAGNOSTIC_ONLY`（仅诊断）。

维护命令：
```bash
python3 tools/generate_cuda_header_wrapper_parity.py --reference ../cu-bridge-muxi --implementation . --inventory config/cuda_api_parity_inventory.json
python3 tools/validate_cuda_header_wrapper_parity.py --reference ../cu-bridge-muxi --implementation . --inventory config/cuda_api_parity_inventory.json
```

---

## `include/` — CUDA 兼容头

这是仓库最大的目录（231 个文件），分为四个层级：

1. **顶层公共头** — 用户 `#include` 的 CUDA 头入口
2. **`bridge/`** — bridge 内部 wrapper 头（按库分类）
3. **`crt/`** — CUDA CRT 兼容层
4. **`shadow/`** — shadow libc 兼容头

### 顶层公共头 — 核心入口（ACTIVE）

| 文件 | 职责 |
|---|---|
| `include/cuda_runtime.h` | **用户 CUDA runtime include 入口**；组合 `__cuda_runtime.h` |
| `include/cuda_runtime_api.h` | CUDA runtime API 兼容入口 |
| `include/__cuda_runtime.h` | **内部 runtime 组合入口**：组装 wrapper 声明 + 宏替换表 + ACL 头 |
| `include/cuda_library_placeholder.h` | 未实现 CUDA 高层库头的统一占位基础；不提供虚假 CANN ABI 或链接目标 |
| `include/device_launch_parameters.h` | `<<<>>>` launch 参数兼容入口 |

### 顶层公共头 — 类型与定义（ACTIVE）

| 文件 | 职责 |
|---|---|
| `include/builtin_types.h` | CUDA built-in types 顶层转发入口 |
| `include/vector_types.h` | CUDA vector types（char1..float4），使用 ccec 原生 vector 类型 |
| `include/vector_functions.h` | vector 构造函数转发 |
| `include/vector_functions.hpp` | vector 构造函数 C++ 转发 |
| `include/device_types.h` | device types 转发 bridge Runtime 类型和 SIMT 内建入口 |
| `include/driver_types.h` | driver types：只提供共享类型，不伪造完整 Driver ABI |
| `include/library_types.h` | library types 转发共享 Runtime/库类型 |
| `include/surface_types.h` | surface 类型：仅提供 descriptor/handle 形状 |
| `include/texture_types.h` | texture 类型：仅提供 descriptor/handle 形状 |
| `include/cuda_surface_types.h` | surface_types 兼容转发 |
| `include/cuda_texture_types.h` | texture_types 兼容转发 |
| `include/math_constants.h` | CUDA math constants 兼容 |
| `include/host_defines.h` | host_defines 兼容入口；不重定义 ccec 关键字 |
| `include/host_config.h` | host config 兼容（RESERVED_NPU） |
| `include/cudart_platform.h` | cudart platform 兼容（RESERVED_NPU） |

### 顶层公共头 — 函数转发（ACTIVE）

| 文件 | 职责 |
|---|---|
| `include/device_functions.h` | device_functions 顶层转发入口 |
| `include/common_functions.h` | common_functions 顶层转发入口 |
| `include/math_functions.h` | math_functions 顶层转发入口 |
| `include/device_atomic_functions.h` | device atomic functions 兼容（RESERVED_NPU） |
| `include/device_atomic_functions.hpp` | device atomic functions C++ 兼容（RESERVED_NPU） |
| `include/device_double_functions.h` | device double functions 兼容（RESERVED_NPU） |
| `include/driver_functions.h` | driver functions 兼容（RESERVED_NPU） |
| `include/sm_60_atomic_functions.h` | sm_60 atomic functions 兼容（RESERVED_NPU） |
| `include/surface_functions.h` | surface functions 兼容（RESERVED_NPU） |
| `include/surface_indirect_functions.h` | surface indirect functions 兼容（RESERVED_NPU） |
| `include/texture_fetch_functions.h` | texture fetch functions 兼容（RESERVED_NPU） |
| `include/texture_fetch_functions.hpp` | texture fetch functions C++ 兼容（RESERVED_NPU） |
| `include/texture_indirect_functions.h` | texture indirect functions 兼容（RESERVED_NPU） |
| `include/texture_indirect_functions.hpp` | texture indirect functions C++ 兼容（RESERVED_NPU） |

### 顶层公共头 — 浮点与管线（ACTIVE/RESERVED_NPU）

| 文件 | 职责 |
|---|---|
| `include/cuda_fp16.h` | half-precision 兼容入口（ACTIVE） |
| `include/cuda_fp16.hpp` | half-precision C++ 兼容（ACTIVE） |
| `include/cuda_bf16.h` | brain-float16 兼容入口（ACTIVE） |
| `include/cuda_bf16.hpp` | brain-float16 C++ 兼容（ACTIVE） |
| `include/cuda_fp8.h` | fp8 兼容入口（RESERVED_NPU） |
| `include/cuda_fp8.hpp` | fp8 C++ 兼容（RESERVED_NPU） |
| `include/cuda_pipeline.h` | CUDA pipeline 兼容（RESERVED_NPU） |
| `include/cuda_pipeline_helpers.h` | pipeline helpers 兼容（RESERVED_NPU） |
| `include/cuda_pipeline_primitives.h` | pipeline primitives 兼容（RESERVED_NPU） |
| `include/cuda_awbarrier.h` | arrived wait barrier 兼容（RESERVED_NPU） |
| `include/cuda_awbarrier_helpers.h` | awbarrier helpers 兼容（RESERVED_NPU） |
| `include/cuda_awbarrier_primitives.h` | awbarrier primitives 兼容（RESERVED_NPU） |
| `include/mma.h` | tensor core mma 兼容（RESERVED_NPU） |
| `include/cuda_occupancy.h` | occupancy 计算兼容（RESERVED_NPU） |
| `include/cudaTypedefs.h` | CUDA 统一 typedefs 兼容（RESERVED_NPU） |

### 顶层公共头 — Profiler 与 NVTX

| 文件 | 职责 |
|---|---|
| `include/cuda_profiler_api.h` | CUDA profiler API 兼容入口（ACTIVE） |
| `include/cudaProfiler.h` | profiler 兼容（RESERVED_NPU） |
| `include/cudaProfilerTypedefs.h` | profiler typedefs 兼容（RESERVED_NPU） |
| `include/nvToolsExt.h` | NVTX 兼容入口（ACTIVE） |
| `include/nvToolsExtCuda.h` | NVTX CUDA 扩展兼容（RESERVED_NPU） |
| `include/nvToolsExtCudaRt.h` | NVTX CUDA Runtime 扩展兼容（RESERVED_NPU） |
| `include/nvToolsExtOpenCL.h` | NVTX OpenCL 扩展兼容（RESERVED_NPU） |
| `include/nvToolsExtSync.h` | NVTX sync 扩展兼容（RESERVED_NPU） |
| `include/nvfunctional` | CUDA functional 兼容（RESERVED_NPU） |

### 顶层公共头 — Driver 与 Runtime 扩展

| 文件 | 职责 |
|---|---|
| `include/cuda.h` | CUDA Driver 兼容入口（RESERVED_NPU） |
| `include/nvrtc.h` | NVRTC 兼容入口（RESERVED_NPU） |
| `include/cuda_device_api.h` | device-side SIMT/warp/vote/reduce/atomic 集中映射（ACTIVE） |
| `include/cuda_device_runtime_api.h` | device runtime API 兼容（RESERVED_NPU） |
| `include/fatbinary.h` | fatbinary 兼容（RESERVED_NPU） |
| `include/fatbinary_section.h` | fatbinary section 兼容（RESERVED_NPU） |
| `include/fatBinaryCtl.h` | fatbinary control 兼容（RESERVED_NPU） |
| `include/nv_decode.h` | NV decode 兼容（RESERVED_NPU） |
| `include/cudalibxt.h` | cuBLASXt 库兼容（RESERVED_NPU） |
| `include/cuComplex.h` | cuComplex 复数运算兼容（RESERVED_NPU） |

### 顶层公共头 — 图形互操作

| 文件 | 职责 |
|---|---|
| `include/cudaEGL.h` | EGL 互操作兼容（RESERVED_NPU） |
| `include/cuda_egl_interop.h` | EGL interop 兼容（RESERVED_NPU） |
| `include/cudaEGLTypedefs.h` | EGL typedefs 兼容（RESERVED_NPU） |
| `include/cudaGL.h` | OpenGL 互操作兼容（RESERVED_NPU） |
| `include/cuda_gl_interop.h` | GL interop 兼容（RESERVED_NPU） |
| `include/cudaGLTypedefs.h` | GL typedefs 兼容（RESERVED_NPU） |
| `include/cudaVDPAU.h` | VDPAU 互操作兼容（RESERVED_NPU） |
| `include/cuda_vdpau_interop.h` | VDPAU interop 兼容（RESERVED_NPU） |
| `include/cudaVDPAUTypedefs.h` | VDPAU typedefs 兼容（RESERVED_NPU） |

### 顶层公共头 — CUDA 库入口

以下顶层头 `#include <cuda_library_placeholder.h>` + 对应 `bridge/<lib>/<wrapper>.h`，
本身不含 API 声明，只做路由。状态由 parity inventory 管理。

| 文件 | 对应 bridge wrapper | 状态 |
|---|---|---|
| `include/cublas.h` | `bridge/blas/cublas_wrapper.h` | EXPERIMENTAL |
| `include/cublas_v2.h` | `bridge/blas/cublas_v2_wrapper.h` | EXPERIMENTAL (ACTIVE) |
| `include/cublas_api.h` | `bridge/blas/cublas_api_wrapper.h` | RESERVED_NPU |
| `include/cublasLt.h` | `bridge/blas/cublasLt_wrapper.h` | RESERVED_NPU |
| `include/cublasXt.h` | `bridge/blas/cublasXt_wrapper.h` | RESERVED_NPU |
| `include/cudnn.h` | `bridge/dnn/cudnn_wrapper.h` | RESERVED_NPU |
| `include/cudnn_version.h` | `bridge/dnn/cudnn_wrapper.h` | RESERVED_NPU |
| `include/cudnn_adv_infer.h` | `bridge/dnn/cudnn_adv_infer_wrapper.h` | RESERVED_NPU |
| `include/cudnn_adv_train.h` | `bridge/dnn/cudnn_adv_train_wrapper.h` | RESERVED_NPU |
| `include/cudnn_backend.h` | `bridge/dnn/cudnn_backend_wrapper.h` | RESERVED_NPU |
| `include/cudnn_cnn_infer.h` | `bridge/dnn/cudnn_cnn_infer_wrapper.h` | RESERVED_NPU |
| `include/cudnn_cnn_train.h` | `bridge/dnn/cudnn_cnn_train_wrapper.h` | RESERVED_NPU |
| `include/cudnn_ops_infer.h` | `bridge/dnn/cudnn_ops_infer_wrapper.h` | RESERVED_NPU |
| `include/cudnn_ops_train.h` | `bridge/dnn/cudnn_ops_train_wrapper.h` | RESERVED_NPU |
| `include/cufft.h` | `bridge/fft/cufft_wrapper.h` | RESERVED_NPU |
| `include/cufftw.h` | `bridge/fft/cufft_wrapper.h` | RESERVED_NPU |
| `include/cufftXt.h` | `bridge/fft/cufftXt_wrapper.h` | RESERVED_NPU |
| `include/curand.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_kernel.h` | `bridge/rand/curand_kernel_wrapper.h` | RESERVED_NPU |
| `include/curand_discrete.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_discrete2.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_globals.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_lognormal.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_mrg32k3a.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_mtgp32.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_mtgp32_host.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_mtgp32_kernel.h` | `bridge/rand/curand_kernel_wrapper.h` | RESERVED_NPU |
| `include/curand_mtgp32dc_p_11213.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_normal.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_normal_static.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_philox4x32_x.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_poisson.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_precalc.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/curand_uniform.h` | `bridge/rand/curand_wrapper.h` | RESERVED_NPU |
| `include/cusolver_common.h` | `bridge/solver/cusolver_common_wrapper.h` | RESERVED_NPU |
| `include/cusolverDn.h` | `bridge/solver/cusolverDn_wrapper.h` | STUB |
| `include/cusolverMg.h` | `bridge/solver/cusolverMg_wrapper.h` | RESERVED_NPU |
| `include/cusolverRf.h` | `bridge/solver/cusolverRf_wrapper.h` | RESERVED_NPU |
| `include/cusolverSp.h` | `bridge/solver/cusolverSp_wrapper.h` | RESERVED_NPU |
| `include/cusolverSp_LOWLEVEL_PREVIEW.h` | `bridge/solver/cusolverSp_lowlevel_wrapper.h` | RESERVED_NPU |
| `include/cusparse.h` | `bridge/sparse/cusparse_wrapper.h` | RESERVED_NPU |
| `include/cusparse_v2.h` | `bridge/sparse/cusparse_wrapper.h` | RESERVED_NPU |
| `include/npp.h` | `bridge/mpp/npp_wrapper.h` | RESERVED_NPU |
| `include/nppcore.h` | `bridge/mpp/nppcore_wrapper.h` | RESERVED_NPU |
| `include/nppdefs.h` | `bridge/mpp/nppdefs_wrapper.h` | RESERVED_NPU |
| `include/nppi.h` | `bridge/mpp/nppi_wrapper.h` | RESERVED_NPU |
| `include/nppi_arithmetic_and_logical_operations.h` | `bridge/mpp/nppi_wrapper.h` | RESERVED_NPU |
| `include/nppi_color_conversion.h` | `bridge/mpp/nppi_wrapper.h` | RESERVED_NPU |
| `include/nppi_data_exchange_and_initialization.h` | `bridge/mpp/nppi_wrapper.h` | RESERVED_NPU |
| `include/nppi_filtering_functions.h` | `bridge/mpp/nppi_wrapper.h` | RESERVED_NPU |
| `include/nppi_geometry_transforms.h` | `bridge/mpp/nppi_wrapper.h` | RESERVED_NPU |
| `include/nppi_linear_transforms.h` | `bridge/mpp/nppi_wrapper.h` | RESERVED_NPU |
| `include/nppi_morphological_operations.h` | `bridge/mpp/nppi_wrapper.h` | RESERVED_NPU |
| `include/nppi_statistics_functions.h` | `bridge/mpp/nppi_wrapper.h` | RESERVED_NPU |
| `include/nppi_support_functions.h` | `bridge/mpp/nppi_wrapper.h` | RESERVED_NPU |
| `include/nppi_threshold_and_compare_operations.h` | `bridge/mpp/nppi_wrapper.h` | RESERVED_NPU |
| `include/npps.h` | `bridge/mpp/npps_wrapper.h` | RESERVED_NPU |
| `include/npps_arithmetic_and_logical_operations.h` | `bridge/mpp/npps_wrapper.h` | RESERVED_NPU |
| `include/npps_conversion_functions.h` | `bridge/mpp/npps_wrapper.h` | RESERVED_NPU |
| `include/npps_filtering_functions.h` | `bridge/mpp/npps_wrapper.h` | RESERVED_NPU |
| `include/npps_initialization.h` | `bridge/mpp/npps_wrapper.h` | RESERVED_NPU |
| `include/npps_statistics_functions.h` | `bridge/mpp/npps_wrapper.h` | RESERVED_NPU |
| `include/npps_support_functions.h` | `bridge/mpp/npps_wrapper.h` | RESERVED_NPU |
| `include/nvjpeg.h` | `bridge/jpeg/nvjpeg_wrapper.h` | RESERVED_NPU |
| `include/nvml.h` | `bridge/sml/nvml_wrapper.h` | RESERVED_NPU |
| `include/nccl.h` | (placeholder only) | RESERVED_NPU |
| `include/cufile.h` | (placeholder only) | RESERVED_NPU |

### `include/cooperative_groups/` — Cooperative Groups

| 文件 | 职责 |
|---|---|
| `include/cooperative_groups.h` | cooperative groups 兼容入口（ACTIVE） |
| `include/cooperative_groups/reduce.h` | cooperative groups reduce 兼容（RESERVED_NPU） |

### `include/nvtx3/` — NVTX3 转发

| 文件 | 职责 |
|---|---|
| `include/nvtx3/nvToolsExt.h` | NVTX3 兼容转发到上层 `nvToolsExt.h` |

---

### `include/bridge/kernel/` — SIMT 原语适配（ACTIVE）

| 文件 | 职责 |
|---|---|
| `include/bridge/kernel/simt_adaptor.h` | **核心 SIMT 原语映射**：`__shared__`→`__ubuf__`、`__syncthreads`→`asc_syncthreads`、CUDA atomics/warp collectives→Ascend SIMT API |
| `include/bridge/kernel/cuda_device_api.h` | device-side warp/vote/reduce/extended atomic 到 CANN `simt_api` 的集中映射 |

### `include/bridge/runtime/` — CUDA Runtime/Driver Wrapper

#### ACTIVE — 有真实实现

| 文件 | 职责 |
|---|---|
| `cuda_runtime_wrapper.h` | **wcuda* 函数声明**：init、device、context、memory、stream、event、memcpy、kernel launch carrier |
| `cuda_to_cce_mcr_adaptor.h` | **核心宏替换表**：`cuda*`→`wcuda*`、CUDA types→ACL types、`cudaError` 枚举 |
| `cuda_wrapper_internal.h` | bridge_init 内部符号 |
| `cuda_wrapper_types.h` | `_cceDeviceProp_t` 和自定义类型 |
| `cuda_runtime_types.h` | CUDA Runtime extent/pitch/pos/pointer/array/descriptor 兼容类型 |
| `cuda_driver_types.h` | CUDA driver 类型兼容 |
| `cuda_benchmark_targets.h` | benchmark target 声明 |
| `cuda_to_cce_reserved_parity.h` | 从 cu-bridge 生成的 RESERVED_NPU CUDA 替换表 inventory；默认 `#if 0` |

#### RESERVED_NPU — parity 占位

| 文件 | 职责 |
|---|---|
| `cuda_driver_wrapper.h` | CUDA Driver wrapper（RESERVED_NPU） |
| `cuda_driver_wrapper_typedef.h` | Driver wrapper typedefs（RESERVED_NPU） |
| `cuda_driver_template_wrapper.h` | Driver template wrapper（RESERVED_NPU） |
| `cuda_runtime_template_wrapper.h` | Runtime template wrapper（RESERVED_NPU） |
| `cuda_channel_descriptor_wrapper.h` | channel descriptor wrapper（RESERVED_NPU） |
| `cuda_egl_wrapper.h` | EGL interop wrapper（RESERVED_NPU） |
| `cuda_egl_wrapper_typedef.h` | EGL wrapper typedefs（RESERVED_NPU） |
| `cuda_gl_wrapper.h` | GL interop wrapper（RESERVED_NPU） |
| `cuda_gl_wrapper_typedef.h` | GL wrapper typedefs（RESERVED_NPU） |
| `cuda_pfn_to_cce_adaptor.h` | function pointer adaptor（RESERVED_NPU） |
| `cuda_to_cce_rtc_adaptor.h` | RTC adaptor（RESERVED_NPU） |
| `cuda_wrapper_internal_types.h` | wrapper internal types（RESERVED_NPU） |
| `nvrtc_wrapper.h` | NVRTC wrapper（RESERVED_NPU） |

### `include/bridge/blas/` — cuBLAS Wrapper

| 文件 | 职责 |
|---|---|
| `cublas_v2_wrapper.h` | **cuBLAS v2 wrapper 声明**（EXPERIMENTAL ACTIVE） |
| `cublas_wrapper.h` | cuBLAS wrapper（RESERVED_NPU） |
| `cublas_api_wrapper.h` | cuBLAS API wrapper（RESERVED_NPU） |
| `cublasLt_wrapper.h` | cuBLASLt wrapper（RESERVED_NPU） |
| `cublasLt_to_cce_adaptor.h` | cuBLASLt adaptor（RESERVED_NPU） |
| `cublasXt_wrapper.h` | cuBLASXt wrapper（RESERVED_NPU） |
| `cublasXt_to_cce_adaptor.h` | cuBLASXt adaptor（RESERVED_NPU） |

### `include/bridge/dnn/` — cuDNN Wrapper（全部 RESERVED_NPU）

| 文件 | 职责 |
|---|---|
| `cudnn_wrapper.h` | cuDNN 主 wrapper parity 占位 |
| `cudnn_backend_wrapper.h` | cuDNN backend wrapper |
| `cudnn_adv_infer_wrapper.h` | cuDNN adv inference wrapper |
| `cudnn_adv_train_wrapper.h` | cuDNN adv training wrapper |
| `cudnn_cnn_infer_wrapper.h` | cuDNN CNN inference wrapper |
| `cudnn_cnn_train_wrapper.h` | cuDNN CNN training wrapper |
| `cudnn_ops_infer_wrapper.h` | cuDNN ops inference wrapper |
| `cudnn_ops_train_wrapper.h` | cuDNN ops training wrapper |

### `include/bridge/fft/` — cuFFT Wrapper（全部 RESERVED_NPU）

| 文件 | 职责 |
|---|---|
| `cufft_wrapper.h` | cuFFT wrapper parity 占位 |
| `cufftXt_wrapper.h` | cuFFT Xt wrapper |
| `cufft_to_cce_adaptor.h` | cuFFT adaptor |
| `cufftXt_to_cce_adaptor.h` | cuFFT Xt adaptor |

### `include/bridge/rand/` — cuRAND Wrapper（全部 RESERVED_NPU）

| 文件 | 职责 |
|---|---|
| `curand_wrapper.h` | cuRAND host wrapper parity 占位 |
| `curand_kernel_wrapper.h` | cuRAND device kernel wrapper |
| `curand_to_cce_adaptor.h` | cuRAND adaptor |

### `include/bridge/solver/` — cuSOLVER Wrapper

| 文件 | 职责 |
|---|---|
| `cusolverDn_wrapper.h` | **cusolverDn compile-time stub**（STUB）：API 声明存在，运行时返回 `CUSOLVER_STATUS_NOT_SUPPORTED` |
| `cusolver_common_wrapper.h` | cusolver 公共类型 wrapper（RESERVED_NPU） |
| `cusolverSp_wrapper.h` | cuSOLVER Sp wrapper（RESERVED_NPU） |
| `cusolverSp_lowlevel_wrapper.h` | cuSOLVER Sp low-level wrapper（RESERVED_NPU） |
| `cusolverMg_wrapper.h` | cuSOLVER Mg wrapper（RESERVED_NPU） |
| `cusolverRf_wrapper.h` | cuSOLVER Rf wrapper（RESERVED_NPU） |

### `include/bridge/sparse/` — cuSPARSE Wrapper（RESERVED_NPU）

| 文件 | 职责 |
|---|---|
| `cusparse_wrapper.h` | cuSPARSE wrapper parity 占位 |

### `include/bridge/mpp/` — NPP→MPP Wrapper（全部 RESERVED_NPU）

| 文件 | 职责 |
|---|---|
| `npp_wrapper.h` | NPP 主 wrapper parity 占位 |
| `nppcore_wrapper.h` | NPP core wrapper |
| `nppdefs_wrapper.h` | NPP defs wrapper |
| `nppi_wrapper.h` | NPP image processing wrapper |
| `npps_wrapper.h` | NPP signal processing wrapper |
| `npp2mpp_functions.h` | NPP→MPP 函数映射 |

### `include/bridge/pti/` — CUPTI/NVPerf Wrapper（全部 RESERVED_NPU）

| 文件 | 职责 |
|---|---|
| `cupti_wrapper.h` | CUPTI wrapper parity 占位 |
| `nvperf_common_wrapper.h` | NVPerf common wrapper |
| `nvperf_cuda_host_wrapper.h` | NVPerf CUDA host wrapper |
| `nvperf_host_wrapper.h` | NVPerf host wrapper |
| `nvperf_target_wrapper.h` | NVPerf target wrapper |

### `include/bridge/jpeg/` — nvJPEG Wrapper（RESERVED_NPU）

| 文件 | 职责 |
|---|---|
| `nvjpeg_wrapper.h` | nvJPEG wrapper parity 占位 |

### `include/bridge/sml/` — NVML Wrapper（RESERVED_NPU）

| 文件 | 职责 |
|---|---|
| `nvml_wrapper.h` | NVML wrapper parity 占位 |

### `include/bridge/mckl/` — Device Runtime Wrapper（RESERVED_NPU）

| 文件 | 职责 |
|---|---|
| `cuda_device_runtime_wrapper.h` | device runtime cooperative groups wrapper parity 占位 |

### `include/bridge/tools_ext/` — NVTX Wrapper

| 文件 | 职责 |
|---|---|
| `nvtx_stub.h` | NVTX stub 实现（ACTIVE） |
| `nvtx_wrapper.h` | NVTX wrapper（RESERVED_NPU） |
| `nvtx_to_cce_adaptor.h` | NVTX adaptor（RESERVED_NPU） |

### `include/bridge/video/` — VDPAU Interop Wrapper（全部 RESERVED_NPU）

| 文件 | 职责 |
|---|---|
| `cuda_vdpau_wrapper.h` | VDPAU wrapper parity 占位 |
| `cuda_vdpau_interop_wrapper.h` | VDPAU interop wrapper |
| `cuda_vdpau_typedefs_wrapper.h` | VDPAU typedefs wrapper |

### `include/crt/` — CUDA CRT 兼容层

| 文件 | 职责 |
|---|---|
| `include/crt/builtin_types.h` | CUDA built-in types CRT 兼容 |
| `include/crt/common_functions.h` | 工具函数（min/max/bitwise），forward-only |
| `include/crt/device_functions.h` | device function 声明，forward-only |
| `include/crt/math_functions.h` | math 函数 forward-only，不伪造 device math |
| `include/crt/host_defines.h` | 文档-only，不重定义 `__host__`/`__device__` 宏 |
| `include/crt/vector_functions_compat.h` | `make_*` vector 构造函数（guarded） |
| `include/crt/vector_types.h` | CUDA vector types（char1..float4） |

### `include/shadow/` — Shadow libc

避免 glibc 2.39 / ccec 不兼容的 shadow libc 头，只在 cucc shadow 模式生效：

| 文件 | 职责 |
|---|---|
| `include/shadow/math.h` | shadow math.h |
| `include/shadow/stdio.h` | shadow stdio.h |
| `include/shadow/stdlib.h` | shadow stdlib.h |
| `include/shadow/string.h` | shadow string.h |

---

## `src/` — Bridge 实现源码

| 文件 | 职责 |
|---|---|
| `src/bridge/runtime/src/cuda_runtime_wrapper.cpp` | **正式 runtime bridge 实现**：`cuda*`→`wcuda*`→`aclrt*`，负责 init、device/context、memory、stream/event、memcpy、kernel launch carrier、错误状态 |
| `src/bridge/runtime/src/cuda_driver_wrapper.cpp` | CUDA Driver typed placeholder；返回 `ACL_ERROR_API_NOT_SUPPORT`，不伪造 Driver ABI |
| `src/bridge/blas/src/cublas_v2_wrapper.cpp` | **experimental** `cublas*`→`aclblas*` 实现；不属于 puresimt blocking 主线 |
| `src/bridge/solver/src/cusolverDn_wrapper.cpp` | cusolverDn compile-time stub 实现；全部返回 `CUSOLVER_STATUS_NOT_SUPPORTED` |

---

## `tests/` — 测试套件

| 文件 | 职责 |
|---|---|
| `tests/test_cucc_rules.py` | cucc 参数规则、query/options/response、架构、host flags、library policy、phase 和 trace 单元测试 |
| `tests/test_cmake_compat.py` | CMake/Make/Ninja/pre wrapper、虚拟工具链生命周期、legacy module、参数边界静态测试 |
| `tests/test_cuda_api_parity.py` | CUDA Runtime/device parity inventory、typed placeholder、状态 inventory 与 active wrapper 声明契约测试 |
| `tests/test_cuda_header_wrapper_parity.py` | CUDA 公共头、wrapper、API inventory、普通文件与服务器回归接线契约测试 |
| `tests/test_artifacts.py` | artifact contract、metadata、ELF 依赖、NEEDED/RPATH 和 server harness 测试 |
| `tests/test_blas_surface.py` | experimental BLAS wrapper surface 测试 |
| `tests/test_cann_api_probes.py` | CANN 官方头文件与 Runtime 链接探针测试 |
| `tests/test_repo_hygiene.py` | 仓库目录、文档入口、gitignore 和 tracked tree 卫生契约测试 |
| `tests/header_wrapper_smoke.cpp` | CUDA Runtime/Driver/库/Interop 公共头编译 smoke 测试 |
| `tests/host_bridge_smoke.cu` | host bridge 编译 smoke 测试 |
| `tests/mock_acl/syntax_check.sh` | mock ACL 头语法检查脚本（macOS 本地可运行） |
| `tests/mock_acl/acl/acl.h` | mock ACL 主头 |
| `tests/mock_acl/acl/acl_base.h` | mock ACL base 头 |
| `tests/mock_acl/acl/acl_rt.h` | mock ACL runtime 头 |
| `tests/mock_acl/acl/ops/acl_cblas.h` | mock ACL cblas ops 头 |

---

## `tools/` — 构建工具与真机回归

### 顶层工具

| 文件 | 职责 |
|---|---|
| `tools/cu2cce_driver.py` | **cucc 规则驱动核心**：参数适配 nvcc→ccec，同时被 CMake mock_nvcc.py 复用 |
| `tools/cmake_cce` | CMake configure/build wrapper：configure 用 mock nvcc，build 用 cucc |
| `tools/make_cce` | Make wrapper：虚拟工具链 + 环境准备后执行系统 make |
| `tools/ninja_cce` | Ninja wrapper：build tree 虚拟工具链恢复后执行系统 ninja |
| `tools/pre_cce` | autotools/configure 环境准备 wrapper |
| `tools/hostcc_cce` | host compiler wrapper：纯 host compile 透传，link 注入 bridge/CANN runtime |
| `tools/gfortran_cce` | host Fortran wrapper：拒绝 CUDA source 误用 |
| `tools/mpi_cce` | MPI wrapper：host-only，拒绝 CUDA source |
| `tools/device_arch.sh` | Ascend `dav-*` 架构解析（env→npu-smi→cache→default） |
| `tools/virtual_toolkit_env.sh` | CMake/Make/Ninja/pre 共用的虚拟工具链环境 helper |
| `tools/compare_cubridge_parity.py` | 与 cu-bridge reference 的 backend-aware parity 报告工具 |
| `tools/validate_cucc_rules.py` | cucc JSON 规则 schema/冲突校验器 |
| `tools/validate_cuda_api_parity.py` | 校验 cu-bridge 条目在 active 或 RESERVED_NPU inventory 中均有归宿 |
| `tools/validate_cuda_header_wrapper_parity.py` | 校验参考公共 CUDA headers/wrappers/API 在当前文件或 inventory 中有归宿 |
| `tools/generate_cuda_api_parity_inventory.py` | 从参考 adaptor 生成 RESERVED_NPU CUDA 条目 inventory |
| `tools/generate_cuda_header_wrapper_parity.py` | 生成公共 CUDA header/wrapper 的占位文件、状态清单与 reserved inventory |
| `tools/link.json` | Make/pre 使用的虚拟 CUDA 工具链布局清单 |

### `tools/mock/` — CMake nvcc Mock

| 文件 | 职责 |
|---|---|
| `tools/mock/mock_nvcc.py` | CMake version/ABI/identity 探针 mock：回答 `--version`、`-v`、preprocess 探针 |
| `tools/mock/nvcc_entry.sh` | CMake configure 阶段的 mock nvcc 入口脚本 |
| `tools/mock/virtualDir.py` | 虚拟 CUDA 工具链目录构建器：读取 link.json/stub_link.json 幂等创建 CUDA-shaped tree |
| `tools/mock/stub_link.json` | CMake configure 使用的 virtual_cmake 布局清单 |
| `tools/mock/CMakeCUDACompilerABI.cpp` | CMake ABI mock 辅助源码 |
| `tools/mock/CMakeCompilerABI.h` | CMake ABI mock 辅助头 |

### `tools/real_test/` — 真机回归

| 文件 | 职责 |
|---|---|
| `tools/real_test/server_cmd.txt` | **唯一 Ascend 真机回归入口**（fast/external/full profiles） |
| `tools/real_test/build_bridge.sh` | 在 CANN 主机上构建并安装 bridge binary tree |
| `tools/real_test/clean.sh` | 清理真机回归构建、安装树和日志 |
| `tools/real_test/env_probe.sh` | 只读 CANN/NPU/ccec 环境探测 |
| `tools/real_test/probe_device.cpp` | 设备探测 C++ 源码 |
| `tools/real_test/verify_artifacts.py` | 真实构建产物类型、依赖、RPATH 和 metadata 检查器 |
| `tools/real_test/inventory_simt_api.sh` | 调查已安装 CANN SIMT API 头文件 |
| `tools/real_test/fetch_external_cuda_samples.sh` | 真机 fetch pinned NVIDIA CUDA Samples |
| `tools/real_test/validate_fetch_url.py` | 验证 fetch URL 安全性 |
| `tools/real_test/override_probe.sh` | CMake override 探测 |
| `tools/real_test/blas_regression.sh` | BLAS experimental 回归脚本 |
| `tools/real_test/README.md` | 真机回归辅助说明 |

### `tools/real_test/diag/` — 诊断资产（DIAGNOSTIC_ONLY）

| 文件 | 职责 |
|---|---|
| `tools/real_test/diag/README.md` | 诊断资产说明 |
| `tools/real_test/diag/acl_alloc_probe.cpp` | ACL 内存分配探针 |
| `tools/real_test/diag/acl_compose_probe.cpp` | ACL compose 探针 |
| `tools/real_test/diag/acl_diff_probe.cpp` | ACL diff 探针 |
| `tools/real_test/diag/echo.cce` | 最小 echo .cce 源码 |
| `tools/real_test/diag/trunc_raw.c` | trunc raw 探针 C 源码 |
| `tools/real_test/diag/cann_api_surface_probe.sh` | CANN 官方 Runtime/SIMT/HCCL 头文件存在性和编译探针 |
| `tools/real_test/diag/cann_runtime_link_probe.sh` | CANN Runtime 库选择、最小链接、NEEDED 和 RPATH 探针 |
| `tools/real_test/diag/cuda_header_surface_probe.sh` | CANN 主机上核心/库/Interop/NVRTC/device header include/compile surface 检查 |
| `tools/real_test/diag/run_acl_probe.sh` | ACL 探针运行脚本 |
| `tools/real_test/diag/run_acl_compose_probe.sh` | ACL compose 探针运行脚本 |
| `tools/real_test/diag/run_acl_diff_probe.sh` | ACL diff 探针运行脚本 |
| `tools/real_test/diag/run_iterate_malloc.sh` | 迭代 malloc 探针运行脚本 |
| `tools/real_test/diag/run_minimal_echo.sh` | 最小 echo 运行脚本 |
| `tools/real_test/diag/run_trunc_raw.sh` | trunc raw 运行脚本 |

---

## `examples/` — 示例工程

### `examples/puresimt/` — 真机验证 CUDA 测试用例（主线）

| 文件 | 职责 |
|---|---|
| `examples/puresimt/README.md` | puresimt 示例使用说明 |
| `examples/puresimt/vectorAdd_minimal.cu` | 最小 vectorAdd |
| `examples/puresimt/vectorAdd/vectorAdd.cu` | 完整 vectorAdd（NVIDIA sample 风格） |
| `examples/puresimt/vectorAdd/README.md` | vectorAdd 说明 |
| `examples/puresimt/matrixMul_minimal.cu` | 最小矩阵乘法 |
| `examples/puresimt/matrixMul/matrixMul.cu` | 完整 matrixMul（NVIDIA sample 风格） |
| `examples/puresimt/matrixMul/README.md` | matrixMul 说明 |
| `examples/puresimt/atomicAdd_minimal.cu` | 全局内存 atomicAdd |
| `examples/puresimt/atomicIndex_minimal.cu` | 数组索引 atomicAdd |
| `examples/puresimt/atomicVariants_minimal.cu` | 多种 atomic 变体 |
| `examples/puresimt/warpShuffle_minimal.cu` | warp shuffle |
| `examples/puresimt/vote_minimal.cu` | warp vote |
| `examples/puresimt/reduction_minimal.cu` | parallel reduction |
| `examples/puresimt/scan_minimal.cu` | prefix scan |
| `examples/puresimt/transpose_minimal.cu` | matrix transpose（shared memory） |
| `examples/puresimt/dynamicShared_minimal.cu` | 动态 shared memory |
| `examples/puresimt/stencil_minimal.cu` | stencil 计算 |
| `examples/puresimt/threadFence_minimal.cu` | thread fence |
| `examples/puresimt/event_minimal.cu` | CUDA event |
| `examples/puresimt/multiKernel_minimal.cu` | 多 kernel 顺序执行 |
| `examples/puresimt/pitchedMemcpy_minimal.cu` | pitched memcpy |
| `examples/puresimt/index3D_minimal.cu` | 3D 索引 |
| `examples/puresimt/half_minimal.cu` | half precision |
| `examples/puresimt/bf16x2_minimal.cu` | bfloat16x2 |
| `examples/puresimt/dot_minimal.cu` | dot product |
| `examples/puresimt/core_ops_minimal.cu` | 核心运算 |
| `examples/puresimt/math_minimal.cu` | device math |
| `examples/puresimt/device_math_minimal.cu` | device math 扩展 |
| `examples/puresimt/double_policy_diag.cu` | FP64 policy 诊断 |
| `examples/puresimt/async_repro_diag.cu` | async 复现诊断（DIAGNOSTIC_ONLY） |

### `examples/blas/` — cuBLAS Experimental

| 文件 | 职责 |
|---|---|
| `examples/blas/README.md` | BLAS 示例说明 |
| `examples/blas/cublas_sgemm.cu` | cuBLAS sgemm |
| `examples/blas/cublas_gemmex.cu` | cuBLAS gemmEx |
| `examples/blas/cublas_gemv.cu` | cuBLAS gemv |
| `examples/blas/gemv_param_probe.cu` | gemv 参数探针 |

### `examples/build_compat/` — 构建兼容载体

| 文件 | 职责 |
|---|---|
| `examples/build_compat/README.md` | 构建兼容说明 |
| `examples/build_compat/cmake/CMakeLists.txt` | CMake Makefiles/Ninja `.o/.a/.so/ELF` carrier |
| `examples/build_compat/cmake/README.md` | CMake 构建兼容说明 |
| `examples/build_compat/cmake/kernel_a.cu` | kernel A 源码 |
| `examples/build_compat/cmake/kernel_b.cu` | kernel B 源码 |
| `examples/build_compat/cmake/main.cpp` | host main |
| `examples/build_compat/make/Makefile` | Makefile nvcc 拦截 carrier |
| `examples/build_compat/make/README.md` | Make 构建兼容说明 |
| `examples/build_compat/make/kernel_a.cu` | kernel A 源码 |
| `examples/build_compat/make/kernel_b.cu` | kernel B 源码 |
| `examples/build_compat/make/main.cpp` | host main |
| `examples/build_compat/make_gxx_link/Makefile` | plain g++ + `-lcudart` bridge carrier |
| `examples/build_compat/make_gxx_link/kernel.cu` | kernel 源码 |
| `examples/build_compat/make_gxx_link/main.cpp` | host main |

### `examples/cmake_compat/` — 多 .cu CMake 兼容

| 文件 | 职责 |
|---|---|
| `examples/cmake_compat/CMakeLists.txt` | multi-.cu、static/shared library、`CUDA::cudart` |
| `examples/cmake_compat/README.md` | 说明 |
| `examples/cmake_compat/compat_api.h` | 兼容 API 头 |
| `examples/cmake_compat/main.cpp` | host main |
| `examples/cmake_compat/shared_kernel.cu` | shared library kernel |
| `examples/cmake_compat/static_kernel.cu` | static library kernel |

### `examples/cmake_compat_legacy/` — Legacy FindCUDA

| 文件 | 职责 |
|---|---|
| `examples/cmake_compat_legacy/CMakeLists.txt` | legacy `FindCUDA`/`cuda_add_executable` |
| `examples/cmake_compat_legacy/README.md` | 说明 |
| `examples/cmake_compat_legacy/legacy_vector_add.cu` | legacy vector add |

### `examples/cmake_demo/` — 最小 CMake CUDA

| 文件 | 职责 |
|---|---|
| `examples/cmake_demo/CMakeLists.txt` | 最小现代 CUDA CMake executable |
| `examples/cmake_demo/README.md` | 说明 |
| `examples/cmake_demo/vector_add.cu` | vector add |

### `examples/external/` — 外部 NVIDIA CUDA Samples

| 文件 | 职责 |
|---|---|
| `examples/external/cuda_samples_project/CMakeLists.txt` | pinned NVIDIA CUDA Samples CMake profile |
| `examples/external/cuda_samples_project/README.md` | 说明 |
| `examples/external/cuda_samples_project/profile_core.json` | core profile 配置 |

### `examples/large_project/` — 多目录 CMake 项目

| 文件 | 职责 |
|---|---|
| `examples/large_project/CMakeLists.txt` | 多目录、跨 target dependency、install/export |
| `examples/large_project/README.md` | 说明 |
| `examples/large_project/app/main.cpp` | host app main |
| `examples/large_project/comp/CMakeLists.txt` | component CMake |
| `examples/large_project/comp/kernel.cu` | kernel 源码 |
| `examples/large_project/comp/kernel.h` | kernel 头 |
| `examples/large_project/util/CMakeLists.txt` | util CMake |
| `examples/large_project/util/data.h` | data 头 |
| `examples/large_project/util/gen_data.cpp` | 数据生成 |
| `examples/large_project/util/store.cpp` | 数据存储 |
| `examples/large_project/util/store.h` | 存储头 |

### `examples/multi_cu_diagnostic/` — 多 .cu 链接诊断

| 文件 | 职责 |
|---|---|
| `examples/multi_cu_diagnostic/CMakeLists.txt` | 多 .cu 聚合 Ascend duplicate registration 诊断 |
| `examples/multi_cu_diagnostic/README.md` | 说明 |
| `examples/multi_cu_diagnostic/kernel_a.cu` | kernel A |
| `examples/multi_cu_diagnostic/kernel_b.cu` | kernel B |
| `examples/multi_cu_diagnostic/main.cpp` | host main |

### `examples/README.md`

| 文件 | 职责 |
|---|---|
| `examples/README.md` | 示例总导航 |

---

## `third_party/` — NVIDIA CUDA Samples Helper

| 文件 | 职责 |
|---|---|
| `third_party/nvidia_cuda_samples/README.md` | helper 头说明 |
| `third_party/nvidia_cuda_samples/Common/exception.h` | 异常处理 helper |
| `third_party/nvidia_cuda_samples/Common/helper_cuda.h` | CUDA device helper |
| `third_party/nvidia_cuda_samples/Common/helper_functions.h` | 通用 helper 函数 |
| `third_party/nvidia_cuda_samples/Common/helper_image.h` | 图像处理 helper |
| `third_party/nvidia_cuda_samples/Common/helper_string.h` | 字符串 helper |
| `third_party/nvidia_cuda_samples/Common/helper_timer.h` | 计时器 helper |

> 这些是未修改的官方 NVIDIA helper 头，只作为 include 依赖，不代表完整 CUDA Toolkit。

---

## `docs/` — 项目文档

### 顶层 wiki 文档

| 文件 | 职责 |
|---|---|
| `docs/WIKI_ARCHITECTURE.md` | 面向 wiki 的架构总览 |
| `docs/WIKI_QUICKSTART.md` | 面向 wiki 的快速上手 |
| `docs/WIKI_FILETREE.md` | 本文件：面向 wiki 的完整文件树导航 |
| `docs/CAPABILITY.md` | 项目能力声明（能做什么/不能做什么） |
| `docs/HANDOVER.md` | 交接文档 |
| `docs/README.md` | 文档导航 |

### `docs/01_架构设计/` — 架构设计

| 文件 | 职责 |
|---|---|
| `docs/01_架构设计/README.md` | 架构设计区导航 |
| `docs/01_架构设计/01_global_overview.md` | 全局架构总览 |
| `docs/01_架构设计/02_deep_dive.md` | 深入分析 |
| `docs/01_架构设计/03_cucc_driver.md` | cucc driver 原理 |
| `docs/01_架构设计/04_shadow_headers.md` | shadow header 原理 |
| `docs/01_架构设计/05_cmake_design.md` | CMake 设计 |
| `docs/01_架构设计/06_cmake_principle_flow.md` | CMake 原理与流程 |
| `docs/01_架构设计/07_cucc_adaptability.md` | cucc 适配性 |
| `docs/01_架构设计/ASCEND_LIBRARY_MAPPING.md` | Ascend 库映射方案说明 |
| `docs/01_架构设计/ASCEND_RESEARCH_NOTES.md` | Ascend 研究笔记 |
| `docs/01_架构设计/CUDA_TO_CCE_PORTING_PLAN.md` | CUDA→CCE 移植计划 |

### `docs/02_能力清单/` — 能力矩阵

| 文件 | 职责 |
|---|---|
| `docs/02_能力清单/README.md` | 能力清单区导航 |
| `docs/02_能力清单/SUPPORTED_SCOPE.md` | 支持范围 |
| `docs/02_能力清单/LIMITS_AND_BOUNDARIES.md` | 限制与边界 |
| `docs/02_能力清单/PURESIMT_COMPATIBILITY_MATRIX.md` | puresimt 兼容矩阵 |
| `docs/02_能力清单/CUDA_OPERATOR_API_SUPPORT_MATRIX.md` | CUDA 算子/API 支持矩阵 |
| `docs/02_能力清单/BLAS_COMPATIBILITY_MATRIX.md` | BLAS 兼容矩阵 |
| `docs/02_能力清单/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md` | 构建系统兼容矩阵 |
| `docs/02_能力清单/CUCC_OPTION_COMPATIBILITY_MATRIX.md` | cucc 选项兼容矩阵 |

### `docs/03_文件目录/` — 文件目录

| 文件 | 职责 |
|---|---|
| `docs/03_文件目录/README.md` | 文件目录区导航 |
| `docs/03_文件目录/REPOSITORY_FILE_GUIDE.md` | 仓库文件指南（详细版，含故障定位） |

### `docs/04_对比与验证/` — 对比与验证

| 文件 | 职责 |
|---|---|
| `docs/04_对比与验证/README.md` | 对比验证区导航 |
| `docs/04_对比与验证/CURRENT_STATE.md` | 当前状态 |
| `docs/04_对比与验证/RELEASE_READINESS_2026-09-04.md` | 最终验证记录 |
| `docs/04_对比与验证/CU2CCE_VS_CUBRIDGE_REVIEW.md` | cu2cce vs cu-bridge 对比 |
| `docs/04_对比与验证/CUBRIDGE_CMAKE_MAKE_DESIGN_ANALYSIS.md` | cu-bridge CMake/Make 设计分析 |
| `docs/04_对比与验证/CMAKE_MAKE_ALIGNMENT_WITH_CUBRIDGE.md` | CMake/Make 对齐分析 |
| `docs/04_对比与验证/CMAKE_MAKE_CUCC_MENTOR_GUIDE_2026-09-01.md` | CMake/Make/cucc 导师指南 |
| `docs/04_对比与验证/mapping.md` | 映射对照 |

### `docs/99_归档/` — 历史归档

| 文件 | 职责 |
|---|---|
| `docs/99_归档/README.md` | 归档区导航 |
| `docs/99_归档/PROJECT_ARCHITECTURE_v1.md` | v1 架构设计（历史） |
| `docs/99_归档/CUCC_RULE_DRIVEN_DRIVER_v1.md` | v1 规则驱动设计（历史） |
| `docs/99_归档/BUILD_COMPATIBILITY_MODEL_v1.md` | v1 构建兼容模型（历史） |
| `docs/99_归档/CUDA_LARGE_PROJECT_COMPATIBILITY_PLAN.md` | 大项目兼容计划（历史） |
| `docs/99_归档/CODE_REVIEW_2026-08-31.md` | code review（历史） |
| `docs/99_归档/CODE_REVIEW_2026-09-01.md` | code review（历史） |
| `docs/99_归档/CODE_REVIEW_SIMPLIFIER_PLAN_2026-08-31.md` | code review simplifier 计划（历史） |
| `docs/99_归档/CODE_REVIEW_SIMPLIFIER_SPEC_2026-08-31.md` | code review simplifier 规格（历史） |
| `docs/99_归档/PARITY_AUDIT_2026-08-28.md` | parity audit（历史） |
| `docs/99_归档/PARITY_AUDIT_2026-08-31.md` | parity audit（历史） |
| `docs/99_归档/PARITY_AUDIT_2026-09-01.md` | parity audit（历史） |
| `docs/99_归档/MENTOR_REPORT_2026-08-28.md` | 导师汇报（历史） |
| `docs/99_归档/RELEASE_READINESS_2026-09-02.md` | 发布前收口记录（历史） |

### `docs/99_归档/参考资料/` — Ascend SIMT 参考资料

| 文件 | 职责 |
|---|---|
| `docs/99_归档/参考资料/asc-devkit-simt/README.md` | SIMT 参考资料说明 |
| `docs/99_归档/参考资料/asc-devkit-simt/asc_simt.h` | Ascend SIMT API 参考 |
| `docs/99_归档/参考资料/asc-devkit-simt/device_atomic_functions.h` | device atomic 参考 |
| `docs/99_归档/参考资料/asc-devkit-simt/device_functions.h` | device functions 参考 |
| `docs/99_归档/参考资料/asc-devkit-simt/device_sync_functions.h` | device sync 参考 |
| `docs/99_归档/参考资料/asc-devkit-simt/device_types.h` | device types 参考 |
| `docs/99_归档/参考资料/asc-devkit-simt/device_warp_functions.h` | device warp 参考 |
| `docs/99_归档/参考资料/asc-devkit-simt/gather_1d.asc` | gather 1d 示例 |
| `docs/99_归档/参考资料/asc-devkit-simt/hello_world.asc` | hello world 示例 |
| `docs/99_归档/参考资料/cann-runtime-kernel-exec.md` | CANN runtime kernel exec 参考 |

> 这些是 Ascend SIMT API 参考资料，不是转换模板。

---

## 维护说明

- 新增/移动文件时，同步更新本文件和 `docs/03_文件目录/REPOSITORY_FILE_GUIDE.md`
- `tests/test_repo_hygiene.py` 会检查每个非 parity-managed 跟踪路径都出现在 REPOSITORY_FILE_GUIDE.md 中
- parity-managed 路径（由 `config/cuda_api_parity_inventory.json` 管理）不需要手工列入
- 本机 macOS 没有 CANN/NPU，本地测试不能替代真机回归
- 真机回归唯一入口：`tools/real_test/server_cmd.txt`
