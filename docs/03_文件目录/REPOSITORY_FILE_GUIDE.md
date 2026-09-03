# cu2cce-bridge 仓库文件与问题定位指南

**适用分支：** `main` / puresimt
**更新日期：** 2026-09-02
**对标仓库：** `cu-bridge-muxi` `6ab1076` / `3.8.0`
**目标后端：** Ascend NPU / CANN / ccec

本文件回答两个问题：**每个目录和关键文件负责什么**，以及**遇到问题应该先看哪里**。
它描述的是当前 `main` 的真实代码边界，不把历史 converter 路线或 MACA 专属能力算作现行实现。

## 1. 先看调用链

### 1.1 用户直接使用二进制安装树

```text
用户 CUDA .cu
  └─ $CUCC_PATH/bin/cucc
       └─ tools/cu2cce_driver.py
            ├─ bin/conf.json                  # cu-bridge 风格主规则
            ├─ include/                       # CUDA/SIMT 兼容头
            └─ ccec -x asc --enable-simt
                 └─ libcce_bridge_runtime.so
                      └─ CANN aclrt/ascendcl/runtime
```

### 1.2 CMake

```text
cmake_cce -S/-B
  └─ virtual_toolkit_env.sh + mock/virtualDir.py
       └─ CUDA_DIR/bin/nvcc -> mock/nvcc_entry.sh -> mock_nvcc.py  # configure

cmake_cce --build
  └─ make_cce 或 ninja_cce
       └─ 同一 CUDA_DIR/bin/nvcc -> cucc                    # build
            └─ .o/.a/.so/ELF
```

### 1.3 Make / Ninja / pre wrapper

```text
make_cce / ninja_cce / pre_cce
  └─ virtual_toolkit_env.sh
       └─ mock/virtualDir.py
            ├─ CUDA_PATH / CUDA_HOME
            ├─ bin/nvcc -> cucc
            ├─ CUDA-named compatibility library aliases
            └─ host compiler wrapper -> hostcc_cce
```

## 2. 与 cu-bridge 的目录/命名对应

对齐的是职责和目录层级，不是把 MACA 后端名称照搬到 Ascend：

| cu-bridge reference | cu2cce-bridge | 说明 |
|---|---|---|
| `bin/cucc` | `bin/cucc` | 保留统一的 cucc 用户入口 |
| `tools/cmake_maca` / `cmake_mock` / `cmake_assist` | `tools/cmake_cce` | 一个 Ascend 入口；内部完成 mock configure 和真实 build 分流 |
| `tools/make_maca` | `tools/make_cce` | Make wrapper，后端改为 ccec/CANN |
| `tools/ninja_maca` | `tools/ninja_cce` | Ninja wrapper，后端改为 ccec/CANN |
| `tools/pre_makefile` | `tools/pre_cce` | configure/autotools 环境 wrapper |
| `bin/gnu` | `tools/hostcc_cce` | host compiler carrier；链接注入 bridge/CANN runtime |
| `tools/link.json` + `stub_link.json` | `tools/link.json` + `tools/mock/stub_link.json` | Make/pre 与 CMake 的虚拟工具链清单 |
| `tools/mock/virtualDir.py` | `tools/mock/virtualDir.py` | virtual toolkit directory builder |
| `bin/conf.json` | `bin/conf.json` | cucc 主规则文件；外层命名贴近参考项目，内容保留 CANN 扩展 |
| `mxcc` / `runtime_cu.so` / `symbol_cu.so` | `ccec` / `libcce_bridge_runtime.so` + CANN | 后端专属名称，不做字面复制 |

因此不要为了“看起来一样”把 `cmake_cce` 重命名成 `cmake_maca`，也不要在 Ascend
安装树中创建 MACA 库或 device artifact 的伪别名。

## 2. 顶层目录与文件

| 路径 | 作用 | 常见问题 |
|---|---|---|
| `bin/` | 用户可直接调用的入口；当前核心入口是 `cucc` | 命令找不到、环境变量、driver 启动失败 |
| `cmake/` | 原生 `cu2cce-bridge` CMake package 配置 | `find_package(cu2cce-bridge CONFIG)` 找不到或 target 路径错误 |
| `cmake_modules/` | CUDA/CMake 兼容模块，不是 CANN 编译器本身 | `FindCUDA`、`FindCUDAToolkit`、legacy CMake 配置问题 |
| `config/` | artifact contract；cucc 规则和虚拟工具链清单位于 `bin/`/`tools/` | 规则覆盖、软链布局、产物契约问题 |
| `include/` | CUDA 兼容头、runtime 声明、SIMT adaptor、shadow libc | `.cu` 解析、CUDA API 声明、host/device 头冲突 |
| `src/` | bridge runtime 和 experimental BLAS 的实现 | 链接符号、ACL 调用、运行时返回码 |
| `examples/` | puresimt、CMake、Make/Ninja、BLAS 和诊断载体 | 复现某个 API 或构建入口的最小案例 |
| `tests/` | 本地规则、CMake、artifact、mock ACL 和仓库卫生测试 | 回归防护、接口契约变更 |
| `third_party/` | 必要的 NVIDIA CUDA Samples helper 头 | 官方 sample 缺少 helper include |
| `tools/` | wrapper、driver、virtual toolkit directory builder、真机脚本和 verifier | 构建阶段、环境、真机回归、产物检查 |
| `docs/` | 架构、范围、对比、维护和参考文档 | 机制解释、支持范围、维护交接 |
| `README.md` | 项目定位、二进制使用入口和能力边界 | 使用者快速判断项目能否满足需求 |
| `QUICKSTART.md` | 面向用户的完整 CANN/编译/运行操作手册 | 从零搭建和运行 CUDA 示例 |
| `CLAUDE.md` | 面向维护 agent/开发者的工作纪律和对标基线 | 贡献流程、真机验证纪律 |
| `CMakeLists.txt` | 构建/安装 bridge runtime 和安装树 | bridge 编译、安装文件缺失 |
| `.gitignore` | 排除构建、日志、扫描状态和本地工具配置 | 误提交生成物 |

