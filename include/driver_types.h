#ifndef __CU2CCE_DRIVER_TYPES_H__
#define __CU2CCE_DRIVER_TYPES_H__

/* CUDA driver/runtime type entry. The bridge does not implement the complete
 * CUDA Driver API; these shared data-shape types are provided so common CUDA
 * headers can be parsed without inventing an Ascend driver ABI. */
#include <bridge/runtime/cuda_runtime_types.h>
#include <bridge/runtime/cuda_driver_types.h>
#include <bridge/runtime/cuda_wrapper_types.h>

#endif /* __CU2CCE_DRIVER_TYPES_H__ */
