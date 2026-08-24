#ifndef __BRIDGE_KERNEL_SIMT_ADAPTOR_H__
#define __BRIDGE_KERNEL_SIMT_ADAPTOR_H__

/*
 * CUDA SIMT compatibility header for the main/puresimt path.
 *
 * CANN 9.2 ccec with --enable-simt natively parses CUDA kernel syntax on
 * dav-c310-vec. This header only supplies source-level aliases for CUDA SIMT
 * primitives whose Ascend spellings differ. It must not wrap kernels, rewrite
 * launches, add hidden parameters, or generate intermediate source files.
 */

#if defined(__has_include)
#if __has_include("simt_api/asc_simt.h")
#include "simt_api/asc_simt.h"
#endif
#endif

/* CUDA synchronization/shared-memory primitives. */
#define __syncthreads() asc_syncthreads()
#define __threadfence() asc_threadfence()
#define __threadfence_block() asc_threadfence_block()
#define __shared__ __ubuf__

/* CUDA warp collectives. Ascend SIMT uses an implicit full-warp mask. */
#define __shfl_sync(mask, var, srcLane) asc_shfl((var), (srcLane))
#define __shfl_up_sync(mask, var, delta) asc_shfl_up((var), (delta))
#define __shfl_down_sync(mask, var, delta) asc_shfl_down((var), (delta))
#define __shfl_xor_sync(mask, var, laneMask) asc_shfl_xor((var), (laneMask))
#define __all_sync(mask, predicate) asc_all((predicate))
#define __any_sync(mask, predicate) asc_any((predicate))
#define __ballot_sync(mask, predicate) asc_ballot((predicate))
#define __reduce_add_sync(mask, value) asc_reduce_add((value))

/* CUDA atomics. */
#define atomicAdd(a, b) asc_atomic_add((a), (b))
#define atomicSub(a, b) asc_atomic_sub((a), (b))
#define atomicExch(a, b) asc_atomic_exch((a), (b))
#define atomicMax(a, b) asc_atomic_max((a), (b))
#define atomicMin(a, b) asc_atomic_min((a), (b))

#ifndef __WARP_SIZE__
#define __WARP_SIZE__ 32
#endif

#endif /* __BRIDGE_KERNEL_SIMT_ADAPTOR_H__ */
