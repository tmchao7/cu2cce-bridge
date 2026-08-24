#ifndef CU2CCE_CUDA_BF16_H
#define CU2CCE_CUDA_BF16_H

/* CUDA BF16 compatibility header for Ascend puresimt. */
#include "simt_api/asc_bf16.h"

typedef bfloat16_t __nv_bfloat16;
typedef bfloat16x2_t __nv_bfloat162;

#endif /* CU2CCE_CUDA_BF16_H */
