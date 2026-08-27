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
#if __has_include("simt_api/device_types.h")
/* Brings CANN's own execution-space selector __SIMT_DEVICE_FUNCTIONS_DECL__
 * (server evidence 2026-08-27: expanded conditionally to __aicore__ or
 * __simt_callee__ depending on the installed backend mode). */
#include "simt_api/device_types.h"
#endif
#endif

/*
 * Execution-space keyword map for NVIDIA spellings.
 *
 * Round-4 server proof: the bisheng puresimt front-end parses __global__
 * natively but rejects standalone __device__ ("unknown type name"). Mentor
 * direction: bridge the recognized-form gap WITHOUT rewriting user source,
 * by macro-mapping the NVIDIA spelling onto whatever CANN itself uses.
 *
 * We deliberately alias __device__ to CANN's own selector instead of
 * hard-coding an attribute: device_types.h picks __aicore__ vs
 * __simt_callee__ per installed backend mode, so this track follows the
 * toolchain automatically. Escape hatch:
 * build with -DCU2CCE_NO_DEVICE_KEYWORD_MAP if a future frontend ships
 * native support and the map ever conflicts.
 *
 * Scope discipline: only __device__ is proven missing today, so only it is
 * mapped. Do not speculatively add __host__ or __forceinline__ maps here;
 * each needs its own failing-evidence case first.
 */
#if !defined(CU2CCE_NO_DEVICE_KEYWORD_MAP)
#if !defined(__device__)
#define __device__ __SIMT_DEVICE_FUNCTIONS_DECL__
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

/*
 * Host-side arithmetic overloads for the CUDA min/max family.
 *
 * NVIDIA's crt/math_functions.h exposes integer/float min and max as
 * callable from both host and device, so upstream sample host code may call
 * them unqualified (simpleStreams, simpleMultiCopy do). CANN 9.2's
 * simt_api/math_functions.h only marks those overloads __device__, which
 * makes the host calls fail with "candidate function not viable: call to
 * __device__ function from __host__ function".
 *
 * A same-signature redeclaration cannot fix this: CANN's declarations are
 * static internal-linkage functions whose execution-space attribute persists,
 * so a matching host declaration would either collide on linkage or keep the
 * device-only space. Instead, value-returning TEMPLATES are added to the
 * global namespace: templates never merge or clash with the native
 * non-template declarations, device-context overload resolution keeps
 * preferring the exact-match native candidates, and host-context calls that
 * previously had no viable candidate now resolve here.
 */
#if defined(__cplusplus) && !defined(__CU2CCE_NO_HOST_ARITHMETIC_OVERLOADS__)
template <typename T>
inline T max(T left, T right) { return left > right ? left : right; }

template <typename T>
inline T min(T left, T right) { return left < right ? left : right; }
#endif

#endif /* __BRIDGE_KERNEL_SIMT_ADAPTOR_H__ */
