#ifndef __BRIDGE_KERNEL_SIMT_ADAPTOR_H__
#define __BRIDGE_KERNEL_SIMT_ADAPTOR_H__

/*
 * AI Core SIMT 适配头（Phase A3，2026-08-19 真机 v4 模板验证后更新）
 *
 * 真机确认的 ccec SIMT 编程模型（cann-9.2.0 / dav-c310 / clang 15.0.5）：
 *   - kernel 入口必须 __global__[aicore]（裸 __global__ 报
 *     "requires the __aicore__ or __aicpu__ qualifier"）
 *   - 计算体放 __simt_vf__ 函数：warp 原语（asc_syncthreads/asc_shfl 等）只能
 *     在 simt_vf/simt_callee 上下文调用（__global__[aicore] kernel 也不行）
 *   - kernel 内用 cce::async_invoke<VF>(cce::dim3{...}, args...) 启动 VF
 *   - blockIdx/blockDim/threadIdx 可用（include simt_api/asc_simt.h 后）
 *   - kernel/VF 指针需 __gm__ 地址空间（CUDA 裸指针的自动处理待真机验证）
 *
 * 本头由 bin/cucc 编译 .cu 时 -include 注入。映射依据：cann-9.2.0 真机头文件
 * + 用户实测 vector.cce（编译通过）+ asc-devkit simt_api（docs/reference/）。
 */

// simt_api 头（提供 blockIdx/threadIdx/blockDim 与 asc_* 原语）
// 需要 -I 指向其父目录（如 cann-9.2.0/x86_64-linux/asc/include），cucc 自动注入
#if defined(__has_include)
#if __has_include("simt_api/asc_simt.h")
#include "simt_api/asc_simt.h"
#endif
#endif

//==================== kernel 声明 ====================//
/* CUDA __global__ -> 昇腾 __global__[aicore]（限定符必须，真机实测）。
 * 自引用宏技巧：替换列表里的 __global__ 在同一次展开中不再展开。 */
#define __global__ __global__[aicore]

/* CUDA __launch_bounds__(n) -> 昇腾 LAUNCH_BOUND(n)（1 参形式；多参形式需人工改写） */
#define __launch_bounds__(n) LAUNCH_BOUND(n)

//==================== 同步 ====================//
/* ⚠️ 以下原语只能在 __simt_vf__ 函数内调用（simt_callee 上下文限制，真机实测） */
#define __syncthreads() asc_syncthreads()
#define __threadfence() asc_threadfence()
#define __threadfence_block() asc_threadfence_block()

//==================== 共享内存 ====================//
/* CUDA: __shared__ -> 昇腾 SIMT: __ubuf__（共享内存/Unified Buffer） */
#define __shared__ __ubuf__

//==================== 线程块/网格索引（同名内置，无需宏） ====================//
/* threadIdx / blockIdx / blockDim / gridDim 在 include simt_api/asc_simt.h 后可用 */

//==================== Warp 原语 ====================//
/* CUDA __shfl_sync(mask, var, srcLane) -> 昇腾 asc_shfl(var, src_lane, width=warpSize)。
 * ⚠️ 昇腾 asc_shfl 无 mask 参数（warp 内全同步），CUDA 的 mask 丢弃；
 *   签名（来自 asc-devkit device_warp_functions.h）：
 *     asc_shfl(int32_t/uint32_t/float/int64_t/uint64_t var, int32_t src_lane, int32_t width=warpSize)
 *   若 CUDA 代码的 mask 非全 1（部分参与），语义有差异，需人工改写。 */
#define __shfl_sync(mask, var, srcLane) asc_shfl((var), (srcLane))
#define __shfl_up_sync(mask, var, delta) asc_shfl_up((var), (delta))
#define __shfl_down_sync(mask, var, delta) asc_shfl_down((var), (delta))
#define __shfl_xor_sync(mask, var, laneMask) asc_shfl_xor((var), (laneMask))

/* CUDA __all_sync/__any_sync/__ballot_sync -> 昇腾 asc_all/asc_any/asc_ballot */
#define __all_sync(mask, predicate) asc_all((predicate))
#define __any_sync(mask, predicate) asc_any((predicate))
#define __ballot_sync(mask, predicate) asc_ballot((predicate))

//==================== 原子操作 ====================//
/* CUDA atomicAdd(a,b) -> 昇腾 asc_atomic_add(addr, val)。
 * ⚠️ 昇腾原子接口参数带地址空间修饰（__ubuf__/__gm__ 指针），签名（来自
 *   asc-devkit device_atomic_functions.h）：
 *     asc_atomic_add(__ubuf__/__gm__ int32_t*/uint32_t*/float* addr, ...)
 *   若 a 是全局/共享内存指针，编译器应能匹配重载；真机验证。 */
#define atomicAdd(a, b) asc_atomic_add((a), (b))
#define atomicSub(a, b) asc_atomic_sub((a), (b))
#define atomicExch(a, b) asc_atomic_exch((a), (b))
#define atomicMax(a, b) asc_atomic_max((a), (b))
#define atomicMin(a, b) asc_atomic_min((a), (b))

//==================== 线程束归约（如需） ====================//
#define __reduce_add_sync(mask, value) asc_reduce_add((value))

//==================== warpSize ====================//
/* SIMT 下 warp=32（asc_simt.h 中 warpSize 内置）。应用若硬编码 32 可运行；
 * 建议用 cudaGetDeviceProperties 返回的 warpSize（wrapper 已填 32）。 */
#define __WARP_SIZE__ 32

#endif //__BRIDGE_KERNEL_SIMT_ADAPTOR_H__
