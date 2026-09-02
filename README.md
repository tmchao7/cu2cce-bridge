# cu2cce-bridge

CUDA → 昇腾 NPU（CANN）的兼容层。项目参考沐曦 `cu-bridge` 的“用 CUDA 一门语言跑多硬件”
思路，但后端不是 MACA，而是 Ascend CANN `ccec` + SIMT。

**当前正式路线：** `main / puresimt`
**目标设备：** Ascend NPU（当前真机主线为 Ascend 950）
**目标编译器：** CANN `ccec`
**对标基线：** `cu-bridge-muxi` `6ab1076` / `3.8.0`

用户保留普通 CUDA `.cu` 源码，项目直接让 `ccec` 解析 CUDA SIMT：

```text
CUDA .cu
  → cucc / cmake_cce / make_cce / ninja_cce
  → ccec -x asc --enable-simt --cce-aicore-arch=dav-*
  → CUDA compatibility headers + bridge runtime
  → cuda* → wcuda* → aclrt*
  → Ascend NPU
```

`main` 不使用 Python kernel converter，不生成用户可见的 `.cce`、PTX、CUBIN、FATBIN 或
Ascend kernel 包装源文件。

## 当前状态（2026-09-02）

- `cucc`、CMake、Make、Ninja 已统一到同一个参数规则、fake toolkit 和 Ascend-aware
  产物契约；最近一次服务器 full profile 汇总为 `FAIL=0`。
- 已验证的构建产物类别包括 `.o`、`.d`、`.a`、`.so` 和 ELF executable，并检查
  `NEEDED`、RPATH/RUNPATH、CMake/Ninja metadata、install/export。
- `sm_*`/`compute_*` 只用于 CUDA 兼容宏或诊断；Ascend 编译目标必须是明确的 `dav-*`。
- `multi_cu_diagnostic` 的重复 registration helper 仍是 Ascend backend 的
  `DIAGNOSTIC_ONLY` 限制，不伪装成 CUDA device-link 能力。
- 本机 macOS 只能执行静态、mock 和单元测试，不能替代 CANN/NPU 真机证据。

## 目录与维护入口

仓库目录和每个 Git 跟踪文件的职责见：

- [`docs/REPOSITORY_FILE_GUIDE.md`](docs/REPOSITORY_FILE_GUIDE.md)：文件级职责、调用链、
  cu-bridge 命名映射和故障定位顺序；
- [`docs/README.md`](docs/README.md)：文档导航；
- [`docs/03_Comparison/RELEASE_READINESS_2026-09-02.md`](docs/03_Comparison/RELEASE_READINESS_2026-09-02.md)：
  code review、目录卫生和发布前审计。

当前目录职责保持与 `cu-bridge` 接近，但保留 Ascend 命名：

```text
bin/                 用户命令入口（cucc）
cmake/               原生 cu2cce CMake package
cmake_modules/       FindCUDA / FindCUDAToolkit 兼容层
config/              cucc 规则、fake toolkit 和 artifact contract
include/             CUDA compatibility headers、SIMT adaptor、runtime headers
src/                 bridge runtime 与 experimental BLAS 实现
examples/             CUDA、CMake、Make、Ninja 和诊断样例
tests/                本地规则、构建、产物和仓库卫生测试
third_party/          必要的 NVIDIA CUDA Samples helper headers
tools/                cucc driver、构建 wrapper、shim builder、真机脚本
docs/                 架构、范围、对比、参考和维护文档
```

目录命名的机制映射如下：

| `cu-bridge` 参考入口 | 本项目入口 | 后端差异 |
|---|---|---|
| `tools/cmake_maca` / `cmake_mock` | `tools/cmake_cce` | CANN/ccec；不做系统 CMake 物理替换 |
| `tools/make_maca` | `tools/make_cce` | host g++ + bridge/CANN runtime |
| `tools/ninja_maca` | `tools/ninja_cce` | 同一 CMake shim 的 Ninja 构建 |
| `tools/pre_makefile` | `tools/pre_cce` | configure/autotools 环境准备 |
| `bin/gnu` | `tools/hostcc_cce` | host link 注入 Ascend/bridge runtime |
| `link.json` / `stub_link.json` | `config/shim_layout.json` | fake CUDA toolkit 目录和 alias 清单 |
| `virtualDir.py` | `tools/build_shim_tree.py` | manifest-driven shim builder |

