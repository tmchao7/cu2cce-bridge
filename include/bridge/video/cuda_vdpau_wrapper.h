#ifndef __CU2CCE_PARITY_BRIDGE_VIDEO_CUDA_VDPAU_WRAPPER_H__
#define __CU2CCE_PARITY_BRIDGE_VIDEO_CUDA_VDPAU_WRAPPER_H__

/* CUDA compatibility header parity entry. */
/* Reference header: include/bridge/video/cuda_vdpau_wrapper.h */
/* This is a cu2cce compatibility surface, not an NVIDIA runtime copy. */
/* Status: RESERVED_NPU or DIAGNOSTIC_ONLY until each API has a safe CANN ABI. */
/* API names and future routes are recorded below; no fake library is linked. */
/* CUDA API: cuGraphicsVDPAURegisterOutputSurface
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuGraphicsVDPAURegisterVideoSurface
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuVDPAUCtxCreate
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuVDPAUGetDevice
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */

#endif /* __CU2CCE_PARITY_BRIDGE_VIDEO_CUDA_VDPAU_WRAPPER_H__ */
