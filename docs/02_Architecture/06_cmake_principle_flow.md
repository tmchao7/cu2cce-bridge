# CMake 原理与完整工作流（Principle & Flow）

> 与 `05_cmake_design.md` 互补：05 讲"设计与能力清单"，本文逐层拆解
> **一次 cmake configure 到底发生了什么**，并给出真实命令 trace。

## 0. 一次 configure 的完整时序（以 cmake_demo 为例）

```text
$ cmake_cce -S examples/cmake_demo -B build

[cmake_cce]                       ① 布置 shim + 注入 override + 导出环境
[cmake]  project(... LANGUAGES CXX CUDA)
[cmake]  → 编译器探测：nvcc --version 等一系列调用
           ↳ 全部落到 shim 的 nvcc_entry.sh → mock_nvcc.py 应答
[cmake]  → 编译器 ABI 探测：编译 CMakeCUDACompilerABI.cu
           ↳ mock 生成占位对象（真实编译不走此处）
[cmake]  → CMakeTestCUDACompiler / compile features
           ↳ 按 override 固定的变量快速通过
[cmake]  → find_package(CUDAToolkit)
           ↳ FindCUDAToolkit.cmake → 桥运行时 + CUDA::* 别名族
[cmake]  → 生成 build/Makefile 与各目标 build.make
$ cmake --build build
[make]   vector_add.cu.o 规则触发
           ↳ 实际执行的编译命令（cucc → 驱动 → ccec）：
cucc -c vector_add.cu -o CMakeFiles/.../vector_add.cu.o \
     -I <install>/include -I <cann>/include -I <install>/include/shadow(按模式) \
     -include <install>/include/bridge/kernel/simt_adaptor.h \
     --cuda-path <shim>/cuda -Xcompiler=-fPIC ...
   驱动最终输出（CU2CCE_DEBUG 可见）：
/usr/local/Ascend/latest/bin/ccec -x asc \
     -I <install>/include ... -include .../simt_adaptor.h \
     -c vector_add.cu -o vector_add.cu.o \
     -std=gnu++17 -D __CU2CCE_DEVICE_COMPILATION__ -D CUDA_VERSION=11060 \
     --cce-aicore-arch=dav-c310-vec --enable-simt
[ld]     host 链接：g++ ... -lcce_bridge_runtime -lascendcl ...（桥注入）
```

要点：**CMake 全程以为自己在和 nvcc 说话**；探测被 mock 应答、真实编译
被规则驱动翻译成 ccec——两条通路共享同一 `RuleDriver`，行为逐字节一致。

## 1. 第①层：cmake_cce（Bash 入口）做了什么

1. 构造 CUDA 形状 shim 目录：`$CU2CCE_CMAKE_SHIM_DIR/cuda/bin/nvcc → nvcc_entry.sh`
   （外加 device-linker/fatbinary 同名占位），`CUDA_PATH` 指向 shim 根；
2. 向 cmake 注入两条 override 变量：
   `-DCMAKE_USER_MAKE_RULES_OVERRIDE=.../CUDAOverrides.cmake` 与
   `..._OVERRIDE_CUDA=同一文件`（CMake 在语言探测前加载）；
3. 导出环境：`ASCEND_TOOLKIT_HOME/CANN_BASE/CU2CCE_NPU_ARCH/
   CU2CCE_LIBC_MODE/...`（继承用户 shell，缺省自动探测）；
4. 透传其余参数给 cmake（`-G`、`-DCMAKE_BUILD_TYPE`、`-D...` 照常）。

## 2. 第②层：mock_nvcc 的探测应答表

| CMake 的调用 | mock 的应答 |
|---|---|
| `nvcc --version` | NVIDIA-compat 身份（vendor 正则匹配 CMake 的 NVIDIA 判定） |
| `--simple-preprocess`（CMake≥3.24） | 预置 `__CUDACC_VER_MAJOR__` 等宏文本 |
| 编译 `CMakeCUDACompilerABI.cu` | 生成占位对象（字节序/位数探测按宿主真实执行） |
| `__cmake_determine_cuda` 类内部探针 | 直接短路 |
| **其余一切真实参数** | 进程内调用 RuleDriver → ccec（行为与命令行 cucc 一致） |

## 3. 第③层：CUDAOverrides.cmake 固定了什么

| 变量 | 值 | 目的 |
|---|---|---|
| `CMAKE_CUDA_COMPILER` | shim nvcc | 探测与编译都命中 mock |
| `CMAKE_CUDA_DEVICE_LINKER/FATBINARY` | shim 占位 | 不再探测缺失的 nvlink/fatbinary |
| `CMAKE_CUDA_RUNTIME_LIBRARY` | `None` | 阻止 CMake 自作主张链接 cudart（由桥运行时承接） |
| `CMAKE_CUDA_ARCHITECTURES` | `OFF` | 用户/上游设的 sm_XX 被强制回 OFF（NVIDIA 架构无意义，且触发 .cubin 产物管线） |
| toolkit 根变量 | shim 布局 | `find_package(CUDAToolkit)` 命中桥模块 |

宿主侧 `CUDAToolkit` 由 `cmake_modules/FindCUDAToolkit.cmake` 承接：
`CUDA::cudart`（桥运行时）与 `CUDA::toolkit/cudart_static/cudadevrt`
别名族，使 `target_link_libraries(app PRIVATE CUDA::cudart)` 照常工作。

## 4. 多目录 / install(EXPORT) / 外部样本

- 多目录：`add_subdirectory` 的每个子目录目标走同一 shim/驱动，include
  传播用 `$<BUILD_INTERFACE>`/`$<INSTALL_INTERFACE>`（真机教训：裸路径
  会被 CMake 判为源目录前缀错误）；
- `install(EXPORT)`：目标图整体导出为 `lib/cmake/<ns>/`，下游
  `find_package` 消费（large_project / build_compat 两案例真机通过）；
- 外部官方样本：`examples/external/cuda_samples_project` 作为薄包装工程，
  `-DCUDA_SAMPLES_SOURCE_DIR=<官方源码树>` 选择性接入 5 个样本；上游
  自设的 `CUDA_ARCHITECTURES sm_61..120` 被扫回 OFF（类型守卫：
  CUBIN/FATBIN 属性仅 OBJECT 目标可设）。

## 5. 失败模式与处理（真机教训表）

| 场景 | 表现 | 处理 |
|---|---|---|
| 上游样本自设多架构 | CMake 生成按架构 `.cubin` 规则，ld.lld 报错 | 强制 `CUDA_ARCHITECTURES OFF`（类型守卫清扫） |
| 属性误用于 executable | configure 即报 "only applied to OBJECT targets" | CUBIN/FATBIN 属性 TYPE 守卫 |
| 构建树陈旧 | configure 报错 | 自动回退全量 reconfigure |
| 交叉引用裸 include 路径 | configure 报 source-dir-prefix 错 | 生成器表达式（BUILD/INSTALL INTERFACE） |

## 6. 关联文档

- 能力清单与边界：`../03_Scope/SUPPORTED_SCOPE.md` / `LIMITS_AND_BOUNDARIES.md`
- 驱动内部：`03_cucc_driver.md`；无硬编码论证：`07_cucc_adaptability.md`
- 设计文档（能力矩阵角度）：`05_cmake_design.md`
