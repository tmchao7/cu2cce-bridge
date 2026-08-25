# CUDA 大型项目构建兼容性模型

## 1. 目标

cu2cce-bridge 面向大型 CUDA 项目时，不把“能执行一条 vectorAdd 命令”当作完整兼容。
兼容性分为四层：

```text
源代码兼容
  → 构建系统兼容
  → 编译器参数兼容
  → CUDA runtime/library 兼容
```

每层独立记录证据，避免把一个层面的成功误写成整个 CUDA 生态已支持。

## 2. 状态定义

| 状态 | 含义 |
|---|---|
| `SUPPORTED` | 已有明确真机证据，语义和结果均通过验证 |
| `EXPERIMENTAL` | 有实现或局部证据，但覆盖范围、语义或环境依赖仍有限 |
| `UNSUPPORTED` | 当前明确不能保持语义，或后端没有验证能力 |
| `DIAGNOSTIC_ONLY` | 可以识别并报告，但不会假装执行成功 |

以下判断必须分开：

```text
CMake configure 成功 ≠ 编译器兼容
编译成功 ≠ 运行时兼容
链接成功 ≠ CUDA library 语义等价
参数被丢弃 ≠ 参数语义被保留
```

## 3. 构建系统调用链

```text
CMake
  → cmake_cce
  → CUDA toolkit shim
  → nvcc_entry.sh
  → mock_nvcc.py
  → cu2cce_driver.py
  → ccec
```

Makefile：

```text
make_cce
  → CUDA toolkit shim
  → nvcc
  → cucc
  → cu2cce_driver.py
  → ccec
```

Ninja：

```text
cmake_cce -G Ninja
  → CMake-generated build.ninja
  → ninja_cce / cmake --build
  → configured nvcc shim
  → cucc
  → ccec
```

## 4. CMake 兼容边界

当前主要覆盖：

- modern `project(... LANGUAGES CUDA)`；
- 多 `.cu` translation unit；
- static/shared/object/interface target 的基础场景；
- host C++ + CUDA 混合；
- `find_package(CUDAToolkit)` 的 bridge-backed `CUDA::cudart`；
- legacy `FindCUDA` 基础宏；
- Unix Makefiles 和 Ninja；
- install/export/package 基础场景。

当前不把以下功能写成已支持：

```text
CUDA device-link
separable compilation / RDC
PTX/CUBIN/FATBIN
CUDA::cufft / CUDA::cusparse / CUDA::curand / CUDA::cudnn
未验证的 CUDA library 自动替换
```

## 5. cucc 参数兼容层

规则文件：

```text
config/cu2cce_rules.json
```

共享驱动：

```text
tools/cu2cce_driver.py
```

规则动作：

```text
forward   直接转发
 drop     警告后丢弃
replace   转换为已验证目标参数
inject    注入 bridge/CANN/ccec 参数
reject    明确拒绝
```

阶段：

```text
query
preprocess
dependency
compile
compile_and_link
link
device_link
```

驱动只处理参数，不读取或改写 CUDA 源码。NVIDIA GPU 架构不会映射成伪造的 Ascend
架构；Ascend 目标由 `CU2CCE_NPU_ARCH` 选择。

## 6. 大型工程排障

C/C++ header mode：

```bash
# 默认：shadow minimal libc headers，用于已验证的最小 puresimt 样例
CU2CCE_LIBC_MODE=shadow cucc -c kernel.cu -o kernel.o

# 外部大型 C++/CUDA 工程：使用系统 C/C++ headers
CU2CCE_LIBC_MODE=system cucc -c kernel.cu -o kernel.o
```

调试：

```bash
CU2CCE_DEBUG=1 cucc -c kernel.cu -o kernel.o
```

机器可读 trace：

```bash
CU2CCE_TRACE_FILE=build/cu2cce_trace.json \
cucc -c kernel.cu -o kernel.o
```

严格模式：

```bash
CU2CCE_STRICT_FLAGS=1 cucc ...
```

严格模式用于发现未验证的 CUDA library、未知长选项和潜在语义丢失；默认模式则对
明确无意义的 NVIDIA 参数给出 warning 并继续处理。
