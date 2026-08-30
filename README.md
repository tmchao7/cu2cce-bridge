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

## 当前进度（2026-08-24）

- main 已切换为 puresimt 直通：`cucc` 不再调用 Python kernel converter，不生成
  legacy `.cce`/kernel wrapper；直通检查已在真机通过。
- 当前已验证通过 23 个 CUDA/runtime/SIMT 用例，覆盖算术、逻辑、比较、转换、
  shared/global memory、原子、归约、warp、vote、scan、transpose、stencil、event、
  pitched copy、multi-kernel pipeline、3D indexing、动态 shared memory、fence、
  FP16/half2 与 BF16/BF16x2。
- 当前唯一验收标志仍是 `PURESIMT_REGRESSION_PASS`；BLAS bridge 保留为 experimental
  secondary capability，不阻塞 puresimt。

## 项目原理与设计文档

完整的项目原理、设计哲学、编译链、Runtime bridge、SIMT 运行逻辑和维护方法见：
[docs/02_Architecture/](docs/02_Architecture/01_global_overview.md)（五篇：全局认知/运行时深度/cucc 驱动/shadow 头/CMake 设计）。

## 兼容性矩阵

完整的 CUDA→Ascend puresimt 证据矩阵、23 个真机用例、API 状态和 BLAS 边界见：
[PURESIMT_COMPATIBILITY_MATRIX.md](docs/03_Comparison/PURESIMT_COMPATIBILITY_MATRIX.md)。

## Quickstart：运行你自己的 `.cu`

下面示例假设你在 Ascend 真机上工作。建议把自己的工程放在仓库外，避免和
`examples/puresimt/` 的回归样例混在一起，例如：

```text
/home/t50063365/workspace/repos/cu2cce-demo/
├── vector_add.cu
└── build/
```

### 1. 准备源码

`vector_add.cu` 保持普通 CUDA 写法，不需要添加 ACL、Ascend C、`__simt_vf__`
或 `cce::async_invoke`：

```cpp
#include <cuda_runtime.h>
#include <stdio.h>

#define N 1024

__global__ void vectorAdd(const float *a, const float *b, float *c, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) c[i] = a[i] + b[i];
}

int main(void) {
    float hA[N], hB[N], hC[N];
    for (int i = 0; i < N; ++i) {
        hA[i] = (float)i;
        hB[i] = 2.0f;
    }

    float *dA = nullptr, *dB = nullptr, *dC = nullptr;
    cudaMalloc((void **)&dA, sizeof(hA));
    cudaMalloc((void **)&dB, sizeof(hB));
    cudaMalloc((void **)&dC, sizeof(hC));
    cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice);
    cudaMemcpy(dB, hB, sizeof(hB), cudaMemcpyHostToDevice);

    vectorAdd<<<(N + 255) / 256, 256>>>(dA, dB, dC, N);
    if (cudaGetLastError() != cudaSuccess) return 1;
    if (cudaDeviceSynchronize() != cudaSuccess) return 1;
    if (cudaMemcpy(hC, dC, sizeof(hC), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int i = 0; i < N; ++i) {
        if (hC[i] != (float)i + 2.0f) return 1;
    }
    printf("user vectorAdd OK (N=%d)\n", N);
    cudaFree(dA); cudaFree(dB); cudaFree(dC);
    cudaDeviceReset();
    return 0;
}
```

### 2. 拉取、准备 CANN 和构建 bridge

```bash
cd /home/t50063365/workspace/repos

git clone git@gitcode.com:tmchao7/cu2cce-bridge.git
cd cu2cce-bridge
git checkout main

export ASCEND_TOOLKIT_HOME=/usr/local/Ascend/ascend-toolkit/latest
export ASCEND_HOME_PATH=/usr/local/Ascend/latest
source /usr/local/Ascend/latest/set_env.sh
export ASCEND_OPP_PATH=/usr/local/Ascend/cann-9.2.0/opp

bash tools/real_test/env_probe.sh
bash tools/real_test/build_bridge.sh
```

