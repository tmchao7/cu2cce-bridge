#ifndef __CU2CCE_CUDA_DEVICE_API_H__
#define __CU2CCE_CUDA_DEVICE_API_H__

/*
 * CUDA device-language compatibility map for CANN SIMT.
 *
 * This header is intentionally separate from the host Runtime wrapper:
 *
 *   CUDA device spelling -> CANN simt_api spelling
 *
 * It only adapts APIs for which the CANN SIMT headers expose a named
 * primitive. It must not invent CUDA warp-mask, system-fence, PTX, or
 * device-link semantics that Ascend does not provide. Product/编译模型依赖的
 * extended APIs are opt-in through CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS.
 */

#if defined(__has_include)
#  if __has_include("simt_api/device_warp_functions.h")
#    include "simt_api/device_warp_functions.h"
#    define CU2CCE_HAS_CANN_SIMT_WARP_API 1
#  endif
#  if __has_include("simt_api/device_atomic_functions.h")
#    include "simt_api/device_atomic_functions.h"
#    define CU2CCE_HAS_CANN_SIMT_ATOMIC_API 1
#  endif
#else
#  include "simt_api/device_warp_functions.h"
#  include "simt_api/device_atomic_functions.h"
#  define CU2CCE_HAS_CANN_SIMT_WARP_API 1
#  define CU2CCE_HAS_CANN_SIMT_ATOMIC_API 1
#endif

/*
 * CUDA warp intrinsics accept either the historical three-argument form or
 * the four-argument form with an explicit width. CANN's asc_* primitives
 * accept the value, lane/delta and optional width; the CUDA mask is not a
 * selectable execution mask on the current Ascend SIMT backend.
 */
#if defined(CU2CCE_HAS_CANN_SIMT_WARP_API)
#  define CU2CCE_SHFL_SYNC3(mask, var, srcLane) \
    ((void)(mask), asc_shfl((var), (srcLane), warpSize))
#  define CU2CCE_SHFL_SYNC4(mask, var, srcLane, width) \
    ((void)(mask), asc_shfl((var), (srcLane), (width)))
#  define CU2CCE_SHFL_UP_SYNC3(mask, var, delta) \
    ((void)(mask), asc_shfl_up((var), (delta), warpSize))
#  define CU2CCE_SHFL_UP_SYNC4(mask, var, delta, width) \
    ((void)(mask), asc_shfl_up((var), (delta), (width)))
#  define CU2CCE_SHFL_DOWN_SYNC3(mask, var, delta) \
    ((void)(mask), asc_shfl_down((var), (delta), warpSize))
#  define CU2CCE_SHFL_DOWN_SYNC4(mask, var, delta, width) \
    ((void)(mask), asc_shfl_down((var), (delta), (width)))
#  define CU2CCE_SHFL_XOR_SYNC3(mask, var, laneMask) \
    ((void)(mask), asc_shfl_xor((var), (laneMask), warpSize))
#  define CU2CCE_SHFL_XOR_SYNC4(mask, var, laneMask, width) \
    ((void)(mask), asc_shfl_xor((var), (laneMask), (width)))

#  define CU2CCE_SELECT_SHFL(_1, _2, _3, _4, NAME, ...) NAME
#  define CU2CCE_SHFL_SYNC(...) \
    CU2CCE_SELECT_SHFL(__VA_ARGS__, CU2CCE_SHFL_SYNC4, CU2CCE_SHFL_SYNC3)(__VA_ARGS__)
#  define CU2CCE_SHFL_UP_SYNC(...) \
    CU2CCE_SELECT_SHFL(__VA_ARGS__, CU2CCE_SHFL_UP_SYNC4, CU2CCE_SHFL_UP_SYNC3)(__VA_ARGS__)
#  define CU2CCE_SHFL_DOWN_SYNC(...) \
    CU2CCE_SELECT_SHFL(__VA_ARGS__, CU2CCE_SHFL_DOWN_SYNC4, CU2CCE_SHFL_DOWN_SYNC3)(__VA_ARGS__)
#  define CU2CCE_SHFL_XOR_SYNC(...) \
    CU2CCE_SELECT_SHFL(__VA_ARGS__, CU2CCE_SHFL_XOR_SYNC4, CU2CCE_SHFL_XOR_SYNC3)(__VA_ARGS__)

#  define __shfl_sync(...) CU2CCE_SHFL_SYNC(__VA_ARGS__)
#  define __shfl_up_sync(...) CU2CCE_SHFL_UP_SYNC(__VA_ARGS__)
#  define __shfl_down_sync(...) CU2CCE_SHFL_DOWN_SYNC(__VA_ARGS__)
#  define __shfl_xor_sync(...) CU2CCE_SHFL_XOR_SYNC(__VA_ARGS__)
#  define __all_sync(mask, predicate) ((void)(mask), asc_all((predicate)))
#  define __any_sync(mask, predicate) ((void)(mask), asc_any((predicate)))
#  define __ballot_sync(mask, predicate) ((void)(mask), asc_ballot((predicate)))
/* CANN exposes the additive warp reduction in the core SIMT surface. */
#  define __reduce_add_sync(mask, value) ((void)(mask), asc_reduce_add((value)))

