/*
 * cu2cce-bridge 内部入口头文件。
 *
 * 仿照 cu-bridge 的 include/__cuda_runtime.h：把 wrapper 声明 + 宏表 + 昇腾 ACL 头
 * 组装成一个整体。用户代码 #include <cuda_runtime.h> 后，所有 CUDA host 侧符号
 * 都会被宏表替换为 wcuda，最终调用 aclrt* 原生 API。
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

#endif //__CUDA_RUNTIME_H__