如果仓库已经存在，只需：

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git pull --ff-only gitcode main
source /usr/local/Ascend/latest/set_env.sh
bash tools/real_test/build_bridge.sh
```

### 3. 用 `cucc` 编译和链接

推荐使用两步法。第一步使用 `-c` 只编译并生成 `.o` 目标文件，第二步才链接生成
最终可执行文件：

```bash
export CUCC_PATH=/home/t50063365/workspace/repos/cu2cce-bridge/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$PATH

mkdir -p /home/t50063365/workspace/repos/cu2cce-demo/build

cucc -c /home/t50063365/workspace/repos/cu2cce-demo/vector_add.cu \
    -o /home/t50063365/workspace/repos/cu2cce-demo/build/vector_add.o

cucc /home/t50063365/workspace/repos/cu2cce-demo/build/vector_add.o \
    -o /home/t50063365/workspace/repos/cu2cce-demo/build/vector_add
```

也不要把 `.o` 目标文件当作最终程序运行。`cucc` 会自动注入 CUDA 兼容头、CANN SIMT 头、运行库和 rpath；`.cu` 会直接
交给 ccec，不会生成 Python converter 或 Ascend kernel 包装源文件。

### 4. 运行

```bash
/home/t50063365/workspace/repos/cu2cce-demo/build/vector_add
```

预期输出：

```text
user vectorAdd OK (N=1024)
```

### 5. 用户代码的边界

- 可以继续使用 CUDA `.cu`、CUDA runtime API、CUDA kernel 语法和 `<<<>>>`。
- 不要在用户源码中直接调用 `acl*`、书写 `__simt_vf__`、`__global__[aicore]`
  或 `cce::async_invoke`。
- 修改 bridge 的 `src/` 或 `include/` 后，需要重新执行 `build_bridge.sh`。
- `tools/real_test/clean.sh` 只清理仓库的 `build/real_test`，不会删除仓库外的
  `cu2cce-demo`。

## 真机回归

唯一入口：

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git pull --ff-only gitcode main
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

成功标志：

```text
FAIL=0
DEVICE_HEALTHY=1
PURESIMT_REGRESSION_PASS
```

## CMake integration (experimental)

核心 CUDA CMake 工程可通过 `cmake_cce` 使用 cucc/ccec，目标是不修改用户
`CMakeLists.txt` 和 `.cu` 源码：

```bash
export CUCC_PATH=/path/to/cu2cce-bridge/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$PATH
# 可选：将 CMake shim 保存在持久目录，避免 /tmp 清理影响后续 cmake --build。
export CU2CCE_CMAKE_SHIM_DIR=/tmp/cu2cce-cmake-shim
cmake_cce -S examples/cmake_demo -B /tmp/cu2cce-cmake-build
cmake --build /tmp/cu2cce-cmake-build
/tmp/cu2cce-cmake-build/cmake_vector_add
```

当前状态：核心 CMake 路径已在 Ascend 真机验证通过，扩展 CUDA 工具链能力仍为
`EXPERIMENTAL`。2026-08-25 的 `tools/real_test/server_cmd.txt` 已通过：

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
- `examples/external/cuda_samples_project/`：固定版本 NVIDIA CUDA Samples 的 CMake 外部工程 harness。
- `examples/puresimt/`：维护中的 CUDA examples。
- `tools/real_test/server_cmd.txt`：唯一主线真机回归入口。
- `tools/real_test/diag/`：非主线 ACL/runtime 排障工具。
- `docs/README.md`：文档导航和阅读顺序。
- `docs/02_Architecture/`：架构五篇（全局认知/深度解析/cucc 驱动/shadow 头/CMake 设计）。
- `docs/03_Scope/`：支持范围总览与边界限制。
- `docs/reference/asc-devkit-simt/`：Ascend SIMT API 参考资料。

## 用户 Quickstart

面向用户的 `.cu` 工程目录、CANN 环境、bridge 构建、`cucc` 编译、链接和运行示例见：
[QUICKSTART.md](QUICKSTART.md)。

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