不要把 `cmake_cce` 改名为 `cmake_maca`，也不要在 Ascend 安装树中伪造 `mxcc`、
`symbol_cu.so` 或 MACA runtime。

## 二进制安装树：先构建，再使用

本节是使用者最需要看的部分。**当前开发和测试阶段，安装产物默认放在仓库的 `build` 目录，
不需要 `/opt` 或 `/usr/local` 权限。**

### 1. 获取源码并准备 CANN

```bash
cd /home/t50063365/workspace/repos
git clone git@gitcode.com:tmchao7/cu2cce-bridge.git
cd cu2cce-bridge
git switch main
```

设置 CANN 环境。不同 CANN 版本的脚本名称可能不同，优先使用机器上实际存在的脚本：

```bash
export CANN_BASE=/usr/local/Ascend/latest
export ASCEND_TOOLKIT_HOME=/usr/local/Ascend/ascend-toolkit/latest

if [ -f "$CANN_BASE/set_env.sh" ]; then
    source "$CANN_BASE/set_env.sh"
elif [ -f "$CANN_BASE/bin/setenv.bash" ]; then
    source "$CANN_BASE/bin/setenv.bash"
elif [ -f "$ASCEND_TOOLKIT_HOME/bin/setenv.bash" ]; then
    source "$ASCEND_TOOLKIT_HOME/bin/setenv.bash"
else
    echo "未找到 CANN 环境脚本，请确认 CANN 安装路径" >&2
    exit 1
fi
```

检查编译器和设备环境：

```bash
test -x "$CANN_BASE/bin/ccec" && echo "ccec is ready"
bash tools/real_test/env_probe.sh
```

`env_probe.sh` 只负责探测 CANN、ccec、SIMT headers、设备节点和可用架构；它的详细日志
位于 `build/real_test/logs/`，探测结果位于 `build/real_test/probe.env`。

### 2. 构建 bridge，产物默认在哪里

直接执行，不加 `--prefix`：

```bash
bash tools/real_test/build_bridge.sh
```

默认安装前缀是：

```text
<仓库根目录>/build/real_test/install
```

例如：

```text
/home/t50063365/workspace/repos/cu2cce-bridge/build/real_test/
├── cmake-build/       # bridge 自身的 CMake 构建中间目录
├── install/           # 可直接使用的二进制安装树（CUCC_PATH）
├── puresimt/          # 真机回归构建产物和日志
├── logs/              # env_probe 完整日志
└── probe.env          # env_probe 输出的环境变量
```

安装树的关键内容：

```text
build/real_test/install/
├── bin/
│   ├── cucc             # 直接编译/链接 CUDA .cu
│   ├── cmake_cce       # CMake configure/build wrapper
│   ├── make_cce        # Make wrapper
│   ├── ninja_cce       # Ninja wrapper
│   └── nvcc             # 兼容入口，指向 cucc
├── lib/
│   └── libcce_bridge_runtime.so
├── include/             # cuda_runtime.h、crt、bridge、SIMT、shadow headers
├── cmake_modules/       # FindCUDA、FindCUDAToolkit 和 CUDA override
├── config/              # cu2cce_rules、shim_layout、artifact_contract
└── tools/               # driver、mock、shim builder、host compiler wrapper
```

也可以显式写出同一个仓库内前缀：

```bash
bash tools/real_test/build_bridge.sh \
    --prefix "$PWD/build/real_test/install"
```

