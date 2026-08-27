#ifndef __CU2CCE_CRT_VECTOR_TYPES_H__
#define __CU2CCE_CRT_VECTOR_TYPES_H__

/*
 * Deliberately a documentation-only header (no type definitions).
 *
 * Server evidence (device_math_minimal probe, 2026-08-27): bisheng/ccec
 * already injects the full CUDA vector-type family into every translation
 * unit via its internal header
 *   <bisheng>/lib/clang/15.0.5/include/__clang_dpp_types.h
 * spelled as ext_vector_type typedefs, e.g.
 *   typedef int32_t int1  __attribute__((ext_vector_type(1)));
 *   typedef float4 …                            (char1..double4)
 * Any bridge-side struct redefinition collides at the TU level:
 *   "typedef redefinition with different types ('struct char1' vs
 *    'int8_t __attribute__((ext_vector_type(1)))')"
 *
 * Therefore the bridge defines NOTHING here. Compiler-native types win;
 * whether the matching make_* factories are provided by the compiler is
 * learned the same way — through the diagnostic probe's remaining errors,
 * never through speculative bridge-side additions.
 *
 * Escape hatch if some future consumer needs old bridge-local struct
 * shapes anyway: recover them from git history of this file; they were
 * removed because they broke against every real CANN installation seen.
 */

#endif /* __CU2CCE_CRT_VECTOR_TYPES_H__ */