当前顶层不保留临时 `plans/`、`.claude/` 等个人工作目录；已完成计划在
`docs/99_归档/`，构建输出在 `build/` 且不进入版本库。

### `bin/conf.json`

`cucc` 的参考项目风格主规则文件。`filter/replacer/adder/skipstring` 对应
当前驱动的 drop/replace/inject/query passthrough；`reject`、`capabilities`、
`frontend` 和诊断字段保留 `Bisheng/ccec`、CANN、`dav-*` 的后端策略。

### `tools/link.json` 与 `tools/mock/stub_link.json`

前者对应 `cu-bridge` 的 `link.json`，负责 Make/pre 的虚拟工具链目录；后者对应
`stub_link.json`，负责 CMake configure 的 virtual_cmake 入口。两者都只描述 CUDA
形状的目录和入口，真实编译器仍是 `Bisheng ccec`，真实运行库仍是 bridge/CANN。

### `tools/mock/virtualDir.py`

虚拟 CUDA 工具链目录构建器，负责读取上述清单、创建目录、生成兼容入口和 runtime
兼容 alias。它只在生成的虚拟工具链目录内创建必要链接，不是仓库文件之间的软链接。

## 3. 用户入口与构建 wrapper

### `bin/cucc`

极薄的 shell 入口，负责解析安装根和启动 Python driver。它不转换 CUDA 源文件，也不负责
CANN API 逻辑。遇到参数过滤、phase、架构或库策略问题，直接看
`tools/cu2cce_driver.py` 和 `bin/conf.json`。

### `tools/cu2cce_driver.py`

cucc 的唯一规则驱动核心，同时被 CMake 的 `mock_nvcc.py` 复用。主要职责：

- 安全展开 `--options-file` 和 `@response-file`；
- 识别 query、preprocess、dependency、compile、link、compile-and-link；
- 按 `reject → replace → drop → forward` 处理参数；
- 注入 bridge include、SIMT adaptor、CANN runtime、RPATH 和 `dav-*` 架构；
- 保留 `__CUDACC__` 等兼容宏，但不把 `sm_*` 传给 ccec；
- 处理 device-side FP64 policy、CUDA library capability policy 和 host flags；
- 生成 `--dryrun`/trace，不生成 `.cce`、converter 源或 PTX/CUBIN/FATBIN。

重点函数：

| 函数/区域 | 定位 |
|---|---|
| `expand_response_files` | options/response file 递归展开和错误检查 |
| `RuleDriver._resolve_npu_arch` | Ascend 架构选择 |
| `RuleDriver._phase` / `_scan_inputs` | 输入和编译阶段识别 |
| `RuleDriver._prepare_query_passthrough` | toolchain query 直通 |
| `RuleDriver._transform_arguments` | 规则优先级管线 |
| `RuleDriver._order_injections` | `-x asc`、include 注入排序 |
| `RuleDriver._finalize_prepared_command` | conda/gcc-toolchain、regex cleanup、trace |
| `RuleDriver.run_hostcc` | host C/C++ link carrier |

### `tools/cmake_cce`

CMake 用户入口，保持 `cu-bridge` 的 configure/build 双阶段机制，但后端换成 Ascend：

- configure：virtual CUDA toolkit + mock nvcc；
- build：根据 `build.ninja` 选择 `ninja_cce`，否则选择 `make_cce`；
- 支持 `--build`/`--target`/`--parallel`/`--config`/`--clean-first`/`--`；
- 默认虚拟工具链目录按 build directory 隔离，旧 build tree 优先读取 `CMakeCache.txt`；
- 不做 `USE_CMAKE_MACA=1` 的系统 CMake 物理替换。

CMake 配置失败先看 configure log；CMake 构建失败先看 build log 和
`CUDA_DIR/bin/nvcc` 的最终指向。

### `tools/make_cce`

Make wrapper。准备 CUDA-shaped toolkit、`CUDA_PATH`/`CUDA_HOME`、CANN 路径、host compiler
entries 后直接执行系统 `make`。Makefile 中的 `nvcc` 会通过虚拟工具链目录进入 cucc；`clean` 会清理
对应目录，`cleanpack` 不会因子串误判而清理它。

### `tools/ninja_cce`

Ninja wrapper。支持直接 `ninja_cce -C <build>`，读取已有 CMake cache 中记录的虚拟工具链目录，或根据 build
目录生成稳定的默认虚拟工具链目录；随后设置 NPU/CANN 环境并执行系统 `ninja`。

### `tools/pre_cce`

configure/autotools 风格 wrapper，对应 `cu-bridge` 的 `pre_makefile`。只负责准备环境和
virtual CUDA toolkit，然后用 `exec "$@"` 原样执行用户命令，不使用 `eval` 重构参数。

### `tools/hostcc_cce`

由虚拟 CUDA 工具链目录中的 `g++`/`gcc`/`cc`/`c++` 入口进入。纯 host compile/query 透传系统编译器；
link 阶段由 driver 的 `hostcc.link.inject` 规则补充 bridge runtime 和已存在的 CANN 库。
`.cu` 源文件不能绕过 cucc 直接进入 hostcc。

### `tools/virtual_toolkit_env.sh`

CMake/Make/Ninja/pre 共用的 shell helper，集中处理：

