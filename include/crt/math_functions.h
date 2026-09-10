#ifndef __CU2CCE_CRT_MATH_FUNCTIONS_H__
#define __CU2CCE_CRT_MATH_FUNCTIONS_H__

/*
 * Parity entry for NVIDIA crt/math_functions.h on the puresimt path.
 *
 * Forwarding policy (evidence-first): include CANN's native simt_api math
 * header when this installation provides one; host-visibility fixes for the
 * integer min/max family live centrally in simt_adaptor.h templates.
 *
 * Boundary:
 * - Double-precision device math is UNSUPPORTED on stable CANN 9.2 SIMT;
 *   no compatibility shims will fake it.
 * - Any single-precision symbol still missing after forwarding fails the
 *   compile with its own name in the error text — feed that into the
 *   device_math_minimal diagnostic loop instead of inventing values here.
 */

#if defined(__CU2CCE_DEVICE_COMPILATION__) || defined(__CUDACC__)
#if defined(__has_include)
#if __has_include(<simt_api/math_functions.h>)
#include <simt_api/math_functions.h>
#endif
#endif
#endif

#endif /* __CU2CCE_CRT_MATH_FUNCTIONS_H__ */
