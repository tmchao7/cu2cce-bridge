# cu2cce-bridge

CUDA→昇腾 NPU（CANN）的兼容层，参考 `cu-bridge` 的“用 CUDA 一门语言跑多硬件”
理念，目标是让原始 NVIDIA CUDA `.cu` 文件不改源码即可在 Ascend 950 / CANN 上运行。

## 当前正式路线：main / puresimt

Ascend 950 的 AI Core 支持 SIMT。main 不再走 Python kernel 转换或新同构包装路线，
而是让 CANN ccec 直接解析 CUDA SIMT 语法：

```text
原始 CUDA .cu
  → ccec -x asc --enable-simt --cce-aicore-arch=dav-c310-vec
  → CUDA runtime compatibility headers
  → cuda* → wcuda* → aclrt*
  → Ascend NPU
```

main 不生成 `__simt_vf__`、`__global__[aicore]`、`cce::async_invoke` 包装源文件。
`include/bridge/kernel/simt_adaptor.h` 只提供必要的 SIMT 原语别名。

## 当前状态（2026-09-02）

- main 使用 puresimt 直通：`cucc` 不调用 Python kernel converter，不生成
  `.cce/.ptx/.cubin/.fatbin` 等中间产物；CUDA `.cu` 直接进入
  `ccec -x asc --enable-simt --cce-aicore-arch=dav-*`。
- `cucc`、CMake、Make、Ninja 已统一到规则驱动参数适配、manifest fake toolkit、
  persistent shim 和 Ascend-aware artifact contract。
- 最新 Ascend 服务器 full profile 由维护者报告：`FAIL=0`、
  `DEVICE_HEALTHY=1`、`TEST_BLOCKED_BY_DEVICE=0`；详细证据以
  [`server_cmd.txt`](tools/real_test/server_cmd.txt) 输出为准。
- 本机 macOS 只执行静态/mock 测试，不能替代 CANN/NPU 验证。BLAS bridge 仍是
  experimental secondary capability，不改变主线回归结论。

## 二进制安装树：使用者优先

如果你只是要使用 CUDA 代码，不需要理解本仓库的全部源码。先在一台装有 CANN 和
Ascend NPU 的 Linux 机器上构建一次 bridge，得到可复制的二进制安装树：

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
source /usr/local/Ascend/latest/bin/setenv.bash  # 或该 CANN 版本提供的 set_env.sh
bash tools/real_test/env_probe.sh
bash tools/real_test/build_bridge.sh --prefix /opt/cu2cce-bridge
```

构建完成后，`/opt/cu2cce-bridge` 就是使用者侧的 SDK 前缀。它至少包含：

```text
/opt/cu2cce-bridge/
├── bin/cucc             # 直接编译/链接 CUDA .cu
├── bin/cmake_cce       # CMake 入口
├── bin/make_cce        # Make 入口
├── bin/ninja_cce       # Ninja 入口
├── lib/libcce_bridge_runtime.so
├── include/             # CUDA/SIMT/runtime 兼容头
├── cmake_modules/       # FindCUDA/FindCUDAToolkit 兼容模块
├── config/              # cucc 规则、shim 和 artifact 契约
└── tools/               # driver、shim builder 和 host wrapper
```

在另一台**相同 CPU 架构、兼容 CANN 版本和 Ascend NPU**的机器上使用时，只需要复制这个
安装树，并设置 CANN 与 bridge 环境：

```bash
source /usr/local/Ascend/latest/bin/setenv.bash
export CUCC_PATH=/opt/cu2cce-bridge
export CUDA_PATH=$CUCC_PATH
export CANN_BASE=/usr/local/Ascend/latest
export PATH=$CUCC_PATH/bin:$CUCC_PATH/tools:$PATH
export LD_LIBRARY_PATH="$CUCC_PATH/lib:$CANN_BASE/lib64:$CANN_BASE/runtime/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

