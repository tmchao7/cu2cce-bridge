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
 * 映射依据：docs/01_Design/ASCEND_RESEARCH_NOTES.md §2.3（SIMT ↔ CUDA 映射表）。
 * ⚠️ 以下映射名依据 CANN 9.x SIMT 文档与 asc-devkit 示例，真机上以实际头文件为准。
 *   包括 simt_api 系列头（asc_syncthreads 等，见 asc-devkit examples/03_simt_api）。
 *
 * 本头由 bin/cucc 在编译 kernel 时自动注入（-include），或由 __cuda_runtime.h 条件包含。
 */

//==================== 同步 ====================//
/* CUDA: __syncthreads() -> 昇腾 SIMT: asc_syncthreads() */
#define __syncthreads() asc_syncthreads()

//==================== 共享内存 ====================//
/* CUDA: __shared__ -> 昇腾 SIMT: __ubuf__（共享内存/Unified Buffer） */
#define __shared__ __ubuf__

//==================== 线程块/网格索引（同名内置，无需宏） ====================//
/* threadIdx / blockIdx / blockDim / gridDim 在 SIMT 下同名内置变量，无需映射 */

//==================== Warp 原语 ====================//
/* CUDA __shfl_sync(mask, var, srcLane) -> 昇腾 asc_shfl。
 * ⚠️ 昇腾 warp=32 与 CUDA 一致，mask 语义可能不同，需真机核对具体签名。 */
// #define __shfl_sync(mask, var, srcLane) asc_shfl(var, srcLane)
// #define __shfl_up_sync(mask, var, delta) asc_shfl_up(var, delta)
// #define __shfl_down_sync(mask, var, delta) asc_shfl_down(var, delta)

//==================== 原子操作 ====================//
/* CUDA atomicAdd -> 昇腾 asc_atomic_add（头文件 simt_api/atomic_functions.h）
 * ⚠️ 昇腾原子接口返回语义/签名待真机核对，先注释避免错误编译。 */
// #define atomicAdd(a, b) asc_atomic_add((a), (b))
// #define atomicSub(a, b) asc_atomic_sub((a), (b))
// #define atomicExch(a, b) asc_atomic_exch((a), (b))

//==================== 线程束归约/广播（如需） ====================//
// #define __reduce_add_sync(mask, value) asc_reduce_add(value)

//==================== 内存栅栏 ====================//
/* CUDA: __threadfence() -> 昇腾 SIMT: asc_threadfence() */
#define __threadfence() asc_threadfence()

//==================== warpSize ====================//
/* SIMT 下 warp=32，与 CUDA 一致。应用若硬编码 32 可运行；
 * 建议用 cudaGetDeviceProperties 返回的 warpSize（wrapper 已填 32）。 */
#define __WARP_SIZE__ 32

#endif //__BRIDGE_KERNEL_SIMT_ADAPTOR_H__
