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

完整真机回归入口只有一个：

```bash
bash tools/real_test/server_cmd.txt
```

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
cd /home/t50063365/workspace/repos
git clone git@gitcode.com:tmchao7/cu2cce-bridge.git
cd cu2cce-bridge
git checkout main
```

### 已经克隆过：更新仓库

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git pull --ff-only gitcode main
```

> 项目实时开发远端是 `gitcode`。不要使用 GitHub `origin` 更新本项目。

---

## 3. 设置 CANN 环境

每个新的终端窗口都建议执行：

```bash
export ASCEND_TOOLKIT_HOME=/usr/local/Ascend/ascend-toolkit/latest
export ASCEND_HOME_PATH=/usr/local/Ascend/latest
source /usr/local/Ascend/latest/set_env.sh
export ASCEND_OPP_PATH=/usr/local/Ascend/cann-9.2.0/opp
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

---

## 4. 构建 cu2cce-bridge

只在 Ascend 真机上执行。

### 4.1 环境探测

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
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
/home/t50063365/workspace/repos/cu2cce-bridge/build/real_test/install/bin/cucc
/home/t50063365/workspace/repos/cu2cce-bridge/build/real_test/install/include/
/home/t50063365/workspace/repos/cu2cce-bridge/build/real_test/install/lib/libcce_bridge_runtime.so
```

以后如果修改了 `src/`、`include/`、`CMakeLists.txt` 或 `bin/cucc`，需要重新执行：

```bash
bash tools/real_test/build_bridge.sh
```

---

## 5. 运行项目完整真机回归

这是验证 bridge 是否正常的推荐第一步：

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git pull --ff-only gitcode main
bash tools/real_test/server_cmd.txt
```

成功标志：

```text
FAIL=0
DEVICE_HEALTHY=1
PURESIMT_REGRESSION_PASS
```

---

## 6. 编写你自己的 CUDA 文件

推荐把自己的工程放在 cu2cce-bridge 仓库外：

```text
/home/t50063365/workspace/repos/cu2cce-demo/
├── vector_add.cu
└── build/
```

创建目录：

```bash
mkdir -p /home/t50063365/workspace/repos/cu2cce-demo/build
cd /home/t50063365/workspace/repos/cu2cce-demo
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
export CU2CCE_BRIDGE=/home/t50063365/workspace/repos/cu2cce-bridge
export CUCC_PATH=$CU2CCE_BRIDGE/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$PATH
```

### 推荐：两步编译/链接

第一步使用 `-c` 只编译 CUDA 源码，生成 `.o` 目标文件；第二步才链接生成可执行文件。

```bash
cd /home/t50063365/workspace/repos/cu2cce-demo
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

cucc 的参数适配由安装目录中的 `config/cu2cce_rules.json` 驱动。需要查看每个参数
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
  /home/t50063365/workspace/repos/cu2cce-bridge/examples/puresimt/matrixMul_minimal.cu \
  /home/t50063365/workspace/repos/cu2cce-demo/matrix_mul.cu

cd /home/t50063365/workspace/repos/cu2cce-demo
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
ldd /home/t50063365/workspace/repos/cu2cce-demo/build/vector_add \
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

## CMake CUDA 工程（实验性）

如果已有标准 CUDA CMake 工程：

```cmake
project(cuda_demo LANGUAGES CXX CUDA)
add_executable(app main.cu)
```

可尝试只替换配置入口，不修改 `CMakeLists.txt`：

```bash
export CU2CCE_BRIDGE=/home/t50063365/workspace/repos/cu2cce-bridge
export CUCC_PATH=$CU2CCE_BRIDGE/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$CUCC_PATH/tools:$PATH
# 可选：使用持久 shim，避免系统清理 /tmp 后 build 目录失效。
export CU2CCE_CMAKE_SHIM_DIR=/tmp/cu2cce-cmake-shim

cmake_cce -S /home/t50063365/workspace/repos/cu2cce-bridge/examples/cmake_demo \
    -B /tmp/cu2cce-cmake-build
cmake --build /tmp/cu2cce-cmake-build
/tmp/cu2cce-cmake-build/cmake_vector_add
```

CMake 的 CUDA compiler detection 会经过：

```text
cmake_cce → CMake CUDA shim → nvcc_entry → mock_nvcc → cucc → ccec
```

当前属于实验性能力，已覆盖 `project(... LANGUAGES CUDA)`、多个 `.cu`、静态/动态
CUDA library、host+CUDA 混合、`find_package(CUDAToolkit)` + `CUDA::cudart`、
基础 legacy `FindCUDA` 以及 Unix Makefiles/Ninja。仍不承诺完整 CUDA device-link、
separable compilation、PTX/CUBIN/FATBIN 或 `CUDA::cublas` package target。

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
