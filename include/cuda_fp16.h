#ifndef CU2CCE_CUDA_FP16_H
#define CU2CCE_CUDA_FP16_H

/* CUDA FP16 compatibility header for Ascend puresimt. CANN's SIMT API
 * provides the native half/half2 types and CUDA-compatible conversion names. */
#include "simt_api/asc_fp16.h"

typedef half __half;
typedef half2 __half2;

#endif /* CU2CCE_CUDA_FP16_H */
