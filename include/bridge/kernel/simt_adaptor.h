#ifndef __BRIDGE_KERNEL_SIMT_ADAPTOR_H__
#define __BRIDGE_KERNEL_SIMT_ADAPTOR_H__

/*
 * AI Core SIMT 适配宏表（Phase A3，路线 A 核心）。
 *
 * 背景：昇腾 950 系列（dav-310）的 AI Core 原生支持 SIMT 编程模型（CANN 9.0+），
 *   与 CUDA 高度同构（__global__、threadIdx/blockIdx、<<<>>>、warp=32），
 *   但部分原语命名不同。本头把 CUDA kernel 侧符号映射到昇腾 SIMT 对应物，
 *   让用户 .cu 里的 kernel 代码不经改写即可由 bisheng 编译。
 *
 * 映射依据（2026-08-07 权威核对）：cann/asc-devkit include/simt_api/ 真实头文件
 *   - asc_simt.h / device_sync_functions.h / device_warp_functions.h
 *   - device_atomic_functions.h（Copyright 2026 Huawei，CANN 9.x）
 *
 * 本头由 bin/cucc 在编译 kernel 时自动注入（-include）。
 * 被注入前需保证 simt_api 头在 include 路径（asc-devkit 或 CANN 提供）。
 */

//==================== 同步 ====================//
/* CUDA: __syncthreads() -> 昇腾 SIMT: asc_syncthreads() */
#define __syncthreads() asc_syncthreads()
#define __threadfence() asc_threadfence()
#define __threadfence_block() asc_threadfence_block()

//==================== 共享内存 ====================//
/* CUDA: __shared__ -> 昇腾 SIMT: __ubuf__（共享内存/Unified Buffer） */
#define __shared__ __ubuf__

//==================== 线程块/网格索引（同名内置，无需宏） ====================//
/* threadIdx / blockIdx / blockDim / gridDim 在 SIMT 下同名内置变量，无需映射 */

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
