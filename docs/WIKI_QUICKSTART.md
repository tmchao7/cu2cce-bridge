# cu2cce-bridge 快速上手

> 面向初次接触本项目的开发者的操作指南。跟着做一遍，10 分钟跑通第一个 CUDA → Ascend NPU 程序。
>
> **前提**：你有一台装了 CANN 9.x + Ascend 950 NPU 的 Linux 真机。

---

## 1. 环境检查

### 1.1 确认 CANN 已安装

```bash
ls /usr/local/Ascend/latest/bin/ccec
# 应输出该路径，说明 CANN 编译器存在
```

### 1.2 每个新终端先 source CANN 环境

```bash
source /usr/local/Ascend/latest/bin/setenv.bash
# 如果上面不存在，试：
# source /usr/local/Ascend/latest/set_env.sh
```

> **这步必须做**。不 source 的话 cucc 调 ccec/ACL 会报一堆路径错误。
> 建议把 source 行写进 `~/.bashrc` 省得每次输入。

### 1.3 确认 NPU 设备就绪

```bash
ls /dev/davinci0 /dev/davinci_manager /dev/hisi_hdc
# 三个设备节点都应存在
```

---

## 2. 获取并构建 cu2cce-bridge

### 2.1 克隆仓库

```bash
cd <工作目录>
git clone https://gitcode.com/tmchao7/cu2cce-bridge.git
cd cu2cce-bridge
git checkout main
```

### 2.2 构建 bridge（只需一次，改了源码才需要重新构建）

```bash
bash tools/real_test/build_bridge.sh
```

构建成功后会看到 `✅ 构建安装成功`，产物在：

```
build/real_test/install/
├── bin/
│   ├── cucc              # 编译驱动（替代 nvcc）
│   ├── cmake_cce         # CMake 包装器
│   ├── make_cce          # Make 包装器
│   └── ninja_cce         # Ninja 包装器
├── lib/
│   └── libcce_bridge_runtime.so   # 运行时桥接库
└── include/              # CUDA 兼容头文件
```

### 2.3 设置环境变量

```bash
export CU2CCE_BRIDGE="$(pwd)"   # 仓库根目录
export CUCC_PATH=$CU2CCE_BRIDGE/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$PATH
```

> 建议把上面 4 行也写进 `~/.bashrc`。

---

## 3. 跑通第一个 CUDA 程序

### 3.1 创建一个普通 CUDA 文件

```bash
mkdir -p ~/my_cuda_demo && cd ~/my_cuda_demo

cat > vector_add.cu <<'EOF'
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

    float *dA = nullptr, *dB = nullptr, *dC = nullptr;
    cudaMalloc((void **)&dA, sizeof(hA));
    cudaMalloc((void **)&dB, sizeof(hB));
    cudaMalloc((void **)&dC, sizeof(hC));
    cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice);
    cudaMemcpy(dB, hB, sizeof(hB), cudaMemcpyHostToDevice);

    vectorAdd<<<(N + 255) / 256, 256>>>(dA, dB, dC, N);
    cudaDeviceSynchronize();
    cudaMemcpy(hC, dC, sizeof(hC), cudaMemcpyDeviceToHost);

    for (int i = 0; i < N; ++i) {
        if (hC[i] != (float)i + 2.0f) {
            printf("FAILED at %d\n", i);
            return 1;
        }
    }

    cudaFree(dA); cudaFree(dB); cudaFree(dC);
    printf("vectorAdd OK (N=%d)\n", N);
    return 0;
}
EOF
```

这就是标准 CUDA 代码，没有任何 Ascend 专属语法。

### 3.2 编译和运行

```bash
# 两步法（推荐）
cucc -c vector_add.cu -o vector_add.o    # 编译
cucc vector_add.o -o vector_add          # 链接
./vector_add                              # 运行

# 或者一步法
cucc vector_add.cu -o vector_add
./vector_add
```

预期输出：

```
vectorAdd OK (N=1024)
```

**成功了！** 你的 CUDA 代码已经在 Ascend NPU 上跑起来了，源码零改写。

---

## 4. 用 CMake 构建大型项目

如果你的工程用 CMake 管理，不需要改 `CMakeLists.txt`，只需把 `cmake` 换成 `cmake_cce`：

### 4.1 标准 CMake CUDA 工程

假设你的 `CMakeLists.txt` 长这样：

```cmake
project(my_app LANGUAGES CXX CUDA)
add_executable(my_app main.cu kernel.cu)
```

### 4.2 用 cmake_cce 构建

```bash
# configure（CMake 探测阶段：虚拟工具链让 CMake 以为有 nvcc）
cmake_cce -S . -B build

# build（编译阶段：虚拟工具链的 nvcc 自动切换为真 cucc → ccec）
cmake_cce --build build --parallel

# 运行
./build/my_app
```

### 4.3 用 Ninja 生成器

```bash
cmake_cce -G Ninja -S . -B build
ninja_cce -C build
./build/my_app
```

