/*
 * cu2cce-bridge 内部入口头文件。
 *
 * 仿照 cu-bridge 的 include/__cuda_runtime.h：把 wrapper 声明 + 宏表 + 昇腾 ACL 头
 * 组装成一个整体。用户代码 #include <cuda_runtime.h> 后，所有 CUDA host 侧符号
 * 都会被宏表替换为 wcuda，最终调用 aclrt* 原生 API。
 *
 * ⚠️ 与 cu-bridge 的差异（2026-08-19 真机定稿）：**不 include 系统标准库头**。
 * ccec（clang 15）+ glibc 2.39（Ubuntu 24.04）的系统头不兼容（真机实测：
 * _Float32/_Float64/__FILE/ferror 等连锁错误），而昇腾能跑的例子（用户
 * vector.cce 等）都不 include 标准库头。因此本头改为提供**最小 C 运行库声明**
 * （printf/malloc/free/memset/memcpy 等，链接期由 libc 提供符号），用户代码
 * 应避免 include <stdio.h>/<stdlib.h>/<string.h> 等系统头（见 README）。
 */
#if !defined(__CUDA_RUNTIME_H__)
#define __CUDA_RUNTIME_H__

/* ---------------- 最小 C 运行库声明（替代系统标准库头） ----------------
 * ccec 编译用户 .cu 时，系统 glibc 头（stdio.h 等）编译不过（真机实测），
 * 故在此提供常用 C 函数的最小声明（仅 ccec 路径，由 cucc 传
 * -D__CU2CCE_NO_LIBC__ 触发）。size_t 用 unsigned long（x86_64 Linux 等价）。
 * 需要更多函数时在此追加。 */
#if defined(__CU2CCE_NO_LIBC__)
#if defined(__cplusplus)
extern "C" {
#endif
int printf(const char *, ...);
int sprintf(char *, const char *, ...);
int snprintf(char *, unsigned long, const char *, ...);
void *malloc(unsigned long);
void *calloc(unsigned long, unsigned long);
void free(void *);
void *memset(void *, int, unsigned long);
void *memcpy(void *, const void *, unsigned long);
int memcmp(const void *, const void *, unsigned long);
unsigned long strlen(const char *);
int strcmp(const char *, const char *);
#if defined(__cplusplus)
}
#endif
#endif /* __CU2CCE_NO_LIBC__ */

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
