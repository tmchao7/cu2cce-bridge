# cu2cce-bridge Quickstart

> **本指南只适用于有 Ascend NPU 和 CANN 的 Linux 真机。**
>
> 当前正式路线是 `main / puresimt`：用户保留原始 CUDA `.cu`，由 CANN `ccec`
> 直接解析 CUDA SIMT kernel，在昇腾 NPU 上运行。
>

## 你需要记住的三件事

1. 你的 CUDA 源码放在自己的工程目录中，推荐放在仓库外。
2. 先构建 bridge，得到 `build/real_test/install/`。
3. 使用安装目录里的 `cucc` 编译和链接你的 `.cu`，然后直接运行生成的程序。

完整真机回归入口（三画像）：

```bash
CU2CCE_REGRESSION_PROFILE=fast     bash tools/real_test/server_cmd.txt   # 日常，≈40s
CU2CCE_REGRESSION_PROFILE=external bash tools/real_test/server_cmd.txt   # 外部官方样本，≈30s
CU2CCE_REGRESSION_PROFILE=full     bash tools/real_test/server_cmd.txt   # 全量，≈3min
```

> 文档地图：**支持范围与限制** → `docs/02_能力清单/`；**架构原理五篇** →
> `docs/01_架构设计/`；**验证记录** → `docs/04_对比与验证/`。

---

## 1. 环境要求

目标环境：

```text
Ascend 950PR/950DT
CANN 9.2.0 或兼容的 CANN 9.x
CANN ccec
Linux x86_64 真机
```

默认路径假设为：

```text
CANN toolkit: /usr/local/Ascend/ascend-toolkit/latest
CANN runtime: /usr/local/Ascend/latest
```

如果你的 CANN 安装路径不同，需要相应修改环境变量。

---

## 2. 获取或更新 cu2cce-bridge

### 第一次使用：克隆仓库

```bash
cd <你的工作目录>
git clone https://gitcode.com/tmchao7/cu2cce-bridge.git
cd cu2cce-bridge
git checkout main
```

后文以 `$HOME/workspace/repos/cu2cce-bridge` 作为示例仓库路径，可按实际位置替换。

### 已经克隆过：更新仓库

```bash
cd <仓库根目录>
REMOTE="${CU2CCE_REGRESSION_REMOTE:-origin}"
git fetch "$REMOTE" main
git pull --ff-only "$REMOTE" main
```

> GitCode URL 在不同 checkout 中可能命名为 `origin` 或 `gitcode`；使用
> `CU2CCE_REGRESSION_REMOTE` 指定实际 remote，不要把它改成 GitHub 地址。

---

## 3. 设置 CANN 环境（必需步骤）

**每个新终端窗口都必须先执行**（真机实测：不 source 就直接 `cucc -c/-o`
会报一堆 ccec/ACL 库路径错误）：

```bash
source /usr/local/Ascend/latest/bin/setenv.bash
```

它提供 ccec 路径、`ASCEND_TOOLKIT_HOME/ASCEND_HOME_PATH/ASCEND_OPP_PATH`
与 ACL 动态库路径。安装形态不同时的变体：

```bash
# 若上述文件不存在，尝试（部分版本命名不同）：
source /usr/local/Ascend/latest/set_env.sh
```

确认 ccec 存在：

```bash
test -x /usr/local/Ascend/latest/bin/ccec && echo "ccec is ready"
```

如果输出：

```text
ccec is ready
```

说明编译器路径基本正确。

> 说明：`tools/real_test/server_cmd.txt` 与 `build_bridge.sh` **内部会自动
> source**，走 harness 不需要手动做；但**手动使用 cucc/cmake_cce 时必须先
> source**。建议把 source 行写进 `~/.bashrc` 免去每次输入。

---

## CUDA 公共头文件与 Wrapper 对齐状态

项目除了 Runtime 宏替换表，还维护 `config/cuda_api_parity_inventory.json`，用于登记参考
`cu-bridge-muxi@6ab1076` 的 CUDA 公共 headers、bridge Wrapper、API 状态和 CANN 迁移路线。

```text
核心 Runtime/SIMT/FP16/BF16       → ACTIVE_ADAPTED
稳定但无 Ascend 等价的 API        → UNSUPPORTED_TYPED（返回 ACL_ERROR_API_NOT_SUPPORT）
Driver/Graph/Interop/复杂库 ABI    → RESERVED_NPU（#if 0 保留 future unlock point）
NVRTC/PTX/CUBIN/FATBIN             → REJECT
CUDA 高层库                        → 仅登记路线，未验证前不创建假库 alias
```

