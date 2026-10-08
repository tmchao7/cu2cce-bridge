# cu2cce-bridge 架构概览

> 面向新员工的技术入门文档。读完本文你将了解：项目做了什么、改了哪些编译入口、
> CUDA 代码如何映射到昇腾 NPU。详细能力边界见 [`CAPABILITY.md`](CAPABILITY.md)。

---

## 1. 项目做什么

cu2cce-bridge 是 **CUDA → 昇腾 NPU 的兼容层**。

用户保留原始 `.cu` 源码（`__global__`、`threadIdx`、`<<<>>>`、`cudaMalloc` 等
全部原样），通过本桥接层在 Ascend 950 NPU 上编译运行。**源码零改写**。

目标环境：Ascend 950 / CANN 9.x / Linux x86_64。

---

## 2. 全景图

```
┌─────────────── 用户侧（不感知桥的存在）────────────────┐
│  原始 .cu / main.cpp / CMakeLists.txt / Makefile        │
└──────┬──────────────────────────────┬───────────────────┘
       │ cucc（替代 nvcc）            │ cmake_cce / make_cce / ninja_cce
       │                              │ （替代 cmake / make / ninja）
       ▼                              ▼
┌─────────────────── 桥接层（本仓库）─────────────────────┐
│                                                         │
│  编译侧                                                 │
│  ├── cucc：nvcc 风格参数 → ccec 参数（规则驱动）         │
│  ├── 头文件注入：cuda_runtime.h → simt_adaptor.h         │
│  └── 最终调用：ccec -x asc --enable-simt                 │
│                                                         │
│  运行时侧                                               │
│  └── libcce_bridge_runtime.so                            │
│      cudaMalloc → wcudaMalloc → aclrtMalloc              │
│      cudaMemcpy → wcudaMemcpy → aclrtMemcpy              │
│      cudaStream  → wcudaStream  → aclrtStream            │
│      ...（全部 cuda* API 走这条桥）                      │
│                                                         │
└────────────────────────┬────────────────────────────────┘
                         ▼
              CANN ccec / Ascend NPU
```

一句话：**编译时把 nvcc 命令变成 ccec 命令，运行时把 cuda API 调用变成 aclrt API 调用**。

---

## 3. 我们改了什么

### 3.1 编译命令：nvcc → cucc → ccec

用户原来用 `nvcc` 编译 `.cu`，现在改用 `cucc`。`cucc` 是一个 shell 入口
（`bin/cucc`），内部调用 Python 规则驱动器（`tools/cu2cce_driver.py`），
把 nvcc 风格的参数翻译成 ccec 风格的参数。

```
用户执行                          桥内部转换                           最终执行
cucc -c vec.cu -o vec.o   →   参数规则匹配/过滤/注入   →   ccec -x asc --enable-simt ...
```

**cucc 做的事：**

| 操作 | 说明 |
|---|---|
| 参数翻译 | `-arch=sm_80` 等 NVIDIA 架构参数被识别并丢弃（Ascend 无意义） |
| 参数注入 | 自动加 `-x asc --enable-simt --cce-aicore-arch=dav-c310-vec` |
| 头文件注入 | 自动加 `-I <bridge>/include`，预包含 `cuda_runtime.h` |
| 库注入 | 链接时自动加 `-lcce_bridge_runtime` + CANN 运行时库 |
| 拒绝不支持的 | `--ptx`、`--cubin`、`-rdc` 等明确报错拒绝 |
| FP64 策略 | 编译前扫描 device 区域的 double 使用，按策略 error/warn/allow 处理 |

**cucc 不做的事：**
- 不读改用户源码内容
- 不生成中间 `.cce` 文件
- 不做源码级 lowering 或转换

规则配置在 `bin/conf.json`（数据驱动，五种动作：reject/drop/replace/forward/inject）。

### 3.2 构建系统包装器：cmake/make/ninja → *_cce

大型 CUDA 工程通常用 CMake 或 Make 构建，里面写的是 `nvcc`。我们不修改用户的
`CMakeLists.txt` 或 `Makefile`，而是提供三个包装器入口：

| 用户原来用的 | 改用 | 做了什么 |
|---|---|---|
| `cmake` | `cmake_cce` | 生成一个"虚拟 CUDA 工具链目录"，让 CMake 探测时以为有 nvcc；实际编译时 nvcc 被替换为 cucc |
| `make` | `make_cce` | 同上，为 Makefile 建立虚拟工具链环境 |
| `ninja` | `ninja_cce` | 同上，为 Ninja 构建建立虚拟工具链环境 |

**虚拟 CUDA 工具链目录**的核心思路：

```
CMake 探测阶段                          CMake 编译阶段
cmake_cce                               cmake_cce --build
  → 生成虚拟目录（bin/nvcc → mock）       → 虚拟目录的 nvcc 自动切换为真 cucc
  → CMake 以为有标准 CUDA toolkit          → 实际走 cucc → ccec
  → 探测通过，生成 build 系统              → 编译到 Ascend NPU
```

这样用户的 `CMakeLists.txt` 里的 `project(... LANGUAGES CUDA)`、
`add_executable`、`find_package(CUDAToolkit)` 等全部不需要改。

### 3.3 CUDA → Ascend NPU 的 Bridge 映射

这是项目的核心。映射分两层：

#### 3.3.1 Device 侧（Kernel 代码里的 CUDA 语法）

通过头文件 `include/bridge/kernel/simt_adaptor.h` 做符号级别名映射。
ccec 编译 `.cu` 时自动预包含这个头文件。

