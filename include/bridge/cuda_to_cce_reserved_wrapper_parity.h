#ifndef __CU2CCE_CUDA_TO_CCE_RESERVED_WRAPPER_PARITY_H__
#define __CU2CCE_CUDA_TO_CCE_RESERVED_WRAPPER_PARITY_H__

/* GENERATED CUDA wrapper parity inventory; intentionally inactive. */
/* Every item below is a future unlock point, not an active CANN symbol. */

#if 0
/*
 * CUDA API: cuArray3DCreate
 * Reference wrapper: wcuArray3DCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuArray3DCreate(MCarray *pHandle, const mcDrvArray3DDescriptor *pAllocateArray);
 */
/* cuArray3DCreate -> wcuArray3DCreate */

/*
 * CUDA API: cuArray3DGetDescriptor
 * Reference wrapper: wcuArray3DGetDescriptor
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuArray3DGetDescriptor(mcDrvArray3DDescriptor *pArrayDescriptor, MCarray hArray);
 */
/* cuArray3DGetDescriptor -> wcuArray3DGetDescriptor */

/*
 * CUDA API: cuArrayCreate
 * Reference wrapper: wcuArrayCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuArrayCreate(MCarray *pHandle, const mcDrvArrayDescriptor *pAllocateArray);
 */
/* cuArrayCreate -> wcuArrayCreate */

/*
 * CUDA API: cuArrayDestroy
 * Reference wrapper: wcuArrayDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuArrayDestroy(MCarray hArray);
 */
/* cuArrayDestroy -> wcuArrayDestroy */

/*
 * CUDA API: cuArrayGetDescriptor
 * Reference wrapper: wcuArrayGetDescriptor
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuArrayGetDescriptor(mcDrvArrayDescriptor *pArrayDescriptor, MCarray hArray);
 */
/* cuArrayGetDescriptor -> wcuArrayGetDescriptor */

/*
 * CUDA API: cuArrayGetMemoryRequirements
 * Reference wrapper: wcuArrayGetMemoryRequirements
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuArrayGetMemoryRequirements(mcDrvArrayMemoryRequirements *memoryRequirements, MCarray array, mcDrvDevice_t device);
 */
/* cuArrayGetMemoryRequirements -> wcuArrayGetMemoryRequirements */

/*
 * CUDA API: cuArrayGetPlane
 * Reference wrapper: wcuArrayGetPlane
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuArrayGetPlane(MCarray *pPlaneArray, MCarray hArray, unsigned int planeIdx);
 */
/* cuArrayGetPlane -> wcuArrayGetPlane */

/*
 * CUDA API: cuArrayGetSparseProperties
 * Reference wrapper: wcuArrayGetSparseProperties
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuArrayGetSparseProperties(mcDrvArraySparseProperties *sparseProperties, MCarray array);
 */
/* cuArrayGetSparseProperties -> wcuArrayGetSparseProperties */

/*
 * CUDA API: cuCtxAttach
 * Reference wrapper: wcuCtxAttach
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxAttach(mcDrvContext_t *pctx, unsigned int flags);
 */
/* cuCtxAttach -> wcuCtxAttach */

/*
 * CUDA API: cuCtxCreate
 * Reference wrapper: wcuCtxCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxCreate(mcDrvContext_t *pctx, unsigned int flags, mcDrvDevice_t dev);
 */
/* cuCtxCreate -> wcuCtxCreate */

/*
 * CUDA API: cuCtxCreate_v3
 * Reference wrapper: wcuCtxCreate_v3
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxCreate_v3(mcDrvContext_t *pctx, mcDrvexecAffinityParam_t *paramsArray, int numParams, unsigned int flags, mcDrvDevice_t dev);
 */
/* cuCtxCreate_v3 -> wcuCtxCreate_v3 */

/*
 * CUDA API: cuCtxDestroy
 * Reference wrapper: wcuCtxDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxDestroy(mcDrvContext_t ctx);
 */
/* cuCtxDestroy -> wcuCtxDestroy */

/*
 * CUDA API: cuCtxDetach
 * Reference wrapper: wcuCtxDetach
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxDetach(mcDrvContext_t ctx);
 */
/* cuCtxDetach -> wcuCtxDetach */

/*
 * CUDA API: cuCtxDisablePeerAccess
 * Reference wrapper: wcuCtxDisablePeerAccess
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxDisablePeerAccess(mcDrvContext_t peerContext);
 */
/* cuCtxDisablePeerAccess -> wcuCtxDisablePeerAccess */

/*
 * CUDA API: cuCtxEnablePeerAccess
 * Reference wrapper: wcuCtxEnablePeerAccess
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxEnablePeerAccess(mcDrvContext_t peerContext, unsigned int Flags);
 */
/* cuCtxEnablePeerAccess -> wcuCtxEnablePeerAccess */

/*
 * CUDA API: cuCtxFromGreenCtx
 * Reference wrapper: wcuCtxFromGreenCtx
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxFromGreenCtx(MCcontext *pContext, MCcontext hCtx);
 */
/* cuCtxFromGreenCtx -> wcuCtxFromGreenCtx */

/*
 * CUDA API: cuCtxGetApiVersion
 * Reference wrapper: wcuCtxGetApiVersion
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxGetApiVersion(mcDrvContext_t ctx, unsigned int *version);
 */
/* cuCtxGetApiVersion -> wcuCtxGetApiVersion */

/*
 * CUDA API: cuCtxGetCacheConfig
 * Reference wrapper: wcuCtxGetCacheConfig
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxGetCacheConfig(mcDrvfunc_cache_t *pconfig);
 */
/* cuCtxGetCacheConfig -> wcuCtxGetCacheConfig */

/*
 * CUDA API: cuCtxGetCurrent
 * Reference wrapper: wcuCtxGetCurrent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxGetCurrent(mcDrvContext_t *pctx);
 */
/* cuCtxGetCurrent -> wcuCtxGetCurrent */

/*
 * CUDA API: cuCtxGetDevResource
 * Reference wrapper: wcuCtxGetDevResource
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxGetDevResource(MCcontext hCtx, mcDrvDevResource *resource, mcDrvDevResourceType type);
 */
/* cuCtxGetDevResource -> wcuCtxGetDevResource */

/*
 * CUDA API: cuCtxGetDevice
 * Reference wrapper: wcuCtxGetDevice
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxGetDevice(mcDrvDevice_t *device);
 */
/* cuCtxGetDevice -> wcuCtxGetDevice */

/*
 * CUDA API: cuCtxGetExecAffinity
 * Reference wrapper: wcuCtxGetExecAffinity
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxGetExecAffinity(mcDrvexecAffinityParam_t *pExecAffinity, mcDrvexecAffinityType type);
 */
/* cuCtxGetExecAffinity -> wcuCtxGetExecAffinity */

/*
 * CUDA API: cuCtxGetFlags
 * Reference wrapper: wcuCtxGetFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxGetFlags(unsigned int *flags);
 */
/* cuCtxGetFlags -> wcuCtxGetFlags */

/*
 * CUDA API: cuCtxGetId
 * Reference wrapper: wcuCtxGetId
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxGetId(mcDrvContext_t ctx, unsigned long long *ctxId);
 */
/* cuCtxGetId -> wcuCtxGetId */

/*
 * CUDA API: cuCtxGetLimit
 * Reference wrapper: wcuCtxGetLimit
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxGetLimit(size_t *pvalue, mcDrvlimit_t limit);
 */
/* cuCtxGetLimit -> wcuCtxGetLimit */

/*
 * CUDA API: cuCtxGetSharedMemConfig
 * Reference wrapper: wcuCtxGetSharedMemConfig
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxGetSharedMemConfig(mcDrvsharedconfig_t *pConfig);
 */
/* cuCtxGetSharedMemConfig -> wcuCtxGetSharedMemConfig */

/*
 * CUDA API: cuCtxGetStreamPriorityRange
 * Reference wrapper: wcuCtxGetStreamPriorityRange
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxGetStreamPriorityRange(int *leastPriority, int *greatestPriority);
 */
/* cuCtxGetStreamPriorityRange -> wcuCtxGetStreamPriorityRange */

/*
 * CUDA API: cuCtxPopCurrent
 * Reference wrapper: wcuCtxPopCurrent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxPopCurrent(mcDrvContext_t *pctx);
 */
/* cuCtxPopCurrent -> wcuCtxPopCurrent */

/*
 * CUDA API: cuCtxPushCurrent
 * Reference wrapper: wcuCtxPushCurrent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxPushCurrent(mcDrvContext_t pctx);
 */
/* cuCtxPushCurrent -> wcuCtxPushCurrent */

/*
 * CUDA API: cuCtxResetPersistingL2Cache
 * Reference wrapper: wcuCtxResetPersistingL2Cache
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxResetPersistingL2Cache(void);
 */
/* cuCtxResetPersistingL2Cache -> wcuCtxResetPersistingL2Cache */

/*
 * CUDA API: cuCtxSetCacheConfig
 * Reference wrapper: wcuCtxSetCacheConfig
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxSetCacheConfig(mcDrvfunc_cache_t config);
 */
/* cuCtxSetCacheConfig -> wcuCtxSetCacheConfig */

/*
 * CUDA API: cuCtxSetCurrent
 * Reference wrapper: wcuCtxSetCurrent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxSetCurrent(mcDrvContext_t ctx);
 */
/* cuCtxSetCurrent -> wcuCtxSetCurrent */

/*
 * CUDA API: cuCtxSetFlags
 * Reference wrapper: wcuCtxSetFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxSetFlags(unsigned int flags);
 */
/* cuCtxSetFlags -> wcuCtxSetFlags */

/*
 * CUDA API: cuCtxSetLimit
 * Reference wrapper: wcuCtxSetLimit
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxSetLimit(mcDrvlimit_t limit, size_t value);
 */
/* cuCtxSetLimit -> wcuCtxSetLimit */

/*
 * CUDA API: cuCtxSetSharedMemConfig
 * Reference wrapper: wcuCtxSetSharedMemConfig
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxSetSharedMemConfig(mcDrvsharedconfig_t config);
 */
/* cuCtxSetSharedMemConfig -> wcuCtxSetSharedMemConfig */

/*
 * CUDA API: cuCtxSynchronize
 * Reference wrapper: wcuCtxSynchronize
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuCtxSynchronize(void);
 */
/* cuCtxSynchronize -> wcuCtxSynchronize */

/*
 * CUDA API: cuDestroyExternalMemory
 * Reference wrapper: wcuDestroyExternalMemory
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDestroyExternalMemory(mcDrvExternalMemory extMem);
 */
/* cuDestroyExternalMemory -> wcuDestroyExternalMemory */

/*
 * CUDA API: cuDestroyExternalSemaphore
 * Reference wrapper: wcuDestroyExternalSemaphore
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDestroyExternalSemaphore(mcDrvExternalSemaphore extSem);
 */
/* cuDestroyExternalSemaphore -> wcuDestroyExternalSemaphore */

/*
 * CUDA API: cuDevResourceGenerateDesc
 * Reference wrapper: wcuDevResourceGenerateDesc
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDevResourceGenerateDesc(mcDevResourceDesc_t *phDesc, mcDrvDevResource *resources, unsigned int nbResources);
 */
/* cuDevResourceGenerateDesc -> wcuDevResourceGenerateDesc */

/*
 * CUDA API: cuDevSmResourceSplitByCount
 * Reference wrapper: wcuDevSmResourceSplitByCount
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDevSmResourceSplitByCount( mcDrvDevResource *result, unsigned int *nbGroups, const mcDrvDevResource *input, mcDrvDevResource *remaining, unsigned int useFlags, unsigned int minCount);
 */
/* cuDevSmResourceSplitByCount -> wcuDevSmResourceSplitByCount */

/*
 * CUDA API: cuDeviceCanAccessPeer
 * Reference wrapper: wcuDeviceCanAccessPeer
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceCanAccessPeer(int *canAccessPeer, mcDrvDevice_t dev, mcDrvDevice_t peerDev);
 */
/* cuDeviceCanAccessPeer -> wcuDeviceCanAccessPeer */

/*
 * CUDA API: cuDeviceComputeCapability
 * Reference wrapper: wcuDeviceComputeCapability
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceComputeCapability(int *major, int *minor, mcDrvDevice_t dev);
 */
/* cuDeviceComputeCapability -> wcuDeviceComputeCapability */

/*
 * CUDA API: cuDeviceGetAttribute
 * Reference wrapper: wcuDeviceGetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetAttribute(int *pi, mcDrvDeviceAttribute_t drv_attr, mcDrvDevice_t device);
 */
/* cuDeviceGetAttribute -> wcuDeviceGetAttribute */

/*
 * CUDA API: cuDeviceGetAttributeInit
 * Reference wrapper: wcuDeviceGetAttributeInit
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetAttributeInit(int *pi, mcDrvDeviceAttribute_t drv_attr, mcDrvDevice_t device);
 */
/* cuDeviceGetAttributeInit -> wcuDeviceGetAttributeInit */

/*
 * CUDA API: cuDeviceGetByPCIBusId
 * Reference wrapper: wcuDeviceGetByPCIBusId
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetByPCIBusId(mcDrvDevice_t *device, const char *pciBusId);
 */
/* cuDeviceGetByPCIBusId -> wcuDeviceGetByPCIBusId */

/*
 * CUDA API: cuDeviceGetDefaultMemPool
 * Reference wrapper: wcuDeviceGetDefaultMemPool
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetDefaultMemPool(mcDrvMemPool_t *poolOut, mcDrvDevice_t dev);
 */
/* cuDeviceGetDefaultMemPool -> wcuDeviceGetDefaultMemPool */

/*
 * CUDA API: cuDeviceGetDevResource
 * Reference wrapper: wcuDeviceGetDevResource
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetDevResource(MCdevice device, mcDrvDevResource *resource, mcDrvDevResourceType type);
 */
/* cuDeviceGetDevResource -> wcuDeviceGetDevResource */

/*
 * CUDA API: cuDeviceGetExecAffinitySupport
 * Reference wrapper: wcuDeviceGetExecAffinitySupport
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetExecAffinitySupport(int *pi, mcDrvexecAffinityType type, mcDrvDevice_t dev);
 */
/* cuDeviceGetExecAffinitySupport -> wcuDeviceGetExecAffinitySupport */

/*
 * CUDA API: cuDeviceGetGraphMemAttribute
 * Reference wrapper: wcuDeviceGetGraphMemAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuDeviceGetGraphMemAttribute to be supported) mcDrvError_t CUBRIDGE_API wcuDeviceGetGraphMemAttribute(mcDrvDevice_t device, mcDrvGraphMem_attribute attr, void *value);
 */
/* cuDeviceGetGraphMemAttribute -> wcuDeviceGetGraphMemAttribute */

/*
 * CUDA API: cuDeviceGetLuid
 * Reference wrapper: wcuDeviceGetLuid
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetLuid(char *luid, unsigned int *deviceNodeMask, mcDrvDevice_t dev);
 */
/* cuDeviceGetLuid -> wcuDeviceGetLuid */

/*
 * CUDA API: cuDeviceGetMemPool
 * Reference wrapper: wcuDeviceGetMemPool
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetMemPool(mcDrvMemPool_t *poolOut, mcDrvDevice_t dev);
 */
/* cuDeviceGetMemPool -> wcuDeviceGetMemPool */

/*
 * CUDA API: cuDeviceGetName
 * Reference wrapper: wcuDeviceGetName
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetName(char *name, int len, mcDrvDevice_t dev);
 */
/* cuDeviceGetName -> wcuDeviceGetName */

/*
 * CUDA API: cuDeviceGetNvSciSyncAttributes
 * Reference wrapper: wcuDeviceGetNvSciSyncAttributes
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetNvSciSyncAttributes(void *mcSciSyncAttrList, mcDrvDevice_t dev, int flags);
 */
/* cuDeviceGetNvSciSyncAttributes -> wcuDeviceGetNvSciSyncAttributes */

/*
 * CUDA API: cuDeviceGetP2PAttribute
 * Reference wrapper: wcuDeviceGetP2PAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetP2PAttribute(int *value, mcDrvDeviceP2PAttribute attrib, mcDrvDevice_t srcDevice, mcDrvDevice_t dstDevice);
 */
/* cuDeviceGetP2PAttribute -> wcuDeviceGetP2PAttribute */

/*
 * CUDA API: cuDeviceGetPCIBusId
 * Reference wrapper: wcuDeviceGetPCIBusId
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetPCIBusId(char *pciBusId, int len, mcDrvDevice_t device);
 */
/* cuDeviceGetPCIBusId -> wcuDeviceGetPCIBusId */

/*
 * CUDA API: cuDeviceGetProperties
 * Reference wrapper: wcuDeviceGetProperties
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetProperties(mcDrvDevprop *prop, mcDrvDevice_t dev);
 */
/* cuDeviceGetProperties -> wcuDeviceGetProperties */

/*
 * CUDA API: cuDeviceGetTexture1DLinearMaxWidth
 * Reference wrapper: wcuDeviceGetTexture1DLinearMaxWidth
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetTexture1DLinearMaxWidth(size_t *maxWidthInElements, mcArray_Format format, unsigned numChannels, mcDrvDevice_t dev);
 */
/* cuDeviceGetTexture1DLinearMaxWidth -> wcuDeviceGetTexture1DLinearMaxWidth */

/*
 * CUDA API: cuDeviceGetUuid
 * Reference wrapper: wcuDeviceGetUuid
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetUuid(mcDrvUuid_t *uuid, mcDrvDevice_t device);
 */
/* cuDeviceGetUuid -> wcuDeviceGetUuid */

/*
 * CUDA API: cuDeviceGetUuid_v2
 * Reference wrapper: wcuDeviceGetUuid_v2
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceGetUuid_v2(mcDrvUuid_t *uuid, mcDrvDevice_t device);
 */
/* cuDeviceGetUuid_v2 -> wcuDeviceGetUuid_v2 */

/*
 * CUDA API: cuDeviceGraphMemTrim
 * Reference wrapper: wcuDeviceGraphMemTrim
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuDeviceGraphMemTrim to be supported) mcDrvError_t CUBRIDGE_API wcuDeviceGraphMemTrim(mcDrvDevice_t device);
 */
/* cuDeviceGraphMemTrim -> wcuDeviceGraphMemTrim */

/*
 * CUDA API: cuDevicePrimaryCtxGetState
 * Reference wrapper: wcuDevicePrimaryCtxGetState
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDevicePrimaryCtxGetState(mcDrvDevice_t dev, unsigned int *flags, int *active);
 */
/* cuDevicePrimaryCtxGetState -> wcuDevicePrimaryCtxGetState */

/*
 * CUDA API: cuDevicePrimaryCtxRelease
 * Reference wrapper: wcuDevicePrimaryCtxRelease
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDevicePrimaryCtxRelease(mcDrvDevice_t dev);
 */
/* cuDevicePrimaryCtxRelease -> wcuDevicePrimaryCtxRelease */

/*
 * CUDA API: cuDevicePrimaryCtxReset
 * Reference wrapper: wcuDevicePrimaryCtxReset
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDevicePrimaryCtxReset(mcDrvDevice_t dev);
 */
/* cuDevicePrimaryCtxReset -> wcuDevicePrimaryCtxReset */

/*
 * CUDA API: cuDevicePrimaryCtxRetain
 * Reference wrapper: wcuDevicePrimaryCtxRetain
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDevicePrimaryCtxRetain(mcDrvContext_t *pctx, mcDrvDevice_t dev);
 */
/* cuDevicePrimaryCtxRetain -> wcuDevicePrimaryCtxRetain */

/*
 * CUDA API: cuDevicePrimaryCtxSetFlags
 * Reference wrapper: wcuDevicePrimaryCtxSetFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDevicePrimaryCtxSetFlags(mcDrvDevice_t dev, unsigned int flags);
 */
/* cuDevicePrimaryCtxSetFlags -> wcuDevicePrimaryCtxSetFlags */

/*
 * CUDA API: cuDeviceSetGraphMemAttribute
 * Reference wrapper: wcuDeviceSetGraphMemAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuDeviceSetGraphMemAttribute to be supported) mcDrvError_t CUBRIDGE_API wcuDeviceSetGraphMemAttribute(mcDrvDevice_t device, mcDrvGraphMem_attribute attr, void *value);
 */
/* cuDeviceSetGraphMemAttribute -> wcuDeviceSetGraphMemAttribute */

/*
 * CUDA API: cuDeviceSetMemPool
 * Reference wrapper: wcuDeviceSetMemPool
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceSetMemPool(mcDrvDevice_t dev, mcDrvMemPool_t poolOut);
 */
/* cuDeviceSetMemPool -> wcuDeviceSetMemPool */

/*
 * CUDA API: cuDeviceTotalMem
 * Reference wrapper: wcuDeviceTotalMem
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuDeviceTotalMem(size_t *bytes, mcDrvDevice_t dev);
 */
/* cuDeviceTotalMem -> wcuDeviceTotalMem */

/*
 * CUDA API: cuEGLStreamConsumerAcquireFrame
 * Reference wrapper: wcuEGLStreamConsumerAcquireFrame
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuEGLStreamConsumerAcquireFrame(mcDrveglStreamConnection *conn, mcDrvGraphicsResource *pMacaResource, mcDrvStream_t *pStream, unsigned int timeout);
 */
/* cuEGLStreamConsumerAcquireFrame -> wcuEGLStreamConsumerAcquireFrame */

/*
 * CUDA API: cuEGLStreamConsumerConnect
 * Reference wrapper: wcuEGLStreamConsumerConnect
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuEGLStreamConsumerConnect(mcDrveglStreamConnection *conn, EGLStreamKHR stream);
 */
/* cuEGLStreamConsumerConnect -> wcuEGLStreamConsumerConnect */

/*
 * CUDA API: cuEGLStreamConsumerConnectWithFlags
 * Reference wrapper: wcuEGLStreamConsumerConnectWithFlags
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuEGLStreamConsumerConnectWithFlags(mcDrveglStreamConnection *conn, EGLStreamKHR stream, unsigned int flags);
 */
/* cuEGLStreamConsumerConnectWithFlags -> wcuEGLStreamConsumerConnectWithFlags */

/*
 * CUDA API: cuEGLStreamConsumerDisconnect
 * Reference wrapper: wcuEGLStreamConsumerDisconnect
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuEGLStreamConsumerDisconnect(mcDrveglStreamConnection *conn);
 */
/* cuEGLStreamConsumerDisconnect -> wcuEGLStreamConsumerDisconnect */

/*
 * CUDA API: cuEGLStreamConsumerReleaseFrame
 * Reference wrapper: wcuEGLStreamConsumerReleaseFrame
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuEGLStreamConsumerReleaseFrame(mcDrveglStreamConnection *conn, mcDrvGraphicsResource pMacaResource, mcDrvStream_t *pStream);
 */
/* cuEGLStreamConsumerReleaseFrame -> wcuEGLStreamConsumerReleaseFrame */

/*
 * CUDA API: cuEGLStreamProducerConnect
 * Reference wrapper: wcuEGLStreamProducerConnect
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuEGLStreamProducerConnect(mcDrveglStreamConnection *conn, EGLStreamKHR stream, EGLint width, EGLint height);
 */
/* cuEGLStreamProducerConnect -> wcuEGLStreamProducerConnect */

/*
 * CUDA API: cuEGLStreamProducerDisconnect
 * Reference wrapper: wcuEGLStreamProducerDisconnect
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuEGLStreamProducerDisconnect(mcDrveglStreamConnection *conn);
 */
/* cuEGLStreamProducerDisconnect -> wcuEGLStreamProducerDisconnect */

/*
 * CUDA API: cuEGLStreamProducerPresentFrame
 * Reference wrapper: wcuEGLStreamProducerPresentFrame
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuEGLStreamProducerPresentFrame(mcDrveglStreamConnection *conn, mcDrveglFrame eglframe, mcDrvStream_t *pStream);
 */
/* cuEGLStreamProducerPresentFrame -> wcuEGLStreamProducerPresentFrame */

/*
 * CUDA API: cuEGLStreamProducerReturnFrame
 * Reference wrapper: wcuEGLStreamProducerReturnFrame
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuEGLStreamProducerReturnFrame(mcDrveglStreamConnection *conn, mcDrveglFrame *eglframe, mcDrvStream_t *pStream);
 */
/* cuEGLStreamProducerReturnFrame -> wcuEGLStreamProducerReturnFrame */

/*
 * CUDA API: cuEventCreate
 * Reference wrapper: wcuEventCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuEventCreate(mcDrvEvent_t *event, unsigned int flags);
 */
/* cuEventCreate -> wcuEventCreate */

/*
 * CUDA API: cuEventCreateFromEGLSync
 * Reference wrapper: wcuEventCreateFromEGLSync
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuEventCreateFromEGLSync(mcDrvEvent_t *phEvent, EGLSyncKHR eglSync, unsigned int flags);
 */
/* cuEventCreateFromEGLSync -> wcuEventCreateFromEGLSync */

/*
 * CUDA API: cuEventDestroy
 * Reference wrapper: wcuEventDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuEventDestroy(mcDrvEvent_t event);
 */
/* cuEventDestroy -> wcuEventDestroy */

/*
 * CUDA API: cuEventElapsedTime
 * Reference wrapper: wcuEventElapsedTime
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuEventElapsedTime(float *ms, mcDrvEvent_t start, mcDrvEvent_t end);
 */
/* cuEventElapsedTime -> wcuEventElapsedTime */

/*
 * CUDA API: cuEventQuery
 * Reference wrapper: wcuEventQuery
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuEventQuery(mcDrvEvent_t event);
 */
/* cuEventQuery -> wcuEventQuery */

/*
 * CUDA API: cuEventRecord
 * Reference wrapper: wcuEventRecord
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuEventRecord(mcDrvEvent_t event, mcDrvStream_t stream);
 */
/* cuEventRecord -> wcuEventRecord */

/*
 * CUDA API: cuEventRecordWithFlags
 * Reference wrapper: wcuEventRecordWithFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuEventRecordWithFlags(mcDrvEvent_t hEvent, mcDrvStream_t hStream, unsigned int flags);
 */
/* cuEventRecordWithFlags -> wcuEventRecordWithFlags */

/*
 * CUDA API: cuEventSynchronize
 * Reference wrapper: wcuEventSynchronize
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuEventSynchronize(mcDrvEvent_t event);
 */
/* cuEventSynchronize -> wcuEventSynchronize */

/*
 * CUDA API: cuExternalMemoryGetMappedBuffer
 * Reference wrapper: wcuExternalMemoryGetMappedBuffer
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuExternalMemoryGetMappedBuffer(mcDrvDeviceptr_t *devPtr, mcDrvExternalMemory extMem, const mcDrvExternalMemoryBufferDesc *bufferDesc);
 */
/* cuExternalMemoryGetMappedBuffer -> wcuExternalMemoryGetMappedBuffer */

/*
 * CUDA API: cuExternalMemoryGetMappedMipmappedArray
 * Reference wrapper: wcuExternalMemoryGetMappedMipmappedArray
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuExternalMemoryGetMappedMipmappedArray(mcDrvMipmappedArray *mipmap, mcDrvExternalMemory extMem, const mcDrvExternalMemoryMipmappedArrayDesc *mipmapDesc);
 */
/* cuExternalMemoryGetMappedMipmappedArray -> wcuExternalMemoryGetMappedMipmappedArray */

/*
 * CUDA API: cuFlushGPUDirectRDMAWrites
 * Reference wrapper: wcuFlushGPUDirectRDMAWrites
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuFlushGPUDirectRDMAWrites(mcDrvFlushGPUDirectRDMAWritesTarget target, mcDrvFlushGPUDirectRDMAWritesScope scope);
 */
/* cuFlushGPUDirectRDMAWrites -> wcuFlushGPUDirectRDMAWrites */

/*
 * CUDA API: cuFuncGetAttribute
 * Reference wrapper: wcuFuncGetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuFuncGetAttribute(int *value, mcDrvFunction_attribute attrib, mcDrvFunction_t hfunc);
 */
/* cuFuncGetAttribute -> wcuFuncGetAttribute */

/*
 * CUDA API: cuFuncGetModule
 * Reference wrapper: wcuFuncGetModule
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuFuncGetModule(mcDrvModule_t *hmod, mcDrvFunction_t hfunc);
 */
/* cuFuncGetModule -> wcuFuncGetModule */

/*
 * CUDA API: cuFuncSetAttribute
 * Reference wrapper: wcuFuncSetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuFuncSetAttribute(mcDrvFunction_t hfunc, mcDrvFunction_attribute attrib, int value);
 */
/* cuFuncSetAttribute -> wcuFuncSetAttribute */

/*
 * CUDA API: cuFuncSetBlockShape
 * Reference wrapper: wcuFuncSetBlockShape
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuFuncSetBlockShape(mcDrvFunction_t hfunc, int x, int y, int z);
 */
/* cuFuncSetBlockShape -> wcuFuncSetBlockShape */

/*
 * CUDA API: cuFuncSetCacheConfig
 * Reference wrapper: wcuFuncSetCacheConfig
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuFuncSetCacheConfig(mcDrvFunction_t hfunc, mcDrvfunc_cache_t config);
 */
/* cuFuncSetCacheConfig -> wcuFuncSetCacheConfig */

