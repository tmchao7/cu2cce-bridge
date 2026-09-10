/*
 * cu2cce-bridge 内部入口头文件。
 *
 * 仿照 cu-bridge 的 include/__cuda_runtime.h：把 wrapper 声明 + 宏表 + 昇腾 ACL 头
 * 组装成一个整体。用户代码 #include <cuda_runtime.h> 后，所有 CUDA host 侧符号
 * 都会被宏表替换为 wcuda，最终调用 aclrt* 原生 API。
 *
 * puresimt（2026-08-21 主线）：同一份源文件在 -x asc --enable-simt 下，
 * kernel 是纯 CUDA 前端直通（__global__/threadIdx/<<<>>> 原语由毕昇处理），
 * host 侧经本头把 cuda* → wcuda* → aclrt*；acl 原生类型/函数由
 * bridge/runtime/cuda_to_cce_mcr_adaptor.h 的宏表（cudaStream_t→aclrtStream）与
 * wrapper 声明（wcudaStreamCreate(aclrtStream*)）接住。
 *
 * ⚠️ 与 cu-bridge 的差异（2026-08-19 真机定稿）：**本头不 include 系统标准库头**
 * （ccec/clang15 + glibc 2.39 不兼容，真机实测）。用户代码的
 * #include <stdio.h> 等保持原样，由 cucc 注入的 shadow 遮蔽头（include/shadow/）
 * 优先命中，提供最小 C 运行库声明，不碰 glibc。
 */
#if !defined(__CUDA_RUNTIME_H__)
#define __CUDA_RUNTIME_H__

#if defined CUDA_VERSION
#define WCUDA_VERSION CUDA_VERSION
#else
#define CUDA_VERSION WCUDA_VERSION
#endif

#define __DRIVER_TYPES_H__
#define ENABLE_CUDA_TO_CCE_ADAPTOR

#include "bridge/runtime/cuda_runtime_wrapper.h"
#include "bridge/runtime/cuda_to_cce_mcr_adaptor.h"
#include "bridge/runtime/cuda_wrapper_internal.h"
#include "bridge/runtime/cuda_wrapper_types.h"

/* Large C++/CUDA projects can opt into the real system math declarations.
 * The default runtime header does not include shadow/math.h transitively: a
 * CUDA sample may include libstdc++'s <math.h> later, and predeclaring the
 * shadow functions would conflict with glibc exception specifications. Minimal
 * tests that need math declarations explicitly preinclude shadow/math.h. */
#if defined(__CU2CCE_SYSTEM_LIBC__)
#include <math.h>
#endif

/* puresimt kernel 原语（__shared__/__syncthreads/atomicAdd）在 -x asc 直通下
 * 由本适配头映射到昇腾 SIMT（__ubuf__/asc_syncthreads/asc_atomic_add）。
 *
 * These headers pull in CANN simt_api device headers that use __aicore__
 * (=[aicore]), which is only valid under ccec --enable-simt. Host-only .cpp
 * translation units get -D__CU2CCE_HOST_ONLY__ from the driver (conf.json
 * source_is_not_cuda condition) and must skip them, or [aicore] is parsed as
 * a C++17 structured binding and fails. .cu files — whether via cucc or
 * direct ccec --enable-simt — do not define this marker and include normally. */
#if !defined(__CU2CCE_HOST_ONLY__)
#include "bridge/kernel/simt_adaptor.h"
#include "bridge/kernel/cuda_device_api.h"
#endif

/* puresimt host（-x asc 直通）也在 acl/acl.h 可用空间内，但 bridge 头优先：
 * 用户代码 #include <cuda_runtime.h> 时，cuda* 由上面的宏表接住。 */

#endif //__CUDA_RUNTIME_H__