本地核对命令：

```bash
python3 tools/validate_cuda_header_wrapper_parity.py \
  --reference ../cu-bridge-muxi \
  --implementation . \
  --inventory config/cuda_api_parity_inventory.json
```

`missing_headers=0`、`missing_wrapper_entries=0` 和 `missing_api_entries=0` 只表示
维护入口完整，不等于所有 CUDA API 已得到 Ascend/CANN 的行为实现。

## 4. 构建 cu2cce-bridge

只在 Ascend 真机上执行。

### 4.1 环境探测

```bash
cd $HOME/workspace/repos/cu2cce-bridge
bash tools/real_test/env_probe.sh
```

它会检查：

- CANN toolkit/runtime；
- ccec；
- Ascend 设备节点；
- ACL 头文件和库；
- SIMT API 头；
- `dav-c310-vec` puresimt 编译条件。

### 4.2 构建并安装 bridge

```bash
bash tools/real_test/build_bridge.sh
```

成功后应存在：

```text
$HOME/workspace/repos/cu2cce-bridge/build/real_test/install/bin/cucc
$HOME/workspace/repos/cu2cce-bridge/build/real_test/install/include/
$HOME/workspace/repos/cu2cce-bridge/build/real_test/install/lib/libcce_bridge_runtime.so
```

安装目录完整产物树（`$CUCC_PATH` = 该 install 目录）：

```text
$CUCC_PATH/
├── bin/
│   ├── cucc                 # nvcc 兼容编译驱动（命令行路径）
│   ├── cmake_cce            # CMake 入口包装器（大型项目用这个）
│   ├── make_cce             # Make 入口包装器
│   └── ninja_cce            # Ninja 入口包装器
├── lib/
│   ├── libcce_bridge_runtime.so   # wcuda* → aclrt* 运行时
│   └── cmake/cu2cce-bridge/       # cu2cce-bridgeConfig.cmake（包定位）
├── include/                       # cuda_runtime.h / crt/ / bridge/ / shadow/ ...
├── bin/conf.json       # cucc 参数规则（数据驱动核心）
├── cmake_modules/                 # FindCUDAToolkit / override / FindCUDA
└── tools/                         # mock/nvcc_entry.sh / cu2cce_driver.py 等
```

构建系统生成物的真实回归由 `config/artifact_contract.json` 描述，服务器上由
`tools/real_test/verify_artifacts.py` 检查。它验证 `.o/.d/.a/.so/ELF`、CMake/Ninja
元数据、`NEEDED`、RPATH/RUNPATH，并拒绝 `.cce/.ptx/.cubin/.fatbin/.maca` 中间物。
详细检查结果位于 `build/real_test/puresimt/artifacts/`；成功时终端只打印一行摘要。

手动检查某个已经生成的构建目录：

```bash
python3 tools/real_test/verify_artifacts.py \
  --contract config/artifact_contract.json \
  --case cmake_demo \
  --root build/real_test/puresimt/cmake-build \
  --log /tmp/cmake-demo-artifacts.log
```

以后如果修改了 `src/`、`include/`、`CMakeLists.txt` 或 `bin/cucc`，需要重新执行：

```bash
bash tools/real_test/build_bridge.sh
```

---

## 5. 运行项目真机回归（三画像）

这是验证 bridge 是否正常的推荐第一步：

```bash
cd $HOME/workspace/repos/cu2cce-bridge
REMOTE="${CU2CCE_REGRESSION_REMOTE:-origin}"
git fetch "$REMOTE" main
git pull --ff-only "$REMOTE" main
CU2CCE_REGRESSION_PROFILE=fast bash tools/real_test/server_cmd.txt     # 日常，≈40s
# external（官方样本，≈30s）/ full（全量，≈3min）按需切换 PROFILE
```

成功标志：

```text
FAIL=0
DEVICE_HEALTHY=1
TEST_BLOCKED_BY_DEVICE=0
PURESIMT_REGRESSION_PASS（以及 CMAKE_CCE / BUILD_COMPATIBILITY /
CUDA_SAMPLES_CMAKE / CUCC_RULES / LARGE_PROJECT 等按画像给出）
```

最近一次完整 `full` profile 的验证记录见
[`docs/04_对比与验证/RELEASE_READINESS_2026-09-04.md`](docs/04_对比与验证/RELEASE_READINESS_2026-09-04.md)。

---

## 6. 编写你自己的 CUDA 文件

推荐把自己的工程放在 cu2cce-bridge 仓库外：

```text
$HOME/workspace/repos/cu2cce-demo/
├── vector_add.cu
└── build/
```