- host triple、NPU 架构和 ccec 路径解析；
- CMake build cache 中记录的虚拟工具链目录恢复；
- build-keyed 虚拟工具链根目录；
- 无尾部空项的 `LD_LIBRARY_PATH`；
- virtual CUDA toolkit builder 调用。

### `tools/mock/virtualDir.py`

读取 `tools/link.json` 或 `tools/mock/stub_link.json` 并幂等创建 CUDA-shaped tree。它负责目录、
生成目录内部链接、CUDA 命名 library alias、host compiler entry 和版本文件；不编译用户源文件。

### `tools/mock/nvcc_entry.sh` 与 `tools/mock/mock_nvcc.py`

`nvcc_entry.sh` 是 configure 阶段的虚拟工具链入口；`mock_nvcc.py` 回答 CMake 的 version、ABI、
identity 和 simple preprocess 探针。ABI stand-in 只用于 configure 探测，真实 build 必须经过
cucc/ccec。

## 4. 配置文件

### `bin/conf.json`

规则数据源。主要 section：

| section | 作用 |
|---|---|
| `data` | 参考项目风格的规则数据版本 |
| `skipstring` | query 类命令直通 |
| `all` / `compile` / `link` | `filter`、`replacer`、`adder` 规则 |
| `cu2cce.frontend` | CANN `ccec`、默认 `dav-*` 和兼容 CUDA version |
| `cu2cce.reject` | Ascend 不支持能力的明确拒绝规则 |
| `cu2cce.capabilities` | cudart、cublas、nccl 等库能力分层和 `map_to` |
| `cu2cce.hostcc` | host compiler link carrier 的 CANN/bridge 注入 |
| `cu2cce.regex_cleanup` | 最终命令的 gcc/clang cleanup |
| `cu2cce.diagnostics` | strict/debug/trace 和 unsupported mode 配置 |

修改此文件后必须运行：

```bash
python3 tools/validate_cucc_rules.py bin/conf.json
```

### `tools/link.json`

对应 `cu-bridge` 的 `link.json`，是 Make/pre 的虚拟 CUDA 工具链清单，定义 `bin/`、`include`、`targets/<host-triple>/`、`lib64` alias、
CUPTI 目录和 host tool wrappers。不要在 `cmake_cce`、`make_cce`、`ninja_cce` 中复制软链逻辑。

### `config/artifact_contract.json`

定义每个 blocking case 的 expected files、类型、禁止后缀、NEEDED/RPATH 和 metadata。它
表达的是 Ascend 产物结构契约，不要求和 MACA ELF 字节级一致。

### `tools/real_test/verify_artifacts.py`

读取 artifact contract，使用 `file`/`readelf` 检查 `.o/.a/.so/ELF/.d`、动态依赖、RPATH 和
CMake/Ninja metadata。终端输出一行摘要，详细结果写到指定 log。

## 5. CMake compatibility module

| 文件 | 作用 |
|---|---|
| `cmake_modules/FindCUDA.cmake` | legacy `find_package(CUDA)`、`cuda_add_*`、`cuda_compile`、`cuda_wrap_srcs` 和常见配置 helper；PTX/CUBIN/FATBIN/RDC 不伪造 |
| `cmake_modules/FindCUDAToolkit.cmake` | bridge-backed `CUDA::cudart`/toolkit alias、CANN include/lib 传播；未验证 math library 不自动造 target |
| `cmake_modules/override/CUDAOverrides.cmake` | configure 阶段固定 compiler/toolkit identity、关闭 NVIDIA architecture/runtime injection |
| `cmake_modules/*/README.md` | 说明 module 的安全子集和不支持边界 |
| `cmake/cu2cce-bridgeConfig.cmake` | `find_package(cu2cce-bridge CONFIG)` 的原生 package 入口 |
| `cmake/cu2cce-bridgeConfigVersion.cmake` | package version compatibility 检查 |

## 6. runtime、header 和源代码

### `include/`

- `cuda_runtime.h`、`cuda_runtime_api.h`：用户 CUDA runtime include 入口；
- `__cuda_runtime.h`：内部 runtime 组合入口；
- `device_launch_parameters.h`、`crt/`：CUDA built-in/type/math 兼容层；
- `bridge/runtime/`：wrapper 内部声明、类型、错误/stream/event/alloc 接口；
- `bridge/kernel/simt_adaptor.h`：`__shared__`、同步、warp、atomic 等 SIMT 原语映射；
- `bridge/blas/`、`cublas_v2.h`：experimental BLAS 兼容入口；
- `cooperative_groups*`、`cuda_fp16.h`、`cuda_bf16.h`、`nvToolsExt.h`：已覆盖或实验中的
  CUDA 头；
- `shadow/`：非 system libc 模式的 stdio/string/stdlib/math 兼容头。

### `src/bridge/runtime/src/cuda_runtime_wrapper.cpp`

正式 runtime bridge 实现：`cuda* → wcuda* → aclrt*`，负责初始化、device/context、memory、
stream/event、memcpy、kernel launch carrier 和错误状态。遇到运行时返回码或 API 语义问题，
优先看这个文件及对应 `include/bridge/runtime/*.h`。

### `src/bridge/blas/src/cublas_v2_wrapper.cpp`

experimental `cublas* → aclblas*` 实现。它不属于默认 puresimt blocking 主线；相关问题看
`examples/blas/`、`tests/test_blas_surface.py` 和 BLAS compatibility matrix。

## 7. examples 与 tests

### examples