/*
 * CUDA API: cuFuncSetSharedMemConfig
 * Reference wrapper: wcuFuncSetSharedMemConfig
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuFuncSetSharedMemConfig(mcDrvFunction_t hfunc, mcDrvsharedconfig_t config);
 */
/* cuFuncSetSharedMemConfig -> wcuFuncSetSharedMemConfig */

/*
 * CUDA API: cuFuncSetSharedSize
 * Reference wrapper: wcuFuncSetSharedSize
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuFuncSetSharedSize(mcDrvFunction_t hfunc, unsigned int bytes);
 */
/* cuFuncSetSharedSize -> wcuFuncSetSharedSize */

/*
 * CUDA API: cuGLCtxCreate
 * Reference wrapper: wcuGLCtxCreate
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGLCtxCreate(mcDrvContext_t *pCtx, unsigned int Flags, mcDrvDevice_t device);
 */
/* cuGLCtxCreate -> wcuGLCtxCreate */

/*
 * CUDA API: cuGLInit
 * Reference wrapper: wcuGLInit
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGLInit(void);
 */
/* cuGLInit -> wcuGLInit */

/*
 * CUDA API: cuGLMapBufferObject
 * Reference wrapper: wcuGLMapBufferObject
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGLMapBufferObject(mcDrvDeviceptrv1_t *dptr, unsigned int *size, GLuint buffer);
 */
/* cuGLMapBufferObject -> wcuGLMapBufferObject */

/*
 * CUDA API: cuGLMapBufferObjectAsync
 * Reference wrapper: wcuGLMapBufferObjectAsync
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGLMapBufferObjectAsync(mcDrvDeviceptrv1_t *dptr, unsigned int *size, GLuint buffer, mcDrvStream_t hStream);
 */
/* cuGLMapBufferObjectAsync -> wcuGLMapBufferObjectAsync */

/*
 * CUDA API: cuGLMapBufferObjectAsync_v2
 * Reference wrapper: wcuGLMapBufferObjectAsync_v2
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGLMapBufferObjectAsync_v2(mcDrvDeviceptr_t *dptr, size_t *size, GLuint buffer, mcDrvStream_t hStream);
 */
/* cuGLMapBufferObjectAsync_v2 -> wcuGLMapBufferObjectAsync_v2 */

/*
 * CUDA API: cuGLMapBufferObject_v2
 * Reference wrapper: wcuGLMapBufferObject_v2
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGLMapBufferObject_v2(mcDrvDeviceptr_t *dptr, size_t *size, GLuint buffer);
 */
/* cuGLMapBufferObject_v2 -> wcuGLMapBufferObject_v2 */

/*
 * CUDA API: cuGLRegisterBufferObject
 * Reference wrapper: wcuGLRegisterBufferObject
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGLRegisterBufferObject(GLuint buffer);
 */
/* cuGLRegisterBufferObject -> wcuGLRegisterBufferObject */

/*
 * CUDA API: cuGLSetBufferObjectMapFlags
 * Reference wrapper: wcuGLSetBufferObjectMapFlags
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGLSetBufferObjectMapFlags(GLuint buffer, unsigned int Flags);
 */
/* cuGLSetBufferObjectMapFlags -> wcuGLSetBufferObjectMapFlags */

/*
 * CUDA API: cuGLUnmapBufferObject
 * Reference wrapper: wcuGLUnmapBufferObject
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGLUnmapBufferObject(GLuint buffer);
 */
/* cuGLUnmapBufferObject -> wcuGLUnmapBufferObject */

/*
 * CUDA API: cuGLUnmapBufferObjectAsync
 * Reference wrapper: wcuGLUnmapBufferObjectAsync
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGLUnmapBufferObjectAsync(GLuint buffer, mcDrvStream_t hStream);
 */
/* cuGLUnmapBufferObjectAsync -> wcuGLUnmapBufferObjectAsync */

/*
 * CUDA API: cuGLUnregisterBufferObject
 * Reference wrapper: wcuGLUnregisterBufferObject
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGLUnregisterBufferObject(GLuint buffer);
 */
/* cuGLUnregisterBufferObject -> wcuGLUnregisterBufferObject */

/*
 * CUDA API: cuGetErrorName
 * Reference wrapper: wcuGetErrorName
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGetErrorName(mcDrvError_t error, const char **pStr);
 */
/* cuGetErrorName -> wcuGetErrorName */

/*
 * CUDA API: cuGetErrorString
 * Reference wrapper: wcuGetErrorString
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGetErrorString(mcDrvError_t error, const char **pStr);
 */
/* cuGetErrorString -> wcuGetErrorString */

/*
 * CUDA API: cuGetExportTable
 * Reference wrapper: wcuGetExportTable
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGetExportTable(const void **ppExportTable, const mcDrvUuid_t *pExportTableId);
 */
/* cuGetExportTable -> wcuGetExportTable */

/*
 * CUDA API: cuGetProcAddress
 * Reference wrapper: wcuGetProcAddress
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGetProcAddress(const char *symbol, void **pfn, int mcVersion, uint64_t flags);
 */
/* cuGetProcAddress -> wcuGetProcAddress */

/*
 * CUDA API: cuGetProcAddress_v2
 * Reference wrapper: wcuGetProcAddress_v2
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGetProcAddress_v2(const char *symbol, void **pfn, int mcVersion, uint64_t flags, mcDrvProcAddressQueryResult *symbolStatus);
 */
/* cuGetProcAddress_v2 -> wcuGetProcAddress_v2 */

/*
 * CUDA API: cuGraphAddBatchMemOpNode
 * Reference wrapper: wcuGraphAddBatchMemOpNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddBatchMemOpNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, const mcDrvBatchMemOpNodeParams *nodeParams);
 */
/* cuGraphAddBatchMemOpNode -> wcuGraphAddBatchMemOpNode */

/*
 * CUDA API: cuGraphAddChildGraphNode
 * Reference wrapper: wcuGraphAddChildGraphNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddChildGraphNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, mcDrvGraph childGraph);
 */
/* cuGraphAddChildGraphNode -> wcuGraphAddChildGraphNode */

/*
 * CUDA API: cuGraphAddDependencies
 * Reference wrapper: wcuGraphAddDependencies
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddDependencies(mcDrvGraph hGraph, const mcDrvGraphNode *from, const mcDrvGraphNode *to, size_t numDependencies);
 */
/* cuGraphAddDependencies -> wcuGraphAddDependencies */

/*
 * CUDA API: cuGraphAddDependencies_v2
 * Reference wrapper: wcuGraphAddDependencies_v2
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddDependencies_v2(mcDrvGraph hGraph, const mcDrvGraphNode *from, const mcDrvGraphNode *to, const mcGraphEdgeData *edgeData, size_t numDependencies);
 */
/* cuGraphAddDependencies_v2 -> wcuGraphAddDependencies_v2 */

/*
 * CUDA API: cuGraphAddEmptyNode
 * Reference wrapper: wcuGraphAddEmptyNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddEmptyNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies);
 */
/* cuGraphAddEmptyNode -> wcuGraphAddEmptyNode */

/*
 * CUDA API: cuGraphAddEventRecordNode
 * Reference wrapper: wcuGraphAddEventRecordNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddEventRecordNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, mcDrvEvent_t event);
 */
/* cuGraphAddEventRecordNode -> wcuGraphAddEventRecordNode */

/*
 * CUDA API: cuGraphAddEventWaitNode
 * Reference wrapper: wcuGraphAddEventWaitNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddEventWaitNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, mcDrvEvent_t event);
 */
/* cuGraphAddEventWaitNode -> wcuGraphAddEventWaitNode */

/*
 * CUDA API: cuGraphAddExternalSemaphoresSignalNode
 * Reference wrapper: wcuGraphAddExternalSemaphoresSignalNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphAddExternalSemaphoresSignalNode to be supported) mcDrvError_t CUBRIDGE_API wcuGraphAddExternalSemaphoresSignalNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, const mcDrvExtSemSignalNodeParams *nodeParams);
 */
/* cuGraphAddExternalSemaphoresSignalNode -> wcuGraphAddExternalSemaphoresSignalNode */

/*
 * CUDA API: cuGraphAddExternalSemaphoresWaitNode
 * Reference wrapper: wcuGraphAddExternalSemaphoresWaitNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphAddExternalSemaphoresWaitNode to be supported) mcDrvError_t CUBRIDGE_API wcuGraphAddExternalSemaphoresWaitNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, const mcDrvExtSemWaitNodeParams *nodeParams);
 */
/* cuGraphAddExternalSemaphoresWaitNode -> wcuGraphAddExternalSemaphoresWaitNode */

/*
 * CUDA API: cuGraphAddHostNode
 * Reference wrapper: wcuGraphAddHostNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddHostNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, const mcDrvHostNodeParams *nodeParams);
 */
/* cuGraphAddHostNode -> wcuGraphAddHostNode */

/*
 * CUDA API: cuGraphAddKernelNode
 * Reference wrapper: wcuGraphAddKernelNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddKernelNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, const mcDrvKernelNodeParams *nodeParams);
 */
/* cuGraphAddKernelNode -> wcuGraphAddKernelNode */

/*
 * CUDA API: cuGraphAddMemAllocNode
 * Reference wrapper: wcuGraphAddMemAllocNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddMemAllocNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, mcDrvMemAllocNodeParams *nodeParams);
 */
/* cuGraphAddMemAllocNode -> wcuGraphAddMemAllocNode */

/*
 * CUDA API: cuGraphAddMemFreeNode
 * Reference wrapper: wcuGraphAddMemFreeNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddMemFreeNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, mcDrvDeviceptr_t dptr);
 */
/* cuGraphAddMemFreeNode -> wcuGraphAddMemFreeNode */

/*
 * CUDA API: cuGraphAddMemcpyNode
 * Reference wrapper: wcuGraphAddMemcpyNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddMemcpyNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, const mcDrvMemcpy3D *copyParams, mcDrvContext_t ctx);
 */
/* cuGraphAddMemcpyNode -> wcuGraphAddMemcpyNode */

/*
 * CUDA API: cuGraphAddMemsetNode
 * Reference wrapper: wcuGraphAddMemsetNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddMemsetNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, const mcDrvMemsetNodeParams *memsetParams, mcDrvContext_t ctx);
 */
/* cuGraphAddMemsetNode -> wcuGraphAddMemsetNode */

/*
 * CUDA API: cuGraphAddNode
 * Reference wrapper: wcuGraphAddNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddNode(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, size_t numDependencies, mcDrvGraphNodeParams *nodeParams);
 */
/* cuGraphAddNode -> wcuGraphAddNode */

/*
 * CUDA API: cuGraphAddNode_v2
 * Reference wrapper: wcuGraphAddNode_v2
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphAddNode_v2(mcDrvGraphNode *phGraphNode, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, const mcGraphEdgeData *dependencyData, size_t numDependencies, mcDrvGraphNodeParams *nodeParams);
 */
/* cuGraphAddNode_v2 -> wcuGraphAddNode_v2 */

/*
 * CUDA API: cuGraphBatchMemOpNodeGetParams
 * Reference wrapper: wcuGraphBatchMemOpNodeGetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphBatchMemOpNodeGetParams(mcDrvGraphNode hNode, mcDrvBatchMemOpNodeParams *nodeParams_out);
 */
/* cuGraphBatchMemOpNodeGetParams -> wcuGraphBatchMemOpNodeGetParams */

/*
 * CUDA API: cuGraphBatchMemOpNodeSetParams
 * Reference wrapper: wcuGraphBatchMemOpNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphBatchMemOpNodeSetParams(mcDrvGraphNode hNode, const mcDrvBatchMemOpNodeParams *nodeParams);
 */
/* cuGraphBatchMemOpNodeSetParams -> wcuGraphBatchMemOpNodeSetParams */

/*
 * CUDA API: cuGraphChildGraphNodeGetGraph
 * Reference wrapper: wcuGraphChildGraphNodeGetGraph
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphChildGraphNodeGetGraph(mcDrvGraphNode hNode, mcDrvGraph *phGraph);
 */
/* cuGraphChildGraphNodeGetGraph -> wcuGraphChildGraphNodeGetGraph */

/*
 * CUDA API: cuGraphClone
 * Reference wrapper: wcuGraphClone
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphClone(mcDrvGraph *phGraphClone, mcDrvGraph originalGraph);
 */
/* cuGraphClone -> wcuGraphClone */

/*
 * CUDA API: cuGraphConditionalHandleCreate
 * Reference wrapper: wcuGraphConditionalHandleCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphConditionalHandleCreate(mcDrvGraphConditionalHandle *pHandle_out, mcDrvGraph hGraph, MCcontext ctx, unsigned int defaultLaunchValue, unsigned int flags);
 */
/* cuGraphConditionalHandleCreate -> wcuGraphConditionalHandleCreate */

/*
 * CUDA API: cuGraphCreate
 * Reference wrapper: wcuGraphCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphCreate(mcDrvGraph *phGraph, unsigned int flags);
 */
/* cuGraphCreate -> wcuGraphCreate */

/*
 * CUDA API: cuGraphDebugDotPrint
 * Reference wrapper: wcuGraphDebugDotPrint
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphDebugDotPrint(mcDrvGraph hGraph, const char *path, unsigned int flags);
 */
/* cuGraphDebugDotPrint -> wcuGraphDebugDotPrint */

/*
 * CUDA API: cuGraphDestroy
 * Reference wrapper: wcuGraphDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphDestroy(mcDrvGraph hGraph);
 */
/* cuGraphDestroy -> wcuGraphDestroy */

/*
 * CUDA API: cuGraphDestroyNode
 * Reference wrapper: wcuGraphDestroyNode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphDestroyNode(mcDrvGraphNode hNode);
 */
/* cuGraphDestroyNode -> wcuGraphDestroyNode */

/*
 * CUDA API: cuGraphEventRecordNodeGetEvent
 * Reference wrapper: wcuGraphEventRecordNodeGetEvent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphEventRecordNodeGetEvent(mcDrvGraphNode hNode, mcDrvEvent_t *event_out);
 */
/* cuGraphEventRecordNodeGetEvent -> wcuGraphEventRecordNodeGetEvent */

/*
 * CUDA API: cuGraphEventRecordNodeSetEvent
 * Reference wrapper: wcuGraphEventRecordNodeSetEvent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphEventRecordNodeSetEvent(mcDrvGraphNode hNode, mcDrvEvent_t event);
 */
/* cuGraphEventRecordNodeSetEvent -> wcuGraphEventRecordNodeSetEvent */

/*
 * CUDA API: cuGraphEventWaitNodeGetEvent
 * Reference wrapper: wcuGraphEventWaitNodeGetEvent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphEventWaitNodeGetEvent(mcDrvGraphNode hNode, mcDrvEvent_t *event_out);
 */
/* cuGraphEventWaitNodeGetEvent -> wcuGraphEventWaitNodeGetEvent */

/*
 * CUDA API: cuGraphEventWaitNodeSetEvent
 * Reference wrapper: wcuGraphEventWaitNodeSetEvent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphEventWaitNodeSetEvent(mcDrvGraphNode hNode, mcDrvEvent_t event);
 */
/* cuGraphEventWaitNodeSetEvent -> wcuGraphEventWaitNodeSetEvent */

/*
 * CUDA API: cuGraphExecBatchMemOpNodeSetParams
 * Reference wrapper: wcuGraphExecBatchMemOpNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphExecBatchMemOpNodeSetParams(mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, const mcDrvBatchMemOpNodeParams *nodeParams);
 */
/* cuGraphExecBatchMemOpNodeSetParams -> wcuGraphExecBatchMemOpNodeSetParams */

/*
 * CUDA API: cuGraphExecChildGraphNodeSetParams
 * Reference wrapper: wcuGraphExecChildGraphNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphExecChildGraphNodeSetParams(mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, mcDrvGraph childGraph);
 */
/* cuGraphExecChildGraphNodeSetParams -> wcuGraphExecChildGraphNodeSetParams */

/*
 * CUDA API: cuGraphExecDestroy
 * Reference wrapper: wcuGraphExecDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphExecDestroy(mcDrvGraphExec hGraphExec);
 */
/* cuGraphExecDestroy -> wcuGraphExecDestroy */

/*
 * CUDA API: cuGraphExecEventRecordNodeSetEvent
 * Reference wrapper: wcuGraphExecEventRecordNodeSetEvent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphExecEventRecordNodeSetEvent(mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, mcDrvEvent_t event);
 */
/* cuGraphExecEventRecordNodeSetEvent -> wcuGraphExecEventRecordNodeSetEvent */

/*
 * CUDA API: cuGraphExecEventWaitNodeSetEvent
 * Reference wrapper: wcuGraphExecEventWaitNodeSetEvent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphExecEventWaitNodeSetEvent(mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, mcDrvEvent_t event);
 */
/* cuGraphExecEventWaitNodeSetEvent -> wcuGraphExecEventWaitNodeSetEvent */

/*
 * CUDA API: cuGraphExecExternalSemaphoresSignalNodeSetParams
 * Reference wrapper: wcuGraphExecExternalSemaphoresSignalNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphExecExternalSemaphoresSignalNodeSetParams to be supported) mcDrvError_t CUBRIDGE_API wcuGraphExecExternalSemaphoresSignalNodeSetParams( mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, const mcDrvExtSemSignalNodeParams *nodeParams);
 */
/* cuGraphExecExternalSemaphoresSignalNodeSetParams -> wcuGraphExecExternalSemaphoresSignalNodeSetParams */

/*
 * CUDA API: cuGraphExecExternalSemaphoresWaitNodeSetParams
 * Reference wrapper: wcuGraphExecExternalSemaphoresWaitNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphExecExternalSemaphoresWaitNodeSetParams to be supported) mcDrvError_t CUBRIDGE_API wcuGraphExecExternalSemaphoresWaitNodeSetParams(mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, const mcDrvExtSemWaitNodeParams *nodeParams);
 */
/* cuGraphExecExternalSemaphoresWaitNodeSetParams -> wcuGraphExecExternalSemaphoresWaitNodeSetParams */

/*
 * CUDA API: cuGraphExecGetFlags
 * Reference wrapper: wcuGraphExecGetFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphExecGetFlags(mcDrvGraphExec hGraphExec, mcuint64_t *flags);
 */
/* cuGraphExecGetFlags -> wcuGraphExecGetFlags */

/*
 * CUDA API: cuGraphExecHostNodeSetParams
 * Reference wrapper: wcuGraphExecHostNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphExecHostNodeSetParams(mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, const mcDrvHostNodeParams *nodeParams);
 */
/* cuGraphExecHostNodeSetParams -> wcuGraphExecHostNodeSetParams */

/*
 * CUDA API: cuGraphExecKernelNodeSetParams
 * Reference wrapper: wcuGraphExecKernelNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphExecKernelNodeSetParams to be supported) mcDrvError_t CUBRIDGE_API wcuGraphExecKernelNodeSetParams(mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, const mcDrvKernelNodeParams *nodeParams);
 */
/* cuGraphExecKernelNodeSetParams -> wcuGraphExecKernelNodeSetParams */

/*
 * CUDA API: cuGraphExecMemcpyNodeSetParams
 * Reference wrapper: wcuGraphExecMemcpyNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphExecMemcpyNodeSetParams(mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, const mcDrvMemcpy3D *copyParams, mcDrvContext_t ctx);
 */
/* cuGraphExecMemcpyNodeSetParams -> wcuGraphExecMemcpyNodeSetParams */

/*
 * CUDA API: cuGraphExecMemsetNodeSetParams
 * Reference wrapper: wcuGraphExecMemsetNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphExecMemsetNodeSetParams(mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, const mcDrvMemsetNodeParams *memsetParams, mcDrvContext_t ctx);
 */
/* cuGraphExecMemsetNodeSetParams -> wcuGraphExecMemsetNodeSetParams */

/*
 * CUDA API: cuGraphExecUpdate
 * Reference wrapper: wcuGraphExecUpdate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphExecUpdate(mcDrvGraphExec hGraphExec, mcDrvGraph hGraph, mcDrvGraphNode *hErrorNode_out, mcGraphExecUpdateResult *updateResult_out);
 */
/* cuGraphExecUpdate -> wcuGraphExecUpdate */

/*
 * CUDA API: cuGraphExecUpdate_V2
 * Reference wrapper: wcuGraphExecUpdate_V2
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphExecUpdate_V2(mcDrvGraphExec hGraphExec, mcDrvGraph hGraph, mcGraphExecUpdateResultInfo *resultInfo);
 */
/* cuGraphExecUpdate_V2 -> wcuGraphExecUpdate_V2 */

/*
 * CUDA API: cuGraphExternalSemaphoresSignalNodeGetParams
 * Reference wrapper: wcuGraphExternalSemaphoresSignalNodeGetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphExternalSemaphoresSignalNodeGetParams to be supported) mcDrvError_t CUBRIDGE_API wcuGraphExternalSemaphoresSignalNodeGetParams(mcDrvGraphNode hNode, mcDrvExtSemSignalNodeParams *params_out);
 */
/* cuGraphExternalSemaphoresSignalNodeGetParams -> wcuGraphExternalSemaphoresSignalNodeGetParams */

/*
 * CUDA API: cuGraphExternalSemaphoresSignalNodeSetParams
 * Reference wrapper: wcuGraphExternalSemaphoresSignalNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphExternalSemaphoresSignalNodeSetParams to be supported) mcDrvError_t CUBRIDGE_API wcuGraphExternalSemaphoresSignalNodeSetParams(mcDrvGraphNode hNode, const mcDrvExtSemSignalNodeParams *nodeParams);
 */
/* cuGraphExternalSemaphoresSignalNodeSetParams -> wcuGraphExternalSemaphoresSignalNodeSetParams */

/*
 * CUDA API: cuGraphExternalSemaphoresWaitNodeGetParams
 * Reference wrapper: wcuGraphExternalSemaphoresWaitNodeGetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphExternalSemaphoresWaitNodeGetParams to be supported) mcDrvError_t CUBRIDGE_API wcuGraphExternalSemaphoresWaitNodeGetParams(mcDrvGraphNode hNode, mcDrvExtSemWaitNodeParams *params_out);
 */
/* cuGraphExternalSemaphoresWaitNodeGetParams -> wcuGraphExternalSemaphoresWaitNodeGetParams */

/*
 * CUDA API: cuGraphExternalSemaphoresWaitNodeSetParams
 * Reference wrapper: wcuGraphExternalSemaphoresWaitNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphExternalSemaphoresWaitNodeSetParams to be supported) mcDrvError_t CUBRIDGE_API wcuGraphExternalSemaphoresWaitNodeSetParams(mcDrvGraphNode hNode, const mcDrvExtSemWaitNodeParams *nodeParams);
 */
/* cuGraphExternalSemaphoresWaitNodeSetParams -> wcuGraphExternalSemaphoresWaitNodeSetParams */

/*
 * CUDA API: cuGraphGetEdges
 * Reference wrapper: wcuGraphGetEdges
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphGetEdges(mcDrvGraph hGraph, mcDrvGraphNode *from, mcDrvGraphNode *to, size_t *numEdges);
 */
/* cuGraphGetEdges -> wcuGraphGetEdges */

/*
 * CUDA API: cuGraphGetEdges_v2
 * Reference wrapper: wcuGraphGetEdges_v2
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphGetEdges_v2(mcDrvGraph hGraph, mcDrvGraphNode *from, mcDrvGraphNode *to, mcGraphEdgeData *edgeData, size_t *numEdges);
 */
/* cuGraphGetEdges_v2 -> wcuGraphGetEdges_v2 */

/*
 * CUDA API: cuGraphGetNodes
 * Reference wrapper: wcuGraphGetNodes
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphGetNodes(mcDrvGraph hGraph, mcDrvGraphNode *nodes, size_t *numNodes);
 */
/* cuGraphGetNodes -> wcuGraphGetNodes */

/*
 * CUDA API: cuGraphGetRootNodes
 * Reference wrapper: wcuGraphGetRootNodes
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphGetRootNodes(mcDrvGraph hGraph, mcDrvGraphNode *rootNodes, size_t *numRootNodes);
 */
/* cuGraphGetRootNodes -> wcuGraphGetRootNodes */

/*
 * CUDA API: cuGraphHostNodeGetParams
 * Reference wrapper: wcuGraphHostNodeGetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphHostNodeGetParams(mcDrvGraphNode hNode, mcDrvHostNodeParams *nodeParams);
 */
/* cuGraphHostNodeGetParams -> wcuGraphHostNodeGetParams */

/*
 * CUDA API: cuGraphHostNodeSetParams
 * Reference wrapper: wcuGraphHostNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphHostNodeSetParams(mcDrvGraphNode hNode, const mcDrvHostNodeParams *nodeParams);
 */
/* cuGraphHostNodeSetParams -> wcuGraphHostNodeSetParams */

/*
 * CUDA API: cuGraphInstantiate
 * Reference wrapper: wcuGraphInstantiate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphInstantiate(mcDrvGraphExec *phGraphExec, mcDrvGraph hGraph, mcDrvGraphNode *phErrorNode, char *logBuffer, size_t bufferSize);
 */
/* cuGraphInstantiate -> wcuGraphInstantiate */

/*
 * CUDA API: cuGraphInstantiateWithFlags
 * Reference wrapper: wcuGraphInstantiateWithFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphInstantiateWithFlags(mcDrvGraphExec *phGraphExec, mcDrvGraph hGraph, unsigned long long flags);
 */
/* cuGraphInstantiateWithFlags -> wcuGraphInstantiateWithFlags */

/*
 * CUDA API: cuGraphInstantiateWithParams
 * Reference wrapper: wcuGraphInstantiateWithParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphInstantiateWithParams(mcDrvGraphExec *phGraphExec, mcDrvGraph hGraph, mcDrvGraphInstantiateParams *instantiateParams);
 */
/* cuGraphInstantiateWithParams -> wcuGraphInstantiateWithParams */

/*
 * CUDA API: cuGraphKernelNodeCopyAttributes
 * Reference wrapper: wcuGraphKernelNodeCopyAttributes
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphKernelNodeCopyAttributes to be supported) mcDrvError_t CUBRIDGE_API wcuGraphKernelNodeCopyAttributes(mcDrvGraphNode dst, mcDrvGraphNode src);
 */
/* cuGraphKernelNodeCopyAttributes -> wcuGraphKernelNodeCopyAttributes */

/*
 * CUDA API: cuGraphKernelNodeGetAttribute
 * Reference wrapper: wcuGraphKernelNodeGetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphKernelNodeGetAttribute to be supported) mcDrvError_t CUBRIDGE_API wcuGraphKernelNodeGetAttribute(mcDrvGraphNode hNode, mcDrvkernelNodeAttrID attr, mcDrvKernelNodeAttrValue *value_out);
 */
/* cuGraphKernelNodeGetAttribute -> wcuGraphKernelNodeGetAttribute */

/*
 * CUDA API: cuGraphKernelNodeGetParams
 * Reference wrapper: wcuGraphKernelNodeGetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphKernelNodeGetParams to be supported) mcDrvError_t CUBRIDGE_API wcuGraphKernelNodeGetParams(mcDrvGraphNode hNode, mcDrvKernelNodeParams *nodeParams);
 */
/* cuGraphKernelNodeGetParams -> wcuGraphKernelNodeGetParams */

/*
 * CUDA API: cuGraphKernelNodeSetAttribute
 * Reference wrapper: wcuGraphKernelNodeSetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphKernelNodeSetAttribute to be supported) mcDrvError_t CUBRIDGE_API wcuGraphKernelNodeSetAttribute(mcDrvGraphNode hNode, mcDrvkernelNodeAttrID attr, const mcDrvKernelNodeAttrValue *value);
 */
/* cuGraphKernelNodeSetAttribute -> wcuGraphKernelNodeSetAttribute */

/*
 * CUDA API: cuGraphKernelNodeSetParams
 * Reference wrapper: wcuGraphKernelNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphKernelNodeSetParams to be supported) mcDrvError_t CUBRIDGE_API wcuGraphKernelNodeSetParams(mcDrvGraphNode hNode, const mcDrvKernelNodeParams *nodeParams);
 */
/* cuGraphKernelNodeSetParams -> wcuGraphKernelNodeSetParams */

/*
 * CUDA API: cuGraphLaunch
 * Reference wrapper: wcuGraphLaunch
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphLaunch(mcDrvGraphExec hGraphExec, mcDrvStream_t hStream);
 */
/* cuGraphLaunch -> wcuGraphLaunch */

/*
 * CUDA API: cuGraphMemAllocNodeGetParams
 * Reference wrapper: wcuGraphMemAllocNodeGetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphMemAllocNodeGetParams(mcDrvGraphNode hNode, mcDrvMemAllocNodeParams *params_out);
 */
/* cuGraphMemAllocNodeGetParams -> wcuGraphMemAllocNodeGetParams */

/*
 * CUDA API: cuGraphMemFreeNodeGetParams
 * Reference wrapper: wcuGraphMemFreeNodeGetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphMemFreeNodeGetParams(mcDrvGraphNode hNode, mcDrvDeviceptr_t *dptr_out);
 */
/* cuGraphMemFreeNodeGetParams -> wcuGraphMemFreeNodeGetParams */

/*
 * CUDA API: cuGraphMemcpyNodeGetParams
 * Reference wrapper: wcuGraphMemcpyNodeGetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphMemcpyNodeGetParams(mcDrvGraphNode hNode, mcDrvMemcpy3D *nodeParams);
 */
