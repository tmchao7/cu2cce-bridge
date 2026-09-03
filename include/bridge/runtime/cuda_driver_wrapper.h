#ifndef __CU2CCE_CUDA_DRIVER_WRAPPER_H__
#define __CU2CCE_CUDA_DRIVER_WRAPPER_H__

/*
 * CUDA Driver API typed placeholder surface.
 *
 * Reference: cu-bridge-muxi/include/bridge/runtime/cuda_driver_wrapper.h.
 * CANN route: aclrtBinary/module APIs may provide a future migration route,
 * but cu2cce does not currently claim the CUDA Driver ABI.  The stable entries
 * below return ACL_ERROR_API_NOT_SUPPORT and clear output parameters.
 *
 * The complete reference API inventory, including Graph/IPC/interop/advanced
 * Driver functions, stays in bridge/cuda_to_cce_reserved_wrapper_parity.h
 * under #if 0 with its reference prototype and future unlock location.
 */

#include "cuda_driver_types.h"
#include <acl/acl_base.h>

#ifdef __cplusplus
extern "C" {
#endif

CUresult wcuInit(unsigned int flags);
CUresult wcuDriverGetVersion(int *driverVersion);
CUresult wcuDeviceGetCount(int *count);
CUresult wcuDeviceGet(CUdevice *device, int ordinal);

#ifdef __cplusplus
}
#endif

/* Keep CUDA spelling source-compatible without forwarding into aclInit or an
 * unrelated aclrt handle. */
#define cuInit wcuInit
#define cuDriverGetVersion wcuDriverGetVersion
#define cuDeviceGetCount wcuDeviceGetCount
#define cuDeviceGet wcuDeviceGet

#endif /* __CU2CCE_CUDA_DRIVER_WRAPPER_H__ */
