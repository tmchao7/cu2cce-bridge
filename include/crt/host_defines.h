#ifndef __CU2CCE_CRT_HOST_DEFINES_H__
#define __CU2CCE_CRT_HOST_DEFINES_H__

/*
 * Documentation-only header. Do NOT "#define __host__ / __device__ /
 * __forceinline__" fallbacks here.
 *
 * Server evidence (2026-08-27, device_math_minimal round 2): ccec handles
 * these spellings as LANGUAGE KEYWORDS with real execution-space checking,
 * not as predefined macros. The earlier fallback block therefore fired
 * (#if !defined(__host__) was always true) and textually replaced every
 * later occurrence with an empty token — a "__device__ __host__" function
 * silently degraded to a pure __host__ one, producing misleading errors
 * like "call to __host__ function from __global__ function".
 *
 * Macro-vs-keyword rule for this bridge: identifiers that are keywords in
 * ccec CUDA mode must never be (re)defined through the preprocessor;
 * compatibility work targets functions/types instead.
 */

#endif /* __CU2CCE_CRT_HOST_DEFINES_H__ */