`--prefix /opt/cu2cce-bridge` 或 `--prefix /usr/local/cu2cce-bridge` 只是未来部署/商用
场景的选择，不是当前开发阶段的要求。当前不要把安装树写入 CANN 目录，也不要为了安装到
`/opt` 而修改源码中的绝对路径。

### 3. 使用安装树前需要 export 什么

构建完成后，在**每个需要使用 cucc 的新 shell**中设置以下变量：

```bash
export BRIDGE_ROOT=/home/t50063365/workspace/repos/cu2cce-bridge
export CUCC_PATH="$BRIDGE_ROOT/build/real_test/install"
export CUDA_PATH="$CUCC_PATH"
export CANN_BASE=/usr/local/Ascend/latest
export ASCEND_TOOLKIT_HOME=/usr/local/Ascend/ascend-toolkit/latest
export CU2CCE_NPU_ARCH=dav-c310-vec

export PATH="$CUCC_PATH/bin:$CUCC_PATH/tools:$PATH"
export LD_LIBRARY_PATH="$CUCC_PATH/lib:$CANN_BASE/lib64:$CANN_BASE/runtime/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
```

说明：

- `CUCC_PATH`：安装树根目录，`cucc`、CMake/Make/Ninja wrapper 和规则文件都从这里查找；
- `CUDA_PATH`：CUDA-shaped toolkit 的兼容路径，通常直接设为 `CUCC_PATH`；
- `CANN_BASE`：CANN runtime 根目录；
- `ASCEND_TOOLKIT_HOME`：CANN toolkit/compiler/include 根目录；
- `CU2CCE_NPU_ARCH`：明确的 Ascend `dav-*` 目标，例如 `dav-c310-vec`；
- `PATH`：让 shell 找到 `cucc`、`cmake_cce`、`make_cce`、`ninja_cce`；
- `LD_LIBRARY_PATH`：让生成的 ELF 找到 bridge 和 CANN runtime。

然后检查安装树：

```bash
command -v cucc
command -v cmake_cce
command -v make_cce
command -v ninja_cce
cucc --version
```

### 4. 可选：把环境保存到当前 shell

不要 `source` `build_bridge.sh`；该脚本会执行 `exit`。正确做法是 source CANN 环境脚本，
再执行上面的 `export`，或者在用户自己的 shell 初始化文件中保存环境：

```bash
cat >> ~/.bashrc <<'EOF'
export BRIDGE_ROOT=/home/t50063365/workspace/repos/cu2cce-bridge
export CUCC_PATH="$BRIDGE_ROOT/build/real_test/install"
export CUDA_PATH="$CUCC_PATH"
export CANN_BASE=/usr/local/Ascend/latest
export ASCEND_TOOLKIT_HOME=/usr/local/Ascend/ascend-toolkit/latest
export CU2CCE_NPU_ARCH=dav-c310-vec
export PATH="$CUCC_PATH/bin:$CUCC_PATH/tools:$PATH"
export LD_LIBRARY_PATH="$CUCC_PATH/lib:$CANN_BASE/lib64:$CANN_BASE/runtime/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
EOF
source ~/.bashrc
```

如果服务器 CANN 安装路径不同，请按实际路径替换，不要盲目复制示例路径。

## 直接使用 `cucc`

假设用户工程在仓库外：

```text
/home/t50063365/workspace/repos/my-cuda-app/
├── vector_add.cu
└── build/
```

进入用户工程并设置输出目录：

```bash
cd /home/t50063365/workspace/repos/my-cuda-app
mkdir -p build
```

### 1. 查看 cucc 版本

```bash
cucc --version
```

`cucc` 会显示诚实的 cu2cce-bridge 身份和兼容 CUDA release；它不会伪装成 MACA `mxcc`。

### 2. `.cu → .o + .d`

```bash
cucc -c vector_add.cu \
    -MD -MF build/vector_add.d \
    -o build/vector_add.o
```

产物：

```text
build/vector_add.o   # 标准 ELF relocatable object
build/vector_add.d   # Make 风格依赖文件
```

### 3. `.o → ELF executable`