| 路径 | 用途 |
|---|---|
| `examples/puresimt/` | 官方 sample 和最小 CUDA operator，验证 SIMT/runtime 语义；文件名和预期输出在 `server_cmd.txt` |
| `examples/cmake_demo/` | 最小现代 CUDA CMake executable |
| `examples/cmake_compat/` | 多 `.cu`、static/shared library、host app、`CUDA::cudart` |
| `examples/cmake_compat_legacy/` | legacy `FindCUDA`/`cuda_add_executable` |
| `examples/large_project/` | 多目录、跨 target dependency、install/export；每个 linked library 一个 `.cu` |
| `examples/build_compat/cmake/` | CMake Makefiles/Ninja 的 `.o/.a/.so/ELF` carrier |
| `examples/build_compat/make/` | Makefile 的 nvcc interception、多个 object、`.d`、archive/shared/executable |
| `examples/build_compat/make_gxx_link/` | plain g++ + `-lcudart` 的 bridge runtime carrier |
| `examples/external/cuda_samples_project/` | pinned NVIDIA CUDA Samples CMake profile；源码在真机 fetch，不进仓库 |
| `examples/multi_cu_diagnostic/` | 多 `.cu` 聚合的 Ascend duplicate registration 诊断载体，非阻塞 |
| `examples/blas/` | experimental cuBLAS/ACL BLAS 载体 |

`third_party/nvidia_cuda_samples/Common/` 只存未修改官方 helper 头，不是完整 CUDA Toolkit。

### tests

| 文件 | 覆盖 |
|---|---|
| `tests/test_cucc_rules.py` | 参数规则、query/options/response、架构、host flags、library policy、phase 和 trace |
| `tests/test_cmake_compat.py` | CMake wrapper、虚拟工具链生命周期、legacy module、Make/Ninja/pre 参数边界 |
| `tests/test_artifacts.py` | artifact contract、metadata、NEEDED/RPATH、server harness 接线 |
| `tests/test_blas_surface.py` | experimental BLAS surface |
| `tests/test_repo_hygiene.py` | 目录、文档入口、gitignore 和发布树卫生 |
| `tests/mock_acl/syntax_check.sh` | mock ACL 头的语法 smoke，不代表真机运行 |

## 8. 真机回归与日志定位

唯一主线入口：

```text
tools/real_test/server_cmd.txt
```

常用输出目录：

| 目录/文件 | 内容 |
|---|---|
| `build/real_test/probe.env` | CANN/设备探测结果，供 bridge build 使用 |
| `build/real_test/install/` | 二进制安装树（`CUCC_PATH`） |
| `build/real_test/puresimt/` | puresimt、cucc、CMake、Make/Ninja 回归产物和日志 |
| `build/real_test/puresimt/artifacts/` | 每个 artifact case 的详细 contract log |
| `build/real_test/logs/` | env/device 探测完整日志 |
| `build/real_test/external/` | 外部 CUDA Samples 源码/缓存，运行时内容 |
| `build/real_test/blas/` | optional experimental BLAS 日志 |

日志定位顺序建议：

1. 先看终端的 section 和 case 名；
2. 构建失败看对应 `*_configure.log`/`*_build.log`；
3. 运行失败看对应 `*_run.log`；
4. 产物失败看 `puresimt/artifacts/<case>.artifact.log`；
5. 参数行为看 `cucc_direct.trace.json` 或 `cucc --dryrun`；
6. 设备/CANN 失败看 `probe.env` 和 `logs/probe.log`；
7. 只有在主线通过后，再看 `DIAG-ONLY` 的 async/multi-CU/backend 证据。

## 9. 状态分层与维护纪律

| 标记 | 含义 | 维护动作 |
|---|---|---|
| `SUPPORTED` | 已验证的用户/构建能力 | 修改必须补本地测试，真机改动跑 full profile |
| `EXPERIMENTAL` | 有限形状或独立能力 | 不得扩大成默认承诺，更新对应矩阵 |
| `UNSUPPORTED` | 明确拒绝或无验证等价物 | 保持 honest reject/warning，不创建伪库/伪产物 |
| `DIAGNOSTIC_ONLY` | 用于取证，不阻塞主线 | 记录原因和日志，不把失败改成 PASS |

修改入口/规则/安装文件时，按以下顺序：

```bash
python3 tools/validate_cucc_rules.py bin/conf.json
python3 -m unittest discover -s tests -p 'test_*.py' -v
bash tests/mock_acl/syntax_check.sh
git diff --check
```

本机 macOS 没有 CANN/NPU，不能用本地测试替代服务器产物和设备证据。服务器更新时使用
实际 checkout 的 remote 名称（可能是 `origin` 或 `gitcode`），并执行
`tools/real_test/clean.sh` 后再跑 `server_cmd.txt`。

## 10. 当前 Git 跟踪文件逐项清单

> 下面的路径以当前仓库 Git inventory 为准（生成日期：2026-09-02）。新增或移动文件时，
> 请同步补充本表；`tests/test_repo_hygiene.py` 会检查每个跟踪路径都出现在本指南中。

### `.gitattributes/`

| 路径 | 职责 |
|---|---|
| `.gitattributes` | 跨平台文本换行与脚本文件属性 |

### `.gitignore/`

| 路径 | 职责 |
|---|---|
| `.gitignore` | 排除构建、日志、缓存和本地工具产物 |

### `CLAUDE.md/`

| 路径 | 职责 |
|---|---|
| `CLAUDE.md` | 维护 agent 的工作纪律、验证要求和对标基线 |

### `CMakeLists.txt/`

| 路径 | 职责 |
|---|---|
| `CMakeLists.txt` | 构建并安装 bridge runtime、头文件、工具和 CMake package |

### `LICENSE.txt/`

| 路径 | 职责 |
|---|---|
| `LICENSE.txt` | 项目许可证正文 |