创建目录：

```bash
mkdir -p $HOME/workspace/repos/cu2cce-demo/build
cd $HOME/workspace/repos/cu2cce-demo
```

创建 `vector_add.cu`：

```bash
cat > vector_add.cu <<'CUDA_SOURCE'
#include <cuda_runtime.h>
#include <stdio.h>

#define N 1024

__global__ void vectorAdd(const float *a, const float *b, float *c, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        c[i] = a[i] + b[i];
    }
}

int main(void) {
    float hA[N], hB[N], hC[N];
    for (int i = 0; i < N; ++i) {
        hA[i] = (float)i;
        hB[i] = 2.0f;
        hC[i] = 0.0f;
    }

    float *dA = nullptr;
    float *dB = nullptr;
    float *dC = nullptr;

    if (cudaMalloc((void **)&dA, sizeof(hA)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dB, sizeof(hB)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dC, sizeof(hC)) != cudaSuccess) return 1;
    if (cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dB, hB, sizeof(hB), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    vectorAdd<<<(N + 255) / 256, 256>>>(dA, dB, dC, N);
    if (cudaGetLastError() != cudaSuccess) return 1;
    if (cudaDeviceSynchronize() != cudaSuccess) return 1;
    if (cudaMemcpy(hC, dC, sizeof(hC), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int i = 0; i < N; ++i) {
        if (hC[i] != (float)i + 2.0f) {
            printf("vectorAdd FAILED at %d: got %.1f expected %.1f\n",
                   i, hC[i], (float)i + 2.0f);
            return 1;
        }
    }

    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    cudaDeviceReset();
    printf("user vectorAdd OK (N=%d)\n", N);
    return 0;
}
CUDA_SOURCE
```

这就是普通 CUDA 代码。不要添加：

```text
acl*
__simt_vf__
__global__[aicore]
cce::async_invoke
__gm__
```

---

## 7. 编译和运行自己的 `.cu`

先设置 cu2cce-bridge 安装目录：

```bash
export CU2CCE_BRIDGE=$HOME/workspace/repos/cu2cce-bridge
export CUCC_PATH=$CU2CCE_BRIDGE/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$PATH
```

### 推荐：两步编译/链接

前置：已完成 §3 的 `source /usr/local/Ascend/latest/bin/setenv.bash`（否则
cucc 调 ccec/ACL 会报一堆路径错误）。

第一步使用 `-c` 只编译 CUDA 源码，生成 `.o` 目标文件；第二步才链接生成可执行文件。

```bash
cd $HOME/workspace/repos/cu2cce-demo
cucc -c vector_add.cu -o build/vector_add.o
cucc build/vector_add.o -o build/vector_add
./build/vector_add
```

预期输出：

```text
user vectorAdd OK (N=1024)
```

### 也可以一步完成

```bash
cucc vector_add.cu -o build/vector_add
./build/vector_add
```

如果要生成目标文件而不是可执行文件，使用：

```bash
cucc -c vector_add.cu -o build/vector_add.o
```

cucc 的参数适配由安装目录中的 `bin/conf.json` 驱动。需要查看每个参数
被保留、丢弃或注入的原因时，可以使用：

```bash
CU2CCE_DEBUG=1 cucc -c vector_add.cu -o build/vector_add.o
```

需要保存机器可读的决策记录时：

```bash
CU2CCE_TRACE_FILE=build/cu2cce_trace.json \
cucc -c vector_add.cu -o build/vector_add.o
```

`-gencode`、`-arch=sm_*` 等 NVIDIA GPU 架构参数不会被伪造成 Ascend 架构；Ascend
架构通过 `CU2CCE_NPU_ARCH` 设置。`--ptx`、`--cubin`、`--fatbin` 和 device-link
等无法保持 puresimt 语义的功能会明确失败。

CUDA device-side 扩展 API 默认关闭。当前主线只承诺已经在 Ascend950 puresimt
回归中验证的基础 SIMT/atomic 形态。以下接口需要目标 CANN/设备单独核验，只有显式
设置 `CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1` 才会暴露：

```text
__activemask / __reduce_max_sync / __reduce_min_sync
atomicCAS / atomicInc / atomicDec / atomicAnd / atomicOr / atomicXor
```

这个开关只表示允许实验性编译探针使用这些接口，不代表它们已经取得当前 NPU
目标的真机运行支持。`__threadfence_system()` 没有被降级成普通 fence，
`cudaMemcpyDefault` 也不会被猜测成 Device-to-Device。