/* cuGraphMemcpyNodeGetParams -> wcuGraphMemcpyNodeGetParams */

/*
 * CUDA API: cuGraphMemcpyNodeSetParams
 * Reference wrapper: wcuGraphMemcpyNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphMemcpyNodeSetParams(mcDrvGraphNode hNode, const mcDrvMemcpy3D *nodeParams);
 */
/* cuGraphMemcpyNodeSetParams -> wcuGraphMemcpyNodeSetParams */

/*
 * CUDA API: cuGraphMemsetNodeGetParams
 * Reference wrapper: wcuGraphMemsetNodeGetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphMemsetNodeGetParams(mcDrvGraphNode hNode, mcDrvMemsetNodeParams *nodeParams);
 */
/* cuGraphMemsetNodeGetParams -> wcuGraphMemsetNodeGetParams */

/*
 * CUDA API: cuGraphMemsetNodeSetParams
 * Reference wrapper: wcuGraphMemsetNodeSetParams
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphMemsetNodeSetParams(mcDrvGraphNode hNode, const mcDrvMemsetNodeParams *nodeParams);
 */
/* cuGraphMemsetNodeSetParams -> wcuGraphMemsetNodeSetParams */

/*
 * CUDA API: cuGraphNodeFindInClone
 * Reference wrapper: wcuGraphNodeFindInClone
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphNodeFindInClone(mcDrvGraphNode *phNode, mcDrvGraphNode hOriginalNode, mcDrvGraph hClonedGraph);
 */
/* cuGraphNodeFindInClone -> wcuGraphNodeFindInClone */

/*
 * CUDA API: cuGraphNodeGetDependencies
 * Reference wrapper: wcuGraphNodeGetDependencies
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphNodeGetDependencies(mcDrvGraphNode hNode, mcDrvGraphNode *dependencies, size_t *numDependencies);
 */
/* cuGraphNodeGetDependencies -> wcuGraphNodeGetDependencies */

/*
 * CUDA API: cuGraphNodeGetDependencies_v2
 * Reference wrapper: wcuGraphNodeGetDependencies_v2
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphNodeGetDependencies_v2(mcDrvGraphNode hNode, mcDrvGraphNode *dependencies, mcGraphEdgeData *edgeData, size_t *numDependencies);
 */
/* cuGraphNodeGetDependencies_v2 -> wcuGraphNodeGetDependencies_v2 */

/*
 * CUDA API: cuGraphNodeGetDependentNodes
 * Reference wrapper: wcuGraphNodeGetDependentNodes
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphNodeGetDependentNodes(mcDrvGraphNode hNode, mcDrvGraphNode *dependentNodes, size_t *numDependentNodes);
 */
/* cuGraphNodeGetDependentNodes -> wcuGraphNodeGetDependentNodes */

/*
 * CUDA API: cuGraphNodeGetDependentNodes_v2
 * Reference wrapper: wcuGraphNodeGetDependentNodes_v2
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphNodeGetDependentNodes_v2(mcDrvGraphNode hNode, mcDrvGraphNode *dependentNodes, mcGraphEdgeData *edgeData, size_t *numDependentNodes);
 */
/* cuGraphNodeGetDependentNodes_v2 -> wcuGraphNodeGetDependentNodes_v2 */

/*
 * CUDA API: cuGraphNodeGetEnabled
 * Reference wrapper: wcuGraphNodeGetEnabled
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphNodeGetEnabled(mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, unsigned int *isEnabled);
 */
/* cuGraphNodeGetEnabled -> wcuGraphNodeGetEnabled */

/*
 * CUDA API: cuGraphNodeGetType
 * Reference wrapper: wcuGraphNodeGetType
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphNodeGetType(mcDrvGraphNode hNode, mcDrvGraphNodeType *type);
 */
/* cuGraphNodeGetType -> wcuGraphNodeGetType */

/*
 * CUDA API: cuGraphNodeSetEnabled
 * Reference wrapper: wcuGraphNodeSetEnabled
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphNodeSetEnabled(mcDrvGraphExec hGraphExec, mcDrvGraphNode hNode, unsigned int isEnabled);
 */
/* cuGraphNodeSetEnabled -> wcuGraphNodeSetEnabled */

/*
 * CUDA API: cuGraphReleaseUserObject
 * Reference wrapper: wcuGraphReleaseUserObject
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphReleaseUserObject(mcDrvGraph graph, mcDrvUserObject object, unsigned int count);
 */
/* cuGraphReleaseUserObject -> wcuGraphReleaseUserObject */

/*
 * CUDA API: cuGraphRemoveDependencies
 * Reference wrapper: wcuGraphRemoveDependencies
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphRemoveDependencies(mcDrvGraph hGraph, const mcDrvGraphNode *from, const mcDrvGraphNode *to, size_t numDependencies);
 */
/* cuGraphRemoveDependencies -> wcuGraphRemoveDependencies */

/*
 * CUDA API: cuGraphRemoveDependencies_v2
 * Reference wrapper: wcuGraphRemoveDependencies_v2
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphRemoveDependencies_v2(mcDrvGraph hGraph, const mcDrvGraphNode *from, const mcDrvGraphNode *to, mcGraphEdgeData *edgeData, size_t numDependencies);
 */
/* cuGraphRemoveDependencies_v2 -> wcuGraphRemoveDependencies_v2 */

/*
 * CUDA API: cuGraphRetainUserObject
 * Reference wrapper: wcuGraphRetainUserObject
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphRetainUserObject(mcDrvGraph graph, mcDrvUserObject object, unsigned int count, unsigned int flags);
 */
/* cuGraphRetainUserObject -> wcuGraphRetainUserObject */

/*
 * CUDA API: cuGraphUpload
 * Reference wrapper: wcuGraphUpload
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGraphUpload(mcDrvGraphExec hGraphExec, mcDrvStream_t hStream);
 */
/* cuGraphUpload -> wcuGraphUpload */

/*
 * CUDA API: cuGraphicsEGLRegisterImage
 * Reference wrapper: wcuGraphicsEGLRegisterImage
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGraphicsEGLRegisterImage(mcDrvGraphicsResource *pMacaResource, EGLImageKHR image, unsigned int flags);
 */
/* cuGraphicsEGLRegisterImage -> wcuGraphicsEGLRegisterImage */

/*
 * CUDA API: cuGraphicsGLRegisterBuffer
 * Reference wrapper: wcuGraphicsGLRegisterBuffer
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: #ifndef __CUDA_GL_WRAPPER_H__ #define __CUDA_GL_WRAPPER_H__ #include <GL/gl.h> #include "mc_gl_types.h" #include "mc_runtime.h" #if defined(WCUDA_API_PER_THREAD_DEFAULT_STREAM) #define __WCUDA_API_PTDS(api) api##_ptds #define __WCUDA_API_PTSZ(api) api##_ptsz #else #define __WCUDA_API_PTDS(api) api #define __WCUDA_API_PTSZ(api) api #endif #define wcuGLCtxCreate wcuGLCtxCreate_v2 #define wcuGLMapBufferObject __WCUDA_API_PTDS(wcuGLMapBufferObject_v2) #define wcuGLMapBufferObjectAsync __WCUDA_API_PTSZ(wcuGLMapBufferObjectAsync_v2) #define wcuGLGetDevices wcuGLGetDevices_v2 mcDrvError_t wcuGraphicsGLRegisterBuffer(mcDrvGraphicsResource *pMacaResource, GLuint buffer, unsigned int Flags);
 */
/* cuGraphicsGLRegisterBuffer -> wcuGraphicsGLRegisterBuffer */

/*
 * CUDA API: cuGraphicsGLRegisterImage
 * Reference wrapper: wcuGraphicsGLRegisterImage
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGraphicsGLRegisterImage(mcDrvGraphicsResource *pMacaResource, GLuint image, GLenum target, unsigned int Flags);
 */
/* cuGraphicsGLRegisterImage -> wcuGraphicsGLRegisterImage */

/*
 * CUDA API: cuGraphicsMapResources
 * Reference wrapper: wcuGraphicsMapResources
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphicsMapResources not supported) mcDrvError_t CUBRIDGE_API wcuGraphicsMapResources(unsigned int count, mcDrvGraphicsResource *resources, mcDrvStream_t hStream);
 */
/* cuGraphicsMapResources -> wcuGraphicsMapResources */

/*
 * CUDA API: cuGraphicsResourceGetMappedEglFrame
 * Reference wrapper: wcuGraphicsResourceGetMappedEglFrame
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t wcuGraphicsResourceGetMappedEglFrame(mcDrveglFrame *eglFrame, mcDrvGraphicsResource resource, unsigned int index, unsigned int mipLevel);
 */
/* cuGraphicsResourceGetMappedEglFrame -> wcuGraphicsResourceGetMappedEglFrame */

/*
 * CUDA API: cuGraphicsResourceGetMappedMipmappedArray
 * Reference wrapper: wcuGraphicsResourceGetMappedMipmappedArray
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphicsResourceGetMappedMipmappedArray not supported) mcDrvError_t CUBRIDGE_API wcuGraphicsResourceGetMappedMipmappedArray(mcDrvMipmappedArray *pMipmappedArray, mcDrvGraphicsResource resource);
 */
/* cuGraphicsResourceGetMappedMipmappedArray -> wcuGraphicsResourceGetMappedMipmappedArray */

/*
 * CUDA API: cuGraphicsResourceGetMappedPointer
 * Reference wrapper: wcuGraphicsResourceGetMappedPointer
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphicsResourceGetMappedPointer) mcDrvError_t CUBRIDGE_API wcuGraphicsResourceGetMappedPointer(mcDrvDeviceptr_t *pDevPtr, size_t *pSize, mcDrvGraphicsResource resource);
 */
/* cuGraphicsResourceGetMappedPointer -> wcuGraphicsResourceGetMappedPointer */

/*
 * CUDA API: cuGraphicsResourceSetMapFlags
 * Reference wrapper: wcuGraphicsResourceSetMapFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphicsResourceSetMapFlags not supported) mcDrvError_t CUBRIDGE_API wcuGraphicsResourceSetMapFlags(mcDrvGraphicsResource resource, unsigned int flags);
 */
/* cuGraphicsResourceSetMapFlags -> wcuGraphicsResourceSetMapFlags */

/*
 * CUDA API: cuGraphicsSubResourceGetMappedArray
 * Reference wrapper: wcuGraphicsSubResourceGetMappedArray
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphicsSubResourceGetMappedArray not supported) mcDrvError_t CUBRIDGE_API wcuGraphicsSubResourceGetMappedArray(MCarray *pArray, mcDrvGraphicsResource resource, unsigned int arrayIndex, unsigned int mipLevel);
 */
/* cuGraphicsSubResourceGetMappedArray -> wcuGraphicsSubResourceGetMappedArray */

/*
 * CUDA API: cuGraphicsUnmapResources
 * Reference wrapper: wcuGraphicsUnmapResources
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphicsUnmapResources not supported) mcDrvError_t CUBRIDGE_API wcuGraphicsUnmapResources(unsigned int count, mcDrvGraphicsResource *resources, mcDrvStream_t hStream);
 */
/* cuGraphicsUnmapResources -> wcuGraphicsUnmapResources */

/*
 * CUDA API: cuGraphicsUnregisterResource
 * Reference wrapper: wcuGraphicsUnregisterResource
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuGraphicsUnregisterResource not supported) mcDrvError_t CUBRIDGE_API wcuGraphicsUnregisterResource(mcDrvGraphicsResource resource);
 */
/* cuGraphicsUnregisterResource -> wcuGraphicsUnregisterResource */

/*
 * CUDA API: cuGraphicsVDPAURegisterOutputSurface
 * Reference wrapper: wcuGraphicsVDPAURegisterOutputSurface
 * Reference files: video/cuda_vdpau_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } mcDrvError_t wcuGraphicsVDPAURegisterOutputSurface(mcDrvGraphicsResource *pMcResource, VdpOutputSurface vdpSurface, unsigned int flags) { return MC_ERROR_NOT_SUPPORTED;
 */
/* cuGraphicsVDPAURegisterOutputSurface -> wcuGraphicsVDPAURegisterOutputSurface */

/*
 * CUDA API: cuGraphicsVDPAURegisterVideoSurface
 * Reference wrapper: wcuGraphicsVDPAURegisterVideoSurface
 * Reference files: video/cuda_vdpau_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } mcDrvError_t wcuGraphicsVDPAURegisterVideoSurface(mcDrvGraphicsResource *pMcResource, VdpVideoSurface vdpSurface, unsigned int flags) { return MC_ERROR_NOT_SUPPORTED;
 */
/* cuGraphicsVDPAURegisterVideoSurface -> wcuGraphicsVDPAURegisterVideoSurface */

/*
 * CUDA API: cuGreenCtxCreate
 * Reference wrapper: wcuGreenCtxCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGreenCtxCreate(MCcontext *phCtx, mcDevResourceDesc_t desc, MCdevice dev, unsigned int flags);
 */
/* cuGreenCtxCreate -> wcuGreenCtxCreate */

/*
 * CUDA API: cuGreenCtxDestroy
 * Reference wrapper: wcuGreenCtxDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGreenCtxDestroy(MCcontext hCtx);
 */
/* cuGreenCtxDestroy -> wcuGreenCtxDestroy */

/*
 * CUDA API: cuGreenCtxGetDevResource
 * Reference wrapper: wcuGreenCtxGetDevResource
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGreenCtxGetDevResource(MCcontext hCtx, mcDrvDevResource *resource, mcDrvDevResourceType type);
 */
/* cuGreenCtxGetDevResource -> wcuGreenCtxGetDevResource */

/*
 * CUDA API: cuGreenCtxGetId
 * Reference wrapper: wcuGreenCtxGetId
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGreenCtxGetId(MCcontext greenCtx, unsigned long long *greenCtxId);
 */
/* cuGreenCtxGetId -> wcuGreenCtxGetId */

/*
 * CUDA API: cuGreenCtxRecordEvent
 * Reference wrapper: wcuGreenCtxRecordEvent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGreenCtxRecordEvent(MCcontext hCtx, MCevent hEvent);
 */
/* cuGreenCtxRecordEvent -> wcuGreenCtxRecordEvent */

/*
 * CUDA API: cuGreenCtxStreamCreate
 * Reference wrapper: wcuGreenCtxStreamCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGreenCtxStreamCreate(MCstream *phStream, MCcontext greenCtx, unsigned int flags, int priority);
 */
/* cuGreenCtxStreamCreate -> wcuGreenCtxStreamCreate */

/*
 * CUDA API: cuGreenCtxWaitEvent
 * Reference wrapper: wcuGreenCtxWaitEvent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuGreenCtxWaitEvent(MCcontext hCtx, MCevent hEvent);
 */
/* cuGreenCtxWaitEvent -> wcuGreenCtxWaitEvent */

/*
 * CUDA API: cuImportExternalMemory
 * Reference wrapper: wcuImportExternalMemory
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuImportExternalMemory(mcDrvExternalMemory *extMem_out, const mcDrvExternalMemoryHandleDesc *memHandleDesc);
 */
/* cuImportExternalMemory -> wcuImportExternalMemory */

/*
 * CUDA API: cuImportExternalSemaphore
 * Reference wrapper: wcuImportExternalSemaphore
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuImportExternalSemaphore(mcDrvExternalSemaphore *extSem_out, const mcDrvExternalSemaphorHandleDesc *semHandleDesc);
 */
/* cuImportExternalSemaphore -> wcuImportExternalSemaphore */

/*
 * CUDA API: cuIpcCloseMemHandle
 * Reference wrapper: wcuIpcCloseMemHandle
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuIpcCloseMemHandle(mcDrvDeviceptr_t dptr);
 */
/* cuIpcCloseMemHandle -> wcuIpcCloseMemHandle */

/*
 * CUDA API: cuIpcGetEventHandle
 * Reference wrapper: wcuIpcGetEventHandle
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuIpcGetEventHandle(mcDrvIpcEventHandle_t *pHandle, mcDrvEvent_t event);
 */
/* cuIpcGetEventHandle -> wcuIpcGetEventHandle */

/*
 * CUDA API: cuIpcGetMemHandle
 * Reference wrapper: wcuIpcGetMemHandle
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuIpcGetMemHandle(mcDrvIpcMemHandle_t *pHandle, mcDrvDeviceptr_t dptr);
 */
/* cuIpcGetMemHandle -> wcuIpcGetMemHandle */

/*
 * CUDA API: cuIpcOpenEventHandle
 * Reference wrapper: wcuIpcOpenEventHandle
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuIpcOpenEventHandle(mcDrvEvent_t *phEvent, mcDrvIpcEventHandle_t handle);
 */
/* cuIpcOpenEventHandle -> wcuIpcOpenEventHandle */

/*
 * CUDA API: cuIpcOpenMemHandle
 * Reference wrapper: wcuIpcOpenMemHandle
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuIpcOpenMemHandle(mcDrvDeviceptr_t *pdptr, mcDrvIpcMemHandle_t handle, unsigned int Flags);
 */
/* cuIpcOpenMemHandle -> wcuIpcOpenMemHandle */

/*
 * CUDA API: cuLaunch
 * Reference wrapper: wcuLaunch
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLaunch(mcDrvFunction_t f);
 */
/* cuLaunch -> wcuLaunch */

/*
 * CUDA API: cuLaunchCooperativeKernel
 * Reference wrapper: wcuLaunchCooperativeKernel
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLaunchCooperativeKernel(mcDrvFunction_t f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, mcDrvStream_t stream, void **kernelParams);
 */
/* cuLaunchCooperativeKernel -> wcuLaunchCooperativeKernel */

/*
 * CUDA API: cuLaunchCooperativeKernelMultiDevice
 * Reference wrapper: wcuLaunchCooperativeKernelMultiDevice
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLaunchCooperativeKernelMultiDevice(mcDrvLaunchParams *launchParamsList, unsigned int numDevices, unsigned int flags);
 */
/* cuLaunchCooperativeKernelMultiDevice -> wcuLaunchCooperativeKernelMultiDevice */

/*
 * CUDA API: cuLaunchGrid
 * Reference wrapper: wcuLaunchGrid
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLaunchGrid(mcDrvFunction_t f, int grid_width, int grid_height);
 */
/* cuLaunchGrid -> wcuLaunchGrid */

/*
 * CUDA API: cuLaunchGridAsync
 * Reference wrapper: wcuLaunchGridAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLaunchGridAsync(mcDrvFunction_t f, int grid_width, int grid_height, mcDrvStream_t hStream);
 */
/* cuLaunchGridAsync -> wcuLaunchGridAsync */

/*
 * CUDA API: cuLaunchHostFunc
 * Reference wrapper: wcuLaunchHostFunc
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLaunchHostFunc(mcDrvStream_t hStream, mcHostFn_t fn, void *userData);
 */
/* cuLaunchHostFunc -> wcuLaunchHostFunc */

/*
 * CUDA API: cuLaunchKernel
 * Reference wrapper: wcuLaunchKernel
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLaunchKernel(mcDrvFunction_t f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, mcDrvStream_t stream, void **kernelParams, void **extra);
 */
/* cuLaunchKernel -> wcuLaunchKernel */

/*
 * CUDA API: cuLaunchKernelEx
 * Reference wrapper: wcuLaunchKernelEx
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLaunchKernelEx(const mcDrvlaunchConfig *config, mcDrvFunction_t f, void **kernelParams, void **extra);
 */
/* cuLaunchKernelEx -> wcuLaunchKernelEx */

/*
 * CUDA API: cuLibraryGetKernel
 * Reference wrapper: wcuLibraryGetKernel
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLibraryGetKernel(mcDrvKernel_t *pKernel, mcDrvLibrary_t library, const char *name);
 */
/* cuLibraryGetKernel -> wcuLibraryGetKernel */

/*
 * CUDA API: cuLibraryLoadData
 * Reference wrapper: wcuLibraryLoadData
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLibraryLoadData(mcDrvLibrary_t *library, const void *image, mcDrvjit_option_t *jitOptions, void **jitOptionsValues, unsigned int numJitOptions, mcDrvlibrary_option_t *libraryOptions, void **libraryOptionValues, unsigned int numLibraryOptions);
 */
/* cuLibraryLoadData -> wcuLibraryLoadData */

/*
 * CUDA API: cuLibraryUnload
 * Reference wrapper: wcuLibraryUnload
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLibraryUnload(mcDrvLibrary_t library);
 */
/* cuLibraryUnload -> wcuLibraryUnload */

/*
 * CUDA API: cuLinkAddData
 * Reference wrapper: wcuLinkAddData
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLinkAddData(mcDrvlinkState_t state, mcDrvJitInputType_t type, void *data, size_t size, const char *name, unsigned int numOptions, mcDrvjit_option_t *options, void **optionValues);
 */
/* cuLinkAddData -> wcuLinkAddData */

/*
 * CUDA API: cuLinkAddFile
 * Reference wrapper: wcuLinkAddFile
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLinkAddFile(mcDrvlinkState_t state, mcDrvJitInputType_t type, const char *path, unsigned int numOptions, mcDrvjit_option_t *options, void **optionValues);
 */
/* cuLinkAddFile -> wcuLinkAddFile */

/*
 * CUDA API: cuLinkComplete
 * Reference wrapper: wcuLinkComplete
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLinkComplete(mcDrvlinkState_t state, void **binOut, size_t *sizeOut);
 */
/* cuLinkComplete -> wcuLinkComplete */

/*
 * CUDA API: cuLinkCreate
 * Reference wrapper: wcuLinkCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLinkCreate(unsigned int numOptions, mcDrvjit_option_t *options, void **optionValues, mcDrvlinkState_t *stateOut);
 */
/* cuLinkCreate -> wcuLinkCreate */

/*
 * CUDA API: cuLinkDestroy
 * Reference wrapper: wcuLinkDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuLinkDestroy(mcDrvlinkState_t state);
 */
/* cuLinkDestroy -> wcuLinkDestroy */

/*
 * CUDA API: cuMemAddressFree
 * Reference wrapper: wcuMemAddressFree
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemAddressFree(mcDrvDeviceptr_t ptr, size_t size);
 */
/* cuMemAddressFree -> wcuMemAddressFree */

/*
 * CUDA API: cuMemAddressReserve
 * Reference wrapper: wcuMemAddressReserve
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemAddressReserve(mcDrvDeviceptr_t *ptr, size_t size, size_t alignment, mcDrvDeviceptr_t addr, unsigned long long flags);
 */
/* cuMemAddressReserve -> wcuMemAddressReserve */

/*
 * CUDA API: cuMemAdvise
 * Reference wrapper: wcuMemAdvise
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemAdvise(mcDrvDeviceptr_t devPtr, size_t count, mcDrvMemoryAdvise advice, mcDrvDevice_t device);
 */
/* cuMemAdvise -> wcuMemAdvise */

/*
 * CUDA API: cuMemAdvise_v2
 * Reference wrapper: wcuMemAdvise_v2
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemAdvise_v2(mcDrvDeviceptr_t devPtr, size_t count, mcDrvMemoryAdvise advice, mcDrvMemLocation_t location);
 */
/* cuMemAdvise_v2 -> wcuMemAdvise_v2 */

/*
 * CUDA API: cuMemAlloc
 * Reference wrapper: wcuMemAlloc
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemAlloc(mcDrvDeviceptr_t *ptr, size_t sizeBytes);
 */
/* cuMemAlloc -> wcuMemAlloc */

/*
 * CUDA API: cuMemAllocAsync
 * Reference wrapper: wcuMemAllocAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemAllocAsync(mcDrvDeviceptr_t *dptr, size_t bytesize, mcDrvStream_t hStream);
 */
/* cuMemAllocAsync -> wcuMemAllocAsync */

/*
 * CUDA API: cuMemAllocFromPoolAsync
 * Reference wrapper: wcuMemAllocFromPoolAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemAllocFromPoolAsync(mcDrvDeviceptr_t *dptr, size_t bytesize, mcDrvMemPool_t pool, mcDrvStream_t hStream);
 */
/* cuMemAllocFromPoolAsync -> wcuMemAllocFromPoolAsync */

/*
 * CUDA API: cuMemAllocHost
 * Reference wrapper: wcuMemAllocHost
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemAllocHost(void **pHost, size_t sizeBytes);
 */
/* cuMemAllocHost -> wcuMemAllocHost */

/*
 * CUDA API: cuMemAllocManaged
 * Reference wrapper: wcuMemAllocManaged
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemAllocManaged(mcDrvDeviceptr_t *dptr, size_t sizeBytes, unsigned int flags);
 */
/* cuMemAllocManaged -> wcuMemAllocManaged */

/*
 * CUDA API: cuMemAllocPitch
 * Reference wrapper: wcuMemAllocPitch
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemAllocPitch(mcDrvDeviceptr_t *dptr, size_t *pPitch, size_t widthInBytes, size_t height, unsigned int elementSizeBytes);
 */
/* cuMemAllocPitch -> wcuMemAllocPitch */

/*
 * CUDA API: cuMemCreate
 * Reference wrapper: wcuMemCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemCreate(mcDrvMemGenericAllocationHandle_t *handle, size_t size, const mcDrvMemAllocationProp_t *prop, unsigned long long flags);
 */
/* cuMemCreate -> wcuMemCreate */

/*
 * CUDA API: cuMemExportToShareableHandle
 * Reference wrapper: wcuMemExportToShareableHandle
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemExportToShareableHandle(void *shareableHandle, mcDrvMemGenericAllocationHandle_t handle, mcDrvMemAllocationHandleType_t handleType, unsigned long long flags);
 */
/* cuMemExportToShareableHandle -> wcuMemExportToShareableHandle */

/*
 * CUDA API: cuMemFree
 * Reference wrapper: wcuMemFree
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemFree(mcDrvDeviceptr_t ptr);
 */
/* cuMemFree -> wcuMemFree */

/*
 * CUDA API: cuMemFreeAsync
 * Reference wrapper: wcuMemFreeAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemFreeAsync(mcDrvDeviceptr_t dptr, mcDrvStream_t hStream);
 */
/* cuMemFreeAsync -> wcuMemFreeAsync */

/*
 * CUDA API: cuMemFreeHost
 * Reference wrapper: wcuMemFreeHost
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemFreeHost(void *ptr);
 */
/* cuMemFreeHost -> wcuMemFreeHost */

/*
 * CUDA API: cuMemGetAccess
 * Reference wrapper: wcuMemGetAccess
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemGetAccess(unsigned long long *flags, const mcDrvMemLocation_t *location, mcDrvDeviceptr_t ptr);
 */
/* cuMemGetAccess -> wcuMemGetAccess */

/*
 * CUDA API: cuMemGetAddressRange
 * Reference wrapper: wcuMemGetAddressRange
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemGetAddressRange(mcDrvDeviceptr_t *pbase, size_t *psize, mcDrvDeviceptr_t dptr);
 */
/* cuMemGetAddressRange -> wcuMemGetAddressRange */

/*
 * CUDA API: cuMemGetAllocationGranularity
 * Reference wrapper: wcuMemGetAllocationGranularity
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemGetAllocationGranularity(size_t *granularity, const mcDrvMemAllocationProp_t *prop, mcDrvMemAllocationGranularity_flags_t option);
 */
/* cuMemGetAllocationGranularity -> wcuMemGetAllocationGranularity */

/*
 * CUDA API: cuMemGetAllocationPropertiesFromHandle
 * Reference wrapper: wcuMemGetAllocationPropertiesFromHandle
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemGetAllocationPropertiesFromHandle(mcDrvMemAllocationProp_t *prop, mcDrvMemGenericAllocationHandle_t handle);
 */
/* cuMemGetAllocationPropertiesFromHandle -> wcuMemGetAllocationPropertiesFromHandle */

/*
 * CUDA API: cuMemGetHandleForAddressRange
 * Reference wrapper: wcuMemGetHandleForAddressRange
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemGetHandleForAddressRange(void *handle, mcDrvDeviceptr_t dptr, size_t size, mcDrvmemRangeHandleType_t handleType, unsigned long long flags);
 */
/* cuMemGetHandleForAddressRange -> wcuMemGetHandleForAddressRange */

/*
 * CUDA API: cuMemGetInfo
 * Reference wrapper: wcuMemGetInfo
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemGetInfo(size_t *free, size_t *total);
 */
/* cuMemGetInfo -> wcuMemGetInfo */

/*
 * CUDA API: cuMemHostAlloc
 * Reference wrapper: wcuMemHostAlloc
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemHostAlloc(void **pHost, size_t size, unsigned int flags);
 */
/* cuMemHostAlloc -> wcuMemHostAlloc */

/*
 * CUDA API: cuMemHostGetDevicePointer
 * Reference wrapper: wcuMemHostGetDevicePointer
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemHostGetDevicePointer(mcDrvDeviceptr_t *pDevice, void *pHost, unsigned int flags);
 */
/* cuMemHostGetDevicePointer -> wcuMemHostGetDevicePointer */

/*
 * CUDA API: cuMemHostGetFlags
 * Reference wrapper: wcuMemHostGetFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemHostGetFlags(unsigned int *flagsPtr, void *hostPtr);
 */
/* cuMemHostGetFlags -> wcuMemHostGetFlags */

/*
 * CUDA API: cuMemHostRegister
 * Reference wrapper: wcuMemHostRegister
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemHostRegister(void *ptr, size_t size, unsigned int flags);
 */
/* cuMemHostRegister -> wcuMemHostRegister */

