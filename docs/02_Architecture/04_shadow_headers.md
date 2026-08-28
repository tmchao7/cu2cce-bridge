# shadow 头机制 与 libc 模式

> 回答一个问题：**用户 `.cu` 里 `#include <stdio.h>/<math.h>/<stdlib.h>` 时
> 到底发生了什么？** 涉及文件：`include/shadow/`、`include/crt/`、
> `include/bridge/kernel/simt_adaptor.h`、`tools/cu2cce_driver.py`。

## 1. 两种 libc 模式

| 模式 | 触发 | 行为 |
|---|---|---|
| `shadow`（默认） | 未设 `CU2CCE_LIBC_MODE=system` | 注入 `-I ${CUCC_PATH}/include/shadow` + `-D__CU2CCE_NO_LIBC__`：**声明式遮蔽头**顶替 glibc 头，宿主符号由链接期 `-lm`/libc 提供 |
| `system` | `CU2CCE_LIBC_MODE=system` | `-D__CU2CCE_SYSTEM_LIBC__`：直接用系统 glibc 头——**大型项目默认推荐**（glibc 2.39 宏/内建与 ccec 前端不完全兼容，shadow 是隔离手段） |

规则谓词：`has_shadow_compat = has_simt && libc_mode != system && shadow 目录存在`。

## 2. shadow 头的形态（include/shadow/）

- **声明-only**：`stdio.h/stdlib.h/string.h/math.h` 只提供 `extern` 声明
  （如 `double sin(double)`），不带 glibc 的宏/内建/`__attribute__` 包袱；
- 设备端数学不受影响——ccec 的 SIMT 数学来自 `simt_api`（经
  `include/crt/math_functions.h` 前向），与宿主 shadow 声明正交；
- 已覆盖：sin/cos/tan/asin/atan/exp/log/pow/sqrt/cbrt/floor/ceil/round/
  fabs/fmod/fmin/fmax 及 `f` 变体、`fmaf`。

## 3. crt 前向兼容层（include/crt/，随 `<cuda_runtime.h>` 自动生效）

| 头 | 内容 | 铁律 |
|---|---|---|
| `host_defines.h` | **文档化空壳**。`__host__/__device__` 是 ccec 语言关键字，#define 兜底会剥离执行空间 | 关键字不可经宏重定义 |
| `math_functions.h` | `__has_include(<simt_api/math_functions.h>)` 前向 + 双精度 UNSUPPORTED 边界 | 缺失符号让编译错误自己报名字 |
| `device_functions.h` / `common_functions.h` | 同上前向 | 无伪实现 |
| `vector_types.h` | **文档化空壳**：向量类型 bisheng 原生注入（ext_vector_type） | 原生已有的类型桥零定义 |
| `builtin_types.h` | 薄壳（不定义 ushort/uint 别名，防系统头冲突） | |
| `vector_functions_compat.h` | 文档化空壳：make_* 工厂原生存在但 device-only，host 层待证据 | |
| `include/device_launch_parameters.h` | NVIDIA 拼写占位（threadIdx 等是编译器内建） | |

设计原则：**链式自动包含的头必须惰性（inert）**——每个符号先在真机证实
存在/缺失，再决定转发或显式 `#error`；绝不投机补齐。

## 4. 宿主/设备同名的两个陷阱（真机实证）

1. `__host__/__device__` 是**关键字**：ccec 前端做真实执行空间检查，
   `#define` 兜底会把声明文本级剥离（曾致 `__device__ __host__` 函数静默
   退化为纯 host）。桥内映射一律走函数/类型层。
2. `.cu` 的 **host 代码禁裸 `malloc/free`**：bisheng 注入
   `__aicore__ void *malloc(uintptr_t)` 劫持该名字。宿主缓冲用
   `cudaMallocHost/cudaFreeHost` 或 `std::`/`new[]`。

## 5. 整数 min/max 的 host 可见性（simpleStreams 教训）

CANN `simt_api/math_functions.h` 把整数 min/max 标成仅 `__device__`，
NVIDIA 语义是 host+device 双向。桥的解法（`simt_adaptor.h`）：
全局命名空间**值返回模板**——模板与原生静态非模板声明永不合并，
设备端精确匹配优先级不变，宿主端获得此前缺失的可行候选。
逃生开关：`CU2CCE_NO_HOST_ARITHMETIC_OVERLOADS`。

## 6. 与外部头文件生态的关系

- NVIDIA Common helper 头（third_party/nvidia_cuda_samples/Common）按需
  经规则注入 include 路径（`has_samples_common`），源码零改写；
- 大型项目自有第三方头：优先 `CU2CCE_LIBC_MODE=system` + 项目自带头；
  桥只补 CUDA 面（cuda_runtime.h 链），不越界接管 POSIX/C++ 标准库。
