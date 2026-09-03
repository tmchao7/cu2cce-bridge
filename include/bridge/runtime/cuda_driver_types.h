#ifndef __CU2CCE_CUDA_DRIVER_TYPES_H__
#define __CU2CCE_CUDA_DRIVER_TYPES_H__

/*
 * CUDA Driver API source-shape types for cu2cce.
 *
 * Status: UNSUPPORTED_TYPED.  These opaque handles make selected, stable CUDA
 * Driver declarations parseable.  They are not aliases for ACL contexts,
 * functions, streams or CANN module handles.  A future implementation must
 * validate aclrtBinary/module APIs and real-device lifecycle semantics before
 * promoting any entry to ACTIVE_ADAPTED.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int CUresult;
typedef int CUdevice;
typedef uint64_t CUdeviceptr;
typedef void *CUcontext;
typedef void *CUmodule;
typedef void *CUfunction;
typedef void *CUstream;
typedef void *CUevent;

enum {
    CUDA_SUCCESS = 0,
    CUDA_ERROR_INVALID_VALUE = 1,
    CUDA_ERROR_NOT_INITIALIZED = 3,
    CUDA_ERROR_NOT_SUPPORTED = 801,
    CUDA_ERROR_UNKNOWN = 999,
};

#ifdef __cplusplus
}
#endif

#endif /* __CU2CCE_CUDA_DRIVER_TYPES_H__ */