旧版无 mask 的 `__shfl` 系列另有独立开关：
`CU2CCE_ENABLE_LEGACY_DEVICE_APIS=1`。默认关闭是因为 CANN 的 FP16 实现内部也会声明
名为 `__shfl` 的函数，全局宏会破坏其声明；优先使用 CUDA 的 `__shfl*_sync` 形式。

Runtime/device 替换表也提供可审计状态：`ACTIVE_DIRECT`、`ACTIVE_ADAPTED`、
`UNSUPPORTED_TYPED`、`RESERVED_NPU`、`DIAGNOSTIC_ONLY`。其中 Graph、IPC、mempool、
texture/surface 与 stream attribute 等无可靠 CANN 等价物的入口会保留类型化声明，并返回
`ACL_ERROR_API_NOT_SUPPORT`；其余未实现参考条目保留在 `#if 0` inventory，不会被静默映射。
在安装树或服务器上可运行：

```bash
python3 "$CUCC_PATH/tools/validate_cuda_api_parity.py" \
  --implementation "$CUCC_PATH/include/bridge/runtime/cuda_to_cce_mcr_adaptor.h" \
  --inventory "$CUCC_PATH/include/bridge/runtime/cuda_to_cce_reserved_parity.h" \
  --runtime-header "$CUCC_PATH/include/bridge/runtime/cuda_runtime_wrapper.h" \
  --status-inventory "$CUCC_PATH/config/cuda_api_parity_inventory.json"
```

`missing_entries=0` 仅表示替换表的每个参考 CUDA 名称都有明确归宿；它不表示上述高级 CUDA
语义已被 CANN 实现。

cucc 也会根据 `.o` 输出后缀兼容识别下面的写法：

```bash
cucc vector_add.cu -o build/vector_add.o
```

但推荐显式使用 `-c`，以免把编译阶段和链接阶段混淆。

---

## 8. 使用项目中的矩阵乘样例

可以把项目中的最小矩阵乘样例复制到自己的工程：

```bash
cp \
  $HOME/workspace/repos/cu2cce-bridge/examples/puresimt/matrixMul_minimal.cu \
  $HOME/workspace/repos/cu2cce-demo/matrix_mul.cu

cd $HOME/workspace/repos/cu2cce-demo
cucc -c matrix_mul.cu -o build/matrix_mul.o
cucc build/matrix_mul.o -o build/matrix_mul
./build/matrix_mul
```

---

## 9. 如何排查失败

### 编译失败

```bash
which ccec
ccec --version
printf 'ASCEND_HOME_PATH=%s\n' "$ASCEND_HOME_PATH"
printf 'ASCEND_TOOLKIT_HOME=%s\n' "$ASCEND_TOOLKIT_HOME"
```

确认使用的是：

```text
/usr/local/Ascend/latest/bin/ccec
```

### 运行时找不到 bridge 动态库

```bash
ls -l "$CUCC_PATH/lib/libcce_bridge_runtime.so"
ldd build/vector_add | grep -E 'cce_bridge|ascendcl|runtime'
```

如果 bridge 源码刚改过：

```bash
cd "$CU2CCE_BRIDGE"
bash tools/real_test/build_bridge.sh
```

然后重新编译、链接用户程序。

### Context / 107002

如果出现：

```text
No valid context is bound to the current thread
107002
```

确认用户程序加载的是最新 bridge：

```bash
ldd $HOME/workspace/repos/cu2cce-demo/build/vector_add \
  | grep libcce_bridge_runtime
```

然后重新构建 bridge，再重新编译、链接用户程序。

### 清理生成物

只清理 cu2cce-bridge 自己的真机构建目录：

```bash
cd "$CU2CCE_BRIDGE"
bash tools/real_test/clean.sh
```

这不会删除仓库外的 `cu2cce-demo`。

---


---

## CMake CUDA 工程（大型项目主线，真机验证）

如果已有标准 CUDA CMake 工程：

```cmake
project(cuda_demo LANGUAGES CXX CUDA)
add_executable(app main.cu)
```

可尝试只替换配置入口，不修改 `CMakeLists.txt`：

```bash
export CU2CCE_BRIDGE=$HOME/workspace/repos/cu2cce-bridge
export CUCC_PATH=$CU2CCE_BRIDGE/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$CUCC_PATH/tools:$PATH
# 虚拟工具链目录默认持久且按 build 目录隔离：~/cu2cce-bridge/cmake-virtual-toolkit-<build-key>。
# configure 与 build 共享同一虚拟工具链目录；如需固定位置或跨工具共享，再设置下面这行。
# export CU2CCE_CMAKE_SHIM_DIR=/tmp/cu2cce-cmake-virtual-toolkit

cmake_cce -S $HOME/workspace/repos/cu2cce-bridge/examples/cmake_demo \
    -B /tmp/cu2cce-cmake-build
cmake_cce --build /tmp/cu2cce-cmake-build     # build 阶段：虚拟工具链 nvcc 自动切换为真 cucc
/tmp/cu2cce-cmake-build/cmake_vector_add
```