### `NOTICE.txt/`

| 路径 | 职责 |
|---|---|
| `NOTICE.txt` | 第三方版权与归属声明 |

### `QUICKSTART.md/`

| 路径 | 职责 |
|---|---|
| `QUICKSTART.md` | 用户从 CANN 环境到编译运行 CUDA 的完整操作手册 |

### `README.md/`

| 路径 | 职责 |
|---|---|
| `README.md` | 项目定位、能力边界和二进制安装树使用入口 |

### `bin/`

| 路径 | 职责 |
|---|---|
| `bin/cucc` | 用户命令入口；当前为 cucc 的启动脚本 |
| `bin/nvcc` | 安装树中的普通 CUDA 兼容入口，转发到 `bin/cucc`，不依赖安装软链接 |
| `bin/conf.json` | cu-bridge 风格主规则文件，保留 Ascend/CANN/Bisheng 扩展 |
| `bin/gnu` | 参考项目风格的 host compiler carrier，实际调用 `tools/hostcc_cce` |
| `bin/gfortran` | 参考项目风格的 host Fortran carrier，实际调用 `tools/gfortran_cce` |

### `cmake/`

| 路径 | 职责 |
|---|---|
| `cmake/cu2cce-bridgeConfig.cmake` | 原生 cu2cce-bridge CMake package 配置/版本文件 |
| `cmake/cu2cce-bridgeConfigVersion.cmake` | 原生 cu2cce-bridge CMake package 配置/版本文件 |

### `cmake_modules/`

| 路径 | 职责 |
|---|---|
| `cmake_modules/FindCUDA.cmake` | legacy FindCUDA 兼容模块与安全子集 |
| `cmake_modules/FindCUDAToolkit.cmake` | bridge-backed FindCUDAToolkit 与 CUDA::* targets |
| `cmake_modules/README.md` | CMake 兼容模块使用边界说明 |
| `cmake_modules/override/CUDAOverrides.cmake` | CMake CUDA compiler identity/runtime/architecture override |
| `cmake_modules/override/README.md` | CMake 兼容模块使用边界说明 |

### `config/`

| 路径 | 职责 |
|---|---|
| `config/artifact_contract.json` | cucc/CMake/Make/Ninja artifact contract 清单 |


### `docs/`

| 路径 | 职责 |
|---|---|
| `docs/01_架构设计/ASCEND_LIBRARY_MAPPING.md` | 底层设计、CANN 事实或历史方案说明 |
| `docs/01_架构设计/ASCEND_RESEARCH_NOTES.md` | 底层设计、CANN 事实或历史方案说明 |
| `docs/01_架构设计/CUDA_TO_CCE_PORTING_PLAN.md` | 底层设计、CANN 事实或历史方案说明 |
| `docs/01_架构设计/01_global_overview.md` | 当前架构、驱动、shadow header 或 CMake 原理说明 |
| `docs/01_架构设计/02_deep_dive.md` | 当前架构、驱动、shadow header 或 CMake 原理说明 |
| `docs/01_架构设计/03_cucc_driver.md` | 当前架构、驱动、shadow header 或 CMake 原理说明 |
| `docs/01_架构设计/04_shadow_headers.md` | 当前架构、驱动、shadow header 或 CMake 原理说明 |
| `docs/01_架构设计/05_cmake_design.md` | 当前架构、驱动、shadow header 或 CMake 原理说明 |
| `docs/01_架构设计/06_cmake_principle_flow.md` | 当前架构、驱动、shadow header 或 CMake 原理说明 |
| `docs/01_架构设计/07_cucc_adaptability.md` | 当前架构、驱动、shadow header 或 CMake 原理说明 |
| `docs/02_能力清单/BLAS_COMPATIBILITY_MATRIX.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/02_能力清单/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/02_能力清单/CUDA_OPERATOR_API_SUPPORT_MATRIX.md` | 已验证算子、Kernel 写法、Runtime/API 映射和构建支持总表 |
| `docs/04_对比与验证/CMAKE_MAKE_ALIGNMENT_WITH_CUBRIDGE.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/CMAKE_MAKE_CUCC_MENTOR_GUIDE_2026-09-01.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/CODE_REVIEW_2026-08-31.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/CODE_REVIEW_2026-09-01.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/CU2CCE_VS_CUBRIDGE_REVIEW.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/CUBRIDGE_CMAKE_MAKE_DESIGN_ANALYSIS.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/02_能力清单/CUCC_OPTION_COMPATIBILITY_MATRIX.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/CURRENT_STATE.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/MENTOR_REPORT_2026-08-28.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/PARITY_AUDIT_2026-08-28.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/PARITY_AUDIT_2026-08-31.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/PARITY_AUDIT_2026-09-01.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/02_能力清单/PURESIMT_COMPATIBILITY_MATRIX.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/RELEASE_READINESS_2026-09-02.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/04_对比与验证/mapping.md` | cu-bridge 对比、兼容矩阵、产物和 code review 证据 |
| `docs/02_能力清单/LIMITS_AND_BOUNDARIES.md` | 支持范围与明确限制 |
| `docs/02_能力清单/SUPPORTED_SCOPE.md` | 支持范围与明确限制 |
| `docs/README.md` | 文档导航或维护指南 |
| `docs/03_文件目录/REPOSITORY_FILE_GUIDE.md` | 本文件：仓库目录、文件职责、调用链和故障定位地图 |
| `docs/99_归档/BUILD_COMPATIBILITY_MODEL_v1.md` | 已完成计划或历史设计归档 |
| `docs/99_归档/CODE_REVIEW_SIMPLIFIER_PLAN_2026-08-31.md` | 已完成计划或历史设计归档 |
| `docs/99_归档/CODE_REVIEW_SIMPLIFIER_SPEC_2026-08-31.md` | 已完成计划或历史设计归档 |
| `docs/99_归档/CUCC_RULE_DRIVEN_DRIVER_v1.md` | 已完成计划或历史设计归档 |
| `docs/99_归档/CUDA_LARGE_PROJECT_COMPATIBILITY_PLAN.md` | 已完成计划或历史设计归档 |
| `docs/99_归档/PROJECT_ARCHITECTURE_v1.md` | 已完成计划或历史设计归档 |
| `docs/99_归档/参考资料/asc-devkit-simt/README.md` | Ascend SIMT/CANN 参考资料 |
| `docs/99_归档/参考资料/asc-devkit-simt/asc_simt.h` | Ascend SIMT/CANN 参考资料 |
| `docs/99_归档/参考资料/asc-devkit-simt/device_atomic_functions.h` | Ascend SIMT/CANN 参考资料 |
| `docs/99_归档/参考资料/asc-devkit-simt/device_functions.h` | Ascend SIMT/CANN 参考资料 |
| `docs/99_归档/参考资料/asc-devkit-simt/device_sync_functions.h` | Ascend SIMT/CANN 参考资料 |
| `docs/99_归档/参考资料/asc-devkit-simt/device_types.h` | Ascend SIMT/CANN 参考资料 |
| `docs/99_归档/参考资料/asc-devkit-simt/device_warp_functions.h` | Ascend SIMT/CANN 参考资料 |
| `docs/99_归档/参考资料/asc-devkit-simt/gather_1d.asc` | Ascend SIMT/CANN 参考资料 |
| `docs/99_归档/参考资料/asc-devkit-simt/hello_world.asc` | Ascend SIMT/CANN 参考资料 |
| `docs/99_归档/参考资料/cann-runtime-kernel-exec.md` | Ascend SIMT/CANN 参考资料 |

