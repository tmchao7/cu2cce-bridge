#ifndef __CU2CCE_CRT_COMMON_FUNCTIONS_H__
#define __CU2CCE_CRT_COMMON_FUNCTIONS_H__

/*
 * Parity entry for NVIDIA crt/common_functions.h (utility helpers such as
 * min/max/bitwise helpers shared across CUDA host and device code).
 * Forward-only; see crt/math_functions.h for the honest boundary rules.
 */

#if defined(__CU2CCE_DEVICE_COMPILATION__) || defined(__CUDACC__)
#if defined(__has_include)
#if __has_include("simt_api/common_functions.h")
#include "simt_api/common_functions.h"
#endif
#endif
#endif

#endif /* __CU2CCE_CRT_COMMON_FUNCTIONS_H__ */