| CUDA 写法 | 映射到 | 说明 |
|---|---|---|
| `__shared__` | `__ubuf__` | shared memory → Ascend Unified Buffer |
| `__syncthreads()` | `asc_syncthreads()` | 块内同步 |
| `__threadfence()` | `asc_threadfence()` | 内存栅栏 |
| `atomicAdd(a,b)` | `asc_atomic_add(a,b)` | 原子加 |
| `atomicSub/Min/Max/Exch` | `asc_atomic_*` | 原子操作族 |
| `__shfl_down_sync(mask,v,delta)` | `asc_shfl_down(v,delta,warpSize)` | warp shuffle（mask 被求值后忽略） |
| `__all_sync/__any_sync/__ballot_sync` | `asc_all/any/ballot` | warp vote |
| `__device__` | `__SIMT_DEVICE_FUNCTIONS_DECL__` | 执行空间关键字 |
| `__global__` | （ccec 原生解析） | 不映射，ccec 直接识别 |
| `threadIdx/blockIdx/blockDim/gridDim` | （ccec 原生 builtins） | 不映射，ccec 直接提供 |

**关键：只做宏别名，不改写 kernel 入口、launch 语法或参数。**
用户写的 `kernel<<<grid,block>>>(args)` 原样交给 ccec 解析。

#### 3.3.2 Host 侧（CUDA Runtime API 调用）

通过动态库 `libcce_bridge_runtime.so` 做函数级桥接。用户代码 `#include <cuda_runtime.h>`
后调用的 `cudaMalloc` 等函数，实际走的是 bridge 里的 `wcudaMalloc`，再转发到 CANN
的 `aclrtMalloc`。

```
用户代码: cudaMalloc(&ptr, size)
      ↓  (头文件宏替换)
bridge:  wcudaMalloc(&ptr, size)
      ↓  (函数调用)
CANN:    aclrtMalloc(&ptr, size, ACL_MEM_MALLOC_HUGE_FIRST)
```

已桥接的核心 API 族：

| CUDA API | Bridge 函数 | CANN API |
|---|---|---|
| `cudaMalloc` / `cudaFree` | `wcudaMalloc` / `wcudaFree` | `aclrtMalloc` / `aclrtFree` |
| `cudaMemcpy` / `cudaMemcpyAsync` | `wcudaMemcpy*` | `aclrtMemcpy*` |
| `cudaMemset` | `wcudaMemset` | `aclrtMemset` |
| `cudaStreamCreate` / `Destroy` / `Sync` | `wcudaStream*` | `aclrtStream*` |
| `cudaEventCreate` / `Record` / `Query` / `ElapsedTime` | `wcudaEvent*` | `aclrtEvent*` |
| `cudaGetDeviceCount` / `cudaSetDevice` | `wcudaGetDevice*` | `aclrtGetDevice*` |
| `cudaDeviceSynchronize` | `wcudaDeviceSynchronize` | `aclrtSynchronizeDevice` |
| `cudaGetLastError` | `wcudaGetLastError` | bridge 内部 error state |

**不支持的** API 返回 `ACL_ERROR_API_NOT_SUPPORT`，不静默成功。

---

## 4. 关键目录速查

```
bin/cucc                  编译驱动入口（替代 nvcc）
bin/conf.json             cucc 参数规则配置（数据驱动）
tools/cu2cce_driver.py    规则解释器（cucc 核心）
tools/cmake_cce           CMake 包装器
tools/make_cce            Make 包装器
tools/ninja_cce           Ninja 包装器
tools/mock/mock_nvcc.py   CMake 探测应答器（虚拟 nvcc）
include/cuda_runtime.h    用户可见的 CUDA 兼容头（入口）
include/bridge/kernel/    Device 侧 SIMT 原语映射头
  ├── simt_adaptor.h      __shared__/__syncthreads/atomic 等
  └── cuda_device_api.h   warp/vote/reduce/扩展 atomic
include/bridge/runtime/   Host 侧 Runtime wrapper 头
src/bridge/runtime/       wcuda* → aclrt* 实现源码
src/bridge/blas/          wcublas* → aclblas* 实验性实现
examples/puresimt/        25 个真机验证用例
tools/real_test/          真机回归入口
```

---

## 5. 我们不做什么

| 不做的 | 原因 |
|---|---|
| Cube Core 矩阵加速 | 只走 Vector Core (`dav-c310-vec`)，SIMT 模型不覆盖 Cube |
| Tensor Core / WMMA | Ascend SIMT 无 warp-level matrix fragment API |
| cutlass / cuDNN / cuFFT / cuSPARSE / cuRAND | 无 Ascend 等价 bridge |
| device FP64 / double | Ascend AICore 无 FP64 计算单元 |
| PTX / CUBIN / FATBIN | puresimt 消费 source，不产生 NVIDIA binary |
| 源码转换 / Python lowering | 旧路线已退出；puresimt 直通更可靠 |

完整能力边界见 [`CAPABILITY.md`](CAPABILITY.md)。

---

## 6. 一句话总结

```
编译侧：nvcc → cucc → ccec --enable-simt（参数翻译，源码不改）
运行侧：cuda* → wcuda* → aclrt*（函数桥接，API 对接）
构建侧：cmake/make/ninja → cmake_cce/make_cce/ninja_cce（虚拟工具链）
Device侧：__shared__ → __ubuf__、atomicAdd → asc_atomic_add（宏别名）
```