```bash
cucc build/vector_add.o -o build/vector_add
./build/vector_add
```

`cucc` 在链接阶段会注入 bridge/CANN runtime 和 RPATH。最终程序是标准 ELF executable，
不是 `.cce`、`.ptx`、`.cubin` 或 `.fatbin`。

### 4. 直接 `.cu → ELF executable`

也支持直接编译并链接：

```bash
cucc vector_add.cu -o build/vector_add
./build/vector_add
```

对于大型工程，推荐使用“两步法”（先 `.o`，再统一链接），这样更容易进行增量构建和问题定位。

### 5. `.cu → shared object`

```bash
cucc -shared -fPIC vector_add.cu \
    -o build/libvector_add.so
```

产物：

```text
build/libvector_add.so   # 标准 ELF shared object
```

如果需要确认依赖：

```bash
file build/vector_add build/libvector_add.so
readelf -h build/vector_add
readelf -d build/vector_add | grep -E 'NEEDED|RPATH|RUNPATH'
```

### 6. 规则调试和 dryrun

```bash
CU2CCE_DEBUG=1 \
    cucc -c vector_add.cu -o build/vector_add.o

CU2CCE_TRACE_FILE=/tmp/cu2cce-trace.json \
    cucc --dryrun -c vector_add.cu -o build/vector_add.o
```

`--dryrun` 只打印最终准备交给 ccec 的命令，不执行编译；trace 记录参数决策，不记录 CUDA
源文件内容。

## 使用 CMake

CMake 工程不需要修改用户的 `.cu` 源码；推荐始终用 `cmake_cce` 完成 configure 和 build。
普通 `cmake --build` 有时也能工作，但会绕过 wrapper 的 build-phase dispatch，不作为推荐入口。

### CMake 最小工程

用户工程的 `CMakeLists.txt` 可以保持普通 CUDA 写法：

```cmake
cmake_minimum_required(VERSION 3.18)
project(my_cuda_app LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)
add_executable(my_cuda_app vector_add.cu)
```

### Configure 和 build

在用户工程根目录执行：

```bash
cmake_cce -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CUDA_ARCHITECTURES=OFF
cmake_cce --build build --parallel
./build/my_cuda_app
```

`cmake_cce` configure 阶段使用 fake CUDA toolkit/mock nvcc 完成 CMake 探测；
`cmake_cce --build` 阶段会把同一个 shim 的 `nvcc` 切换到真实 `cucc`，再由 `cucc → ccec`
生成真实产物。CMake 的 `build/` 下通常可以看到：

```text
build/
├── CMakeCache.txt
├── CMakeFiles/
├── CMakeFiles/<target>.dir/*.o
├── build.make 或 build.ninja
├── link.txt 或 CMakeFiles/rules.ninja
└── my_cuda_app
```

### CMake 的 static/shared/executable

```cmake
add_library(kernel_static STATIC kernel.cu)
add_library(kernel_shared SHARED kernel.cu)
add_executable(app main.cpp)
find_package(CUDAToolkit REQUIRED)
target_link_libraries(app PRIVATE kernel_static kernel_shared CUDA::cudart)
```

对应产物：

```text
build/libkernel_static.a
build/libkernel_shared.so
build/app
```

`CUDA::cudart` 在本项目中表达 bridge-backed CUDA runtime，不代表完整 NVIDIA CUDA Toolkit。
未验证的 `CUDA::cublas/cufft/cusparse/...` 不会被自动伪造。

### CMake legacy FindCUDA

常见的旧工程仍可尝试：

```cmake
find_package(CUDA REQUIRED)
cuda_add_executable(legacy_app legacy_app.cu)
```

仍不支持的 NVIDIA 专属产物/流程会明确失败：

```text
PTX / CUBIN / FATBIN / CUDA separable compilation / device-link / RDC
```

### CMake 参数转发

`cmake_cce --build` 支持常见形式：

