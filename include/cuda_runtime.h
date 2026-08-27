/*
 * CUDA Runtime 兼容入口头文件。
 *
 * 仿照 cu-bridge 的 include/cuda_runtime.h：用户代码 #include <cuda_runtime.h> 时
 * 落入此文件，再转到底层 __cuda_runtime.h，最终由宏表把 CUDA 符号替换为 wcuda（经 acl 原生 API）。
 */
#if !defined(__CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__)
#define __CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__

#include<__cuda_runtime.h>

/* NVIDIA parity: the crt compatibility surface travels with <cuda_runtime.h>
 * exactly like upstream. These are forward-only headers — see include/crt/
 * for the per-header honesty boundaries (no fabricated device math). */
#include <crt/host_defines.h>
#include <crt/common_functions.h>
#include <crt/math_functions.h>
#include <crt/vector_functions_compat.h>

#if !defined(__CUDACC_RTC__)
#define EXCLUDE_FROM_RTC
#include "cuda_runtime_api.h"
#undef EXCLUDE_FROM_RTC
#endif /* !__CUDACC_RTC__ */

#undef __CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__
#endif