/*
 * CUDA API: cuMemHostUnregister
 * Reference wrapper: wcuMemHostUnregister
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemHostUnregister(void *ptr);
 */
/* cuMemHostUnregister -> wcuMemHostUnregister */

/*
 * CUDA API: cuMemImportFromShareableHandle
 * Reference wrapper: wcuMemImportFromShareableHandle
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemImportFromShareableHandle(mcDrvMemGenericAllocationHandle_t *handle, void *osHandle, mcDrvMemAllocationHandleType_t shHandleType);
 */
/* cuMemImportFromShareableHandle -> wcuMemImportFromShareableHandle */

/*
 * CUDA API: cuMemMap
 * Reference wrapper: wcuMemMap
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemMap(mcDrvDeviceptr_t ptr, size_t size, size_t offset, mcDrvMemGenericAllocationHandle_t handle, unsigned long long flags);
 */
/* cuMemMap -> wcuMemMap */

/*
 * CUDA API: cuMemMapArrayAsync
 * Reference wrapper: wcuMemMapArrayAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemMapArrayAsync(mcDrvArrayMapInfo *mapInfoList, unsigned int count, mcDrvStream_t hStream);
 */
/* cuMemMapArrayAsync -> wcuMemMapArrayAsync */

/*
 * CUDA API: cuMemPoolCreate
 * Reference wrapper: wcuMemPoolCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPoolCreate(mcDrvMemPool_t *pool, const mcDrvMemPoolProps_t *poolProps);
 */
/* cuMemPoolCreate -> wcuMemPoolCreate */

/*
 * CUDA API: cuMemPoolDestroy
 * Reference wrapper: wcuMemPoolDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPoolDestroy(mcDrvMemPool_t pool);
 */
/* cuMemPoolDestroy -> wcuMemPoolDestroy */

/*
 * CUDA API: cuMemPoolExportPointer
 * Reference wrapper: wcuMemPoolExportPointer
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPoolExportPointer(mcDrvMemPoolPtrExportData_t *shareData_out, mcDrvDeviceptr_t ptr);
 */
/* cuMemPoolExportPointer -> wcuMemPoolExportPointer */

/*
 * CUDA API: cuMemPoolExportToShareableHandle
 * Reference wrapper: wcuMemPoolExportToShareableHandle
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPoolExportToShareableHandle(void *handle_out, mcDrvMemPool_t pool, mcDrvMemAllocationHandleType_t handleType, unsigned long long flags);
 */
/* cuMemPoolExportToShareableHandle -> wcuMemPoolExportToShareableHandle */

/*
 * CUDA API: cuMemPoolGetAccess
 * Reference wrapper: wcuMemPoolGetAccess
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPoolGetAccess(mcDrvMemAccess_flags_t *flags, mcDrvMemPool_t memPool, mcDrvMemLocation_t *location);
 */
/* cuMemPoolGetAccess -> wcuMemPoolGetAccess */

/*
 * CUDA API: cuMemPoolGetAttribute
 * Reference wrapper: wcuMemPoolGetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPoolGetAttribute(mcDrvMemPool_t pool, mcDrvMemPoolAttr_t attr, void *value);
 */
/* cuMemPoolGetAttribute -> wcuMemPoolGetAttribute */

/*
 * CUDA API: cuMemPoolImportFromShareableHandle
 * Reference wrapper: wcuMemPoolImportFromShareableHandle
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPoolImportFromShareableHandle(mcDrvMemPool_t *pool_out, void *handle, mcDrvMemAllocationHandleType_t handleType, unsigned long long flags);
 */
/* cuMemPoolImportFromShareableHandle -> wcuMemPoolImportFromShareableHandle */

/*
 * CUDA API: cuMemPoolImportPointer
 * Reference wrapper: wcuMemPoolImportPointer
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPoolImportPointer(mcDrvDeviceptr_t *ptr_out, mcDrvMemPool_t pool, mcDrvMemPoolPtrExportData_t *shareData);
 */
/* cuMemPoolImportPointer -> wcuMemPoolImportPointer */

/*
 * CUDA API: cuMemPoolSetAccess
 * Reference wrapper: wcuMemPoolSetAccess
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPoolSetAccess(mcDrvMemPool_t pool, const mcDrvMemAccessDesc_t *map, size_t count);
 */
/* cuMemPoolSetAccess -> wcuMemPoolSetAccess */

/*
 * CUDA API: cuMemPoolSetAttribute
 * Reference wrapper: wcuMemPoolSetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPoolSetAttribute(mcDrvMemPool_t pool, mcDrvMemPoolAttr_t attr, void *value);
 */
/* cuMemPoolSetAttribute -> wcuMemPoolSetAttribute */

/*
 * CUDA API: cuMemPoolTrimTo
 * Reference wrapper: wcuMemPoolTrimTo
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPoolTrimTo(mcDrvMemPool_t pool, size_t minBytesToKeep);
 */
/* cuMemPoolTrimTo -> wcuMemPoolTrimTo */

/*
 * CUDA API: cuMemPrefetchAsync
 * Reference wrapper: wcuMemPrefetchAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemPrefetchAsync(mcDrvDeviceptr_t devPtr, size_t count, mcDrvDevice_t dstDevice, mcDrvStream_t hStream);
 */
/* cuMemPrefetchAsync -> wcuMemPrefetchAsync */

/*
 * CUDA API: cuMemRangeGetAttribute
 * Reference wrapper: wcuMemRangeGetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemRangeGetAttribute(void *data, size_t dataSize, mcDrvMemoryAttributeType_t attribute, mcDrvDeviceptr_t devPtr, size_t count);
 */
/* cuMemRangeGetAttribute -> wcuMemRangeGetAttribute */

/*
 * CUDA API: cuMemRangeGetAttributes
 * Reference wrapper: wcuMemRangeGetAttributes
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemRangeGetAttributes(void **data, size_t *dataSizes, mcDrvMemoryAttributeType_t *attributes, size_t numAttributes, mcDrvDeviceptr_t devPtr, size_t count);
 */
/* cuMemRangeGetAttributes -> wcuMemRangeGetAttributes */

/*
 * CUDA API: cuMemRelease
 * Reference wrapper: wcuMemRelease
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemRelease(mcDrvMemGenericAllocationHandle_t handle);
 */
/* cuMemRelease -> wcuMemRelease */

/*
 * CUDA API: cuMemRetainAllocationHandle
 * Reference wrapper: wcuMemRetainAllocationHandle
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemRetainAllocationHandle(mcDrvMemGenericAllocationHandle_t *handle, void *addr);
 */
/* cuMemRetainAllocationHandle -> wcuMemRetainAllocationHandle */

/*
 * CUDA API: cuMemSetAccess
 * Reference wrapper: wcuMemSetAccess
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemSetAccess(mcDrvDeviceptr_t ptr, size_t size, const mcDrvMemAccessDesc_t *desc, size_t count);
 */
/* cuMemSetAccess -> wcuMemSetAccess */

/*
 * CUDA API: cuMemUnmap
 * Reference wrapper: wcuMemUnmap
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemUnmap(mcDrvDeviceptr_t ptr, size_t size);
 */
/* cuMemUnmap -> wcuMemUnmap */

/*
 * CUDA API: cuMemcpy
 * Reference wrapper: wcuMemcpy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpy(mcDrvDeviceptr_t dst, mcDrvDeviceptr_t src, size_t sizeBytes);
 */
/* cuMemcpy -> wcuMemcpy */

/*
 * CUDA API: cuMemcpy2D
 * Reference wrapper: wcuMemcpy2D
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpy2D(const mcDrvMemcpy2D *pCopy);
 */
/* cuMemcpy2D -> wcuMemcpy2D */

/*
 * CUDA API: cuMemcpy2DAsync
 * Reference wrapper: wcuMemcpy2DAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpy2DAsync(const mcDrvMemcpy2D *pCopy, mcDrvStream_t hStream);
 */
/* cuMemcpy2DAsync -> wcuMemcpy2DAsync */

/*
 * CUDA API: cuMemcpy2DUnaligned
 * Reference wrapper: wcuMemcpy2DUnaligned
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpy2DUnaligned(const mcDrvMemcpy2D *pCopy);
 */
/* cuMemcpy2DUnaligned -> wcuMemcpy2DUnaligned */

/*
 * CUDA API: cuMemcpy3D
 * Reference wrapper: wcuMemcpy3D
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpy3D(const mcDrvMemcpy3D *pCopy);
 */
/* cuMemcpy3D -> wcuMemcpy3D */

/*
 * CUDA API: cuMemcpy3DAsync
 * Reference wrapper: wcuMemcpy3DAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpy3DAsync(const mcDrvMemcpy3D *pCopy, mcDrvStream_t hStream);
 */
/* cuMemcpy3DAsync -> wcuMemcpy3DAsync */

/*
 * CUDA API: cuMemcpy3DPeer
 * Reference wrapper: wcuMemcpy3DPeer
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpy3DPeer(const mcDrvMemcpy3DPeer *pCopy);
 */
/* cuMemcpy3DPeer -> wcuMemcpy3DPeer */

/*
 * CUDA API: cuMemcpy3DPeerAsync
 * Reference wrapper: wcuMemcpy3DPeerAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpy3DPeerAsync(const mcDrvMemcpy3DPeer *pCopy, mcDrvStream_t hStream);
 */
/* cuMemcpy3DPeerAsync -> wcuMemcpy3DPeerAsync */

/*
 * CUDA API: cuMemcpyAsync
 * Reference wrapper: wcuMemcpyAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyAsync(mcDrvDeviceptr_t dst, mcDrvDeviceptr_t src, size_t ByteCount, mcDrvStream_t hStream);
 */
/* cuMemcpyAsync -> wcuMemcpyAsync */

/*
 * CUDA API: cuMemcpyAtoA
 * Reference wrapper: wcuMemcpyAtoA
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyAtoA(MCarray dstArray, size_t dstOffset, MCarray srcArray, size_t srcOffset, size_t ByteCount);
 */
/* cuMemcpyAtoA -> wcuMemcpyAtoA */

/*
 * CUDA API: cuMemcpyAtoD
 * Reference wrapper: wcuMemcpyAtoD
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyAtoD(mcDrvDeviceptr_t dstDevice, MCarray srcArray, size_t srcOffset, size_t ByteCount);
 */
/* cuMemcpyAtoD -> wcuMemcpyAtoD */

/*
 * CUDA API: cuMemcpyAtoH
 * Reference wrapper: wcuMemcpyAtoH
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyAtoH(void *dstHost, MCarray srcArray, size_t srcOffset, size_t ByteCount);
 */
/* cuMemcpyAtoH -> wcuMemcpyAtoH */

/*
 * CUDA API: cuMemcpyAtoHAsync
 * Reference wrapper: wcuMemcpyAtoHAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyAtoHAsync(void *dstHost, MCarray srcArray, size_t srcOffset, size_t ByteCount, mcDrvStream_t hStream);
 */
/* cuMemcpyAtoHAsync -> wcuMemcpyAtoHAsync */

/*
 * CUDA API: cuMemcpyDtoA
 * Reference wrapper: wcuMemcpyDtoA
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyDtoA(MCarray dstArray, size_t dstOffset, mcDrvDeviceptr_t srcDevice, size_t ByteCount);
 */
/* cuMemcpyDtoA -> wcuMemcpyDtoA */

/*
 * CUDA API: cuMemcpyDtoD
 * Reference wrapper: wcuMemcpyDtoD
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyDtoD(mcDrvDeviceptr_t dstDevice, mcDrvDeviceptr_t srcDevice, size_t byteCount);
 */
/* cuMemcpyDtoD -> wcuMemcpyDtoD */

/*
 * CUDA API: cuMemcpyDtoDAsync
 * Reference wrapper: wcuMemcpyDtoDAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyDtoDAsync(mcDrvDeviceptr_t dstDevice, mcDrvDeviceptr_t srcDevice, size_t byteCount, mcDrvStream_t hStream);
 */
/* cuMemcpyDtoDAsync -> wcuMemcpyDtoDAsync */

/*
 * CUDA API: cuMemcpyDtoH
 * Reference wrapper: wcuMemcpyDtoH
 * Reference files: runtime/cuda_driver_wrapper.h, runtime/cuda_runtime_template_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static inline __host__ mcError_t wcuMemcpyDtoH(T *dstHost, mcDeviceptr_t srcDevice, size_t ByteCount) { return wcuMemcpyDtoH((void *)dstHost, srcDevice, ByteCount);
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyDtoH(void *dstHost, mcDrvDeviceptr_t srcDevice, size_t byteCount);
 */
/* cuMemcpyDtoH -> wcuMemcpyDtoH */

/*
 * CUDA API: cuMemcpyDtoHAsync
 * Reference wrapper: wcuMemcpyDtoHAsync
 * Reference files: runtime/cuda_driver_wrapper.h, runtime/cuda_runtime_template_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static inline __host__ mcError_t wcuMemcpyDtoHAsync(T *dstHost, mcDeviceptr_t srcDevice, size_t ByteCount, mcStream_t hStream) { return wcuMemcpyDtoHAsync((void *)dstHost, srcDevice, ByteCount, hStream);
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyDtoHAsync(void *dstHost, mcDrvDeviceptr_t srcDevice, size_t byteCount, mcDrvStream_t hStream);
 */
/* cuMemcpyDtoHAsync -> wcuMemcpyDtoHAsync */

/*
 * CUDA API: cuMemcpyHtoA
 * Reference wrapper: wcuMemcpyHtoA
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyHtoA(MCarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount);
 */
/* cuMemcpyHtoA -> wcuMemcpyHtoA */

/*
 * CUDA API: cuMemcpyHtoAAsync
 * Reference wrapper: wcuMemcpyHtoAAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyHtoAAsync(MCarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount, mcDrvStream_t hStream);
 */
/* cuMemcpyHtoAAsync -> wcuMemcpyHtoAAsync */

/*
 * CUDA API: cuMemcpyHtoD
 * Reference wrapper: wcuMemcpyHtoD
 * Reference files: runtime/cuda_driver_wrapper.h, runtime/cuda_runtime_template_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static inline __host__ mcError_t wcuMemcpyHtoD(mcDeviceptr_t dstDevice, const T *srcHost, size_t ByteCount) { return wcuMemcpyHtoD(dstDevice, (const void *)srcHost, ByteCount);
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyHtoD(mcDrvDeviceptr_t dstDevice, const void *srcHost, size_t byteCount);
 */
/* cuMemcpyHtoD -> wcuMemcpyHtoD */

/*
 * CUDA API: cuMemcpyHtoDAsync
 * Reference wrapper: wcuMemcpyHtoDAsync
 * Reference files: runtime/cuda_driver_wrapper.h, runtime/cuda_runtime_template_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static inline __host__ mcError_t wcuMemcpyHtoDAsync(mcDeviceptr_t dstDevice, const T *srcHost, size_t ByteCount, mcStream_t hStream) { return wcuMemcpyHtoDAsync(dstDevice, (const void *)srcHost, ByteCount, hStream);
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyHtoDAsync(mcDrvDeviceptr_t dstDevice, const void *srcHost, size_t byteCount, mcDrvStream_t hStream);
 */
/* cuMemcpyHtoDAsync -> wcuMemcpyHtoDAsync */

/*
 * CUDA API: cuMemcpyPeer
 * Reference wrapper: wcuMemcpyPeer
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyPeer(mcDrvDeviceptr_t dstDevice, mcDrvContext_t dstContext, mcDrvDeviceptr_t srcDevice, mcDrvContext_t srcContext, size_t byteCount);
 */
/* cuMemcpyPeer -> wcuMemcpyPeer */

/*
 * CUDA API: cuMemcpyPeerAsync
 * Reference wrapper: wcuMemcpyPeerAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemcpyPeerAsync(mcDrvDeviceptr_t dstDevice, mcDrvContext_t dstContext, mcDrvDeviceptr_t srcDevice, mcDrvContext_t srcContext, size_t byteCount, mcDrvStream_t hStream);
 */
/* cuMemcpyPeerAsync -> wcuMemcpyPeerAsync */

/*
 * CUDA API: cuMemsetD16
 * Reference wrapper: wcuMemsetD16
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD16(mcDrvDeviceptr_t dstDevice, unsigned short us, size_t N);
 */
/* cuMemsetD16 -> wcuMemsetD16 */

/*
 * CUDA API: cuMemsetD16Async
 * Reference wrapper: wcuMemsetD16Async
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD16Async(mcDrvDeviceptr_t dstDevice, unsigned short us, size_t N, mcDrvStream_t hStream);
 */
/* cuMemsetD16Async -> wcuMemsetD16Async */

/*
 * CUDA API: cuMemsetD2D16
 * Reference wrapper: wcuMemsetD2D16
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD2D16(mcDrvDeviceptr_t dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height);
 */
/* cuMemsetD2D16 -> wcuMemsetD2D16 */

/*
 * CUDA API: cuMemsetD2D16Async
 * Reference wrapper: wcuMemsetD2D16Async
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD2D16Async(mcDrvDeviceptr_t dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height, mcDrvStream_t hStream);
 */
/* cuMemsetD2D16Async -> wcuMemsetD2D16Async */

/*
 * CUDA API: cuMemsetD2D32
 * Reference wrapper: wcuMemsetD2D32
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD2D32(mcDrvDeviceptr_t dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height);
 */
/* cuMemsetD2D32 -> wcuMemsetD2D32 */

/*
 * CUDA API: cuMemsetD2D32Async
 * Reference wrapper: wcuMemsetD2D32Async
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD2D32Async(mcDrvDeviceptr_t dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height, mcDrvStream_t hStream);
 */
/* cuMemsetD2D32Async -> wcuMemsetD2D32Async */

/*
 * CUDA API: cuMemsetD2D8
 * Reference wrapper: wcuMemsetD2D8
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD2D8(mcDrvDeviceptr_t dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height);
 */
/* cuMemsetD2D8 -> wcuMemsetD2D8 */

/*
 * CUDA API: cuMemsetD2D8Async
 * Reference wrapper: wcuMemsetD2D8Async
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD2D8Async(mcDrvDeviceptr_t dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height, mcDrvStream_t hStream);
 */
/* cuMemsetD2D8Async -> wcuMemsetD2D8Async */

/*
 * CUDA API: cuMemsetD32
 * Reference wrapper: wcuMemsetD32
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD32(mcDrvDeviceptr_t dstDevice, unsigned int ui, size_t N);
 */
/* cuMemsetD32 -> wcuMemsetD32 */

/*
 * CUDA API: cuMemsetD32Async
 * Reference wrapper: wcuMemsetD32Async
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD32Async(mcDrvDeviceptr_t dstDevice, unsigned int ui, size_t N, mcDrvStream_t hStream);
 */
/* cuMemsetD32Async -> wcuMemsetD32Async */

/*
 * CUDA API: cuMemsetD8
 * Reference wrapper: wcuMemsetD8
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD8(mcDrvDeviceptr_t dstDevice, unsigned char uc, size_t N);
 */
/* cuMemsetD8 -> wcuMemsetD8 */

/*
 * CUDA API: cuMemsetD8Async
 * Reference wrapper: wcuMemsetD8Async
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMemsetD8Async(mcDrvDeviceptr_t dstDevice, unsigned char uc, size_t N, mcDrvStream_t hStream);
 */
/* cuMemsetD8Async -> wcuMemsetD8Async */

/*
 * CUDA API: cuMipmappedArrayCreate
 * Reference wrapper: wcuMipmappedArrayCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMipmappedArrayCreate(mcDrvMipmappedArray *pHandle, const mcDrvArray3DDescriptor *pMipmappedArrayDesc, unsigned int numMipmapLevels);
 */
/* cuMipmappedArrayCreate -> wcuMipmappedArrayCreate */

/*
 * CUDA API: cuMipmappedArrayDestroy
 * Reference wrapper: wcuMipmappedArrayDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMipmappedArrayDestroy(mcDrvMipmappedArray hMipmappedArray);
 */
/* cuMipmappedArrayDestroy -> wcuMipmappedArrayDestroy */

/*
 * CUDA API: cuMipmappedArrayGetLevel
 * Reference wrapper: wcuMipmappedArrayGetLevel
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMipmappedArrayGetLevel(MCarray *pLevelArray, mcDrvMipmappedArray hMipmappedArray, unsigned int level);
 */
/* cuMipmappedArrayGetLevel -> wcuMipmappedArrayGetLevel */

/*
 * CUDA API: cuMipmappedArrayGetMemoryRequirements
 * Reference wrapper: wcuMipmappedArrayGetMemoryRequirements
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMipmappedArrayGetMemoryRequirements(mcDrvArrayMemoryRequirements *memoryRequirements, mcDrvMipmappedArray mipmap, mcDrvDevice_t device);
 */
/* cuMipmappedArrayGetMemoryRequirements -> wcuMipmappedArrayGetMemoryRequirements */

/*
 * CUDA API: cuMipmappedArrayGetSparseProperties
 * Reference wrapper: wcuMipmappedArrayGetSparseProperties
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMipmappedArrayGetSparseProperties(mcDrvArraySparseProperties *sparseProperties, mcDrvMipmappedArray mipmap);
 */
/* cuMipmappedArrayGetSparseProperties -> wcuMipmappedArrayGetSparseProperties */

/*
 * CUDA API: cuModuleGetFunction
 * Reference wrapper: wcuModuleGetFunction
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuModuleGetFunction(mcDrvFunction_t *hfunc, mcDrvModule_t hmod, const char *name);
 */
/* cuModuleGetFunction -> wcuModuleGetFunction */

/*
 * CUDA API: cuModuleGetGlobal
 * Reference wrapper: wcuModuleGetGlobal
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuModuleGetGlobal(mcDrvDeviceptr_t *dptr, size_t *bytes, mcDrvModule_t hmod, const char *name);
 */
/* cuModuleGetGlobal -> wcuModuleGetGlobal */

/*
 * CUDA API: cuModuleGetLoadingMode
 * Reference wrapper: wcuModuleGetLoadingMode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuModuleGetLoadingMode(mcModuleLoadingMode *mode);
 */
/* cuModuleGetLoadingMode -> wcuModuleGetLoadingMode */

/*
 * CUDA API: cuModuleGetSurfRef
 * Reference wrapper: wcuModuleGetSurfRef
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuModuleGetSurfRef(mcDrvSurfref *pSurfRef, mcDrvModule_t hmod, const char *name);
 */
/* cuModuleGetSurfRef -> wcuModuleGetSurfRef */

/*
 * CUDA API: cuModuleGetTexRef
 * Reference wrapper: wcuModuleGetTexRef
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuModuleGetTexRef(mcDrvTexref *pTexRef, mcDrvModule_t hmod, const char *name);
 */
/* cuModuleGetTexRef -> wcuModuleGetTexRef */

/*
 * CUDA API: cuModuleLoad
 * Reference wrapper: wcuModuleLoad
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuModuleLoad(mcDrvModule_t *module, const char *fname);
 */
/* cuModuleLoad -> wcuModuleLoad */

/*
 * CUDA API: cuModuleLoadData
 * Reference wrapper: wcuModuleLoadData
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuModuleLoadData(mcDrvModule_t *module, const void *image);
 */
/* cuModuleLoadData -> wcuModuleLoadData */

/*
 * CUDA API: cuModuleLoadDataEx
 * Reference wrapper: wcuModuleLoadDataEx
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuModuleLoadDataEx(mcDrvModule_t *module, const void *image, unsigned int numOptions, mcDrvjit_option_t *options, void **optionValues);
 */
/* cuModuleLoadDataEx -> wcuModuleLoadDataEx */

/*
 * CUDA API: cuModuleLoadFatBinary
 * Reference wrapper: wcuModuleLoadFatBinary
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuModuleLoadFatBinary(mcDrvModule_t *module, const void *fatCubin);
 */
/* cuModuleLoadFatBinary -> wcuModuleLoadFatBinary */

/*
 * CUDA API: cuModuleUnload
 * Reference wrapper: wcuModuleUnload
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuModuleUnload(mcDrvModule_t hmod);
 */
/* cuModuleUnload -> wcuModuleUnload */

/*
 * CUDA API: cuMulticastAddDevice
 * Reference wrapper: wcuMulticastAddDevice
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMulticastAddDevice(mcDrvMemGenericAllocationHandle_t mcHandle, mcDrvDevice_t dev);
 */
/* cuMulticastAddDevice -> wcuMulticastAddDevice */

/*
 * CUDA API: cuMulticastBindAddr
 * Reference wrapper: wcuMulticastBindAddr
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMulticastBindAddr(mcDrvMemGenericAllocationHandle_t mcHandle, size_t mcOffset, mcDrvDeviceptr_t memptr, size_t size, unsigned long long flags);
 */
/* cuMulticastBindAddr -> wcuMulticastBindAddr */

/*
 * CUDA API: cuMulticastBindMem
 * Reference wrapper: wcuMulticastBindMem
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMulticastBindMem(mcDrvMemGenericAllocationHandle_t mcHandle, size_t mcOffset, mcDrvMemGenericAllocationHandle_t memHandle, size_t memOffset, size_t size, unsigned long long flags);
 */
/* cuMulticastBindMem -> wcuMulticastBindMem */

/*
 * CUDA API: cuMulticastCreate
 * Reference wrapper: wcuMulticastCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMulticastCreate(mcDrvMemGenericAllocationHandle_t *mcHandle, const mcDrvMulticastObjectProp *prop);
 */
/* cuMulticastCreate -> wcuMulticastCreate */

/*
 * CUDA API: cuMulticastGetGranularity
 * Reference wrapper: wcuMulticastGetGranularity
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMulticastGetGranularity(size_t *granularity, const mcDrvMulticastObjectProp *prop, mcDrvMulticastGranularity_flags option);
 */
/* cuMulticastGetGranularity -> wcuMulticastGetGranularity */

/*
 * CUDA API: cuMulticastUnbind
 * Reference wrapper: wcuMulticastUnbind
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuMulticastUnbind(mcDrvMemGenericAllocationHandle_t mcHandle, mcDrvDevice_t dev, size_t mcOffset, size_t size);
 */
/* cuMulticastUnbind -> wcuMulticastUnbind */

/*
 * CUDA API: cuOccupancyAvailableDynamicSMemPerBlock
 * Reference wrapper: wcuOccupancyAvailableDynamicSMemPerBlock
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuOccupancyAvailableDynamicSMemPerBlock(size_t *dynamicSmemSize, mcDrvFunction_t func, int numBlocks, int blockSize);
 */
/* cuOccupancyAvailableDynamicSMemPerBlock -> wcuOccupancyAvailableDynamicSMemPerBlock */

/*
 * CUDA API: cuOccupancyMaxActiveBlocksPerMultiprocessor
 * Reference wrapper: wcuOccupancyMaxActiveBlocksPerMultiprocessor
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuOccupancyMaxActiveBlocksPerMultiprocessor(int *numBlocks, mcDrvFunction_t func, int blockSize, size_t dynamicSMemSize);
 */
/* cuOccupancyMaxActiveBlocksPerMultiprocessor -> wcuOccupancyMaxActiveBlocksPerMultiprocessor */

/*
 * CUDA API: cuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags
 * Reference wrapper: wcuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags(int *numBlocks, mcDrvFunction_t func, int blockSize, size_t dynamicSMemSize, unsigned int flags);
 */
/* cuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags -> wcuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags */

/*
 * CUDA API: cuOccupancyMaxActiveClusters
 * Reference wrapper: wcuOccupancyMaxActiveClusters
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuOccupancyMaxActiveClusters(int *numClusters, mcDrvFunction_t func, const void *config);
 */
/* cuOccupancyMaxActiveClusters -> wcuOccupancyMaxActiveClusters */

/*
 * CUDA API: cuOccupancyMaxPotentialBlockSize
 * Reference wrapper: wcuOccupancyMaxPotentialBlockSize
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuOccupancyMaxPotentialBlockSize(int *minGridSize, int *blockSize, mcDrvFunction_t func, mcOccupancyB2DSize blockSizeToDynamicSMemSize, size_t dynamicSMemSize, int blockSizeLimit);
 */
/* cuOccupancyMaxPotentialBlockSize -> wcuOccupancyMaxPotentialBlockSize */

/*
 * CUDA API: cuOccupancyMaxPotentialBlockSizeWithFlags
 * Reference wrapper: wcuOccupancyMaxPotentialBlockSizeWithFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuOccupancyMaxPotentialBlockSizeWithFlags(int *minGridSize, int *blockSize, mcDrvFunction_t func, mcOccupancyB2DSize blockSizeToDynamicSMemSize, size_t dynamicSMemSize, int blockSizeLimit, unsigned int flags);
 */
/* cuOccupancyMaxPotentialBlockSizeWithFlags -> wcuOccupancyMaxPotentialBlockSizeWithFlags */

/*
 * CUDA API: cuOccupancyMaxPotentialClusterSize
 * Reference wrapper: wcuOccupancyMaxPotentialClusterSize
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuOccupancyMaxPotentialClusterSize(int *clusterSize, mcDrvFunction_t func, const void *config);
 */
/* cuOccupancyMaxPotentialClusterSize -> wcuOccupancyMaxPotentialClusterSize */

/*
 * CUDA API: cuParamSetSize
 * Reference wrapper: wcuParamSetSize
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuParamSetSize(mcDrvFunction_t hfunc, unsigned int numbytes);
 */
/* cuParamSetSize -> wcuParamSetSize */