### `examples/`

| 路径 | 职责 |
|---|---|
| `examples/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/blas/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/blas/cublas_gemmex.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/blas/cublas_gemv.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/blas/cublas_sgemm.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/blas/gemv_param_probe.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/build_compat/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/build_compat/cmake/CMakeLists.txt` | 该示例的 CMake 构建入口/target 定义 |
| `examples/build_compat/cmake/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/build_compat/cmake/kernel_a.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/build_compat/cmake/kernel_b.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/build_compat/cmake/main.cpp` | 示例的 host C++、接口或辅助源码 |
| `examples/build_compat/make/Makefile` | 示例工程文件 |
| `examples/build_compat/make/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/build_compat/make/kernel_a.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/build_compat/make/kernel_b.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/build_compat/make/main.cpp` | 示例的 host C++、接口或辅助源码 |
| `examples/build_compat/make_gxx_link/Makefile` | 示例工程文件 |
| `examples/build_compat/make_gxx_link/kernel.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/build_compat/make_gxx_link/main.cpp` | 示例的 host C++、接口或辅助源码 |
| `examples/cmake_compat/CMakeLists.txt` | 该示例的 CMake 构建入口/target 定义 |
| `examples/cmake_compat/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/cmake_compat/compat_api.h` | 示例的 host C++、接口或辅助源码 |
| `examples/cmake_compat/main.cpp` | 示例的 host C++、接口或辅助源码 |
| `examples/cmake_compat/shared_kernel.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/cmake_compat/static_kernel.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/cmake_compat_legacy/CMakeLists.txt` | 该示例的 CMake 构建入口/target 定义 |
| `examples/cmake_compat_legacy/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/cmake_compat_legacy/legacy_vector_add.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/cmake_demo/CMakeLists.txt` | 该示例的 CMake 构建入口/target 定义 |
| `examples/cmake_demo/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/cmake_demo/vector_add.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/external/cuda_samples_project/CMakeLists.txt` | 该示例的 CMake 构建入口/target 定义 |
| `examples/external/cuda_samples_project/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/external/cuda_samples_project/profile_core.json` | 外部 CUDA Samples profile 配置 |
| `examples/large_project/CMakeLists.txt` | 该示例的 CMake 构建入口/target 定义 |
| `examples/large_project/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/large_project/app/main.cpp` | 示例的 host C++、接口或辅助源码 |
| `examples/large_project/comp/CMakeLists.txt` | 该示例的 CMake 构建入口/target 定义 |
| `examples/large_project/comp/kernel.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/large_project/comp/kernel.h` | 示例的 host C++、接口或辅助源码 |
| `examples/large_project/util/CMakeLists.txt` | 该示例的 CMake 构建入口/target 定义 |
| `examples/large_project/util/data.h` | 示例的 host C++、接口或辅助源码 |
| `examples/large_project/util/gen_data.cpp` | 示例的 host C++、接口或辅助源码 |
| `examples/large_project/util/store.cpp` | 示例的 host C++、接口或辅助源码 |
| `examples/large_project/util/store.h` | 示例的 host C++、接口或辅助源码 |
| `examples/multi_cu_diagnostic/CMakeLists.txt` | 该示例的 CMake 构建入口/target 定义 |
| `examples/multi_cu_diagnostic/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/multi_cu_diagnostic/kernel_a.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/multi_cu_diagnostic/kernel_b.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/multi_cu_diagnostic/main.cpp` | 示例的 host C++、接口或辅助源码 |
| `examples/puresimt/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/puresimt/async_repro_diag.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/atomicAdd_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/atomicVariants_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/bf16x2_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/core_ops_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/device_math_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/dot_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/double_policy_diag.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/dynamicShared_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/event_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/half_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/index3D_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/math_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/matrixMul/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/puresimt/matrixMul/matrixMul.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/matrixMul_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/multiKernel_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/pitchedMemcpy_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/reduction_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/scan_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/stencil_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/threadFence_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/transpose_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/vectorAdd/README.md` | 该示例的使用说明、预期产物或限制 |
| `examples/puresimt/vectorAdd/vectorAdd.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/vectorAdd_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/vote_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |
| `examples/puresimt/warpShuffle_minimal.cu` | CUDA/puresimt、构建兼容或诊断示例源码 |

### `include/`

| 路径 | 职责 |
|---|---|
| `include/__cuda_runtime.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/bridge/blas/cublas_v2_wrapper.h` | experimental cuBLAS/ACL BLAS 兼容头 |
| `include/bridge/kernel/simt_adaptor.h` | CUDA SIMT 基础原语、执行空间和 shared/atomic 基础映射 |
| `include/bridge/kernel/cuda_device_api.h` | CUDA device-side SIMT/warp/vote/reduce/atomic 到 CANN simt_api 的集中映射 |
| `include/bridge/runtime/cuda_benchmark_targets.h` | CUDA runtime bridge 的类型、声明和内部接口头 |
| `include/bridge/runtime/cuda_runtime_wrapper.h` | CUDA runtime bridge 的类型、声明和内部接口头 |
| `include/bridge/runtime/cuda_to_cce_mcr_adaptor.h` | CUDA runtime bridge 的类型、声明和内部接口头 |
| `include/bridge/runtime/cuda_wrapper_internal.h` | CUDA runtime bridge 的类型、声明和内部接口头 |
| `include/bridge/runtime/cuda_wrapper_internal_types.h` | CUDA runtime bridge 的类型、声明和内部接口头 |
| `include/bridge/runtime/cuda_wrapper_types.h` | CUDA runtime bridge 的类型、声明和内部接口头 |
| `include/bridge/tools_ext/nvtx_stub.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/cooperative_groups.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/cooperative_groups/reduce.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/crt/builtin_types.h` | CUDA CRT/builtin/type/math 兼容头 |
| `include/crt/common_functions.h` | CUDA CRT/builtin/type/math 兼容头 |
| `include/crt/device_functions.h` | CUDA CRT/builtin/type/math 兼容头 |
| `include/crt/host_defines.h` | CUDA CRT/builtin/type/math 兼容头 |
| `include/crt/math_functions.h` | CUDA CRT/builtin/type/math 兼容头 |
| `include/crt/vector_functions_compat.h` | CUDA CRT/builtin/type/math 兼容头 |
| `include/crt/vector_types.h` | CUDA CRT/builtin/type/math 兼容头 |
| `include/cublas_v2.h` | experimental cuBLAS/ACL BLAS 兼容头 |
| `include/cuda_bf16.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/cuda_fp16.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/cuda_profiler_api.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/cuda_runtime.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/cuda_runtime_api.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/device_launch_parameters.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/math_constants.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/nvToolsExt.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/nvtx3/nvToolsExt.h` | 用户可见 CUDA 兼容头或第三方 API 声明 |
| `include/shadow/math.h` | shadow libc 兼容头（shadow 模式） |
| `include/shadow/stdio.h` | shadow libc 兼容头（shadow 模式） |
| `include/shadow/stdlib.h` | shadow libc 兼容头（shadow 模式） |
| `include/shadow/string.h` | shadow libc 兼容头（shadow 模式） |

### `src/`

| 路径 | 职责 |
|---|---|
| `src/bridge/blas/src/cublas_v2_wrapper.cpp` | experimental cuBLAS 到 ACL BLAS 实现 |
| `src/bridge/runtime/src/cuda_runtime_wrapper.cpp` | bridge runtime 的 CUDA API 到 ACL runtime 实现 |

### `tests/`

| 路径 | 职责 |
|---|---|
| `tests/host_bridge_smoke.cu` | 测试或测试辅助源码 |
| `tests/mock_acl/acl/acl.h` | mock ACL 头/语法检查资产，不代表真机运行 |
| `tests/mock_acl/acl/acl_base.h` | mock ACL 头/语法检查资产，不代表真机运行 |
| `tests/mock_acl/acl/acl_rt.h` | mock ACL 头/语法检查资产，不代表真机运行 |
| `tests/mock_acl/acl/ops/acl_cblas.h` | mock ACL 头/语法检查资产，不代表真机运行 |
| `tests/mock_acl/syntax_check.sh` | mock ACL 头/语法检查资产，不代表真机运行 |
| `tests/test_artifacts.py` | artifact contract、metadata、ELF 依赖和 server harness 测试 |
| `tests/test_blas_surface.py` | experimental BLAS surface 测试 |
| `tests/test_cmake_compat.py` | CMake/Make/Ninja/pre/虚拟工具链/FindCUDA 兼容测试 |
| `tests/test_cann_api_probes.py` | CANN 官方头文件与 Runtime 链接探针测试 |
| `tests/test_cucc_rules.py` | cucc 规则、phase、query、response 和 NPU policy 测试 |
| `tests/test_repo_hygiene.py` | 仓库目录、文档入口、gitignore 和 tracked tree 卫生契约 |

### `third_party/`

| 路径 | 职责 |
|---|---|
| `third_party/nvidia_cuda_samples/Common/exception.h` | 必要的 NVIDIA CUDA Samples helper 头/说明；不代表完整 CUDA Toolkit |
| `third_party/nvidia_cuda_samples/Common/helper_cuda.h` | 必要的 NVIDIA CUDA Samples helper 头/说明；不代表完整 CUDA Toolkit |
| `third_party/nvidia_cuda_samples/Common/helper_functions.h` | 必要的 NVIDIA CUDA Samples helper 头/说明；不代表完整 CUDA Toolkit |
| `third_party/nvidia_cuda_samples/Common/helper_image.h` | 必要的 NVIDIA CUDA Samples helper 头/说明；不代表完整 CUDA Toolkit |
| `third_party/nvidia_cuda_samples/Common/helper_string.h` | 必要的 NVIDIA CUDA Samples helper 头/说明；不代表完整 CUDA Toolkit |
| `third_party/nvidia_cuda_samples/Common/helper_timer.h` | 必要的 NVIDIA CUDA Samples helper 头/说明；不代表完整 CUDA Toolkit |
| `third_party/nvidia_cuda_samples/README.md` | 必要的 NVIDIA CUDA Samples helper 头/说明；不代表完整 CUDA Toolkit |

### `tools/`

| 路径 | 职责 |
|---|---|
| `tools/link.json` | Make/pre 使用的虚拟 CUDA 工具链布局清单 |
| `tools/mock/stub_link.json` | CMake configure 使用的 virtual_cmake 布局清单 |
| `tools/virtual_toolkit_env.sh` | CMake/Make/Ninja/pre 共用的虚拟 CUDA 工具链环境 helper |
| `tools/cmake_cce` | CMake configure/build wrapper；mock 到真实 cucc 双阶段入口 |
| `tools/compare_cubridge_parity.py` | 与 cu-bridge reference 的 backend-aware parity 报告工具 |
| `tools/cu2cce_driver.py` | cucc 与 CMake mock 共用的规则驱动核心 |
| `tools/device_arch.sh` | Ascend dav-* 架构解析（环境、npu-smi、cache、default） |
| `tools/gfortran_cce` | host-only Fortran/MPI wrapper；拒绝 CUDA source 误用 |
| `tools/hostcc_cce` | host compiler wrapper；link 注入 bridge/CANN runtime |
| `tools/make_cce` | Make wrapper；虚拟工具链目录中的 nvcc 入口到 cucc 的构建入口 |
| `tools/mock/CMakeCUDACompilerABI.cpp` | CMake ABI mock 辅助源码 |
| `tools/mock/CMakeCompilerABI.h` | CMake ABI mock 辅助源码 |
| `tools/mock/mock_nvcc.py` | CMake version/ABI/identity 探针 mock |
| `tools/mpi_cce` | host-only Fortran/MPI wrapper；拒绝 CUDA source 误用 |
| `tools/ninja_cce` | Ninja wrapper；build tree 虚拟工具链恢复和真实构建入口 |
| `tools/mock/nvcc_entry.sh` | CMake configure 阶段的 mock nvcc 入口 |
| `tools/pre_cce` | autotools/configure 环境准备 wrapper |
| `tools/real_test/README.md` | 真机回归辅助、外部样本或 URL 安全工具 |
| `tools/real_test/blas_regression.sh` | 真机回归辅助、外部样本或 URL 安全工具 |
| `tools/real_test/build_bridge.sh` | 在 CANN 主机上构建并安装 bridge binary tree |
| `tools/real_test/clean.sh` | 清理真机回归构建、安装树和日志 |
| `tools/real_test/diag/README.md` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/diag/acl_alloc_probe.cpp` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/diag/cann_api_surface_probe.sh` | CANN 官方 Runtime/SIMT/HCCL 头文件存在性和 include 编译探针 |
| `tools/real_test/diag/cann_runtime_link_probe.sh` | CANN Runtime 库选择、最小链接、NEEDED 和 RPATH 探针 |
| `tools/real_test/diag/acl_compose_probe.cpp` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/diag/acl_diff_probe.cpp` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/diag/echo.cce` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/diag/run_acl_compose_probe.sh` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/diag/run_acl_diff_probe.sh` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/diag/run_acl_probe.sh` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/diag/run_iterate_malloc.sh` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/diag/run_minimal_echo.sh` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/diag/run_trunc_raw.sh` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/diag/trunc_raw.c` | 非阻塞 ACL/runtime/backend 诊断资产 |
| `tools/real_test/env_probe.sh` | 只读 CANN/NPU/ccec 环境探测 |
| `tools/real_test/fetch_external_cuda_samples.sh` | 真机回归辅助、外部样本或 URL 安全工具 |
| `tools/real_test/inventory_simt_api.sh` | 真机回归辅助、外部样本或 URL 安全工具 |
| `tools/real_test/override_probe.sh` | 真机回归辅助、外部样本或 URL 安全工具 |
| `tools/real_test/probe_device.cpp` | 真机回归辅助、外部样本或 URL 安全工具 |
| `tools/real_test/server_cmd.txt` | 唯一 Ascend 真机 full/fast/external 回归入口 |
| `tools/real_test/validate_fetch_url.py` | 真机回归辅助、外部样本或 URL 安全工具 |
| `tools/real_test/verify_artifacts.py` | 真实构建产物类型、依赖、RPATH 和 metadata 检查器 |
| `tools/validate_cucc_rules.py` | cucc JSON 规则 schema/冲突校验器 |

## 8. 中文文档分区入口

| 路径 | 职责 |
|---|---|
| `docs/01_架构设计/README.md` | 架构设计区导航和编译/运行时数据流 |
| `docs/02_能力清单/README.md` | 能力矩阵、状态定义和边界入口 |
| `docs/03_文件目录/README.md` | 文件目录区的快速问题定位 |
| `docs/99_归档/README.md` | 历史方案、旧版设计和参考资料说明 |
| `docs/04_对比与验证/README.md` | 对比、审计和真机验证文档导航 |
| `docs/99_归档/README.md` | 归档范围和历史资料使用约定 |
