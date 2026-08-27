#ifndef __CU2CCE_CRT_VECTOR_FUNCTIONS_COMPAT_H__
#define __CU2CCE_CRT_VECTOR_FUNCTIONS_COMPAT_H__

/*
 * make_* factory parity for CUDA vector construction.
 *
 * Server evidence (2026-08-27 device_math_minimal probe):
 *   <CANN>/asc/impl/simt_api/vector_functions_impl.h declares e.g.
 *   __SIMT_DEVICE_FUNCTIONS_DECL__ inline float3 make_float3(float,float,float)
 * i.e. factories exist but are DEVICE-ONLY, while NVIDIA makes the whole
 * family callable from host and device.
 *
 * Resolution strategy — identical to the integer min/max templates in
 * simt_adaptor.h: add global VALUE-RETURNING TEMPLATES. Templates cannot
 * merge with, nor outrank, the native non-template overloads (exact match
 * always prefers the native candidate in device code); in host code they
 * provide the previously-missing viable candidate. Escape hatch:
 * define __CU2CCE_NO_VECTOR_FUNCTION_FACTORIES__ to compile without them.
 *
 * Scope policy: only names that real CUDA sources or probes requested get
 * an entry here; never enumerate speculatively across the full surface.
 */
#if defined(__cplusplus) && !defined(__CU2CCE_NO_VECTOR_FUNCTION_FACTORIES__)

template <typename A_, typename B_>
static inline float2 make_float2(A_ x, B_ y) { float2 v; v.x = x; v.y = y; return v; }
template <typename A_, typename B_, typename C_>
static inline float3 make_float3(A_ x, B_ y, C_ z) { float3 v; v.x = x; v.y = y; v.z = z; return v; }
template <typename A_, typename B_, typename C_, typename D_>
static inline float4 make_float4(A_ x, B_ y, C_ z, D_ w) { float4 v; v.x = x; v.y = y; v.z = z; v.w = w; return v; }

template <typename A_, typename B_>
static inline double2 make_double2(A_ x, B_ y) { double2 v; v.x = x; v.y = y; return v; }

template <typename A_, typename B_>
static inline int2 make_int2(A_ x, B_ y) { int2 v; v.x = x; v.y = y; return v; }
template <typename A_, typename B_, typename C_>
static inline int3 make_int3(A_ x, B_ y, C_ z) { int3 v; v.x = x; v.y = y; v.z = z; return v; }
template <typename A_, typename B_, typename C_, typename D_>
static inline int4 make_int4(A_ x, B_ y, C_ z, D_ w) { int4 v; v.x = x; v.y = y; v.z = z; v.w = w; return v; }

template <typename A_, typename B_>
static inline uint2 make_uint2(A_ x, B_ y) { uint2 v; v.x = x; v.y = y; return v; }
template <typename A_, typename B_, typename C_>
static inline uint3 make_uint3(A_ x, B_ y, C_ z) { uint3 v; v.x = x; v.y = y; v.z = z; return v; }
template <typename A_, typename B_, typename C_, typename D_>
static inline uint4 make_uint4(A_ x, B_ y, C_ z, D_ w) { uint4 v; v.x = x; v.y = y; v.z = z; v.w = w; return v; }

#endif /* __cplusplus && !__CU2CCE_NO_VECTOR_FUNCTION_FACTORIES__ */
#endif /* __CU2CCE_CRT_VECTOR_FUNCTIONS_COMPAT_H__ */