/*
 * CUDA API: cuParamSetTexRef
 * Reference wrapper: wcuParamSetTexRef
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuParamSetTexRef(mcDrvFunction_t hfunc, int texunit, mcDrvTexref hTexRef);
 */
/* cuParamSetTexRef -> wcuParamSetTexRef */

/*
 * CUDA API: cuParamSetf
 * Reference wrapper: wcuParamSetf
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuParamSetf(mcDrvFunction_t hfunc, int offset, float value);
 */
/* cuParamSetf -> wcuParamSetf */

/*
 * CUDA API: cuParamSeti
 * Reference wrapper: wcuParamSeti
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuParamSeti(mcDrvFunction_t hfunc, int offset, unsigned int value);
 */
/* cuParamSeti -> wcuParamSeti */

/*
 * CUDA API: cuParamSetv
 * Reference wrapper: wcuParamSetv
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuParamSetv(mcDrvFunction_t hfunc, int offset, void *ptr, unsigned int numbytes);
 */
/* cuParamSetv -> wcuParamSetv */

/*
 * CUDA API: cuPointerGetAttribute
 * Reference wrapper: wcuPointerGetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuPointerGetAttribute(void *data, mcDrvPointerAttributeType attribute, mcDrvDeviceptr_t ptr);
 */
/* cuPointerGetAttribute -> wcuPointerGetAttribute */

/*
 * CUDA API: cuPointerGetAttributes
 * Reference wrapper: wcuPointerGetAttributes
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuPointerGetAttributes(unsigned int numAttributes, mcDrvPointerAttributeType *attributes, void **data, mcDrvDeviceptr_t ptr);
 */
/* cuPointerGetAttributes -> wcuPointerGetAttributes */

/*
 * CUDA API: cuPointerSetAttribute
 * Reference wrapper: wcuPointerSetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuPointerSetAttribute(const void *value, mcDrvPointerAttributeType attribute, mcDrvDeviceptr_t ptr);
 */
/* cuPointerSetAttribute -> wcuPointerSetAttribute */

/*
 * CUDA API: cuProfilerInitialize
 * Reference wrapper: wcuProfilerInitialize
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuProfilerInitialize(const char *configFile, const char *outputFile, enum mcOutputMode outputMode);
 */
/* cuProfilerInitialize -> wcuProfilerInitialize */

/*
 * CUDA API: cuProfilerStart
 * Reference wrapper: wcuProfilerStart
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuProfilerStart(void);
 */
/* cuProfilerStart -> wcuProfilerStart */

/*
 * CUDA API: cuProfilerStop
 * Reference wrapper: wcuProfilerStop
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuProfilerStop(void);
 */
/* cuProfilerStop -> wcuProfilerStop */

/*
 * CUDA API: cuSignalExternalSemaphoresAsync
 * Reference wrapper: wcuSignalExternalSemaphoresAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuSignalExternalSemaphoresAsync(const mcDrvExternalSemaphore *extSemArray, const mcDrvExternalSemaphoreSignalParams *paramsArray, unsigned int numExtSems, mcDrvStream_t stream);
 */
/* cuSignalExternalSemaphoresAsync -> wcuSignalExternalSemaphoresAsync */

/*
 * CUDA API: cuStreamAddCallback
 * Reference wrapper: wcuStreamAddCallback
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamAddCallback(mcDrvStream_t hStream, mcDrvStreamCallback_t callback, void *userData, unsigned int flags);
 */
/* cuStreamAddCallback -> wcuStreamAddCallback */

/*
 * CUDA API: cuStreamAttachMemAsync
 * Reference wrapper: wcuStreamAttachMemAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamAttachMemAsync(mcDrvStream_t hStream, mcDrvDeviceptr_t dptr, size_t length, unsigned int flags);
 */
/* cuStreamAttachMemAsync -> wcuStreamAttachMemAsync */

/*
 * CUDA API: cuStreamBatchMemOp
 * Reference wrapper: wcuStreamBatchMemOp
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamBatchMemOp(mcDrvStream_t stream, unsigned int count, mcDrvStreamBatchMemOpParams *paramArray, unsigned int flags);
 */
/* cuStreamBatchMemOp -> wcuStreamBatchMemOp */

/*
 * CUDA API: cuStreamBeginCapture
 * Reference wrapper: wcuStreamBeginCapture
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamBeginCapture(mcDrvStream_t hStream, mcStreamCaptureMode mode);
 */
/* cuStreamBeginCapture -> wcuStreamBeginCapture */

/*
 * CUDA API: cuStreamBeginCaptureToGraph
 * Reference wrapper: wcuStreamBeginCaptureToGraph
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamBeginCaptureToGraph(mcDrvStream_t hStream, mcDrvGraph hGraph, const mcDrvGraphNode *dependencies, const mcGraphEdgeData *dependencyData, size_t numDependencies, mcStreamCaptureMode mode);
 */
/* cuStreamBeginCaptureToGraph -> wcuStreamBeginCaptureToGraph */

/*
 * CUDA API: cuStreamCopyAttributes
 * Reference wrapper: wcuStreamCopyAttributes
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamCopyAttributes(mcDrvStream_t dst, mcDrvStream_t src);
 */
/* cuStreamCopyAttributes -> wcuStreamCopyAttributes */

/*
 * CUDA API: cuStreamCreate
 * Reference wrapper: wcuStreamCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamCreate(mcDrvStream_t *phStream, unsigned int Flags);
 */
/* cuStreamCreate -> wcuStreamCreate */

/*
 * CUDA API: cuStreamCreateWithPriority
 * Reference wrapper: wcuStreamCreateWithPriority
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamCreateWithPriority(mcDrvStream_t *phStream, unsigned int flags, int priority);
 */
/* cuStreamCreateWithPriority -> wcuStreamCreateWithPriority */

/*
 * CUDA API: cuStreamDestroy
 * Reference wrapper: wcuStreamDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamDestroy(mcDrvStream_t hStream);
 */
/* cuStreamDestroy -> wcuStreamDestroy */

/*
 * CUDA API: cuStreamEndCapture
 * Reference wrapper: wcuStreamEndCapture
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamEndCapture(mcDrvStream_t hStream, mcDrvGraph *phGraph);
 */
/* cuStreamEndCapture -> wcuStreamEndCapture */

/*
 * CUDA API: cuStreamGetAttribute
 * Reference wrapper: wcuStreamGetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamGetAttribute(mcDrvStream_t hStream, mcDrvStreamAttrID attr, mcDrvStreamAttrValue *value_out);
 */
/* cuStreamGetAttribute -> wcuStreamGetAttribute */

/*
 * CUDA API: cuStreamGetCaptureInfo
 * Reference wrapper: wcuStreamGetCaptureInfo
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamGetCaptureInfo(mcDrvStream_t hStream, mcStreamCaptureStatus *captureStatus_out, mcuint64_t *id_out);
 */
/* cuStreamGetCaptureInfo -> wcuStreamGetCaptureInfo */

/*
 * CUDA API: cuStreamGetCaptureInfo_v2
 * Reference wrapper: wcuStreamGetCaptureInfo_v2
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamGetCaptureInfo_v2(mcDrvStream_t hStream, mcStreamCaptureStatus *captureStatus_out, mcuint64_t *id_out, mcDrvGraph *graph_out, const mcDrvGraphNode **dependencies_out, size_t *numDependencies_out);
 */
/* cuStreamGetCaptureInfo_v2 -> wcuStreamGetCaptureInfo_v2 */

/*
 * CUDA API: cuStreamGetCtx
 * Reference wrapper: wcuStreamGetCtx
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamGetCtx(mcDrvStream_t hStream, mcDrvContext_t *pctx);
 */
/* cuStreamGetCtx -> wcuStreamGetCtx */

/*
 * CUDA API: cuStreamGetDevice
 * Reference wrapper: wcuStreamGetDevice
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamGetDevice(mcDrvStream_t hStream, mcDrvDevice_t *device);
 */
/* cuStreamGetDevice -> wcuStreamGetDevice */

/*
 * CUDA API: cuStreamGetFlags
 * Reference wrapper: wcuStreamGetFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamGetFlags(mcDrvStream_t hStream, unsigned int *flags);
 */
/* cuStreamGetFlags -> wcuStreamGetFlags */

/*
 * CUDA API: cuStreamGetGreenCtx
 * Reference wrapper: wcuStreamGetGreenCtx
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamGetGreenCtx(MCstream hStream, MCcontext *phCtx);
 */
/* cuStreamGetGreenCtx -> wcuStreamGetGreenCtx */

/*
 * CUDA API: cuStreamGetId
 * Reference wrapper: wcuStreamGetId
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamGetId(mcDrvStream_t hStream, unsigned long long *streamId);
 */
/* cuStreamGetId -> wcuStreamGetId */

/*
 * CUDA API: cuStreamGetPriority
 * Reference wrapper: wcuStreamGetPriority
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamGetPriority(mcDrvStream_t hStream, int *priority);
 */
/* cuStreamGetPriority -> wcuStreamGetPriority */

/*
 * CUDA API: cuStreamIsCapturing
 * Reference wrapper: wcuStreamIsCapturing
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamIsCapturing(mcDrvStream_t hStream, mcStreamCaptureStatus *captureStatus);
 */
/* cuStreamIsCapturing -> wcuStreamIsCapturing */

/*
 * CUDA API: cuStreamQuery
 * Reference wrapper: wcuStreamQuery
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamQuery(mcDrvStream_t hStream);
 */
/* cuStreamQuery -> wcuStreamQuery */

/*
 * CUDA API: cuStreamSetAttribute
 * Reference wrapper: wcuStreamSetAttribute
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamSetAttribute(mcDrvStream_t hStream, mcDrvStreamAttrID attr, const mcDrvStreamAttrValue *value);
 */
/* cuStreamSetAttribute -> wcuStreamSetAttribute */

/*
 * CUDA API: cuStreamSynchronize
 * Reference wrapper: wcuStreamSynchronize
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamSynchronize(mcDrvStream_t hStream);
 */
/* cuStreamSynchronize -> wcuStreamSynchronize */

/*
 * CUDA API: cuStreamUpdateCaptureDependencies
 * Reference wrapper: wcuStreamUpdateCaptureDependencies
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamUpdateCaptureDependencies(mcDrvStream_t hStream, mcDrvGraphNode *dependencies, size_t numDependencies, unsigned int flags);
 */
/* cuStreamUpdateCaptureDependencies -> wcuStreamUpdateCaptureDependencies */

/*
 * CUDA API: cuStreamWaitEvent
 * Reference wrapper: wcuStreamWaitEvent
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamWaitEvent(mcDrvStream_t hStream, mcDrvEvent_t hEvent, unsigned int Flags);
 */
/* cuStreamWaitEvent -> wcuStreamWaitEvent */

/*
 * CUDA API: cuStreamWaitValue32
 * Reference wrapper: wcuStreamWaitValue32
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamWaitValue32(mcDrvStream_t stream, mcDrvDeviceptr_t addr, uint32_t value, unsigned int flags);
 */
/* cuStreamWaitValue32 -> wcuStreamWaitValue32 */

/*
 * CUDA API: cuStreamWaitValue64
 * Reference wrapper: wcuStreamWaitValue64
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamWaitValue64(mcDrvStream_t stream, mcDrvDeviceptr_t addr, uint64_t value, unsigned int flags);
 */
/* cuStreamWaitValue64 -> wcuStreamWaitValue64 */

/*
 * CUDA API: cuStreamWriteValue32
 * Reference wrapper: wcuStreamWriteValue32
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamWriteValue32(mcDrvStream_t stream, mcDrvDeviceptr_t addr, uint32_t value, unsigned int flags);
 */
/* cuStreamWriteValue32 -> wcuStreamWriteValue32 */

/*
 * CUDA API: cuStreamWriteValue64
 * Reference wrapper: wcuStreamWriteValue64
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuStreamWriteValue64(mcDrvStream_t stream, mcDrvDeviceptr_t addr, uint64_t value, unsigned int flags);
 */
/* cuStreamWriteValue64 -> wcuStreamWriteValue64 */

/*
 * CUDA API: cuSurfObjectCreate
 * Reference wrapper: wcuSurfObjectCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuSurfObjectCreate(mcDrvSurfObject *pSurfObject, const mcDrvResourceDesc *pResDesc);
 */
/* cuSurfObjectCreate -> wcuSurfObjectCreate */

/*
 * CUDA API: cuSurfObjectDestroy
 * Reference wrapper: wcuSurfObjectDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuSurfObjectDestroy(mcDrvSurfObject surfObject);
 */
/* cuSurfObjectDestroy -> wcuSurfObjectDestroy */

/*
 * CUDA API: cuSurfObjectGetResourceDesc
 * Reference wrapper: wcuSurfObjectGetResourceDesc
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuSurfObjectGetResourceDesc(mcDrvResourceDesc *pResDesc, mcDrvSurfObject surfObject);
 */
/* cuSurfObjectGetResourceDesc -> wcuSurfObjectGetResourceDesc */

/*
 * CUDA API: cuSurfRefGetArray
 * Reference wrapper: wcuSurfRefGetArray
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuSurfRefGetArray not supported) mcDrvError_t CUBRIDGE_API wcuSurfRefGetArray(MCarray *phArray, mcDrvSurfref hSurfRef);
 */
/* cuSurfRefGetArray -> wcuSurfRefGetArray */

/*
 * CUDA API: cuSurfRefSetArray
 * Reference wrapper: wcuSurfRefSetArray
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuSurfRefSetArray not supported) mcDrvError_t CUBRIDGE_API wcuSurfRefSetArray(mcDrvSurfref hSurfRef, MCarray hArray, unsigned int Flags);
 */
/* cuSurfRefSetArray -> wcuSurfRefSetArray */

/*
 * CUDA API: cuTexObjectCreate
 * Reference wrapper: wcuTexObjectCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuTexObjectCreate(mcDrvTexObject *pTexObject, const mcDrvResourceDesc *pResDesc, const mcDrvTextureDesc *pTexDesc, const mcDrvResourceViewDesc *pResViewDesc);
 */
/* cuTexObjectCreate -> wcuTexObjectCreate */

/*
 * CUDA API: cuTexObjectDestroy
 * Reference wrapper: wcuTexObjectDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuTexObjectDestroy(mcDrvTexObject texObject);
 */
/* cuTexObjectDestroy -> wcuTexObjectDestroy */

/*
 * CUDA API: cuTexObjectGetResourceDesc
 * Reference wrapper: wcuTexObjectGetResourceDesc
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuTexObjectGetResourceDesc(mcDrvResourceDesc *pResDesc, mcDrvTexObject texObject);
 */
/* cuTexObjectGetResourceDesc -> wcuTexObjectGetResourceDesc */

/*
 * CUDA API: cuTexObjectGetResourceViewDesc
 * Reference wrapper: wcuTexObjectGetResourceViewDesc
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuTexObjectGetResourceViewDesc(mcDrvResourceViewDesc *pResViewDesc, mcDrvTexObject texObject);
 */
/* cuTexObjectGetResourceViewDesc -> wcuTexObjectGetResourceViewDesc */

/*
 * CUDA API: cuTexObjectGetTextureDesc
 * Reference wrapper: wcuTexObjectGetTextureDesc
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuTexObjectGetTextureDesc(mcDrvTextureDesc *pTexDesc, mcDrvTexObject texObject);
 */
/* cuTexObjectGetTextureDesc -> wcuTexObjectGetTextureDesc */

/*
 * CUDA API: cuTexRefCreate
 * Reference wrapper: wcuTexRefCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefCreate not supported) mcDrvError_t CUBRIDGE_API wcuTexRefCreate(mcDrvTexref *pTexRef);
 */
/* cuTexRefCreate -> wcuTexRefCreate */

/*
 * CUDA API: cuTexRefDestroy
 * Reference wrapper: wcuTexRefDestroy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefDestroy not supported) mcDrvError_t CUBRIDGE_API wcuTexRefDestroy(mcDrvTexref hTexRef);
 */
/* cuTexRefDestroy -> wcuTexRefDestroy */

/*
 * CUDA API: cuTexRefGetAddress
 * Reference wrapper: wcuTexRefGetAddress
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetAddress not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetAddress(mcDrvDeviceptr_t *pdptr, mcDrvTexref hTexRef);
 */
/* cuTexRefGetAddress -> wcuTexRefGetAddress */

/*
 * CUDA API: cuTexRefGetAddressMode
 * Reference wrapper: wcuTexRefGetAddressMode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetAddressMode not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetAddressMode(mcDrvAddress_mode *pam, mcDrvTexref hTexRef, int dim);
 */
/* cuTexRefGetAddressMode -> wcuTexRefGetAddressMode */

/*
 * CUDA API: cuTexRefGetArray
 * Reference wrapper: wcuTexRefGetArray
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetArray not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetArray(MCarray *phArray, mcDrvTexref hTexRef);
 */
/* cuTexRefGetArray -> wcuTexRefGetArray */

/*
 * CUDA API: cuTexRefGetBorderColor
 * Reference wrapper: wcuTexRefGetBorderColor
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetBorderColor not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetBorderColor(float *pBorderColor, mcDrvTexref hTexRef);
 */
/* cuTexRefGetBorderColor -> wcuTexRefGetBorderColor */

/*
 * CUDA API: cuTexRefGetFilterMode
 * Reference wrapper: wcuTexRefGetFilterMode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetFilterMode not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetFilterMode(mcDrvFilter_mode *pfm, mcDrvTexref hTexRef);
 */
/* cuTexRefGetFilterMode -> wcuTexRefGetFilterMode */

/*
 * CUDA API: cuTexRefGetFlags
 * Reference wrapper: wcuTexRefGetFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetFlags not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetFlags(unsigned int *pFlags, mcDrvTexref hTexRef);
 */
/* cuTexRefGetFlags -> wcuTexRefGetFlags */

/*
 * CUDA API: cuTexRefGetFormat
 * Reference wrapper: wcuTexRefGetFormat
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetFormat not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetFormat(mcArray_Format *pFormat, int *pNumChannels, mcDrvTexref hTexRef);
 */
/* cuTexRefGetFormat -> wcuTexRefGetFormat */

/*
 * CUDA API: cuTexRefGetMaxAnisotropy
 * Reference wrapper: wcuTexRefGetMaxAnisotropy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetMaxAnisotropy not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetMaxAnisotropy(int *pmaxAniso, mcDrvTexref hTexRef);
 */
/* cuTexRefGetMaxAnisotropy -> wcuTexRefGetMaxAnisotropy */

/*
 * CUDA API: cuTexRefGetMipmapFilterMode
 * Reference wrapper: wcuTexRefGetMipmapFilterMode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetMipmapFilterMode not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetMipmapFilterMode(mcDrvFilter_mode *pfm, mcDrvTexref hTexRef);
 */
/* cuTexRefGetMipmapFilterMode -> wcuTexRefGetMipmapFilterMode */

/*
 * CUDA API: cuTexRefGetMipmapLevelBias
 * Reference wrapper: wcuTexRefGetMipmapLevelBias
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetMipmapLevelBias not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetMipmapLevelBias(float *pbias, mcDrvTexref hTexRef);
 */
/* cuTexRefGetMipmapLevelBias -> wcuTexRefGetMipmapLevelBias */

/*
 * CUDA API: cuTexRefGetMipmapLevelClamp
 * Reference wrapper: wcuTexRefGetMipmapLevelClamp
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetMipmapLevelClamp not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetMipmapLevelClamp(float *pminMipmapLevelClamp, float *pmaxMipmapLevelClamp, mcDrvTexref hTexRef);
 */
/* cuTexRefGetMipmapLevelClamp -> wcuTexRefGetMipmapLevelClamp */

/*
 * CUDA API: cuTexRefGetMipmappedArray
 * Reference wrapper: wcuTexRefGetMipmappedArray
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefGetMipmappedArray not supported) mcDrvError_t CUBRIDGE_API wcuTexRefGetMipmappedArray(mcDrvMipmappedArray *phMipmappedArray, mcDrvTexref hTexRef);
 */
/* cuTexRefGetMipmappedArray -> wcuTexRefGetMipmappedArray */

/*
 * CUDA API: cuTexRefSetAddress
 * Reference wrapper: wcuTexRefSetAddress
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetAddress not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetAddress(size_t *ByteOffset, mcDrvTexref hTexRef, mcDrvDeviceptr_t dptr, size_t bytes);
 */
/* cuTexRefSetAddress -> wcuTexRefSetAddress */

/*
 * CUDA API: cuTexRefSetAddress2D
 * Reference wrapper: wcuTexRefSetAddress2D
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetAddress2D not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetAddress2D(mcDrvTexref hTexRef, const mcDrvArrayDescriptor *desc, mcDrvDeviceptr_t dptr, size_t Pitch);
 */
/* cuTexRefSetAddress2D -> wcuTexRefSetAddress2D */

/*
 * CUDA API: cuTexRefSetAddressMode
 * Reference wrapper: wcuTexRefSetAddressMode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetAddressMode not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetAddressMode(mcDrvTexref hTexRef, int dim, mcDrvAddress_mode am);
 */
/* cuTexRefSetAddressMode -> wcuTexRefSetAddressMode */

/*
 * CUDA API: cuTexRefSetArray
 * Reference wrapper: wcuTexRefSetArray
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetArray not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetArray(mcDrvTexref hTexRef, MCarray hArray, unsigned int Flags);
 */
/* cuTexRefSetArray -> wcuTexRefSetArray */

/*
 * CUDA API: cuTexRefSetBorderColor
 * Reference wrapper: wcuTexRefSetBorderColor
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetBorderColor not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetBorderColor(mcDrvTexref hTexRef, float *pBorderColor);
 */
/* cuTexRefSetBorderColor -> wcuTexRefSetBorderColor */

/*
 * CUDA API: cuTexRefSetFilterMode
 * Reference wrapper: wcuTexRefSetFilterMode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetFilterMode not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetFilterMode(mcDrvTexref hTexRef, mcDrvFilter_mode fm);
 */
/* cuTexRefSetFilterMode -> wcuTexRefSetFilterMode */

/*
 * CUDA API: cuTexRefSetFlags
 * Reference wrapper: wcuTexRefSetFlags
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetFlags not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetFlags(mcDrvTexref hTexRef, unsigned int Flags);
 */
/* cuTexRefSetFlags -> wcuTexRefSetFlags */

/*
 * CUDA API: cuTexRefSetFormat
 * Reference wrapper: wcuTexRefSetFormat
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetFormat not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetFormat(mcDrvTexref hTexRef, mcArray_Format fmt, int NumPackedComponents);
 */
/* cuTexRefSetFormat -> wcuTexRefSetFormat */

/*
 * CUDA API: cuTexRefSetMaxAnisotropy
 * Reference wrapper: wcuTexRefSetMaxAnisotropy
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetMaxAnisotropy not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetMaxAnisotropy(mcDrvTexref hTexRef, unsigned int maxAniso);
 */
/* cuTexRefSetMaxAnisotropy -> wcuTexRefSetMaxAnisotropy */

/*
 * CUDA API: cuTexRefSetMipmapFilterMode
 * Reference wrapper: wcuTexRefSetMipmapFilterMode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetMipmapFilterMode not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetMipmapFilterMode(mcDrvTexref hTexRef, mcDrvFilter_mode fm);
 */
/* cuTexRefSetMipmapFilterMode -> wcuTexRefSetMipmapFilterMode */

/*
 * CUDA API: cuTexRefSetMipmapLevelBias
 * Reference wrapper: wcuTexRefSetMipmapLevelBias
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetMipmapLevelBias not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetMipmapLevelBias(mcDrvTexref hTexRef, float bias);
 */
/* cuTexRefSetMipmapLevelBias -> wcuTexRefSetMipmapLevelBias */

/*
 * CUDA API: cuTexRefSetMipmapLevelClamp
 * Reference wrapper: wcuTexRefSetMipmapLevelClamp
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetMipmapLevelClamp not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetMipmapLevelClamp(mcDrvTexref hTexRef, float minMipmapLevelClamp, float maxMipmapLevelClamp);
 */
/* cuTexRefSetMipmapLevelClamp -> wcuTexRefSetMipmapLevelClamp */

/*
 * CUDA API: cuTexRefSetMipmappedArray
 * Reference wrapper: wcuTexRefSetMipmappedArray
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: __dparm_deprecated(cuTexRefSetMipmappedArray not supported) mcDrvError_t CUBRIDGE_API wcuTexRefSetMipmappedArray(mcDrvTexref hTexRef, mcDrvMipmappedArray hMipmappedArray, unsigned int Flags);
 */
/* cuTexRefSetMipmappedArray -> wcuTexRefSetMipmappedArray */

/*
 * CUDA API: cuThreadExchangeStreamCaptureMode
 * Reference wrapper: wcuThreadExchangeStreamCaptureMode
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuThreadExchangeStreamCaptureMode(mcStreamCaptureMode *mode);
 */
/* cuThreadExchangeStreamCaptureMode -> wcuThreadExchangeStreamCaptureMode */

/*
 * CUDA API: cuUserObjectCreate
 * Reference wrapper: wcuUserObjectCreate
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuUserObjectCreate(mcDrvUserObject *object_out, void *ptr, mcHostFn_t destroy, unsigned int initialRefcount, unsigned int flags);
 */
/* cuUserObjectCreate -> wcuUserObjectCreate */

/*
 * CUDA API: cuUserObjectRelease
 * Reference wrapper: wcuUserObjectRelease
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuUserObjectRelease(mcDrvUserObject object, unsigned int count);
 */
/* cuUserObjectRelease -> wcuUserObjectRelease */

/*
 * CUDA API: cuUserObjectRetain
 * Reference wrapper: wcuUserObjectRetain
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuUserObjectRetain(mcDrvUserObject object, unsigned int count);
 */
/* cuUserObjectRetain -> wcuUserObjectRetain */

/*
 * CUDA API: cuVDPAUCtxCreate
 * Reference wrapper: wcuVDPAUCtxCreate
 * Reference files: video/cuda_vdpau_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } mcDrvError_t wcuVDPAUCtxCreate(mcDrvcontext_t *pCtx, unsigned int flags, mcDrvDevice_t device, VdpDevice vdpDevice, VdpGetProcAddress *vdpGetProcAddress) { return MC_ERROR_NOT_SUPPORTED;
 */
/* cuVDPAUCtxCreate -> wcuVDPAUCtxCreate */

/*
 * CUDA API: cuWGLGetDevice
 * Reference wrapper: wcuWGLGetDevice
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: #endif mcDrvError_t wcuWGLGetDevice(mcDrvDevice_t *pDevice, HGPUNV hGpu);
 */
/* cuWGLGetDevice -> wcuWGLGetDevice */

/*
 * CUDA API: cuWaitExternalSemaphoresAsync
 * Reference wrapper: wcuWaitExternalSemaphoresAsync
 * Reference files: runtime/cuda_driver_wrapper.h
 * CANN route: CANN aclrtBinary/module route; no active CUDA Driver ABI
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcDrvError_t CUBRIDGE_API wcuWaitExternalSemaphoresAsync(const mcDrvExternalSemaphore *extSemArray, const mcDrvExternalSemaphoreWaitParams *paramsArray, unsigned int numExtSems, mcDrvStream_t stream);
 */
/* cuWaitExternalSemaphoresAsync -> wcuWaitExternalSemaphoresAsync */

/*
 * CUDA API: cudaArrayGetMemoryRequirements
 * Reference wrapper: wcudaArrayGetMemoryRequirements
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaArrayGetMemoryRequirements( mcArrayMemoryRequirements *memoryRequirements, mcArray_t array, int device);
 */
/* cudaArrayGetMemoryRequirements -> wcudaArrayGetMemoryRequirements */

/*
 * CUDA API: cudaArrayGetPlane
 * Reference wrapper: wcudaArrayGetPlane
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaArrayGetPlane(mcArray_t *pPlaneArray, mcArray_t hArray, unsigned int planeIdx);
 */
/* cudaArrayGetPlane -> wcudaArrayGetPlane */

/*
 * CUDA API: cudaArrayGetSparseProperties
 * Reference wrapper: wcudaArrayGetSparseProperties
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaArrayGetSparseProperties(mcArraySparseProperties *sparseProperties, mcArray_t array);
 */
/* cudaArrayGetSparseProperties -> wcudaArrayGetSparseProperties */

/*
 * CUDA API: cudaBindSurfaceToArray
 * Reference wrapper: wcudaBindSurfaceToArray
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T, int dim> static __inline__ __host__ mcError_t wcudaBindSurfaceToArray(const struct surface<T, dim> &surf, const mcArray *array, const mcChannelFormatDesc &desc) { return wcudaBindSurfaceToArray(&surf, array, &desc);
 * Reference prototype: } template <class T, int dim> static __inline__ __host__ mcError_t wcudaBindSurfaceToArray(const struct surface<T, dim> &surf, const mcArray *array) { return wcudaBindSurfaceToArray(&surf, array, &surf.channelDesc);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaBindSurfaceToArray(const struct surfaceReference *surfref, mcArray_const_t array, const mcChannelFormatDesc *desc);
 */
/* cudaBindSurfaceToArray -> wcudaBindSurfaceToArray */