CMake 的 CUDA compiler detection 会经过：

```text
cmake_cce → CMake CUDA 虚拟工具链目录 → nvcc_entry → mock_nvcc → cucc → ccec
```

**双阶段协议（与 cu-bridge 的 cmake_mock/make_maca 同构）**：configure 阶段
虚拟工具链 nvcc 走 mock（应答探测 + 编译器自检真编译回退）；用
`cmake_cce --build build` 触发 build 时，虚拟工具链 nvcc 自动切换为真 cucc
（绕过 mock）。经 `cmake --build` 直接构建也兼容（mock 保留真编译回退）。
虚拟工具链目录的 `lib64/` 提供 libcudart/libcuda → 桥运行时软链，纯 g++ 链接的
Make 工程也能解析 `-lcudart`。

**能力现状（详见 `docs/01_架构设计/05_cmake_design.md` 与构建矩阵）**：
真机验证覆盖 `project(... LANGUAGES CUDA)`、多目录 `add_subdirectory`、
多个 `.cu`、静态/动态 CUDA library、host+CUDA 混合、
`find_package(CUDAToolkit)` + `CUDA::cudart/toolkit/cudart_static/cudadevrt`
别名族、`install(EXPORT)`、基础 legacy `FindCUDA`、Unix Makefiles/Ninja、
跨目录生成文件依赖，以及 NVIDIA CUDA Samples 官方样例零改写接入
（vectorAdd/matrixMul/simpleStreams/simpleMultiCopy）。

仍不承诺：完整 CUDA device-link/RDC、separable compilation、
PTX/CUBIN/FATBIN、`CUDA::cublas` package target（cuBLAS 走独立实验线，
`CU2CCE_BLAS_TEST=1`），以及 multi-`.cu` 静态库（后端注册符号聚合缺口，
见 `docs/02_能力清单/LIMITS_AND_BOUNDARIES.md`）。

大型项目建议：`export CU2CCE_LIBC_MODE=system`（系统 glibc 头）；
上游/项目自设的 `CUDA_ARCHITECTURES sm_XX` 会被强制回 OFF（NVIDIA 架构
在 Ascend 无意义）。

如果工程要显式使用 cu2cce 自身运行库，可以在 `CMakeLists.txt` 中使用：

```cmake
find_package(cu2cce-bridge CONFIG REQUIRED)
target_link_libraries(app PRIVATE cu2cce::runtime)
```

这个 target 是 cu2cce-bridge 原生 CMake package，不等价于完整的 CUDA Toolkit。

### Makefile 项目

已有 Makefile 项目如果使用 `nvcc`，可以先尝试：

```bash
export CUCC_PATH=/path/to/cu2cce-bridge/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$PATH

make_cce -C . -j2
```

`make_cce` 会建立临时 CUDA Toolkit 目录，让 Makefile 中的 `nvcc` 进入同一套
`cucc → cu2cce_driver → ccec` 规则。

### Ninja 项目

对于通过 `cmake_cce -G Ninja` 生成的构建目录，可以使用标准命令：

```bash
cmake_cce -G Ninja -S . -B build
ninja_cce -C build
```

高级 CUDA device-link/RDC、PTX/CUBIN/FATBIN 和未验证的 CUDA libraries 会被明确
诊断或拒绝，不会生成伪造的 GPU 后端产物。

## 10. 构建环境与静态检查

Ascend 程序需要在安装 CANN、ccec 和 Ascend 设备的 Linux 环境中构建与运行。
源码修改后可以执行以下静态检查：

```bash
bash -n bin/cucc tools/real_test/server_cmd.txt tools/real_test/env_probe.sh
git diff --check
bash tests/mock_acl/syntax_check.sh
```

## 11. 一句话流程

```text
写普通 CUDA .cu
  → source CANN 环境
  → build_bridge.sh
  → 设置 CUCC_PATH / CUDA_PATH / PATH
  → cucc -c xxx.cu -o xxx.o   # 编译阶段
  → cucc xxx.o -o xxx         # 链接阶段
  → ./xxx                     # 运行最终可执行文件
  → ./xxx
```