### 4.4 用 Makefile 构建

如果工程直接用 Makefile + `nvcc`：

```bash
make_cce -C . -j2
```

`make_cce` 会建立临时 CUDA 工具链目录，让 Makefile 里的 `nvcc` 走到 `cucc → ccec`。

---

## 5. 跑项目自带的回归测试

验证 bridge 是否正常工作的最快方法：

```bash
cd <仓库根目录>
bash tools/real_test/clean.sh
CU2CCE_REGRESSION_PROFILE=fast bash tools/real_test/server_cmd.txt
```

三个画像可选：

| 画像 | 命令 | 耗时 | 说明 |
|---|---|---|---|
| fast（默认） | `CU2CCE_REGRESSION_PROFILE=fast bash tools/real_test/server_cmd.txt` | ~40s | 核心子集，日常用 |
| external | `CU2CCE_REGRESSION_PROFILE=external bash tools/real_test/server_cmd.txt` | ~30s | 官方 NVIDIA Samples |
| full | `CU2CCE_REGRESSION_PROFILE=full bash tools/real_test/server_cmd.txt` | ~3min | 全量回归 |

成功标志：

```
FAIL=0
DEVICE_HEALTHY=1
PURESIMT_REGRESSION_PASS
CMAKE_CCE_REGRESSION_PASS
```

---

## 6. 试跑项目自带示例

项目自带 25 个已验证的 CUDA 示例，可以直接编译运行：

```bash
cd <仓库根目录>

# 矩阵乘法（shared memory + __syncthreads）
cucc examples/puresimt/matrixMul_minimal.cu -o /tmp/mm
/tmp/mm
# 预期：matrixMul_minimal OK

# 原子操作
cucc examples/puresimt/atomicAdd_minimal.cu -o /tmp/atom
/tmp/atom
# 预期：atomicAdd_minimal OK

# Warp shuffle
cucc examples/puresimt/warpShuffle_minimal.cu -o /tmp/shfl
/tmp/shfl
# 预期：warpShuffle_minimal OK

# FP16
cucc examples/puresimt/half_minimal.cu -o /tmp/half
/tmp/half
# 预期：half_minimal OK
```

---

## 7. 常见问题排查

### 编译报错：找不到 ccec

```bash
which ccec
# 应输出 /usr/local/Ascend/latest/bin/ccec
# 如果没有，说明 CANN 环境 没 source：
source /usr/local/Ascend/latest/bin/setenv.bash
```

### 运行报错：找不到 libcce_bridge_runtime.so

```bash
ldd ./vector_add | grep cce_bridge
# 如果显示 "not found"，检查库路径：
ls $CUCC_PATH/lib/libcce_bridge_runtime.so
# 确认 LD_LIBRARY_PATH 包含该目录：
export LD_LIBRARY_PATH=$CUCC_PATH/lib:$LD_LIBRARY_PATH
```

### 运行报错：No valid context (107002)

```bash
# 确认设备存在：
ls /dev/davinci0
# 确认加载的是最新 bridge：
ldd ./vector_add | grep cce_bridge
# 如果还不行，重新构建 bridge 再重新编译：
cd $CU2CCE_BRIDGE && bash tools/real_test/build_bridge.sh
```

### 编译报错：double precision operation is not allowed

Ascend AICore **不支持 FP64**。device 代码里不能用 `double`。

```bash
# 解决方法：把 device 代码里的 double 改成 float
# 例如：double x → float x
#       2.0 → 2.0f
```

### 想看 cucc 到底执行了什么命令

```bash
CU2CCE_DEBUG=1 cucc -c vector_add.cu -o vector_add.o
# 会打印每个参数被保留/丢弃/注入的决策过程

# 或者只看最终命令不执行：
cucc --dryrun -c vector_add.cu -o vector_add.o
```

### 清理构建产物

```bash
cd $CU2CCE_BRIDGE
bash tools/real_test/clean.sh
# 只清理 bridge 自己的构建目录，不影响你的工程
```

---

## 8. 一句话流程

```
source CANN 环境 → build_bridge.sh → 设置环境变量 → cucc 编译 .cu → 运行
```

```
source /usr/local/Ascend/latest/bin/setenv.bash
bash tools/real_test/build_bridge.sh
export CUCC_PATH=$(pwd)/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$PATH
cucc my_code.cu -o my_app
./my_app
```

---

## 9. 注意事项

1. **每个新终端必须先 source CANN 环境**，否则编译/运行都会报错
2. **device 代码不能用 double**，Ascend 不支持 FP64
3. **不要在源码里加 Ascend 专属语法**（`acl*`、`__simt_vf__`、`__gm__` 等），写标准 CUDA 就行
4. **每个静态库最多一个 `.cu` 文件**，多个 `.cu` 聚合会有链接冲突（ccec 后端限制）
5. **只覆盖 Vector Core**，不覆盖 Cube Core / Tensor Core / cutlass
6. 完整能力边界见 [`CAPABILITY.md`](CAPABILITY.md)
