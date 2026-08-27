#ifndef __CU2CCE_CRT_VECTOR_TYPES_H__
#define __CU2CCE_CRT_VECTOR_TYPES_H__

/*
 * CUDA vector_type surface (parity with NVIDIA crt/vector_types.h, subset).
 *
 * Scope decisions, evidence-driven like the rest of this bridge:
 * - `dim3` is intentionally NOT defined here: bisheng/ccec provides it
 *   natively (see include/bridge/runtime/cuda_runtime_wrapper.h:215).
 * - Plain aggregates with x/y/z/w members plus make_* factories cover the
 *   shape usage that dominates real CUDA sources. Type-punning unions
 *   (e.g. .x/.y views on short4) are NOT provided yet; add them when a
 *   real project or sample proves they are needed — never speculatively.
 */

#if !defined(__CU2CCE_NO_VECTOR_TYPES__)

/* ---------------- scalar-length 1 ---------------- */
typedef struct { char x; } char1;
typedef struct { unsigned char x; } uchar1;
typedef struct { short x; } short1;
typedef struct { unsigned short x; } ushort1;
typedef struct { int x; } int1;
typedef struct { unsigned int x; } uint1;
typedef struct { long x; } long1;
typedef struct { unsigned long x; } ulong1;
typedef struct { long long x; } longlong1;
typedef struct { unsigned long long x; } ulonglong1;
typedef struct { float x; } float1;
typedef struct { double x; } double1;

/* ---------------- pairs and triples/quads ---------------- */
typedef struct { char x, y; } char2;
typedef struct { unsigned char x, y; } uchar2;
typedef struct { short x, y; } short2;
typedef struct { unsigned short x, y; } ushort2;
typedef struct { int x, y; } int2;
typedef struct { unsigned int x, y; } uint2;
typedef struct { long x, y; } long2;
typedef struct { unsigned long x, y; } ulong2;
typedef struct { long long x, y; } longlong2;
typedef struct { unsigned long long x, y; } ulonglong2;
typedef struct { float x, y; } float2;
typedef struct { double x, y; } double2;

typedef struct { char x, y, z; } char3;
typedef struct { unsigned char x, y, z; } uchar3;
typedef struct { short x, y, z; } short3;
typedef struct { unsigned short x, y, z; } ushort3;
typedef struct { int x, y, z; } int3;
typedef struct { unsigned int x, y, z; } uint3;
typedef struct { float x, y, z; } float3;
typedef struct { double x, y, z; } double3;

typedef struct { char x, y, z, w; } char4;
typedef struct { unsigned char x, y, z, w; } uchar4;
typedef struct { short x, y, z, w; } short4;
typedef struct { unsigned short x, y, z, w; } ushort4;
typedef struct { int x, y, z, w; } int4;
typedef struct { unsigned int x, y, z, w; } uint4;
typedef struct { long long x, y, z, w; } longlong4;
typedef struct { float x, y, z, w; } float4;
typedef struct { double x, y, z, w; } double4;

#define __CU2CCE_MAKE1(name, T) \
    static __forceinline__ name##1 make_##name##1(T x) { name##1 v; v.x = x; return v; }
#define __CU2CCE_MAKE2(name, T) \
    static __forceinline__ name##2 make_##name##2(T x, T y) { name##2 v; v.x = x; v.y = y; return v; }
#define __CU2CCE_MAKE3(name, T) \
    static __forceinline__ name##3 make_##name##3(T x, T y, T z) { name##3 v; v.x = x; v.y = y; v.z = z; return v; }
#define __CU2CCE_MAKE4(name, T) \
    static __forceinline__ name##4 make_##name##4(T x, T y, T z, T w) { name##4 v; v.x = x; v.y = y; v.z = z; v.w = w; return v; }

__CU2CCE_MAKE1(char, signed char)
__CU2CCE_MAKE2(char, signed char)
__CU2CCE_MAKE3(char, signed char)
__CU2CCE_MAKE4(char, signed char)

__CU2CCE_MAKE1(uchar, unsigned char)
__CU2CCE_MAKE2(uchar, unsigned char)
__CU2CCE_MAKE3(uchar, unsigned char)
__CU2CCE_MAKE4(uchar, unsigned char)

__CU2CCE_MAKE1(short, signed short)
__CU2CCE_MAKE2(short, signed short)
__CU2CCE_MAKE3(short, signed short)
__CU2CCE_MAKE4(short, signed short)

__CU2CCE_MAKE1(ushort, unsigned short)
__CU2CCE_MAKE2(ushort, unsigned short)
__CU2CCE_MAKE3(ushort, unsigned short)
__CU2CCE_MAKE4(ushort, unsigned short)

__CU2CCE_MAKE1(int, signed int)
__CU2CCE_MAKE2(int, signed int)
__CU2CCE_MAKE3(int, signed int)
__CU2CCE_MAKE4(int, signed int)

__CU2CCE_MAKE1(uint, unsigned int)
__CU2CCE_MAKE2(uint, unsigned int)
__CU2CCE_MAKE3(uint, unsigned int)
__CU2CCE_MAKE4(uint, unsigned int)

__CU2CCE_MAKE1(long, signed long)
__CU2CCE_MAKE2(long, signed long)

__CU2CCE_MAKE1(longlong, signed long long)
__CU2CCE_MAKE2(longlong, signed long long)
__CU2CCE_MAKE4(longlong, signed long long)

__CU2CCE_MAKE1(float, float)
__CU2CCE_MAKE2(float, float)
__CU2CCE_MAKE3(float, float)
__CU2CCE_MAKE4(float, float)

__CU2CCE_MAKE1(double, double)
__CU2CCE_MAKE2(double, double)
__CU2CCE_MAKE3(double, double)
__CU2CCE_MAKE4(double, double)

#undef __CU2CCE_MAKE1
#undef __CU2CCE_MAKE2
#undef __CU2CCE_MAKE3
#undef __CU2CCE_MAKE4

#endif /* !__CU2CCE_NO_VECTOR_TYPES__ */
#endif /* __CU2CCE_CRT_VECTOR_TYPES_H__ */
