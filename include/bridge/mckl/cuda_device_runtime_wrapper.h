#ifndef __CU2CCE_PARITY_BRIDGE_MCKL_CUDA_DEVICE_RUNTIME_WRAPPER_H__
#define __CU2CCE_PARITY_BRIDGE_MCKL_CUDA_DEVICE_RUNTIME_WRAPPER_H__

/* CUDA compatibility header parity entry. */
/* Reference header: include/bridge/mckl/cuda_device_runtime_wrapper.h */
/* This is a cu2cce compatibility surface, not an NVIDIA runtime copy. */
/* Status: RESERVED_NPU or DIAGNOSTIC_ONLY until each API has a safe CANN ABI. */
/* API names and future routes are recorded below; no fake library is linked. */
/* CUDA API: cudaCGGetIntrinsicHandle
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaCGGetRank
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaCGGetSize
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaCGSynchronize
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaCGSynchronizeGrid
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaDeviceGetAttribute
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaDeviceGetCacheConfig
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaDeviceGetLimit
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaDeviceGetSharedMemConfig
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaDeviceSynchronize
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaEventCreateWithFlags
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaEventDestroy
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaEventRecord
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaEventRecordWithFlags
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaEventRecordWithFlags_ptsz
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaEventRecord_ptsz
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaFree
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaFuncGetAttributes
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetDevice
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetDeviceCount
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetErrorName
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetErrorString
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetLastError
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetParameterBuffer
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetParameterBufferV2
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaLaunchDevice
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaLaunchDeviceV2
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaLaunchDeviceV2_ptsz
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaLaunchDevice_ptsz
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMalloc
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpy2DAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpy2DAsync_ptsz
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpy3DAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpy3DAsync_ptsz
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpyAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpyAsync_ptsz
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemset2DAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemset2DAsync_ptsz
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemset3DAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemset3DAsync_ptsz
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemsetAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemsetAsync_ptsz
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaOccupancyMaxActiveBlocksPerMultiprocessor
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaOccupancyMaxActiveBlocksPerMultiprocessorWithFlags
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaPeekAtLastError
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaRuntimeGetVersion
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaStreamCreateWithFlags
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaStreamDestroy
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaStreamWaitEvent
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaStreamWaitEvent_ptsz
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */

#endif /* __CU2CCE_PARITY_BRIDGE_MCKL_CUDA_DEVICE_RUNTIME_WRAPPER_H__ */
