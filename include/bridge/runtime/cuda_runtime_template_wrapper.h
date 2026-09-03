#ifndef __CU2CCE_PARITY_BRIDGE_RUNTIME_CUDA_RUNTIME_TEMPLATE_WRAPPER_H__
#define __CU2CCE_PARITY_BRIDGE_RUNTIME_CUDA_RUNTIME_TEMPLATE_WRAPPER_H__

/* CUDA compatibility header parity entry. */
/* Reference header: include/bridge/runtime/cuda_runtime_template_wrapper.h */
/* This is a cu2cce compatibility surface, not an NVIDIA runtime copy. */
/* Status: RESERVED_NPU or DIAGNOSTIC_ONLY until each API has a safe CANN ABI. */
/* API names and future routes are recorded below; no fake library is linked. */
/* CUDA API: cuMemcpyDtoH
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuMemcpyDtoHAsync
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuMemcpyHtoD
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cuMemcpyHtoDAsync
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaBindSurfaceToArray
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaBindTexture
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaBindTexture2D
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaBindTextureToArray
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaBindTextureToMipmappedArray
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaFree
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaFreeHost
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaFuncGetAttributes
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaFuncSetAttribute
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaFuncSetCacheConfig
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaFuncSetSharedMemConfig
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetChannelDesc
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetMipmappedArrayLevel
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetSymbolAddress
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetSymbolSize
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGetTextureAlignmentOffset
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGraphAddMemcpyNodeFromSymbol
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGraphAddMemcpyNodeToSymbol
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGraphExecMemcpyNodeSetParamsFromSymbol
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGraphExecMemcpyNodeSetParamsToSymbol
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGraphExecUpdate_V2
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGraphInstantiate_V2
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGraphMemcpyNodeSetParamsFromSymbol
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaGraphMemcpyNodeSetParamsToSymbol
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaHostAlloc
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaHostGetDevicePointer
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaLaunchCooperativeKernel
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaLaunchKernel
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaLaunchKernelEx
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaLaunchKernelExC
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMalloc
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMallocAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMallocFromPoolAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMallocHost
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMallocManaged
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMallocPitch
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpy2D
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpy2DAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpyFromSymbol
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpyFromSymbolAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpyToSymbol
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaMemcpyToSymbolAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaOccupancyAvailableDynamicSMemPerBlock
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
/* CUDA API: cudaOccupancyMaxActiveClusters
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaOccupancyMaxPotentialBlockSize
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaOccupancyMaxPotentialBlockSizeVariableSMem
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaOccupancyMaxPotentialBlockSizeVariableSMemWithFlags
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaOccupancyMaxPotentialBlockSizeWithFlags
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaOccupancyMaxPotentialClusterSize
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaStreamAttachMemAsync
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaUnbindTexture
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */
/* CUDA API: cudaUserObjectCreate
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: add a typed wrapper and real CANN probe.
 */

#endif /* __CU2CCE_PARITY_BRIDGE_RUNTIME_CUDA_RUNTIME_TEMPLATE_WRAPPER_H__ */