/*
 * CUDA API: cudaBindTexture2D
 * Reference wrapper: wcudaBindTexture2D
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T, int dim, enum mcTextureReadMode readMode> static __inline__ __host__ mcError_t wcudaBindTexture2D(size_t *offset, const struct texture<T, dim, readMode> &tex, const void *devPtr, const mcChannelFormatDesc &desc, size_t width, size_t height, size_t pitch) { return wcudaBindTexture2D(offset, &tex, devPtr, &desc, width, height, pitch);
 * Reference prototype: } template <class T, int dim, enum mcTextureReadMode readMode> static __inline__ __host__ mcError_t wcudaBindTexture2D(size_t *offset, const struct texture<T, dim, readMode> &tex, const void *devPtr, size_t width, size_t height, size_t pitch) { return wcudaBindTexture2D(offset, &tex, devPtr, &tex.channelDesc, width, height, pitch);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaBindTexture2D(size_t *offset, const struct textureReference *texref, const void *devPtr, const mcChannelFormatDesc *desc, size_t width, size_t height, size_t pitch);
 */
/* cudaBindTexture2D -> wcudaBindTexture2D */

/*
 * CUDA API: cudaBindTextureToArray
 * Reference wrapper: wcudaBindTextureToArray
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T, int dim, enum mcTextureReadMode readMode> static __inline__ __host__ mcError_t wcudaBindTextureToArray(const struct texture<T, dim, readMode> &tex, mcArray_const_t array, const mcChannelFormatDesc &desc) { return wcudaBindTextureToArray(&tex, array, &desc);
 * Reference prototype: } template <class T, int dim, enum mcTextureReadMode readMode> static __inline__ __host__ mcError_t wcudaBindTextureToArray(const struct texture<T, dim, readMode> &tex, mcArray_const_t array) { mcChannelFormatDesc desc;
 * Reference prototype: return err == mcSuccess ? wcudaBindTextureToArray(tex, array, desc) : err;
 * Reference prototype: mcError_t CUBRIDGE_API wcudaBindTextureToArray(const struct textureReference *texref, mcArray_const_t array, const mcChannelFormatDesc *desc);
 */
/* cudaBindTextureToArray -> wcudaBindTextureToArray */

/*
 * CUDA API: cudaBindTextureToMipmappedArray
 * Reference wrapper: wcudaBindTextureToMipmappedArray
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T, int dim, enum mcTextureReadMode readMode> static __inline__ __host__ mcError_t wcudaBindTextureToMipmappedArray( const struct texture<T, dim, readMode> &tex, mcMipmappedArray_const_t mipmappedArray, const mcChannelFormatDesc &desc) { return wcudaBindTextureToMipmappedArray(&tex, mipmappedArray, &desc);
 * Reference prototype: } template <class T, int dim, enum mcTextureReadMode readMode> static __inline__ __host__ mcError_t wcudaBindTextureToMipmappedArray( const struct texture<T, dim, readMode> &tex, mcMipmappedArray_const_t mipmappedArray) { mcChannelFormatDesc desc;
 * Reference prototype: return err == mcSuccess ? wcudaBindTextureToMipmappedArray(tex, mipmappedArray, desc) : err;
 * Reference prototype: mcError_t CUBRIDGE_API wcudaBindTextureToMipmappedArray(const struct textureReference *texref, mcMipmappedArray_const_t mipmappedArray, const mcChannelFormatDesc *desc);
 */
/* cudaBindTextureToMipmappedArray -> wcudaBindTextureToMipmappedArray */

/*
 * CUDA API: cudaCGGetIntrinsicHandle
 * Reference wrapper: wcudaCGGetIntrinsicHandle
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ unsigned long long wcudaCGGetIntrinsicHandle(enum mcCGScope scope) { return mcCGGetIntrinsicHandle(scope);
 */
/* cudaCGGetIntrinsicHandle -> wcudaCGGetIntrinsicHandle */

/*
 * CUDA API: cudaCGGetRank
 * Reference wrapper: wcudaCGGetRank
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaCGGetRank(unsigned int *threadRank, unsigned int *gridRank, unsigned long long handle) { return mcCGGetRank(threadRank, gridRank, handle);
 */
/* cudaCGGetRank -> wcudaCGGetRank */

/*
 * CUDA API: cudaCGGetSize
 * Reference wrapper: wcudaCGGetSize
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaCGGetSize(unsigned int *numThreads, unsigned int *numGrids, unsigned long long handle) { return mcCGGetSize(numThreads, numGrids, handle);
 */
/* cudaCGGetSize -> wcudaCGGetSize */

/*
 * CUDA API: cudaCGSynchronize
 * Reference wrapper: wcudaCGSynchronize
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaCGSynchronize(unsigned long long handle, unsigned int flags) { return mcCGSynchronize(handle, flags);
 */
/* cudaCGSynchronize -> wcudaCGSynchronize */

/*
 * CUDA API: cudaCGSynchronizeGrid
 * Reference wrapper: wcudaCGSynchronizeGrid
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaCGSynchronizeGrid(unsigned long long handle, unsigned int flags) { return mcCGSynchronizeGrid(handle, flags);
 */
/* cudaCGSynchronizeGrid -> wcudaCGSynchronizeGrid */

/*
 * CUDA API: cudaChooseDevice
 * Reference wrapper: wcudaChooseDevice
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaChooseDevice(int *device, const mcDeviceProp_t *prop);
 */
/* cudaChooseDevice -> wcudaChooseDevice */

/*
 * CUDA API: cudaCreateChannelDescHalf
 * Reference wrapper: wcudaCreateChannelDescHalf
 * Reference files: runtime/cuda_channel_descriptor_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } static __inline__ __host__ mcChannelFormatDesc wcudaCreateChannelDescHalf(void) { int e = (int)sizeof(unsigned short) * 8;
 */
/* cudaCreateChannelDescHalf -> wcudaCreateChannelDescHalf */

/*
 * CUDA API: cudaCreateChannelDescHalf1
 * Reference wrapper: wcudaCreateChannelDescHalf1
 * Reference files: runtime/cuda_channel_descriptor_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } static __inline__ __host__ mcChannelFormatDesc wcudaCreateChannelDescHalf1(void) { int e = (int)sizeof(unsigned short) * 8;
 */
/* cudaCreateChannelDescHalf1 -> wcudaCreateChannelDescHalf1 */

/*
 * CUDA API: cudaCreateChannelDescHalf2
 * Reference wrapper: wcudaCreateChannelDescHalf2
 * Reference files: runtime/cuda_channel_descriptor_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } static __inline__ __host__ mcChannelFormatDesc wcudaCreateChannelDescHalf2(void) { int e = (int)sizeof(unsigned short) * 8;
 */
/* cudaCreateChannelDescHalf2 -> wcudaCreateChannelDescHalf2 */

/*
 * CUDA API: cudaCreateChannelDescHalf4
 * Reference wrapper: wcudaCreateChannelDescHalf4
 * Reference files: runtime/cuda_channel_descriptor_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } static __inline__ __host__ mcChannelFormatDesc wcudaCreateChannelDescHalf4(void) { int e = (int)sizeof(unsigned short) * 8;
 */
/* cudaCreateChannelDescHalf4 -> wcudaCreateChannelDescHalf4 */

/*
 * CUDA API: cudaCreateChannelDescNV12
 * Reference wrapper: wcudaCreateChannelDescNV12
 * Reference files: runtime/cuda_channel_descriptor_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } static __inline__ __host__ mcChannelFormatDesc wcudaCreateChannelDescNV12(void) { int e = (int)sizeof(char) * 8;
 */
/* cudaCreateChannelDescNV12 -> wcudaCreateChannelDescNV12 */

/*
 * CUDA API: cudaCreateTextureObject_v2
 * Reference wrapper: wcudaCreateTextureObject_v2
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaCreateTextureObject_v2(mcTextureObject_t *pTexObject, const struct mcResourceDesc *pResDesc, const struct mcTextureDesc *pTexDesc, const struct mcResourceViewDesc *pResViewDesc);
 */
/* cudaCreateTextureObject_v2 -> wcudaCreateTextureObject_v2 */

/*
 * CUDA API: cudaCtxResetPersistingL2Cache
 * Reference wrapper: wcudaCtxResetPersistingL2Cache
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaCtxResetPersistingL2Cache(void);
 */
/* cudaCtxResetPersistingL2Cache -> wcudaCtxResetPersistingL2Cache */

/*
 * CUDA API: cudaDestroyExternalSemaphore
 * Reference wrapper: wcudaDestroyExternalSemaphore
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDestroyExternalSemaphore(mcExternalSemaphore_t extSem);
 */
/* cudaDestroyExternalSemaphore -> wcudaDestroyExternalSemaphore */

/*
 * CUDA API: cudaDevResourceGenerateDesc
 * Reference wrapper: wcudaDevResourceGenerateDesc
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDevResourceGenerateDesc(mcDevResourceDesc_t *phDesc, mcDrvDevResource *resources, unsigned int nbResources);
 */
/* cudaDevResourceGenerateDesc -> wcudaDevResourceGenerateDesc */

/*
 * CUDA API: cudaDevSmResourceSplit
 * Reference wrapper: wcudaDevSmResourceSplit
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDevSmResourceSplit(mcDrvDevResource *result, unsigned int nbGroups, const mcDrvDevResource *input, mcDrvDevResource *remainder, unsigned int flags, mcDrvDevApResourceGroupParams *groupParams);
 */
/* cudaDevSmResourceSplit -> wcudaDevSmResourceSplit */

/*
 * CUDA API: cudaDevSmResourceSplitByCount
 * Reference wrapper: wcudaDevSmResourceSplitByCount
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDevSmResourceSplitByCount(mcDrvDevResource *result, unsigned int *nbGroups, const mcDrvDevResource *input, mcDrvDevResource *remaining, unsigned int flags, unsigned int minCount);
 */
/* cudaDevSmResourceSplitByCount -> wcudaDevSmResourceSplitByCount */

/*
 * CUDA API: cudaDeviceFlushGPUDirectRDMAWrites
 * Reference wrapper: wcudaDeviceFlushGPUDirectRDMAWrites
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDeviceFlushGPUDirectRDMAWrites( enum mcFlushGPUDirectRDMAWritesTarget target, enum mcFlushGPUDirectRDMAWritesScope scope);
 */
/* cudaDeviceFlushGPUDirectRDMAWrites -> wcudaDeviceFlushGPUDirectRDMAWrites */

/*
 * CUDA API: cudaDeviceGetDefaultMemPool
 * Reference wrapper: wcudaDeviceGetDefaultMemPool
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDeviceGetDefaultMemPool(mcMemPool_t *memPool, int device);
 */
/* cudaDeviceGetDefaultMemPool -> wcudaDeviceGetDefaultMemPool */

/*
 * CUDA API: cudaDeviceGetDevResource
 * Reference wrapper: wcudaDeviceGetDevResource
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDeviceGetDevResource(mcDevice_t dev, mcDrvDevResource *resource, mcDevResourceType type);
 */
/* cudaDeviceGetDevResource -> wcudaDeviceGetDevResource */

/*
 * CUDA API: cudaDeviceGetExecutionCtx
 * Reference wrapper: wcudaDeviceGetExecutionCtx
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDeviceGetExecutionCtx(mcExecutionContext_t *ctx, int device);
 */
/* cudaDeviceGetExecutionCtx -> wcudaDeviceGetExecutionCtx */

/*
 * CUDA API: cudaDeviceGetGraphMemAttribute
 * Reference wrapper: wcudaDeviceGetGraphMemAttribute
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: #endif mcError_t CUBRIDGE_API wcudaDeviceGetGraphMemAttribute(int device, mcGraphMemAttributeType attr, void *value);
 */
/* cudaDeviceGetGraphMemAttribute -> wcudaDeviceGetGraphMemAttribute */

/*
 * CUDA API: cudaDeviceGetNvSciSyncAttributes
 * Reference wrapper: wcudaDeviceGetNvSciSyncAttributes
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDeviceGetNvSciSyncAttributes(void *mcSciSyncAttrList, int device, int flags);
 */
/* cudaDeviceGetNvSciSyncAttributes -> wcudaDeviceGetNvSciSyncAttributes */

/*
 * CUDA API: cudaDeviceGetP2PAttribute
 * Reference wrapper: wcudaDeviceGetP2PAttribute
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDeviceGetP2PAttribute(int *value, enum mcDeviceP2PAttr attr, int srcDevice, int dstDevice);
 */
/* cudaDeviceGetP2PAttribute -> wcudaDeviceGetP2PAttribute */

/*
 * CUDA API: cudaDeviceGetTexture1DLinearMaxWidth
 * Reference wrapper: wcudaDeviceGetTexture1DLinearMaxWidth
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDeviceGetTexture1DLinearMaxWidth(size_t *maxWidthInElements, const mcChannelFormatDesc *fmtDesc, int device);
 */
/* cudaDeviceGetTexture1DLinearMaxWidth -> wcudaDeviceGetTexture1DLinearMaxWidth */

/*
 * CUDA API: cudaDeviceGraphMemTrim
 * Reference wrapper: wcudaDeviceGraphMemTrim
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDeviceGraphMemTrim(int device);
 */
/* cudaDeviceGraphMemTrim -> wcudaDeviceGraphMemTrim */

/*
 * CUDA API: cudaDeviceSetGraphMemAttribute
 * Reference wrapper: wcudaDeviceSetGraphMemAttribute
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaDeviceSetGraphMemAttribute(int device, mcGraphMemAttributeType attr, void *value);
 */
/* cudaDeviceSetGraphMemAttribute -> wcudaDeviceSetGraphMemAttribute */

/*
 * CUDA API: cudaEGLStreamConsumerAcquireFrame
 * Reference wrapper: wcudaEGLStreamConsumerAcquireFrame
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaEGLStreamConsumerAcquireFrame(mcEglStreamConnection *conn, mcGraphicsResource_t *pMacaResource, mcStream_t *pStream, unsigned int timeout);
 */
/* cudaEGLStreamConsumerAcquireFrame -> wcudaEGLStreamConsumerAcquireFrame */

/*
 * CUDA API: cudaEGLStreamConsumerConnect
 * Reference wrapper: wcudaEGLStreamConsumerConnect
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaEGLStreamConsumerConnect(mcEglStreamConnection *conn, EGLStreamKHR eglStream);
 */
/* cudaEGLStreamConsumerConnect -> wcudaEGLStreamConsumerConnect */

/*
 * CUDA API: cudaEGLStreamConsumerConnectWithFlags
 * Reference wrapper: wcudaEGLStreamConsumerConnectWithFlags
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaEGLStreamConsumerConnectWithFlags(mcEglStreamConnection *conn, EGLStreamKHR eglStream, unsigned int flags);
 */
/* cudaEGLStreamConsumerConnectWithFlags -> wcudaEGLStreamConsumerConnectWithFlags */

/*
 * CUDA API: cudaEGLStreamConsumerDisconnect
 * Reference wrapper: wcudaEGLStreamConsumerDisconnect
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaEGLStreamConsumerDisconnect(mcEglStreamConnection *conn);
 */
/* cudaEGLStreamConsumerDisconnect -> wcudaEGLStreamConsumerDisconnect */

/*
 * CUDA API: cudaEGLStreamConsumerReleaseFrame
 * Reference wrapper: wcudaEGLStreamConsumerReleaseFrame
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaEGLStreamConsumerReleaseFrame(mcEglStreamConnection *conn, mcGraphicsResource_t pMacaResource, mcStream_t *pStream);
 */
/* cudaEGLStreamConsumerReleaseFrame -> wcudaEGLStreamConsumerReleaseFrame */

/*
 * CUDA API: cudaEGLStreamProducerConnect
 * Reference wrapper: wcudaEGLStreamProducerConnect
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaEGLStreamProducerConnect(mcEglStreamConnection *conn, EGLStreamKHR eglStream, EGLint width, EGLint height);
 */
/* cudaEGLStreamProducerConnect -> wcudaEGLStreamProducerConnect */

/*
 * CUDA API: cudaEGLStreamProducerDisconnect
 * Reference wrapper: wcudaEGLStreamProducerDisconnect
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaEGLStreamProducerDisconnect(mcEglStreamConnection *conn);
 */
/* cudaEGLStreamProducerDisconnect -> wcudaEGLStreamProducerDisconnect */

/*
 * CUDA API: cudaEGLStreamProducerPresentFrame
 * Reference wrapper: wcudaEGLStreamProducerPresentFrame
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaEGLStreamProducerPresentFrame(mcEglStreamConnection *conn, mcEglFrame eglframe, mcStream_t *pStream);
 */
/* cudaEGLStreamProducerPresentFrame -> wcudaEGLStreamProducerPresentFrame */

/*
 * CUDA API: cudaEGLStreamProducerReturnFrame
 * Reference wrapper: wcudaEGLStreamProducerReturnFrame
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaEGLStreamProducerReturnFrame(mcEglStreamConnection *conn, mcEglFrame *eglframe, mcStream_t *pStream);
 */
/* cudaEGLStreamProducerReturnFrame -> wcudaEGLStreamProducerReturnFrame */

/*
 * CUDA API: cudaEventCreateFromEGLSync
 * Reference wrapper: wcudaEventCreateFromEGLSync
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaEventCreateFromEGLSync(mcEvent_t *phEvent, EGLSyncKHR eglSync, unsigned int flags);
 */
/* cudaEventCreateFromEGLSync -> wcudaEventCreateFromEGLSync */

/*
 * CUDA API: cudaEventRecordWithFlags_ptsz
 * Reference wrapper: wcudaEventRecordWithFlags_ptsz
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaEventRecordWithFlags_ptsz( mcEvent_t event __attribute__((unused)), mcStream_t stream __attribute__((unused)), unsigned int flags __attribute__((unused))) { return mcErrorNotSupported;
 */
/* cudaEventRecordWithFlags_ptsz -> wcudaEventRecordWithFlags_ptsz */

/*
 * CUDA API: cudaEventRecord_ptsz
 * Reference wrapper: wcudaEventRecord_ptsz
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaEventRecord_ptsz( mcEvent_t event __attribute__((unused)), mcStream_t stream __attribute__((unused))) { return mcErrorNotSupported;
 */
/* cudaEventRecord_ptsz -> wcudaEventRecord_ptsz */

/*
 * CUDA API: cudaExecutionCtxDestroy
 * Reference wrapper: wcudaExecutionCtxDestroy
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaExecutionCtxDestroy(mcExecutionContext_t ctx);
 */
/* cudaExecutionCtxDestroy -> wcudaExecutionCtxDestroy */

/*
 * CUDA API: cudaExecutionCtxGetDevResource
 * Reference wrapper: wcudaExecutionCtxGetDevResource
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaExecutionCtxGetDevResource(mcExecutionContext_t ctx, mcDrvDevResource *resource, mcDevResourceType type);
 */
/* cudaExecutionCtxGetDevResource -> wcudaExecutionCtxGetDevResource */

/*
 * CUDA API: cudaExecutionCtxGetDevice
 * Reference wrapper: wcudaExecutionCtxGetDevice
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaExecutionCtxGetDevice(int *device, mcExecutionContext_t ctx);
 */
/* cudaExecutionCtxGetDevice -> wcudaExecutionCtxGetDevice */

/*
 * CUDA API: cudaExecutionCtxGetId
 * Reference wrapper: wcudaExecutionCtxGetId
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaExecutionCtxGetId(mcExecutionContext_t ctx, unsigned long long *ctxId);
 */
/* cudaExecutionCtxGetId -> wcudaExecutionCtxGetId */

/*
 * CUDA API: cudaExecutionCtxRecordEvent
 * Reference wrapper: wcudaExecutionCtxRecordEvent
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaExecutionCtxRecordEvent(mcExecutionContext_t ctx, mcEvent_t event);
 */
/* cudaExecutionCtxRecordEvent -> wcudaExecutionCtxRecordEvent */

/*
 * CUDA API: cudaExecutionCtxStreamCreate
 * Reference wrapper: wcudaExecutionCtxStreamCreate
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaExecutionCtxStreamCreate(mcStream_t *phStream, mcExecutionContext_t ctx, unsigned int flags, int priority);
 */
/* cudaExecutionCtxStreamCreate -> wcudaExecutionCtxStreamCreate */

/*
 * CUDA API: cudaExecutionCtxSynchronize
 * Reference wrapper: wcudaExecutionCtxSynchronize
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaExecutionCtxSynchronize(mcExecutionContext_t ctx);
 */
/* cudaExecutionCtxSynchronize -> wcudaExecutionCtxSynchronize */

/*
 * CUDA API: cudaExecutionCtxWaitEvent
 * Reference wrapper: wcudaExecutionCtxWaitEvent
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaExecutionCtxWaitEvent(mcExecutionContext_t ctx, mcEvent_t event);
 */
/* cudaExecutionCtxWaitEvent -> wcudaExecutionCtxWaitEvent */

/*
 * CUDA API: cudaExternalMemoryGetMappedBuffer
 * Reference wrapper: wcudaExternalMemoryGetMappedBuffer
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaExternalMemoryGetMappedBuffer( void **devPtr, mcExternalMemory_t extMem, const mcExternalMemoryBufferDesc *bufferDesc);
 */
/* cudaExternalMemoryGetMappedBuffer -> wcudaExternalMemoryGetMappedBuffer */

/*
 * CUDA API: cudaExternalMemoryGetMappedMipmappedArray
 * Reference wrapper: wcudaExternalMemoryGetMappedMipmappedArray
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaExternalMemoryGetMappedMipmappedArray(mcMipmappedArray_t *mipmap, mcExternalMemory_t extMem, const mcExternalMemoryMipmappedArrayDesc *mipmapDesc);
 */
/* cudaExternalMemoryGetMappedMipmappedArray -> wcudaExternalMemoryGetMappedMipmappedArray */

/*
 * CUDA API: cudaFuncGetParamInfo
 * Reference wrapper: wcudaFuncGetParamInfo
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaFuncGetParamInfo(const void *func, size_t paramIndex, size_t *paramOffset, size_t *paramSize);
 */
/* cudaFuncGetParamInfo -> wcudaFuncGetParamInfo */

/*
 * CUDA API: cudaGLGetDevices
 * Reference wrapper: wcudaGLGetDevices
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaGLGetDevices(unsigned int *pMacaDeviceCount, int *pMacaDevices, unsigned int mcDeviceCount, enum mcGLDeviceList deviceList);
 */
/* cudaGLGetDevices -> wcudaGLGetDevices */

/*
 * CUDA API: cudaGLMapBufferObject
 * Reference wrapper: wcudaGLMapBufferObject
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: inline mcError_t wcudaGLMapBufferObject(void **devPtr, GLuint bufObj) { return mcErrorNotSupported;
 */
/* cudaGLMapBufferObject -> wcudaGLMapBufferObject */

/*
 * CUDA API: cudaGLMapBufferObjectAsync
 * Reference wrapper: wcudaGLMapBufferObjectAsync
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } inline mcError_t wcudaGLMapBufferObjectAsync(void **devPtr, GLuint bufObj, mcStream_t stream) { return mcErrorNotSupported;
 */
/* cudaGLMapBufferObjectAsync -> wcudaGLMapBufferObjectAsync */

/*
 * CUDA API: cudaGLRegisterBufferObject
 * Reference wrapper: wcudaGLRegisterBufferObject
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: inline mcError_t wcudaGLRegisterBufferObject(GLuint bufObj) { return mcErrorNotSupported; }
 */
/* cudaGLRegisterBufferObject -> wcudaGLRegisterBufferObject */

/*
 * CUDA API: cudaGLSetBufferObjectMapFlags
 * Reference wrapper: wcudaGLSetBufferObjectMapFlags
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: inline mcError_t wcudaGLSetBufferObjectMapFlags(GLuint bufObj, unsigned int flags) { return mcErrorNotSupported;
 */
/* cudaGLSetBufferObjectMapFlags -> wcudaGLSetBufferObjectMapFlags */

/*
 * CUDA API: cudaGLUnmapBufferObject
 * Reference wrapper: wcudaGLUnmapBufferObject
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } inline mcError_t wcudaGLUnmapBufferObject(GLuint bufObj) { return mcErrorNotSupported; }
 */
/* cudaGLUnmapBufferObject -> wcudaGLUnmapBufferObject */

/*
 * CUDA API: cudaGLUnmapBufferObjectAsync
 * Reference wrapper: wcudaGLUnmapBufferObjectAsync
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } inline mcError_t wcudaGLUnmapBufferObjectAsync(GLuint bufObj, mcStream_t stream) { return mcErrorNotSupported;
 */
/* cudaGLUnmapBufferObjectAsync -> wcudaGLUnmapBufferObjectAsync */

/*
 * CUDA API: cudaGLUnregisterBufferObject
 * Reference wrapper: wcudaGLUnregisterBufferObject
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: inline mcError_t wcudaGLUnregisterBufferObject(GLuint bufObj) { return mcErrorNotSupported; }
 */
/* cudaGLUnregisterBufferObject -> wcudaGLUnregisterBufferObject */

/*
 * CUDA API: cudaGetDriverEntryPoint
 * Reference wrapper: wcudaGetDriverEntryPoint
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGetDriverEntryPoint(const char *symbol, void **funcPtr, unsigned long long flags);
 */
/* cudaGetDriverEntryPoint -> wcudaGetDriverEntryPoint */

/*
 * CUDA API: cudaGetDriverEntryPoint_V2
 * Reference wrapper: wcudaGetDriverEntryPoint_V2
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: #if defined(__cplusplus) extern mcError_t CUBRIDGE_API wcudaGetDriverEntryPoint_V2(const char *symbol, void **funcPtr, unsigned long long flags, enum mcDriverEntryPointQueryResult *driverStatus = NULL);
 * Reference prototype: #else extern mcError_t CUBRIDGE_API wcudaGetDriverEntryPoint_V2(const char *symbol, void **funcPtr, unsigned long long flags, enum mcDriverEntryPointQueryResult *driverStatus);
 */
/* cudaGetDriverEntryPoint_V2 -> wcudaGetDriverEntryPoint_V2 */

/*
 * CUDA API: cudaGetExportTable
 * Reference wrapper: wcudaGetExportTable
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGetExportTable(const void **ppExportTable, const mcUuid_t *pExportTableId);
 */
/* cudaGetExportTable -> wcudaGetExportTable */

/*
 * CUDA API: cudaGetFuncBySymbol
 * Reference wrapper: wcudaGetFuncBySymbol
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGetFuncBySymbol(mcFunction_t *functionPtr, const void *symbolPtr);
 */
/* cudaGetFuncBySymbol -> wcudaGetFuncBySymbol */

/*
 * CUDA API: cudaGetParameterBuffer
 * Reference wrapper: wcudaGetParameterBuffer
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ void *wcudaGetParameterBuffer(size_t alignment, size_t size) { return mcGetParameterBuffer(alignment, size);
 */
/* cudaGetParameterBuffer -> wcudaGetParameterBuffer */

/*
 * CUDA API: cudaGetParameterBufferV2
 * Reference wrapper: wcudaGetParameterBufferV2
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ void *wcudaGetParameterBufferV2(void *func, dim3 gridDimension, dim3 blockDimension, unsigned int sharedMemSize) { return mcGetParameterBufferV2(func, gridDimension, blockDimension, sharedMemSize);
 */
/* cudaGetParameterBufferV2 -> wcudaGetParameterBufferV2 */

/*
 * CUDA API: cudaGetSurfaceReference
 * Reference wrapper: wcudaGetSurfaceReference
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGetSurfaceReference(const struct surfaceReference **surfref, const void *symbol);
 */
/* cudaGetSurfaceReference -> wcudaGetSurfaceReference */

/*
 * CUDA API: cudaGetTextureAlignmentOffset
 * Reference wrapper: wcudaGetTextureAlignmentOffset
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T, int dim, enum mcTextureReadMode readMode> static __inline__ __host__ mcError_t wcudaGetTextureAlignmentOffset(size_t *offset, const struct texture<T, dim, readMode> &tex) { return wcudaGetTextureAlignmentOffset(offset, &tex);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGetTextureAlignmentOffset(size_t *offset, const struct textureReference *texref);
 */
/* cudaGetTextureAlignmentOffset -> wcudaGetTextureAlignmentOffset */

/*
 * CUDA API: cudaGetTextureReference
 * Reference wrapper: wcudaGetTextureReference
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGetTextureReference(const struct textureReference **texref, const void *symbol);
 */
/* cudaGetTextureReference -> wcudaGetTextureReference */

/*
 * CUDA API: cudaGraphAddChildGraphNode
 * Reference wrapper: wcudaGraphAddChildGraphNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddChildGraphNode(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, mcGraph_t childGraph);
 */
/* cudaGraphAddChildGraphNode -> wcudaGraphAddChildGraphNode */

/*
 * CUDA API: cudaGraphAddDependencies
 * Reference wrapper: wcudaGraphAddDependencies
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddDependencies(mcGraph_t graph, const mcGraphNode_t *from, const mcGraphNode_t *to, size_t numDependencies);
 */
/* cudaGraphAddDependencies -> wcudaGraphAddDependencies */

/*
 * CUDA API: cudaGraphAddDependencies_v2
 * Reference wrapper: wcudaGraphAddDependencies_v2
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddDependencies_v2(mcGraph_t graph, const mcGraphNode_t *from, const mcGraphNode_t *to, const mcGraphEdgeData *edgeData, size_t numDependencies);
 */
/* cudaGraphAddDependencies_v2 -> wcudaGraphAddDependencies_v2 */

/*
 * CUDA API: cudaGraphAddEmptyNode
 * Reference wrapper: wcudaGraphAddEmptyNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddEmptyNode(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies);
 */
