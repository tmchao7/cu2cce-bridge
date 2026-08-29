#ifndef __CU2CCE_MATH_CONSTANTS_H__
#define __CU2CCE_MATH_CONSTANTS_H__

/*
 * Parity with NVIDIA math_constants.h: the single/double precision math
 * constants device and host code commonly reference. Values are the IEEE
 * literals every conforming toolchain agrees on (no backend-specific
 * semantics). FP64 constants are provided for HOST-side use; remember the
 * Ascend device side has no double arithmetic (see
 * docs/03_Scope/LIMITS_AND_BOUNDARIES.md) — using the double constants in
 * device code fails at the FP64 type policy exactly like any other double.
 */

/* single precision */
#ifndef M_Ef
#define M_Ef 2.71828182845904523536f
#endif
#ifndef M_LOG2Ef
#define M_LOG2Ef 1.44269504088896340736f
#endif
#ifndef M_LOG10Ef
#define M_LOG10Ef 0.434294481903251827651f
#endif
#ifndef M_LN2f
#define M_LN2f 0.693147180559945309417f
#endif
#ifndef M_LN10f
#define M_LN10f 2.30258509299404568402f
#endif
#ifndef M_PIf
#define M_PIf 3.14159265358979323846f
#endif
#ifndef M_PI_2f
#define M_PI_2f 1.57079632679489661923f
#endif
#ifndef M_PI_4f
#define M_PI_4f 0.785398163397448309616f
#endif
#ifndef M_1_PIf
#define M_1_PIf 0.318309886183790671538f
#endif
#ifndef M_2_PIf
#define M_2_PIf 0.636619772367581343076f
#endif
#ifndef M_2_SQRTPIf
#define M_2_SQRTPIf 1.12837916709551257390f
#endif
#ifndef M_SQRT2f
#define M_SQRT2f 1.41421356237309504880f
#endif
#ifndef M_SQRT1_2f
#define M_SQRT1_2f 0.707106781186547524401f
#endif

/* double precision (host-side only on this backend) */
#ifndef M_E
#define M_E 2.7182818284590452354
#endif
#ifndef M_LOG2E
#define M_LOG2E 1.4426950408889634074
#endif
#ifndef M_LOG10E
#define M_LOG10E 0.43429448190325182765
#endif
#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif
#ifndef M_LN10
#define M_LN10 2.30258509299404568402
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4 0.78539816339744830962
#endif
#ifndef M_1_PI
#define M_1_PI 0.31830988618379067154
#endif
#ifndef M_2_PI
#define M_2_PI 0.63661977236758134308
#endif
#ifndef M_2_SQRTPI
#define M_2_SQRTPI 1.12837916709551257390
#endif
#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2 0.70710678118654752440
#endif

/* CUDA-specific float specials */
#ifndef CUDART_INF_F
#define CUDART_INF_F __builtin_inff()
#endif
#ifndef CUDART_NAN_F
#define CUDART_NAN_F __builtin_nanf("")
#endif
#ifndef CUDART_PI_F
#define CUDART_PI_F M_PIf
#endif
#ifndef CUDART_PI2_F
#define CUDART_PI2_F M_PI_2f
#endif
#ifndef CUDART_PI4_F
#define CUDART_PI4_F M_PI_4f
#endif

#endif /* __CU2CCE_MATH_CONSTANTS_H__ */
