#ifndef __CUDA_BENCHMARK_TARGETS_H__
#define __CUDA_BENCHMARK_TARGETS_H__

/*
 * cu2cce-bridge —— 仿照 cu-bridge 的版本模拟机制。
 *
 * 模拟一个 CUDA 版本号（默认 11.6，对应 cu-bridge 默认；可用 CUCC_CUDA_VERSION 覆盖，
 * 由 bin/cucc 在编译时通过 -DW CUDA_VERSION=xxxx 注入，见 tools/ 目录规划）。
 * 目标是让应用代码里 `#if CUDA_VERSION >= 11000` 这类条件编译分支按预期工作。
 */

#if defined WCUDA_VERSION && !defined WCUDART_VERSION
#define WCUDART_VERSION WCUDA_VERSION
#endif

#if !defined WCUDA_VERSION && defined WCUDART_VERSION
#define WCUDA_VERSION WCUDART_VERSION
#endif

#ifndef WCUDART_VERSION
#define WCUDART_VERSION 11060
#endif

#ifndef WCUDA_VERSION
#define WCUDA_VERSION 11060
#endif

#ifdef WCUDART_VERSION
#define __WCUDACC_VER_MAJOR__ (WCUDART_VERSION / 1000)
#define __WCUDACC_VER_MINOR__ ((WCUDART_VERSION % 1000) / 10)
#define __WCUDACC_VER_BUILD__ 55 /* CUDA 11.6.0 的 build 值 */
#endif

#ifndef __WCUDA_API_VER_MAJOR__
#define __WCUDA_API_VER_MAJOR__ __WCUDACC_VER_MAJOR__
#endif

#ifndef __WCUDA_API_VER_MINOR__
#define __WCUDA_API_VER_MINOR__ __WCUDACC_VER_MINOR__
#endif

#if defined(__WCUDA_API_VER_MAJOR__) && defined(__WCUDA_API_VER_MINOR__)
#define __WCUDART_API_VERSION ((__WCUDA_API_VER_MAJOR__ * 1000) + (__WCUDA_API_VER_MINOR__ * 10))
#else
#define __WCUDART_API_VERSION WCUDART_VERSION
#endif

/* CUDA 运行时常见上限（仿照 cu-bridge，字段/枚举与 CUDA 对齐，取值按昇腾实测调整） */
#define MAX_STACK_SIZE                       0x80000000  /**< max stack size per thread can set*/
#define MAX_MALLOC_HEAP_SIZE                 0x41DE16420 /**< max heap size can set*/
#define MAX_PRINTF_FIFO_SIZE                 0x80000000  /**< max printf fifo size can set*/
#define MAX_DEV_RUNTIME_SYNC_DEPTH           0x18    /**< max device synchronize detpth can set*/
#define MAX_DEV_RUNTIME_PENDING_LAUNCH_COUNT 0x2E320 /**< max pending launch count can set*/

#ifdef _WIN32
#ifdef CUBRIDGE_EXPORT_SYMBOL
#define CUBRIDGE_API __declspec(dllexport)
#else
#define CUBRIDGE_API __declspec(dllimport)
#endif
#else
#define CUBRIDGE_API
#endif

#endif //__CUDA_BENCHMARK_TARGETS_H__
