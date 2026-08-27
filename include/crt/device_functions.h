#ifndef __CU2CCE_CRT_DEVICE_FUNCTIONS_H__
#define __CU2CCE_CRT_DEVICE_FUNCTIONS_H__

/*
 * Parity entry for NVIDIA crt/device_functions.h: fast intrinsic math and
 * warp primitives usable from __device__/__global__ code. Same policy as
 * crt/math_functions.h — forward what this CANN installation really ships;
 * never fabricate semantics for symbols it does not provide.
 */

#if defined(__has_include)
#if __has_include("simt_api/device_functions.h")
#include "simt_api/device_functions.h"
#endif
#if __has_include(<simt_api/fast_math.h>)
#include <simt_api/fast_math.h>
#endif
#endif

#endif /* __CU2CCE_CRT_DEVICE_FUNCTIONS_H__ */