/* cudaGraphAddEmptyNode -> wcudaGraphAddEmptyNode */

/*
 * CUDA API: cudaGraphAddEventRecordNode
 * Reference wrapper: wcudaGraphAddEventRecordNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddEventRecordNode(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, mcEvent_t event);
 */
/* cudaGraphAddEventRecordNode -> wcudaGraphAddEventRecordNode */

/*
 * CUDA API: cudaGraphAddEventWaitNode
 * Reference wrapper: wcudaGraphAddEventWaitNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddEventWaitNode(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, mcEvent_t event);
 */
/* cudaGraphAddEventWaitNode -> wcudaGraphAddEventWaitNode */

/*
 * CUDA API: cudaGraphAddExternalSemaphoresSignalNode
 * Reference wrapper: wcudaGraphAddExternalSemaphoresSignalNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddExternalSemaphoresSignalNode( mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, const mcExternalSemaphoreSignalNodeParams *nodeParams);
 */
/* cudaGraphAddExternalSemaphoresSignalNode -> wcudaGraphAddExternalSemaphoresSignalNode */

/*
 * CUDA API: cudaGraphAddExternalSemaphoresWaitNode
 * Reference wrapper: wcudaGraphAddExternalSemaphoresWaitNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddExternalSemaphoresWaitNode( mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, const mcExternalSemaphoreWaitNodeParams *nodeParams);
 */
/* cudaGraphAddExternalSemaphoresWaitNode -> wcudaGraphAddExternalSemaphoresWaitNode */

/*
 * CUDA API: cudaGraphAddHostNode
 * Reference wrapper: wcudaGraphAddHostNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddHostNode(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, const mcHostNodeParams *pNodeParams);
 */
/* cudaGraphAddHostNode -> wcudaGraphAddHostNode */

/*
 * CUDA API: cudaGraphAddMemAllocNode
 * Reference wrapper: wcudaGraphAddMemAllocNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddMemAllocNode(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, struct mcMemAllocNodeParams *nodeParams);
 */
/* cudaGraphAddMemAllocNode -> wcudaGraphAddMemAllocNode */

/*
 * CUDA API: cudaGraphAddMemFreeNode
 * Reference wrapper: wcudaGraphAddMemFreeNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddMemFreeNode(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, void *dptr);
 */
/* cudaGraphAddMemFreeNode -> wcudaGraphAddMemFreeNode */

/*
 * CUDA API: cudaGraphAddMemcpyNode
 * Reference wrapper: wcudaGraphAddMemcpyNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddMemcpyNode(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, const struct mcMemcpy3DParms *pCopyParams);
 */
/* cudaGraphAddMemcpyNode -> wcudaGraphAddMemcpyNode */

/*
 * CUDA API: cudaGraphAddMemcpyNode1D
 * Reference wrapper: wcudaGraphAddMemcpyNode1D
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddMemcpyNode1D(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, void *dst, const void *src, size_t count, mcMemcpyKind kind);
 */
/* cudaGraphAddMemcpyNode1D -> wcudaGraphAddMemcpyNode1D */

/*
 * CUDA API: cudaGraphAddMemcpyNodeFromSymbol
 * Reference wrapper: wcudaGraphAddMemcpyNodeFromSymbol
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static __inline__ __host__ mcError_t wcudaGraphAddMemcpyNodeFromSymbol( mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, void *dst, const T &symbol, size_t count, size_t offset, mcMemcpyKind kind) { return wcudaGraphAddMemcpyNodeFromSymbol(pGraphNode, graph, pDependencies, numDependencies, dst, (const void *)&symbol, count, offset, kind);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddMemcpyNodeFromSymbol(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, void *dst, const void *symbol, size_t count, size_t offset, mcMemcpyKind kind);
 */
/* cudaGraphAddMemcpyNodeFromSymbol -> wcudaGraphAddMemcpyNodeFromSymbol */

/*
 * CUDA API: cudaGraphAddMemcpyNodeToSymbol
 * Reference wrapper: wcudaGraphAddMemcpyNodeToSymbol
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static __inline__ __host__ mcError_t wcudaGraphAddMemcpyNodeToSymbol( mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, const T &symbol, const void *src, size_t count, size_t offset, mcMemcpyKind kind) { return wcudaGraphAddMemcpyNodeToSymbol(pGraphNode, graph, pDependencies, numDependencies, (const void *)&symbol, src, count, offset, kind);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddMemcpyNodeToSymbol(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, const void *symbol, const void *src, size_t count, size_t offset, mcMemcpyKind kind);
 */
/* cudaGraphAddMemcpyNodeToSymbol -> wcudaGraphAddMemcpyNodeToSymbol */

/*
 * CUDA API: cudaGraphAddMemsetNode
 * Reference wrapper: wcudaGraphAddMemsetNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddMemsetNode(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, const mcMemsetParams *pMemsetParams);
 */
/* cudaGraphAddMemsetNode -> wcudaGraphAddMemsetNode */

/*
 * CUDA API: cudaGraphAddNode
 * Reference wrapper: wcudaGraphAddNode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddNode(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, size_t numDependencies, mcGraphNodeParams *nodeParams);
 */
/* cudaGraphAddNode -> wcudaGraphAddNode */

/*
 * CUDA API: cudaGraphAddNode_v2
 * Reference wrapper: wcudaGraphAddNode_v2
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphAddNode_v2(mcGraphNode_t *pGraphNode, mcGraph_t graph, const mcGraphNode_t *pDependencies, const mcGraphEdgeData *dependencyData, size_t numDependencies, mcGraphNodeParams *nodeParams);
 */
/* cudaGraphAddNode_v2 -> wcudaGraphAddNode_v2 */

/*
 * CUDA API: cudaGraphChildGraphNodeGetGraph
 * Reference wrapper: wcudaGraphChildGraphNodeGetGraph
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphChildGraphNodeGetGraph(mcGraphNode_t node, mcGraph_t *pGraph);
 */
/* cudaGraphChildGraphNodeGetGraph -> wcudaGraphChildGraphNodeGetGraph */

/*
 * CUDA API: cudaGraphConditionalHandleCreate
 * Reference wrapper: wcudaGraphConditionalHandleCreate
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphConditionalHandleCreate(mcGraphConditionalHandle *pHandle_out, mcGraph_t graph, unsigned int defaultLaunchValue __dv(0), unsigned int flags __dv(0));
 */
/* cudaGraphConditionalHandleCreate -> wcudaGraphConditionalHandleCreate */

/*
 * CUDA API: cudaGraphDebugDotPrint
 * Reference wrapper: wcudaGraphDebugDotPrint
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphDebugDotPrint(mcGraph_t graph, const char *path, unsigned int flags);
 */
/* cudaGraphDebugDotPrint -> wcudaGraphDebugDotPrint */

/*
 * CUDA API: cudaGraphEventRecordNodeGetEvent
 * Reference wrapper: wcudaGraphEventRecordNodeGetEvent
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphEventRecordNodeGetEvent(mcGraphNode_t node, mcEvent_t *event_out);
 */
/* cudaGraphEventRecordNodeGetEvent -> wcudaGraphEventRecordNodeGetEvent */

/*
 * CUDA API: cudaGraphEventRecordNodeSetEvent
 * Reference wrapper: wcudaGraphEventRecordNodeSetEvent
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphEventRecordNodeSetEvent(mcGraphNode_t node, mcEvent_t event);
 */
/* cudaGraphEventRecordNodeSetEvent -> wcudaGraphEventRecordNodeSetEvent */

/*
 * CUDA API: cudaGraphEventWaitNodeGetEvent
 * Reference wrapper: wcudaGraphEventWaitNodeGetEvent
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphEventWaitNodeGetEvent(mcGraphNode_t node, mcEvent_t *event_out);
 */
/* cudaGraphEventWaitNodeGetEvent -> wcudaGraphEventWaitNodeGetEvent */

/*
 * CUDA API: cudaGraphEventWaitNodeSetEvent
 * Reference wrapper: wcudaGraphEventWaitNodeSetEvent
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphEventWaitNodeSetEvent(mcGraphNode_t node, mcEvent_t event);
 */
/* cudaGraphEventWaitNodeSetEvent -> wcudaGraphEventWaitNodeSetEvent */

/*
 * CUDA API: cudaGraphExecChildGraphNodeSetParams
 * Reference wrapper: wcudaGraphExecChildGraphNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecChildGraphNodeSetParams(mcGraphExec_t hGraphExec, mcGraphNode_t node, mcGraph_t childGraph);
 */
/* cudaGraphExecChildGraphNodeSetParams -> wcudaGraphExecChildGraphNodeSetParams */

/*
 * CUDA API: cudaGraphExecEventRecordNodeSetEvent
 * Reference wrapper: wcudaGraphExecEventRecordNodeSetEvent
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecEventRecordNodeSetEvent(mcGraphExec_t hGraphExec, mcGraphNode_t hNode, mcEvent_t event);
 */
/* cudaGraphExecEventRecordNodeSetEvent -> wcudaGraphExecEventRecordNodeSetEvent */

/*
 * CUDA API: cudaGraphExecEventWaitNodeSetEvent
 * Reference wrapper: wcudaGraphExecEventWaitNodeSetEvent
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecEventWaitNodeSetEvent(mcGraphExec_t hGraphExec, mcGraphNode_t hNode, mcEvent_t event);
 */
/* cudaGraphExecEventWaitNodeSetEvent -> wcudaGraphExecEventWaitNodeSetEvent */

/*
 * CUDA API: cudaGraphExecExternalSemaphoresSignalNodeSetParams
 * Reference wrapper: wcudaGraphExecExternalSemaphoresSignalNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecExternalSemaphoresSignalNodeSetParams( mcGraphExec_t hGraphExec, mcGraphNode_t hNode, const mcExternalSemaphoreSignalNodeParams *nodeParams);
 */
/* cudaGraphExecExternalSemaphoresSignalNodeSetParams -> wcudaGraphExecExternalSemaphoresSignalNodeSetParams */

/*
 * CUDA API: cudaGraphExecExternalSemaphoresWaitNodeSetParams
 * Reference wrapper: wcudaGraphExecExternalSemaphoresWaitNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecExternalSemaphoresWaitNodeSetParams( mcGraphExec_t hGraphExec, mcGraphNode_t hNode, const mcExternalSemaphoreWaitNodeParams *nodeParams);
 */
/* cudaGraphExecExternalSemaphoresWaitNodeSetParams -> wcudaGraphExecExternalSemaphoresWaitNodeSetParams */

/*
 * CUDA API: cudaGraphExecGetFlags
 * Reference wrapper: wcudaGraphExecGetFlags
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecGetFlags(mcGraphExec_t graphExec, unsigned long long *flags);
 */
/* cudaGraphExecGetFlags -> wcudaGraphExecGetFlags */

/*
 * CUDA API: cudaGraphExecHostNodeSetParams
 * Reference wrapper: wcudaGraphExecHostNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecHostNodeSetParams(mcGraphExec_t hGraphExec, mcGraphNode_t node, const mcHostNodeParams *pNodeParams);
 */
/* cudaGraphExecHostNodeSetParams -> wcudaGraphExecHostNodeSetParams */

/*
 * CUDA API: cudaGraphExecKernelNodeSetParams
 * Reference wrapper: wcudaGraphExecKernelNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecKernelNodeSetParams(mcGraphExec_t hGraphExec, mcGraphNode_t node, const mcKernelNodeParams *pNodeParams);
 */
/* cudaGraphExecKernelNodeSetParams -> wcudaGraphExecKernelNodeSetParams */

/*
 * CUDA API: cudaGraphExecMemcpyNodeSetParams
 * Reference wrapper: wcudaGraphExecMemcpyNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecMemcpyNodeSetParams(mcGraphExec_t hGraphExec, mcGraphNode_t node, const struct mcMemcpy3DParms *pNodeParams);
 */
/* cudaGraphExecMemcpyNodeSetParams -> wcudaGraphExecMemcpyNodeSetParams */

/*
 * CUDA API: cudaGraphExecMemcpyNodeSetParams1D
 * Reference wrapper: wcudaGraphExecMemcpyNodeSetParams1D
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecMemcpyNodeSetParams1D(mcGraphExec_t hGraphExec, mcGraphNode_t node, void *dst, const void *src, size_t count, mcMemcpyKind kind);
 */
/* cudaGraphExecMemcpyNodeSetParams1D -> wcudaGraphExecMemcpyNodeSetParams1D */

/*
 * CUDA API: cudaGraphExecMemcpyNodeSetParamsFromSymbol
 * Reference wrapper: wcudaGraphExecMemcpyNodeSetParamsFromSymbol
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static __inline__ __host__ mcError_t wcudaGraphExecMemcpyNodeSetParamsFromSymbol( mcGraphExec_t hGraphExec, mcGraphNode_t node, void *dst, const T &symbol, size_t count, size_t offset, mcMemcpyKind kind) { return wcudaGraphExecMemcpyNodeSetParamsFromSymbol(hGraphExec, node, dst, (const void *)&symbol, count, offset, kind);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecMemcpyNodeSetParamsFromSymbol(mcGraphExec_t hGraphExec, mcGraphNode_t node, void *dst, const void *symbol, size_t count, size_t offset, mcMemcpyKind kind);
 */
/* cudaGraphExecMemcpyNodeSetParamsFromSymbol -> wcudaGraphExecMemcpyNodeSetParamsFromSymbol */

/*
 * CUDA API: cudaGraphExecMemcpyNodeSetParamsToSymbol
 * Reference wrapper: wcudaGraphExecMemcpyNodeSetParamsToSymbol
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static __inline__ __host__ mcError_t wcudaGraphExecMemcpyNodeSetParamsToSymbol( mcGraphExec_t hGraphExec, mcGraphNode_t node, const T &symbol, const void *src, size_t count, size_t offset, mcMemcpyKind kind) { return wcudaGraphExecMemcpyNodeSetParamsToSymbol(hGraphExec, node, (const void *)&symbol, src, count, offset, kind);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecMemcpyNodeSetParamsToSymbol(mcGraphExec_t hGraphExec, mcGraphNode_t node, const void *symbol, const void *src, size_t count, size_t offset, mcMemcpyKind kind);
 */
/* cudaGraphExecMemcpyNodeSetParamsToSymbol -> wcudaGraphExecMemcpyNodeSetParamsToSymbol */

/*
 * CUDA API: cudaGraphExecMemsetNodeSetParams
 * Reference wrapper: wcudaGraphExecMemsetNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecMemsetNodeSetParams(mcGraphExec_t hGraphExec, mcGraphNode_t node, const mcMemsetParams *pNodeParams);
 */
/* cudaGraphExecMemsetNodeSetParams -> wcudaGraphExecMemsetNodeSetParams */

/*
 * CUDA API: cudaGraphExecUpdate_V2
 * Reference wrapper: wcudaGraphExecUpdate_V2
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } static __inline__ __host__ mcError_t wcudaGraphExecUpdate_V2(mcGraphExec_t hGraphExec, mcGraph_t hGraph, mcGraphNode_t *hErrorNode_out, mcGraphExecUpdateResult *updateResult_out) { if(updateResult_out == nullptr){ return mcErrorInvalidValue;
 * Reference prototype: mcError_t status = wcudaGraphExecUpdate_V2(hGraphExec, hGraph, &resultInfo);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExecUpdate_V2(mcGraphExec_t hGraphExec, mcGraph_t hGraph, mcGraphExecUpdateResultInfo *resultInfo);
 */
/* cudaGraphExecUpdate_V2 -> wcudaGraphExecUpdate_V2 */

/*
 * CUDA API: cudaGraphExternalSemaphoresSignalNodeGetParams
 * Reference wrapper: wcudaGraphExternalSemaphoresSignalNodeGetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExternalSemaphoresSignalNodeGetParams( mcGraphNode_t hNode, mcExternalSemaphoreSignalNodeParams *params_out);
 */
/* cudaGraphExternalSemaphoresSignalNodeGetParams -> wcudaGraphExternalSemaphoresSignalNodeGetParams */

/*
 * CUDA API: cudaGraphExternalSemaphoresSignalNodeSetParams
 * Reference wrapper: wcudaGraphExternalSemaphoresSignalNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExternalSemaphoresSignalNodeSetParams( mcGraphNode_t hNode, const mcExternalSemaphoreSignalNodeParams *nodeParams);
 */
/* cudaGraphExternalSemaphoresSignalNodeSetParams -> wcudaGraphExternalSemaphoresSignalNodeSetParams */

/*
 * CUDA API: cudaGraphExternalSemaphoresWaitNodeGetParams
 * Reference wrapper: wcudaGraphExternalSemaphoresWaitNodeGetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExternalSemaphoresWaitNodeGetParams( mcGraphNode_t hNode, mcExternalSemaphoreWaitNodeParams *params_out);
 */
/* cudaGraphExternalSemaphoresWaitNodeGetParams -> wcudaGraphExternalSemaphoresWaitNodeGetParams */

/*
 * CUDA API: cudaGraphExternalSemaphoresWaitNodeSetParams
 * Reference wrapper: wcudaGraphExternalSemaphoresWaitNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphExternalSemaphoresWaitNodeSetParams( mcGraphNode_t hNode, const mcExternalSemaphoreWaitNodeParams *nodeParams);
 */
/* cudaGraphExternalSemaphoresWaitNodeSetParams -> wcudaGraphExternalSemaphoresWaitNodeSetParams */

/*
 * CUDA API: cudaGraphGetEdges
 * Reference wrapper: wcudaGraphGetEdges
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphGetEdges(mcGraph_t graph, mcGraphNode_t *from, mcGraphNode_t *to, size_t *numEdges);
 */
/* cudaGraphGetEdges -> wcudaGraphGetEdges */

/*
 * CUDA API: cudaGraphGetEdges_v2
 * Reference wrapper: wcudaGraphGetEdges_v2
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphGetEdges_v2(mcGraph_t graph, mcGraphNode_t *from, mcGraphNode_t *to, mcGraphEdgeData *edgeData, size_t *numEdges);
 */
/* cudaGraphGetEdges_v2 -> wcudaGraphGetEdges_v2 */

/*
 * CUDA API: cudaGraphHostNodeGetParams
 * Reference wrapper: wcudaGraphHostNodeGetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphHostNodeGetParams(mcGraphNode_t node, mcHostNodeParams *pNodeParams);
 */
/* cudaGraphHostNodeGetParams -> wcudaGraphHostNodeGetParams */

/*
 * CUDA API: cudaGraphHostNodeSetParams
 * Reference wrapper: wcudaGraphHostNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphHostNodeSetParams(mcGraphNode_t node, const mcHostNodeParams *pNodeParams);
 */
/* cudaGraphHostNodeSetParams -> wcudaGraphHostNodeSetParams */

/*
 * CUDA API: cudaGraphInstantiateWithFlags
 * Reference wrapper: wcudaGraphInstantiateWithFlags
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphInstantiateWithFlags(mcGraphExec_t *pGraphExec, mcGraph_t graph, unsigned long long flags);
 */
/* cudaGraphInstantiateWithFlags -> wcudaGraphInstantiateWithFlags */

/*
 * CUDA API: cudaGraphInstantiateWithParams
 * Reference wrapper: wcudaGraphInstantiateWithParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphInstantiateWithParams(mcGraphExec_t *pGraphExec, mcGraph_t graph, mcGraphInstantiateParams *instantiateParams);
 */
/* cudaGraphInstantiateWithParams -> wcudaGraphInstantiateWithParams */

/*
 * CUDA API: cudaGraphInstantiate_V2
 * Reference wrapper: wcudaGraphInstantiate_V2
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } static __inline__ __host__ mcError_t wcudaGraphInstantiate_V2(mcGraphExec_t *pGraphExec, mcGraph_t graph, mcGraphNode_t *pErrorNode, char *pLogBuffer, size_t bufferSize) { (void)pErrorNode;
 * Reference prototype: mcError_t status = wcudaGraphInstantiate_V2(pGraphExec, graph, 0);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphInstantiate_V2(mcGraphExec_t *pGraphExec, mcGraph_t graph, unsigned long long flags __dv(0));
 */
/* cudaGraphInstantiate_V2 -> wcudaGraphInstantiate_V2 */

/*
 * CUDA API: cudaGraphKernelNodeCopyAttributes
 * Reference wrapper: wcudaGraphKernelNodeCopyAttributes
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphKernelNodeCopyAttributes(mcGraphNode_t hDst, mcGraphNode_t hSrc);
 */
/* cudaGraphKernelNodeCopyAttributes -> wcudaGraphKernelNodeCopyAttributes */

/*
 * CUDA API: cudaGraphKernelNodeGetAttribute
 * Reference wrapper: wcudaGraphKernelNodeGetAttribute
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphKernelNodeGetAttribute(mcGraphNode_t hNode, mcKernelNodeAttrID attr, mcKernelNodeAttrValue *value_out);
 */
/* cudaGraphKernelNodeGetAttribute -> wcudaGraphKernelNodeGetAttribute */

/*
 * CUDA API: cudaGraphKernelNodeGetParams
 * Reference wrapper: wcudaGraphKernelNodeGetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphKernelNodeGetParams(mcGraphNode_t node, mcKernelNodeParams *pNodeParams);
 */
/* cudaGraphKernelNodeGetParams -> wcudaGraphKernelNodeGetParams */

/*
 * CUDA API: cudaGraphKernelNodeSetAttribute
 * Reference wrapper: wcudaGraphKernelNodeSetAttribute
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphKernelNodeSetAttribute(mcGraphNode_t hNode, mcKernelNodeAttrID attr, const mcKernelNodeAttrValue *value);
 */
/* cudaGraphKernelNodeSetAttribute -> wcudaGraphKernelNodeSetAttribute */

/*
 * CUDA API: cudaGraphKernelNodeSetParams
 * Reference wrapper: wcudaGraphKernelNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphKernelNodeSetParams(mcGraphNode_t node, const mcKernelNodeParams *pNodeParams);
 */
/* cudaGraphKernelNodeSetParams -> wcudaGraphKernelNodeSetParams */

/*
 * CUDA API: cudaGraphMemAllocNodeGetParams
 * Reference wrapper: wcudaGraphMemAllocNodeGetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphMemAllocNodeGetParams(mcGraphNode_t node, struct mcMemAllocNodeParams *params_out);
 */
/* cudaGraphMemAllocNodeGetParams -> wcudaGraphMemAllocNodeGetParams */

/*
 * CUDA API: cudaGraphMemFreeNodeGetParams
 * Reference wrapper: wcudaGraphMemFreeNodeGetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphMemFreeNodeGetParams(mcGraphNode_t node, void *dptr_out);
 */
/* cudaGraphMemFreeNodeGetParams -> wcudaGraphMemFreeNodeGetParams */

/*
 * CUDA API: cudaGraphMemcpyNodeGetParams
 * Reference wrapper: wcudaGraphMemcpyNodeGetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphMemcpyNodeGetParams(mcGraphNode_t node, struct mcMemcpy3DParms *pNodeParams);
 */
/* cudaGraphMemcpyNodeGetParams -> wcudaGraphMemcpyNodeGetParams */

/*
 * CUDA API: cudaGraphMemcpyNodeSetParams
 * Reference wrapper: wcudaGraphMemcpyNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphMemcpyNodeSetParams(mcGraphNode_t node, const struct mcMemcpy3DParms *pNodeParams);
 */
/* cudaGraphMemcpyNodeSetParams -> wcudaGraphMemcpyNodeSetParams */

/*
 * CUDA API: cudaGraphMemcpyNodeSetParamsFromSymbol
 * Reference wrapper: wcudaGraphMemcpyNodeSetParamsFromSymbol
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static __inline__ __host__ mcError_t wcudaGraphMemcpyNodeSetParamsFromSymbol( mcGraphNode_t node, void *dst, const T &symbol, size_t count, size_t offset, mcMemcpyKind kind) { return wcudaGraphMemcpyNodeSetParamsFromSymbol(node, dst, (const void *)&symbol, count, offset, kind);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphMemcpyNodeSetParamsFromSymbol(mcGraphNode_t node, void *dst, const void *symbol, size_t count, size_t offset, mcMemcpyKind kind);
 */
/* cudaGraphMemcpyNodeSetParamsFromSymbol -> wcudaGraphMemcpyNodeSetParamsFromSymbol */

/*
 * CUDA API: cudaGraphMemcpyNodeSetParamsToSymbol
 * Reference wrapper: wcudaGraphMemcpyNodeSetParamsToSymbol
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static __inline__ __host__ mcError_t wcudaGraphMemcpyNodeSetParamsToSymbol(mcGraphNode_t node, const T &symbol, const void *src, size_t count, size_t offset, mcMemcpyKind kind) { return wcudaGraphMemcpyNodeSetParamsToSymbol(node, (const void *)&symbol, src, count, offset, kind);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphMemcpyNodeSetParamsToSymbol(mcGraphNode_t node, const void *symbol, const void *src, size_t count, size_t offset, mcMemcpyKind kind);
 */
/* cudaGraphMemcpyNodeSetParamsToSymbol -> wcudaGraphMemcpyNodeSetParamsToSymbol */

/*
 * CUDA API: cudaGraphMemsetNodeGetParams
 * Reference wrapper: wcudaGraphMemsetNodeGetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphMemsetNodeGetParams(mcGraphNode_t node, mcMemsetParams *pNodeParams);
 */
/* cudaGraphMemsetNodeGetParams -> wcudaGraphMemsetNodeGetParams */

/*
 * CUDA API: cudaGraphMemsetNodeSetParams
 * Reference wrapper: wcudaGraphMemsetNodeSetParams
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphMemsetNodeSetParams(mcGraphNode_t node, const mcMemsetParams *pNodeParams);
 */
/* cudaGraphMemsetNodeSetParams -> wcudaGraphMemsetNodeSetParams */

/*
 * CUDA API: cudaGraphNodeFindInClone
 * Reference wrapper: wcudaGraphNodeFindInClone
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphNodeFindInClone(mcGraphNode_t *pNode, mcGraphNode_t originalNode, mcGraph_t clonedGraph);
 */
/* cudaGraphNodeFindInClone -> wcudaGraphNodeFindInClone */

/*
 * CUDA API: cudaGraphNodeGetDependencies
 * Reference wrapper: wcudaGraphNodeGetDependencies
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphNodeGetDependencies(mcGraphNode_t node, mcGraphNode_t *pDependencies, size_t *pNumDependencies);
 */
/* cudaGraphNodeGetDependencies -> wcudaGraphNodeGetDependencies */

/*
 * CUDA API: cudaGraphNodeGetDependencies_v2
 * Reference wrapper: wcudaGraphNodeGetDependencies_v2
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphNodeGetDependencies_v2(mcGraphNode_t node, mcGraphNode_t *pDependencies, mcGraphEdgeData *edgeData, size_t *pNumDependencies);
 */
/* cudaGraphNodeGetDependencies_v2 -> wcudaGraphNodeGetDependencies_v2 */

/*
 * CUDA API: cudaGraphNodeGetDependentNodes
 * Reference wrapper: wcudaGraphNodeGetDependentNodes
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphNodeGetDependentNodes(mcGraphNode_t node, mcGraphNode_t *pDependentNodes, size_t *pNumDependentNodes);
 */
/* cudaGraphNodeGetDependentNodes -> wcudaGraphNodeGetDependentNodes */

/*
 * CUDA API: cudaGraphNodeGetDependentNodes_v2
 * Reference wrapper: wcudaGraphNodeGetDependentNodes_v2
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaGraphNodeGetDependentNodes_v2(mcGraphNode_t node, mcGraphNode_t *pDependentNodes, mcGraphEdgeData *edgeData, size_t *pNumDependentNodes);
 */
/* cudaGraphNodeGetDependentNodes_v2 -> wcudaGraphNodeGetDependentNodes_v2 */

/*
 * CUDA API: cudaGraphNodeGetEnabled
 * Reference wrapper: wcudaGraphNodeGetEnabled
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphNodeGetEnabled(mcGraphExec_t hGraphExec, mcGraphNode_t hNode, unsigned int *isEnabled);
 */
/* cudaGraphNodeGetEnabled -> wcudaGraphNodeGetEnabled */

/*
 * CUDA API: cudaGraphNodeGetType
 * Reference wrapper: wcudaGraphNodeGetType
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphNodeGetType(mcGraphNode_t node, mcGraphNodeType *pType);
 */
/* cudaGraphNodeGetType -> wcudaGraphNodeGetType */

/*
 * CUDA API: cudaGraphNodeSetEnabled
 * Reference wrapper: wcudaGraphNodeSetEnabled
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphNodeSetEnabled(mcGraphExec_t hGraphExec, mcGraphNode_t hNode, unsigned int isEnabled);
 */
/* cudaGraphNodeSetEnabled -> wcudaGraphNodeSetEnabled */

/*
 * CUDA API: cudaGraphReleaseUserObject
 * Reference wrapper: wcudaGraphReleaseUserObject
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphReleaseUserObject(mcGraph_t graph, mcUserObject_t object, unsigned int count __dv(1));
 */
/* cudaGraphReleaseUserObject -> wcudaGraphReleaseUserObject */

/*
 * CUDA API: cudaGraphRemoveDependencies
 * Reference wrapper: wcudaGraphRemoveDependencies
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphRemoveDependencies(mcGraph_t graph, const mcGraphNode_t *from, const mcGraphNode_t *to, size_t numDependencies);
 */