cucc --version
mkdir -p build
cucc -c vector_add.cu -MD -MF build/vector_add.d -o build/vector_add.o
cucc build/vector_add.o -o build/vector_add
./build/vector_add
```

`cucc` 会将 CUDA `.cu` 直接交给 Ascend `ccec`，生成标准 `.o`/ELF 产物，不生成
`.cce`、`.ptx`、`.cubin` 或 `.fatbin`。二进制安装树不是完整 NVIDIA CUDA Toolkit，
仍然需要目标机安装匹配的 CANN 和可用 NPU；`sm_*` 不是 Ascend 架构，目标由
`CU2CCE_NPU_ARCH`/`dav-*` 规则选择。

已有 CMake/Make/Ninja 工程可以直接使用安装树中的入口：

```bash
cmake_cce -S . -B build
cmake_cce --build build --parallel
make_cce -C . -j
ninja_cce -C build
```

CMake 的 configure/build、Make/Ninja 产物和限制见
[`docs/REPOSITORY_FILE_GUIDE.md`](docs/REPOSITORY_FILE_GUIDE.md) 与
[`QUICKSTART.md`](QUICKSTART.md)。

## 项目原理与设计文档

完整的项目原理、设计哲学、编译链、Runtime bridge、SIMT 运行逻辑和维护方法见：
[docs/02_Architecture/](docs/02_Architecture/01_global_overview.md)（五篇：全局认知/运行时深度/cucc 驱动/shadow 头/CMake 设计）。

## 兼容性矩阵

完整的 CUDA→Ascend puresimt 证据矩阵、API 状态和 BLAS 边界见：
[PURESIMT_COMPATIBILITY_MATRIX.md](docs/03_Comparison/PURESIMT_COMPATIBILITY_MATRIX.md)。

构建系统、cucc 参数和实际产物的对齐证据见：
[BUILD_SYSTEM_COMPATIBILITY_MATRIX.md](docs/03_Comparison/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md)、
[CUCC_OPTION_COMPATIBILITY_MATRIX.md](docs/03_Comparison/CUCC_OPTION_COMPATIBILITY_MATRIX.md)
和[发布前收口审计](docs/03_Comparison/RELEASE_READINESS_2026-09-02.md)。

## 用户 Quickstart

用户工程目录、CANN 环境准备、bridge 构建、`cucc` 编译/链接/运行步骤统一维护在：
[QUICKSTART.md](QUICKSTART.md)。README 只保留项目定位、状态、能力边界和入口，避免
两份操作手册产生漂移。

## 真机回归

唯一入口：

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
REMOTE="${CU2CCE_REGRESSION_REMOTE:-origin}"
git fetch "$REMOTE" main
git pull --ff-only "$REMOTE" main
bash tools/real_test/clean.sh
CU2CCE_REGRESSION_REMOTE="$REMOTE" CU2CCE_REGRESSION_PROFILE=full \
  bash tools/real_test/server_cmd.txt
```

回归画像（`CU2CCE_REGRESSION_PROFILE`，默认 `fast`）：

- `fast`：设备门禁 + bridge 重编 + cucc 直通 + puresimt 核心子集
  （官方 vectorAdd/matrixMul + event/multiKernel/atomic/math minimal）+
  CMake 仅 cmake_demo 与 cmake_large_project。日常开发用，最短反馈。
- `external`：设备门禁 + bridge 重编 + 整个 External NVIDIA Samples 段
  （含诊断样本逐个编译运行）。改 wrapper/runtime 后定向验证。
- `full`：全部段落（含 legacy CMake、install/export、Ninja、multi_cu、
  make/ninja smoke 与外部样本），里程碑或冻结前使用。

```bash
CU2CCE_REGRESSION_PROFILE=full bash tools/real_test/server_cmd.txt
```

被跳过的段会打 `[SKIP] ... (profile=...)`，最终汇总输出对应
`*_SECTION_SKIPPED`；skip 既不计 FAIL 也绝不冒充 PASS。各段有 `[timing]`
计时行，慢点可直接定位。

当前回归包括：

- NVIDIA 官方 `vectorAdd`
- NVIDIA 官方 shared-memory `matrixMul`
- 最小 vector add
- 最小 tiled matrix multiply
- `atomicAdd`/`atomicMin`/`atomicMax`
- shared-memory tree reduction + cross-block atomic add
- FP16/half2 and BF16/BF16x2 conversion round-trips

完整 full profile 的主线标志：

```text
FAIL=0
DEVICE_HEALTHY=1
TEST_BLOCKED_BY_DEVICE=0
PURESIMT_REGRESSION_PASS
CMAKE_CCE_REGRESSION_PASS
BUILD_COMPATIBILITY_REGRESSION_PASS
CUCC_RULES_REGRESSION_PASS
LARGE_PROJECT_REGRESSION_PASS
CUDA_SAMPLES_CMAKE_REGRESSION_PASS
ARTIFACT_CONTRACT_REGRESSION_PASS
```

## CMake / Make / Ninja integration (Ascend-aware)

核心 CUDA CMake 工程可通过 `cmake_cce` 使用 cucc/ccec，目标是不修改用户
`CMakeLists.txt` 和 `.cu` 源码：

```bash
export CUCC_PATH=/path/to/cu2cce-bridge/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$PATH
# 可选：将 CMake shim 保存在持久目录，避免 /tmp 清理影响后续构建。
export CU2CCE_CMAKE_SHIM_DIR=/tmp/cu2cce-cmake-shim
cmake_cce -S examples/cmake_demo -B /tmp/cu2cce-cmake-build
cmake_cce --build /tmp/cu2cce-cmake-build
/tmp/cu2cce-cmake-build/cmake_vector_add
```

当前状态：核心 CMake、Make、Ninja 路径和已声明的产物契约已纳入 full profile；
扩展 CUDA 工具链能力仍按 `SUPPORTED/EXPERIMENTAL/UNSUPPORTED` 分层。
2026-09-02 的服务器 full profile 报告通过：

- 标准 CUDA CMake 工程（`project(... LANGUAGES CUDA)`）；
- 多 `.cu`、静态/动态 CUDA library 和 host+CUDA 混合工程 smoke；
- `find_package(CUDAToolkit)` 的 bridge-backed `CUDA::cudart`；
- 常见 legacy `find_package(CUDA)`、`cuda_add_executable` 和 `cuda_add_library`；
- Unix Makefiles 和 Ninja。

