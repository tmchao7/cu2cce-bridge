/*
 * cu2cce-bridge 内部入口头文件。
 *
 * 仿照 cu-bridge 的 include/__cuda_runtime.h：把 wrapper 声明 + 宏表 + 昇腾 ACL 头
 * 组装成一个整体。用户代码 #include <cuda_runtime.h> 后，所有 CUDA host 侧符号
 * 都会被宏表替换为 wcuda，最终调用 aclrt* 原生 API。
 *
 * 注意：本头被 bisheng 编译用户 .cu 时使用（-I$CUDA_PATH/include 注入），
 * 它既提供 host 侧 API 替换，也需兼容 device 侧（kernel）语法不冲突。
 * kernel 侧（__global__/<<<>>>）由 bisheng 原生识别，此处不做改动。
 */
#if !defined(__CUDA_RUNTIME_H__)
#define __CUDA_RUNTIME_H__

#include <stdio.h>
#include <stdlib.h>
/* glibc 2.39（Ubuntu 24.04）的 math.h 使用 C23 类型 _Float32，ccec（clang 15）
 * 不识别（真机实测 20 个错误，2026-08-19）。cucc 编译时传
 * -D__CU2CCE_NO_MATH_H__ 跳过系统 math.h；host 侧数学函数（sinf 等）的
 * 昇腾映射 TODO（见 ASCEND_RESEARCH_NOTES）。 */
#if !defined(__CU2CCE_NO_MATH_H__)
#include <math.h>
#endif
#include <string.h>
#ifdef __cplusplus
/* fstream/algorithm 会拉入 wchar.h 等 glibc 2.39 头链，在 ccec(clang15) 下
 * 兼容性问题多（真机实测 _Float32/__FILE 等）；cucc 编译时
 * -D__CU2CCE_NO_MATH_H__ 一并跳过。host 侧流/算法支持随昇腾兼容性推进再放开。 */
#if !defined(__CU2CCE_NO_MATH_H__)
#include <fstream>
#endif
#endif

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

#ifdef __cplusplus
#if !defined(__CU2CCE_NO_MATH_H__)
#include <algorithm>
#endif
#endif

#ifdef __CUDACC_EXTENDED_LAMBDA__
#include <functional>
#include <utility>
#endif

#endif //__CUDA_RUNTIME_H__