/* These operations are present in CANN SIMT headers, but their support is
 * product/编译模型 dependent. They remain opt-in until the current NPU
 * target has dedicated evidence. */
#  if defined(CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS)
#    define __activemask() asc_activemask()
#    define __reduce_max_sync(mask, value) ((void)(mask), asc_reduce_max((value)))
#    define __reduce_min_sync(mask, value) ((void)(mask), asc_reduce_min((value)))
#  endif

/* Deprecated CUDA warp spellings are still common in older kernels. They do
 * not carry a mask; preserve their optional width argument instead of
 * silently forcing every call to the CANN default. */
#  define CU2CCE_LEGACY_SHFL2(var, srcLane) asc_shfl((var), (srcLane), warpSize)
#  define CU2CCE_LEGACY_SHFL3(var, srcLane, width) asc_shfl((var), (srcLane), (width))
#  define CU2CCE_LEGACY_SHFL_UP2(var, delta) asc_shfl_up((var), (delta), warpSize)
#  define CU2CCE_LEGACY_SHFL_UP3(var, delta, width) asc_shfl_up((var), (delta), (width))
#  define CU2CCE_LEGACY_SHFL_DOWN2(var, delta) asc_shfl_down((var), (delta), warpSize)
#  define CU2CCE_LEGACY_SHFL_DOWN3(var, delta, width) asc_shfl_down((var), (delta), (width))
#  define CU2CCE_LEGACY_SHFL_XOR2(var, laneMask) asc_shfl_xor((var), (laneMask), warpSize)
#  define CU2CCE_LEGACY_SHFL_XOR3(var, laneMask, width) asc_shfl_xor((var), (laneMask), (width))
#  define CU2CCE_SELECT_LEGACY(_1, _2, _3, NAME, ...) NAME
#  define CU2CCE_LEGACY_SHFL(...) \
    CU2CCE_SELECT_LEGACY(__VA_ARGS__, CU2CCE_LEGACY_SHFL3, CU2CCE_LEGACY_SHFL2)(__VA_ARGS__)
#  define CU2CCE_LEGACY_SHFL_UP(...) \
    CU2CCE_SELECT_LEGACY(__VA_ARGS__, CU2CCE_LEGACY_SHFL_UP3, CU2CCE_LEGACY_SHFL_UP2)(__VA_ARGS__)
#  define CU2CCE_LEGACY_SHFL_DOWN(...) \
    CU2CCE_SELECT_LEGACY(__VA_ARGS__, CU2CCE_LEGACY_SHFL_DOWN3, CU2CCE_LEGACY_SHFL_DOWN2)(__VA_ARGS__)
#  define CU2CCE_LEGACY_SHFL_XOR(...) \
    CU2CCE_SELECT_LEGACY(__VA_ARGS__, CU2CCE_LEGACY_SHFL_XOR3, CU2CCE_LEGACY_SHFL_XOR2)(__VA_ARGS__)
#  define __shfl(...) CU2CCE_LEGACY_SHFL(__VA_ARGS__)
#  define __shfl_up(...) CU2CCE_LEGACY_SHFL_UP(__VA_ARGS__)
#  define __shfl_down(...) CU2CCE_LEGACY_SHFL_DOWN(__VA_ARGS__)
#  define __shfl_xor(...) CU2CCE_LEGACY_SHFL_XOR(__VA_ARGS__)
#  define __all(predicate) asc_all((predicate))
#  define __any(predicate) asc_any((predicate))
#  define __ballot(predicate) asc_ballot((predicate))
#endif

/* CUDA atomics return the value observed before the operation. CANN's SIMT
 * atomic API uses the same return-value convention for the supported types.
 * The basic Add/Sub/Exch/Min/Max aliases remain in simt_adaptor.h because
 * they are covered by the existing Ascend950 regression. The broader atomic
 * family below is deliberately opt-in until its target-specific evidence is
 * complete. */
#if defined(CU2CCE_HAS_CANN_SIMT_ATOMIC_API) && \
    defined(CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS)
#  define atomicInc(address, value) asc_atomic_inc((address), (value))
#  define atomicDec(address, value) asc_atomic_dec((address), (value))
#  define atomicCAS(address, compare, value) \
    asc_atomic_cas((address), (compare), (value))
#  define atomicAnd(address, value) asc_atomic_and((address), (value))
#  define atomicOr(address, value) asc_atomic_or((address), (value))
#  define atomicXor(address, value) asc_atomic_xor((address), (value))
#endif

/*
 * There is no CANN SIMT primitive with CUDA __threadfence_system semantics.
 * Deliberately do not define __threadfence_system: an unsupported system-wide
 * visibility guarantee must fail at compile time rather than silently weaken
 * to asc_threadfence().
 */
#define CU2CCE_CUDA_DEVICE_API_THREADFENCE_SYSTEM_UNSUPPORTED 1

#endif /* __CU2CCE_CUDA_DEVICE_API_H__ */