仍不承诺 CUDA device-link/separable compilation、PTX/CUBIN/FATBIN、
`CUDA::cublas` 等未完成或未真机验证的能力。

如果工程希望显式链接 cu2cce 自身的运行库，而不是使用 CUDA 兼容名称，
可以使用安装后提供的原生 CMake package：

```cmake
find_package(cu2cce-bridge CONFIG REQUIRED)
target_link_libraries(app PRIVATE cu2cce::runtime)
```

`cu2cce::runtime` 只表示 cu2cce-bridge 的桥接运行库，不代表完整 CUDA Toolkit。

## 四层命令与产物对齐

| 层 | 推荐入口 | 主要产物 | 关键验证 |
|---|---|---|---|
| cucc | `cucc -c` / `cucc <obj>` | `.o`、`.d`、ELF executable/shared object | `ccec` 命令、`NEEDED`、RPATH/RUNPATH |
| CMake | `cmake_cce -S/-B`、`cmake_cce --build` | `.o`、`.a`、`.so`、ELF、install/export | Makefiles/Ninja metadata 与 target graph |
| Make | `make_cce` | `.o`、`.d`、`.a`、`.so`、ELF | fake toolkit 生命周期、clean/cleanpack |
| Ninja | `ninja_cce` | `build.ninja`、`rules.ninja` 及同类最终产物 | configure/build 共用 `CUDA_DIR` |

Ascend 侧不伪造 MACA 的 `mxcc`、`symbol_cu.so`、device-link/RDC 或 PTX/CUBIN/FATBIN；
多 `.cu` 静态库的重复 registration helper 是已知 `DIAGNOSTIC_ONLY` 后端边界。

## Experimental BLAS branch

CUDA cuBLAS to CANN ACL CBLAS development is maintained separately on
`exp/cublas-aclblas`. See the [BLAS compatibility matrix](docs/03_Comparison/BLAS_COMPATIBILITY_MATRIX.md).

## 目录

- `include/`：CUDA runtime、CUDA BLAS 兼容头、SIMT 原语别名和 shadow headers。
- `src/bridge/runtime/`：`cuda* → wcuda* → aclrt*`。
- `src/bridge/blas/`：experimental cublas→aclblas，独立于 puresimt 验收。
- `bin/cucc`：规则驱动的 nvcc 兼容入口；`.cu` 直接传给 ccec，不做 Python lowering。
- `tools/cu2cce_driver.py`：cucc 与 CMake mock 共用的参数规则驱动（含 `--hostcc-mode`）。
- `config/cu2cce_rules.json`：CUDA 参数 filter/replacer/inject/reject + queries/hostcc/regex_cleanup 规则。
- `tools/cmake_cce`（安装树 `bin/cmake_cce`）：CMake CUDA 工程入口；配置用
  `cmake_cce -S . -B build`，构建用 `cmake_cce --build build`（按 build.ninja 分发 make/ninja）。
- `tools/make_cce` / `tools/ninja_cce`（安装树 `bin/` 同名）：Makefile/Ninja 构建入口。
- `tools/pre_cce`：configure 脚本包裹器（autotools FAQ Q5 等价）。
- `tools/hostcc_cce`：宿主编译器包装器（链接自动补桥运行时，规则驱动）。
- `tools/build_shim_tree.py` + `config/shim_layout.json`：假 CUDA SDK 目录的单一清单与构建器。
- `docs/03_Comparison/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md`：大型项目构建兼容矩阵。
- `docs/03_Comparison/RELEASE_READINESS_2026-09-02.md`：发布前 code review、产物和仓库卫生收口。
- `docs/REPOSITORY_FILE_GUIDE.md`：目录/文件职责、调用链和问题定位地图。
- `examples/external/cuda_samples_project/`：固定版本 NVIDIA CUDA Samples 的 CMake 外部工程 harness。
- `examples/puresimt/`：维护中的 CUDA examples。
- `tools/real_test/server_cmd.txt`：唯一主线真机回归入口。
- `tools/real_test/diag/`：非主线 ACL/runtime 排障工具。
- `docs/README.md`：文档导航和阅读顺序。
- `docs/02_Architecture/`：架构五篇（全局认知/深度解析/cucc 驱动/shadow 头/CMake 设计）。
- `docs/03_Scope/`：支持范围总览与边界限制。
- `docs/reference/asc-devkit-simt/`：Ascend SIMT API 参考资料。

## 构建要求

Ascend 目标程序需要在安装 CANN、ccec 和 Ascend 设备的 Linux 环境中构建与运行。
源码修改后的静态检查可以使用：

```bash
bash -n bin/cucc tools/real_test/server_cmd.txt tools/real_test/diag/*.sh
git diff --check
bash tests/mock_acl/syntax_check.sh
```

## 历史路线

旧的新同构/文本 lowering 实验位于 `exp/new-homogeneous-v1` 分支；main 不再维护该
路线。BLAS bridge 仍保留，但属于 experimental secondary capability。