```bash
cmake_cce --build build
cmake_cce --build=build --target app
cmake_cce --build build -t app --parallel 4
cmake_cce --build build --parallel
cmake_cce --build build --config Release
cmake_cce --build build --clean-first
cmake_cce --build build --verbose
cmake_cce --build build -- app-specific-generator-argument
```

Ninja Multi-Config 工程在存在 `build-Release.ninja` 等文件时会选择对应配置；Makefiles 和
Ninja 最终产物类别保持一致。

如果多个项目或多个 bridge 版本并发 configure/build，建议为每个项目设置不同的：

```bash
export CU2CCE_CMAKE_SHIM_DIR=/tmp/my-cuda-app-cmake-shim
```

未设置时，CMake 默认 shim 会按 canonical build directory 生成稳定隔离 key。

## 使用 Make

Makefile 可以继续使用 `nvcc`，不需要把它改写成 `ccec`。例如：

```make
NVCC ?= nvcc
CXX  ?= g++
BUILD := build

$(BUILD)/kernel.o: kernel.cu
	mkdir -p $(BUILD)
	$(NVCC) -c $< -MD -MF $(BUILD)/kernel.d -o $@

$(BUILD)/app: $(BUILD)/kernel.o main.cpp
	$(CXX) $(BUILD)/kernel.o main.cpp -o $@ -L$(CUDA_PATH)/lib64 -lcudart
```

使用安装树中的 Make wrapper：

```bash
make_cce -C . -j2
./build/app
```

`make_cce` 会：

1. 创建 CUDA-shaped fake toolkit；
2. 设置 `CUDA_PATH`/`CUDA_HOME`、CANN 路径和 host compiler wrapper；
3. 让 Makefile 中的 `nvcc` 进入 `cucc`，再进入 `ccec`；
4. 让 plain host link 能在已验证形态下找到 bridge/CANN runtime。

Make 产物通常位于用户工程自己的 `build/`：

```text
build/kernel.o
build/kernel.d
build/lib*.a
build/lib*.so
build/app
```

清理：

```bash
make_cce -C . clean
```

## 使用 Ninja

### 方式 A：通过 CMake 生成 Ninja 工程

```bash
cmake_cce -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CUDA_ARCHITECTURES=OFF
cmake_cce --build build --parallel
./build/my_cuda_app
```

### 方式 B：直接运行 ninja_cce

```bash
ninja_cce -C build
```

也支持：

```bash
ninja_cce -C build -j2
ninja_cce -C build -v
ninja_cce -C build -t clean
```

`ninja_cce` 会优先读取现有 `CMakeCache.txt` 中记录的 `CUDA_DIR/bin/nvcc`，确保增量构建继续
使用同一份 shim；没有旧 cache 时则按 build directory 创建稳定的默认 shim。

Ninja 的构建 metadata 主要包括：

```text
build/build.ninja
build/CMakeFiles/rules.ninja
build/CMakeFiles/<target>.dir/*.o
build/<executable or library>
```

## `pre_cce` 和 host compiler wrapper

autotools/configure 风格工程可以先通过：

```bash
pre_cce ./configure --prefix="$PWD/build/install"
pre_cce make -j2
```

`pre_cce` 只准备 CUDA-shaped toolkit 和 CANN 环境，然后使用原始 argv 执行命令；它不通过
`eval` 重构用户参数。

如果工程或脚本直接调用 `g++`/`gcc`，可使用：

```bash
export PATH="$CUCC_PATH/bin:$CUCC_PATH/tools:$PATH"
```

fake toolkit 中的 host compiler entry 会经过 `hostcc_cce`；host-only compile 透传系统编译器，
link 阶段按规则注入 bridge/CANN runtime。`.cu` 源必须交给 `cucc`，不能绕过到 host compiler。

## CANN/NPU 边界

以下能力没有经过等价的 Ascend 验证，因此不会为了“看起来像 CUDA”而伪造：