/* cudaGraphRemoveDependencies -> wcudaGraphRemoveDependencies */

/*
 * CUDA API: cudaGraphRemoveDependencies_v2
 * Reference wrapper: wcudaGraphRemoveDependencies_v2
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaGraphRemoveDependencies_v2(mcGraph_t graph, const mcGraphNode_t *from, const mcGraphNode_t *to, const mcGraphEdgeData *edgeData, size_t numDependencies);
 */
/* cudaGraphRemoveDependencies_v2 -> wcudaGraphRemoveDependencies_v2 */

/*
 * CUDA API: cudaGraphRetainUserObject
 * Reference wrapper: wcudaGraphRetainUserObject
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphRetainUserObject(mcGraph_t graph, mcUserObject_t object, unsigned int count __dv(1), unsigned int flags __dv(0));
 */
/* cudaGraphRetainUserObject -> wcudaGraphRetainUserObject */

/*
 * CUDA API: cudaGraphUpload
 * Reference wrapper: wcudaGraphUpload
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphUpload(mcGraphExec_t graphExec, mcStream_t stream);
 */
/* cudaGraphUpload -> wcudaGraphUpload */

/*
 * CUDA API: cudaGraphicsEGLRegisterImage
 * Reference wrapper: wcudaGraphicsEGLRegisterImage
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: #ifndef __CUDA_EGL_WRAPPER_H__ #define __CUDA_EGL_WRAPPER_H__ #include "mcEGL.h" #include "mc_egl_interop.h" mcError_t wcudaGraphicsEGLRegisterImage(struct mcGraphicsResource **pMacaResource, EGLImageKHR image, unsigned int flags);
 */
/* cudaGraphicsEGLRegisterImage -> wcudaGraphicsEGLRegisterImage */

/*
 * CUDA API: cudaGraphicsGLRegisterBuffer
 * Reference wrapper: wcudaGraphicsGLRegisterBuffer
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } inline mcError_t wcudaGraphicsGLRegisterBuffer(struct mcGraphicsResource **resource, GLuint buffer, unsigned int flags) { return mcErrorNotSupported;
 */
/* cudaGraphicsGLRegisterBuffer -> wcudaGraphicsGLRegisterBuffer */

/*
 * CUDA API: cudaGraphicsGLRegisterImage
 * Reference wrapper: wcudaGraphicsGLRegisterImage
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: inline mcError_t wcudaGraphicsGLRegisterImage(struct mcGraphicsResource **resource, GLuint image, GLenum target, unsigned int flags) { return mcErrorNotSupported;
 */
/* cudaGraphicsGLRegisterImage -> wcudaGraphicsGLRegisterImage */

/*
 * CUDA API: cudaGraphicsMapResources
 * Reference wrapper: wcudaGraphicsMapResources
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphicsMapResources(int count, mcGraphicsResource_t *resources, mcStream_t stream __dv(0));
 */
/* cudaGraphicsMapResources -> wcudaGraphicsMapResources */

/*
 * CUDA API: cudaGraphicsResourceGetMappedEglFrame
 * Reference wrapper: wcudaGraphicsResourceGetMappedEglFrame
 * Reference files: runtime/cuda_egl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t wcudaGraphicsResourceGetMappedEglFrame(mcEglFrame *eglFrame, mcGraphicsResource_t resource, unsigned int index, unsigned int mipLevel);
 */
/* cudaGraphicsResourceGetMappedEglFrame -> wcudaGraphicsResourceGetMappedEglFrame */

/*
 * CUDA API: cudaGraphicsResourceGetMappedMipmappedArray
 * Reference wrapper: wcudaGraphicsResourceGetMappedMipmappedArray
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphicsResourceGetMappedMipmappedArray( mcMipmappedArray_t *mipmappedArray, mcGraphicsResource_t resource);
 */
/* cudaGraphicsResourceGetMappedMipmappedArray -> wcudaGraphicsResourceGetMappedMipmappedArray */

/*
 * CUDA API: cudaGraphicsResourceGetMappedPointer
 * Reference wrapper: wcudaGraphicsResourceGetMappedPointer
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphicsResourceGetMappedPointer(void **devPtr, size_t *size, mcGraphicsResource_t resource);
 */
/* cudaGraphicsResourceGetMappedPointer -> wcudaGraphicsResourceGetMappedPointer */

/*
 * CUDA API: cudaGraphicsResourceSetMapFlags
 * Reference wrapper: wcudaGraphicsResourceSetMapFlags
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphicsResourceSetMapFlags(mcGraphicsResource_t resource, unsigned int flags);
 */
/* cudaGraphicsResourceSetMapFlags -> wcudaGraphicsResourceSetMapFlags */

/*
 * CUDA API: cudaGraphicsSubResourceGetMappedArray
 * Reference wrapper: wcudaGraphicsSubResourceGetMappedArray
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphicsSubResourceGetMappedArray(mcArray_t *array, mcGraphicsResource_t resource, unsigned int arrayIndex, unsigned int mipLevel);
 */
/* cudaGraphicsSubResourceGetMappedArray -> wcudaGraphicsSubResourceGetMappedArray */

/*
 * CUDA API: cudaGraphicsUnmapResources
 * Reference wrapper: wcudaGraphicsUnmapResources
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphicsUnmapResources(int count, mcGraphicsResource_t *resources, mcStream_t stream __dv(0));
 */
/* cudaGraphicsUnmapResources -> wcudaGraphicsUnmapResources */

/*
 * CUDA API: cudaGraphicsUnregisterResource
 * Reference wrapper: wcudaGraphicsUnregisterResource
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGraphicsUnregisterResource(mcGraphicsResource_t resource);
 */
/* cudaGraphicsUnregisterResource -> wcudaGraphicsUnregisterResource */

/*
 * CUDA API: cudaGraphicsVDPAURegisterOutputSurface
 * Reference wrapper: wcudaGraphicsVDPAURegisterOutputSurface
 * Reference files: video/cuda_vdpau_interop_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __host__ mcError_t wcudaGraphicsVDPAURegisterOutputSurface(struct cudaGraphicsResource **resource, VdpOutputSurface vdpSurface, unsigned int flags) { return mcErrorNotSupported;
 */
/* cudaGraphicsVDPAURegisterOutputSurface -> wcudaGraphicsVDPAURegisterOutputSurface */

/*
 * CUDA API: cudaGraphicsVDPAURegisterVideoSurface
 * Reference wrapper: wcudaGraphicsVDPAURegisterVideoSurface
 * Reference files: video/cuda_vdpau_interop_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __host__ mcError_t wcudaGraphicsVDPAURegisterVideoSurface(struct cudaGraphicsResource **resource, VdpVideoSurface vdpSurface, unsigned int flags) { return mcErrorNotSupported;
 */
/* cudaGraphicsVDPAURegisterVideoSurface -> wcudaGraphicsVDPAURegisterVideoSurface */

/*
 * CUDA API: cudaGreenCtxCreate
 * Reference wrapper: wcudaGreenCtxCreate
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaGreenCtxCreate(mcExecutionContext_t *phCtx, mcDevResourceDesc_t desc, int device, unsigned int flags);
 */
/* cudaGreenCtxCreate -> wcudaGreenCtxCreate */

/*
 * CUDA API: cudaImportExternalSemaphore
 * Reference wrapper: wcudaImportExternalSemaphore
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaImportExternalSemaphore( mcExternalSemaphore_t *extSem_out, const mcExternalSemaphoreHandleDesc *semHandleDesc);
 */
/* cudaImportExternalSemaphore -> wcudaImportExternalSemaphore */

/*
 * CUDA API: cudaLaunchCooperativeKernelMultiDevice
 * Reference wrapper: wcudaLaunchCooperativeKernelMultiDevice
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaLaunchCooperativeKernelMultiDevice(mcLaunchParams *launchParamsList, unsigned int numDevices, unsigned flags __dv(0));
 */
/* cudaLaunchCooperativeKernelMultiDevice -> wcudaLaunchCooperativeKernelMultiDevice */

/*
 * CUDA API: cudaLaunchDevice
 * Reference wrapper: wcudaLaunchDevice
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaLaunchDevice(void *func, void *parameterBuffer, dim3 gridDimension, dim3 blockDimension, unsigned int sharedMemSize, mcStream_t stream) { return mcLaunchDevice(func, parameterBuffer, gridDimension, blockDimension, sharedMemSize, stream);
 */
/* cudaLaunchDevice -> wcudaLaunchDevice */

/*
 * CUDA API: cudaLaunchDeviceV2
 * Reference wrapper: wcudaLaunchDeviceV2
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaLaunchDeviceV2(void *parameterBuffer, mcStream_t stream) { return mcLaunchDeviceV2(parameterBuffer, stream);
 */
/* cudaLaunchDeviceV2 -> wcudaLaunchDeviceV2 */

/*
 * CUDA API: cudaLaunchDeviceV2_ptsz
 * Reference wrapper: wcudaLaunchDeviceV2_ptsz
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaLaunchDeviceV2_ptsz( void *parameterBuffer __attribute__((unused)), mcStream_t stream __attribute__((unused))) { return mcErrorNotSupported;
 */
/* cudaLaunchDeviceV2_ptsz -> wcudaLaunchDeviceV2_ptsz */

/*
 * CUDA API: cudaLaunchDevice_ptsz
 * Reference wrapper: wcudaLaunchDevice_ptsz
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaLaunchDevice_ptsz( void *func __attribute__((unused)), void *parameterBuffer __attribute__((unused)), dim3 gridDimension __attribute__((unused)), dim3 blockDimension __attribute__((unused)), unsigned int sharedMemSize __attribute__((unused)), mcStream_t stream __attribute__((unused))) { return mcErrorNotSupported;
 */
/* cudaLaunchDevice_ptsz -> wcudaLaunchDevice_ptsz */

/*
 * CUDA API: cudaLaunchKernelEx
 * Reference wrapper: wcudaLaunchKernelEx
 * Reference files: runtime/cuda_runtime_template_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: template<typename... ExpTypes, typename... ActTypes> static __inline__ __host__ mcError_t wcudaLaunchKernelEx( const mcLaunchConfig_t* config, void (*kernel)(ExpTypes...), ActTypes&&... args) { std::tuple<ExpTypes...> coercedArgs{static_cast<ExpTypes>(std::forward<ActTypes>(args))...};
 */
/* cudaLaunchKernelEx -> wcudaLaunchKernelEx */

/*
 * CUDA API: cudaLaunchKernelExC
 * Reference wrapper: wcudaLaunchKernelExC
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static __inline__ __host__ mcError_t wcudaLaunchKernelExC(const mcLaunchConfig_t *config, const T *func, void **args) { return wcudaLaunchKernelExC(config, (const void *)func, args);
 * Reference prototype: return wcudaLaunchKernelExC(config, (const void*)kernel, pArgs);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaLaunchKernelExC(const mcLaunchConfig_t *config, const void *func, void **args);
 */
/* cudaLaunchKernelExC -> wcudaLaunchKernelExC */

/*
 * CUDA API: cudaMemAdvise_v2
 * Reference wrapper: wcudaMemAdvise_v2
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMemAdvise_v2(const void *dev_ptr, size_t count, mcMemoryAdvise_t advice, struct mcMemLocation location);
 */
/* cudaMemAdvise_v2 -> wcudaMemAdvise_v2 */

/*
 * CUDA API: cudaMemPoolExportPointer
 * Reference wrapper: wcudaMemPoolExportPointer
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMemPoolExportPointer(struct mcMemPoolPtrExportData *exportData, void *ptr);
 */
/* cudaMemPoolExportPointer -> wcudaMemPoolExportPointer */

/*
 * CUDA API: cudaMemPoolExportToShareableHandle
 * Reference wrapper: wcudaMemPoolExportToShareableHandle
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMemPoolExportToShareableHandle(void *shareableHandle, mcMemPool_t memPool, enum mcMemAllocationHandleType handleType, unsigned int flags);
 */
/* cudaMemPoolExportToShareableHandle -> wcudaMemPoolExportToShareableHandle */

/*
 * CUDA API: cudaMemPoolGetAccess
 * Reference wrapper: wcudaMemPoolGetAccess
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMemPoolGetAccess(enum mcMemAccessFlags *flags, mcMemPool_t memPool, struct mcMemLocation *location);
 */
/* cudaMemPoolGetAccess -> wcudaMemPoolGetAccess */

/*
 * CUDA API: cudaMemPoolImportFromShareableHandle
 * Reference wrapper: wcudaMemPoolImportFromShareableHandle
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMemPoolImportFromShareableHandle( mcMemPool_t *memPool, void *shareableHandle, enum mcMemAllocationHandleType handleType, unsigned int flags);
 */
/* cudaMemPoolImportFromShareableHandle -> wcudaMemPoolImportFromShareableHandle */

/*
 * CUDA API: cudaMemPoolImportPointer
 * Reference wrapper: wcudaMemPoolImportPointer
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMemPoolImportPointer(void **ptr, mcMemPool_t memPool, struct mcMemPoolPtrExportData *exportData);
 */
/* cudaMemPoolImportPointer -> wcudaMemPoolImportPointer */

/*
 * CUDA API: cudaMemPoolSetAccess
 * Reference wrapper: wcudaMemPoolSetAccess
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMemPoolSetAccess(mcMemPool_t memPool, const struct mcMemAccessDesc *descList, size_t count);
 */
/* cudaMemPoolSetAccess -> wcudaMemPoolSetAccess */

/*
 * CUDA API: cudaMemRangeGetAttribute
 * Reference wrapper: wcudaMemRangeGetAttribute
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMemRangeGetAttribute(void *data, size_t dataSize, mcMemRangeAttribute_t attribute, const void *devPtr, size_t count);
 */
/* cudaMemRangeGetAttribute -> wcudaMemRangeGetAttribute */

/*
 * CUDA API: cudaMemRangeGetAttributes
 * Reference wrapper: wcudaMemRangeGetAttributes
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMemRangeGetAttributes(void **data, size_t *dataSizes, mcMemRangeAttribute_t *attributes, size_t numAttributes, const void *devPtr, size_t count);
 */
/* cudaMemRangeGetAttributes -> wcudaMemRangeGetAttributes */

/*
 * CUDA API: cudaMemRemap
 * Reference wrapper: wcudaMemRemap
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMemRemap(void **hostPtr, mcDeviceptr_t ptr, size_t size, size_t offset, unsigned long long flags);
 */
/* cudaMemRemap -> wcudaMemRemap */

/*
 * CUDA API: cudaMemUnremap
 * Reference wrapper: wcudaMemUnremap
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMemUnremap(void *hostPtr, size_t size);
 */
/* cudaMemUnremap -> wcudaMemUnremap */

/*
 * CUDA API: cudaMemcpy2DAsync_ptsz
 * Reference wrapper: wcudaMemcpy2DAsync_ptsz
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaMemcpy2DAsync_ptsz( void *dst __attribute__((unused)), size_t dpitch __attribute__((unused)), const void *src __attribute__((unused)), size_t spitch __attribute__((unused)), size_t width __attribute__((unused)), size_t height __attribute__((unused)), enum _mcMemcpyKind kind __attribute__((unused)), mcStream_t stream __attribute__((unused))) { return mcErrorNotSupported;
 */
/* cudaMemcpy2DAsync_ptsz -> wcudaMemcpy2DAsync_ptsz */

/*
 * CUDA API: cudaMemcpy3DAsync_ptsz
 * Reference wrapper: wcudaMemcpy3DAsync_ptsz
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaMemcpy3DAsync_ptsz( const struct mcMemcpy3DParms *p __attribute__((unused)), mcStream_t stream __attribute__((unused))) { return mcErrorNotSupported;
 */
/* cudaMemcpy3DAsync_ptsz -> wcudaMemcpy3DAsync_ptsz */

/*
 * CUDA API: cudaMemcpyAsync_ptsz
 * Reference wrapper: wcudaMemcpyAsync_ptsz
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaMemcpyAsync_ptsz( void *dst __attribute__((unused)), const void *src __attribute__((unused)), size_t count __attribute__((unused)), enum _mcMemcpyKind kind __attribute__((unused)), mcStream_t stream __attribute__((unused))) { return mcErrorNotSupported;
 */
/* cudaMemcpyAsync_ptsz -> wcudaMemcpyAsync_ptsz */

/*
 * CUDA API: cudaMemset2DAsync_ptsz
 * Reference wrapper: wcudaMemset2DAsync_ptsz
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaMemset2DAsync_ptsz( void *devPtr __attribute__((unused)), size_t pitch __attribute__((unused)), int value __attribute__((unused)), size_t width __attribute__((unused)), size_t height __attribute__((unused)), mcStream_t stream __attribute__((unused))) { return mcErrorNotSupported;
 */
/* cudaMemset2DAsync_ptsz -> wcudaMemset2DAsync_ptsz */

/*
 * CUDA API: cudaMemset3DAsync_ptsz
 * Reference wrapper: wcudaMemset3DAsync_ptsz
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaMemset3DAsync_ptsz( struct mcPitchedPtr pitchedDevPtr __attribute__((unused)), int value __attribute__((unused)), struct mcExtent extent __attribute__((unused)), mcStream_t stream __attribute__((unused))) { return mcErrorNotSupported;
 */
/* cudaMemset3DAsync_ptsz -> wcudaMemset3DAsync_ptsz */

/*
 * CUDA API: cudaMemsetAsync_ptsz
 * Reference wrapper: wcudaMemsetAsync_ptsz
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaMemsetAsync_ptsz( void *devPtr __attribute__((unused)), int value __attribute__((unused)), size_t count __attribute__((unused)), mcStream_t stream __attribute__((unused))) { return mcErrorNotSupported;
 */
/* cudaMemsetAsync_ptsz -> wcudaMemsetAsync_ptsz */

/*
 * CUDA API: cudaMipmappedArrayGetMemoryRequirements
 * Reference wrapper: wcudaMipmappedArrayGetMemoryRequirements
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMipmappedArrayGetMemoryRequirements( mcArrayMemoryRequirements *memoryRequirements, mcMipmappedArray_t mipmap, int device);
 */
/* cudaMipmappedArrayGetMemoryRequirements -> wcudaMipmappedArrayGetMemoryRequirements */

/*
 * CUDA API: cudaMipmappedArrayGetSparseProperties
 * Reference wrapper: wcudaMipmappedArrayGetSparseProperties
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaMipmappedArrayGetSparseProperties( mcArraySparseProperties *sparseProperties, mcMipmappedArray_t mipmap);
 */
/* cudaMipmappedArrayGetSparseProperties -> wcudaMipmappedArrayGetSparseProperties */

/*
 * CUDA API: cudaOccupancyMaxActiveClusters
 * Reference wrapper: wcudaOccupancyMaxActiveClusters
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static __inline__ __host__ mcError_t wcudaOccupancyMaxActiveClusters(int *numClusters, T *func, const void *launchConfig) { return wcudaOccupancyMaxActiveClusters(numClusters, (const void *)func, launchConfig);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaOccupancyMaxActiveClusters(int *numClusters, const void *func, const void *launchConfig);
 */
/* cudaOccupancyMaxActiveClusters -> wcudaOccupancyMaxActiveClusters */

/*
 * CUDA API: cudaOccupancyMaxPotentialBlockSizeVariableSMem
 * Reference wrapper: wcudaOccupancyMaxPotentialBlockSizeVariableSMem
 * Reference files: runtime/cuda_runtime_template_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <typename UnaryFunction, class T> static __inline__ __host__ mcError_t wcudaOccupancyMaxPotentialBlockSizeVariableSMem( int *gridSize, int *blockSize, T f, UnaryFunction blockSizeToDynamicSMemSize, int blockSizeLimit = 0) { return wcudaOccupancyMaxPotentialBlockSizeVariableSMemWithFlags( gridSize, blockSize, f, blockSizeToDynamicSMemSize, blockSizeLimit, mcOccupancyDefault);
 */
/* cudaOccupancyMaxPotentialBlockSizeVariableSMem -> wcudaOccupancyMaxPotentialBlockSizeVariableSMem */

/*
 * CUDA API: cudaOccupancyMaxPotentialBlockSizeVariableSMemWithFlags
 * Reference wrapper: wcudaOccupancyMaxPotentialBlockSizeVariableSMemWithFlags
 * Reference files: runtime/cuda_runtime_template_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <typename UnaryFunction, class T> static inline mcError_t wcudaOccupancyMaxPotentialBlockSizeVariableSMemWithFlags( int *gridSize, int *blockSize, T f, UnaryFunction blockSizeToDynamicSMemSize, int blockSizeLimit = 0, unsigned int flags = 0) { mcError_t status;
 * Reference prototype: } template <typename UnaryFunction, class T> static __inline__ __host__ mcError_t wcudaOccupancyMaxPotentialBlockSizeVariableSMem( int *gridSize, int *blockSize, T f, UnaryFunction blockSizeToDynamicSMemSize, int blockSizeLimit = 0) { return wcudaOccupancyMaxPotentialBlockSizeVariableSMemWithFlags( gridSize, blockSize, f, blockSizeToDynamicSMemSize, blockSizeLimit, mcOccupancyDefault);
 */
/* cudaOccupancyMaxPotentialBlockSizeVariableSMemWithFlags -> wcudaOccupancyMaxPotentialBlockSizeVariableSMemWithFlags */

/*
 * CUDA API: cudaOccupancyMaxPotentialClusterSize
 * Reference wrapper: wcudaOccupancyMaxPotentialClusterSize
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static __inline__ __host__ mcError_t wcudaOccupancyMaxPotentialClusterSize(int *clusterSize, T *func, const void *launchConfig) { return wcudaOccupancyMaxPotentialClusterSize(clusterSize, (const void *)func, launchConfig);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaOccupancyMaxPotentialClusterSize(int *clusterSize, const void *func, const void *launchConfig);
 */
/* cudaOccupancyMaxPotentialClusterSize -> wcudaOccupancyMaxPotentialClusterSize */

/*
 * CUDA API: cudaProfilerInitialize
 * Reference wrapper: wcudaProfilerInitialize
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaProfilerInitialize(const char *configFile, const char *outputFile, enum mcOutputMode outputMode);
 */
/* cudaProfilerInitialize -> wcudaProfilerInitialize */

/*
 * CUDA API: cudaSetDoubleForDevice
 * Reference wrapper: wcudaSetDoubleForDevice
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaSetDoubleForDevice(double *d);
 */
/* cudaSetDoubleForDevice -> wcudaSetDoubleForDevice */

/*
 * CUDA API: cudaSetDoubleForHost
 * Reference wrapper: wcudaSetDoubleForHost
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaSetDoubleForHost(double *d);
 */
/* cudaSetDoubleForHost -> wcudaSetDoubleForHost */

/*
 * CUDA API: cudaSetValidDevices
 * Reference wrapper: wcudaSetValidDevices
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaSetValidDevices(int *deviceArr, int len);
 */
/* cudaSetValidDevices -> wcudaSetValidDevices */

/*
 * CUDA API: cudaSignalExternalSemaphoresAsync
 * Reference wrapper: wcudaSignalExternalSemaphoresAsync
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaSignalExternalSemaphoresAsync( const mcExternalSemaphore_t *extSemArray, const mcExternalSemaphoreSignalParams *paramsArray, unsigned int numExtSems, mcStream_t stream);
 */
/* cudaSignalExternalSemaphoresAsync -> wcudaSignalExternalSemaphoresAsync */

/*
 * CUDA API: cudaStreamBeginCaptureToGraph
 * Reference wrapper: wcudaStreamBeginCaptureToGraph
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaStreamBeginCaptureToGraph(mcStream_t stream, mcGraph_t graph, const mcGraphNode_t *dependencies, const mcGraphEdgeData *dependencyData, size_t numDependencies, mcStreamCaptureMode mode);
 */
/* cudaStreamBeginCaptureToGraph -> wcudaStreamBeginCaptureToGraph */

/*
 * CUDA API: cudaStreamGetDevResource
 * Reference wrapper: wcudaStreamGetDevResource
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaStreamGetDevResource(mcStream_t hStream, mcDrvDevResource *resource, mcDevResourceType type);
 */
/* cudaStreamGetDevResource -> wcudaStreamGetDevResource */

/*
 * CUDA API: cudaStreamUpdateCaptureDependencies
 * Reference wrapper: wcudaStreamUpdateCaptureDependencies
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaStreamUpdateCaptureDependencies(mcStream_t stream, mcGraphNode_t *dependencies, size_t numDependencies, unsigned int flags __dv(0));
 */
/* cudaStreamUpdateCaptureDependencies -> wcudaStreamUpdateCaptureDependencies */

/*
 * CUDA API: cudaStreamWaitEvent_ptsz
 * Reference wrapper: wcudaStreamWaitEvent_ptsz
 * Reference files: mckl/cuda_device_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __forceinline__ __device__ mcError_t wcudaStreamWaitEvent_ptsz( mcStream_t stream __attribute__((unused)), mcEvent_t event __attribute__((unused)), unsigned int flags __attribute__((unused))) { return mcErrorNotSupported;
 */
/* cudaStreamWaitEvent_ptsz -> wcudaStreamWaitEvent_ptsz */

/*
 * CUDA API: cudaThreadExchangeStreamCaptureMode
 * Reference wrapper: wcudaThreadExchangeStreamCaptureMode
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaThreadExchangeStreamCaptureMode(mcStreamCaptureMode *mode);
 */
/* cudaThreadExchangeStreamCaptureMode -> wcudaThreadExchangeStreamCaptureMode */

/*
 * CUDA API: cudaUnbindTexture
 * Reference wrapper: wcudaUnbindTexture
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T, int dim, enum mcTextureReadMode readMode> static __inline__ __host__ mcError_t wcudaUnbindTexture(const struct texture<T, dim, readMode> &tex) { return wcudaUnbindTexture(&tex);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaUnbindTexture(const struct textureReference *texref);
 */
/* cudaUnbindTexture -> wcudaUnbindTexture */

/*
 * CUDA API: cudaUserObjectCreate
 * Reference wrapper: wcudaUserObjectCreate
 * Reference files: runtime/cuda_runtime_template_wrapper.h, runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } template <class T> static __inline__ __host__ mcError_t wcudaUserObjectCreate(mcUserObject_t *object_out, T *objectToWrap, unsigned int initialRefcount, unsigned int flags) { return wcudaUserObjectCreate( object_out, objectToWrap, [](void *vpObj) { delete reinterpret_cast<T *>(vpObj); },
 * Reference prototype: } template <class T> static __inline__ __host__ mcError_t wcudaUserObjectCreate(mcUserObject_t *object_out, T *objectToWrap, unsigned int initialRefcount, enum mcUserObjectFlags flags) { return wcudaUserObjectCreate(object_out, objectToWrap, initialRefcount, (unsigned int)flags);
 * Reference prototype: mcError_t CUBRIDGE_API wcudaUserObjectCreate(mcUserObject_t *object_out, void *ptr, mcHostFn_t destroy, unsigned int initialRefcount, unsigned int flags);
 */
/* cudaUserObjectCreate -> wcudaUserObjectCreate */

/*
 * CUDA API: cudaUserObjectRelease
 * Reference wrapper: wcudaUserObjectRelease
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaUserObjectRelease(mcUserObject_t object, unsigned int count __dv(1));
 */
/* cudaUserObjectRelease -> wcudaUserObjectRelease */

/*
 * CUDA API: cudaUserObjectRetain
 * Reference wrapper: wcudaUserObjectRetain
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaUserObjectRetain(mcUserObject_t object, unsigned int count __dv(1));
 */
/* cudaUserObjectRetain -> wcudaUserObjectRetain */

/*
 * CUDA API: cudaVDPAUSetVDPAUDevice
 * Reference wrapper: wcudaVDPAUSetVDPAUDevice
 * Reference files: video/cuda_vdpau_interop_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } __host__ mcError_t wcudaVDPAUSetVDPAUDevice(int device, VdpDevice vdpDevice, VdpGetProcAddress *vdpGetProcAddress) { return mcErrorNotSupported;
 */
/* cudaVDPAUSetVDPAUDevice -> wcudaVDPAUSetVDPAUDevice */

/*
 * CUDA API: cudaWGLGetDevice
 * Reference wrapper: wcudaWGLGetDevice
 * Reference files: runtime/cuda_gl_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: } #ifdef _WIN32 mcError_t wcudaWGLGetDevice(int *device, HGPUNV hGpu);
 */
/* cudaWGLGetDevice -> wcudaWGLGetDevice */

/*
 * CUDA API: cudaWaitExternalSemaphoresAsync
 * Reference wrapper: wcudaWaitExternalSemaphoresAsync
 * Reference files: runtime/cuda_runtime_wrapper.h
 * CANN route: CANN Runtime/SIMT route must be verified against the installed toolkit
 * Status: RESERVED_NPU
 * Future implementation: include/bridge wrapper + typed CANN probe.
 * Reference prototype: mcError_t CUBRIDGE_API wcudaWaitExternalSemaphoresAsync( const mcExternalSemaphore_t *extSemArray, const mcExternalSemaphoreWaitParams *paramsArray, unsigned int numExtSems, mcStream_t stream);
 */
/* cudaWaitExternalSemaphoresAsync -> wcudaWaitExternalSemaphoresAsync */

#endif /* RESERVED_NPU */

#endif /* __CU2CCE_CUDA_TO_CCE_RESERVED_WRAPPER_PARITY_H__ */