- `mxcc`、`-fgpu-rdc`、`--maca-link`、`runtime_cu.so`、`symbol_cu.so`；
- PTX、CUBIN、FATBIN、CUDA device-link、RDC/separable compilation；
- 完整 CUDA driver API、UVM/managed memory 和未验证 CUDA math/communication libraries；
- 多 `.cu` 直接聚合到同一 static archive 的无条件成功；
- 将 NVIDIA `sm_*` 架构直接传给 Ascend ccec。

当前支持状态和证据：

- [`PURESIMT_COMPATIBILITY_MATRIX.md`](docs/03_Comparison/PURESIMT_COMPATIBILITY_MATRIX.md)
- [`BUILD_SYSTEM_COMPATIBILITY_MATRIX.md`](docs/03_Comparison/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md)
- [`CUCC_OPTION_COMPATIBILITY_MATRIX.md`](docs/03_Comparison/CUCC_OPTION_COMPATIBILITY_MATRIX.md)
- [`RELEASE_READINESS_2026-09-02.md`](docs/03_Comparison/RELEASE_READINESS_2026-09-02.md)

## 真机回归（维护者）

这是验证 bridge 本身和四层构建入口的命令，不是普通使用者每次都需要执行的步骤：

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
REMOTE="${CU2CCE_REGRESSION_REMOTE:-origin}"
git fetch "$REMOTE" main
git pull --ff-only "$REMOTE" main
bash tools/real_test/clean.sh
CU2CCE_REGRESSION_REMOTE="$REMOTE" \
CU2CCE_REGRESSION_PROFILE=full \
  bash tools/real_test/server_cmd.txt
```

最终应区分：

```text
FAIL=0
DEVICE_HEALTHY=1
TEST_BLOCKED_BY_DEVICE=0
```

并查看对应的：

```text
PURESIMT_REGRESSION_PASS
CMAKE_CCE_REGRESSION_PASS
BUILD_COMPATIBILITY_REGRESSION_PASS
CUCC_RULES_REGRESSION_PASS
LARGE_PROJECT_REGRESSION_PASS
CUDA_SAMPLES_CMAKE_REGRESSION_PASS
ARTIFACT_CONTRACT_REGRESSION_PASS
```

详细产物日志在：

```text
build/real_test/puresimt/artifacts/
```

## 相关文档

- [`QUICKSTART.md`](QUICKSTART.md)：从零开始的用户操作手册；
- [`docs/REPOSITORY_FILE_GUIDE.md`](docs/REPOSITORY_FILE_GUIDE.md)：逐文件维护/故障定位；
- [`docs/02_Architecture/03_cucc_driver.md`](docs/02_Architecture/03_cucc_driver.md)：cucc 驱动；
- [`docs/02_Architecture/05_cmake_design.md`](docs/02_Architecture/05_cmake_design.md)：CMake 机制；
- [`docs/03_Comparison/CMAKE_MAKE_ALIGNMENT_WITH_CUBRIDGE.md`](docs/03_Comparison/CMAKE_MAKE_ALIGNMENT_WITH_CUBRIDGE.md)：
  与 cu-bridge 的 CMake/Make/Ninja 对齐；
- [`docs/03_Comparison/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md`](docs/03_Comparison/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md)：
  构建与产物矩阵；
- [`docs/03_Comparison/CUCC_OPTION_COMPATIBILITY_MATRIX.md`](docs/03_Comparison/CUCC_OPTION_COMPATIBILITY_MATRIX.md)：
  cucc 参数矩阵。

## 本地验证（维护者）

macOS 本地不构建 CANN/NPU 产物，但可以执行：

```bash
python3 tools/validate_cucc_rules.py config/cu2cce_rules.json
python3 -m unittest discover -s tests -p 'test_*.py' -v
bash tests/mock_acl/syntax_check.sh
git diff --check
```

修改 `src/`、`include/`、`CMakeLists.txt`、`tools/` 或规则后，必须重新构建安装树并在
Ascend 服务器运行 full profile；不要把本地 mock 结果描述成真机通过。
