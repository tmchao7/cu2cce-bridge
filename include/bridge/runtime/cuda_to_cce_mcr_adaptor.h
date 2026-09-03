#ifndef __CUDA_TO_CCE_ADAPTOR_H__
#define __CUDA_TO_CCE_ADAPTOR_H__

/*
 * 核心宏替换表（Phase A1 最小核心集）。
 *
 * 仿照 cu-bridge 的 include/bridge/runtime/cuda_to_maca_mcr_adaptor.h：
 *   宏把用户代码里的 CUDA 符号（函数名、枚举值、错误码、类型）替换为
 *   cu2cce-bridge 的 wcuda wrapper / acl 原生符号。
 *
 * 本文件是"编译期宏替换"这一迁移线的核心。目标平台从 MXMACA(mc) 换成昇腾(acl)：
 *   - cudaMalloc   -> wcudaMalloc   （wrapper，签名保持 CUDA 形状）
 *   - cudaError_t  -> aclError      （原生 ACL 错误码类型）
 *   - cudaSuccess  -> ACL_SUCCESS   （原生 ACL 错误码）
 *
 * ⚠️ 说明：以下宏值/枚举值依据 CANN 文档（见 ASCEND_RESEARCH_NOTES.md §1-§2）编写，
 *    具体 ACL 枚举名（ACL_MEMCPY_HOST_TO_DEVICE 等）以真机 acl_base.h / acl_rt.h 为准，
 *    若命名不符，在真机上核对后批量替换即可，宏表结构不变。
 */

#include "acl/acl_base.h"
#include "acl/acl_rt.h"
#include "cuda_to_cce_reserved_parity.h"

//============================CUDA Runtime API====================//

//------------ error code ------------//
/* cudaError_t 是枚举，aclError 是 int32。宏替换后 wcuda 返回 aclError，
 * 用户代码里 `cudaError_t err = cudaMalloc(...)` 编译为 `aclError err = wcudaMalloc(...)`，
 * 类型一致，无需强转。 */
#define cudaError_t aclError
#define cudaSuccess ACL_SUCCESS

// 错误码映射。错误码值以真实 ACL 头为准（MegEngine 镜像 acl_base.h / error_codes/rt_error_codes.h，
// 见 ASCEND_RESEARCH_NOTES.md §1.5）。昇腾无一一对应 CUDA 全部错误码，映射常用且语义接近的；
// 其余 cudaError* 若不定义，应用代码引用会编译报错，按需补充即可。
#define cudaErrorInvalidValue ACL_ERROR_INVALID_PARAM           // 100000
#define cudaErrorMemoryAllocation ACL_ERROR_RT_MEMORY_ALLOCATION // 207001
#define cudaErrorInitializationError ACL_ERROR_RT_INTERNAL_ERROR // 507000
#define cudaErrorInvalidDevice ACL_ERROR_INVALID_DEVICE          // 200002
#define cudaErrorNoDevice ACL_ERROR_RT_NO_DEVICE                 // 207004
#define cudaErrorInvalidResourceHandle ACL_ERROR_RT_INVALID_HANDLE // 107017
#define cudaErrorNotReady ACL_ERROR_RT_EVENT_NOT_COMPLETE        // 507006
#define cudaErrorInvalidConfiguration ACL_ERROR_RT_PARAM_INVALID // 107000
#define cudaErrorLaunchOutOfResources ACL_ERROR_RT_NO_STREAM_RESOURCE // 207008
#define cudaErrorInvalidPitchValue ACL_ERROR_RT_PARAM_INVALID
#define cudaErrorInvalidDevicePointer ACL_ERROR_RT_PARAM_INVALID
#define cudaErrorIllegalAddress ACL_ERROR_RT_AICORE_EXCEPTION    // 507015
#define cudaErrorUnknown ACL_ERROR_FAILURE                        // 500001
#define cudaErrorApiFailureBase ACL_ERROR_FAILURE

//------------ memory kind ------------//
/* cudaMemcpyKind: 枚举值 0..3, 顺序与 aclrtMemcpyKind 一致（HtoH, HtoD, DtoH, DtoD） */
#define cudaMemcpyKind aclrtMemcpyKind
#define cudaMemcpyHostToHost ACL_MEMCPY_HOST_TO_HOST
#define cudaMemcpyHostToDevice ACL_MEMCPY_HOST_TO_DEVICE
#define cudaMemcpyDeviceToHost ACL_MEMCPY_DEVICE_TO_HOST
#define cudaMemcpyDeviceToDevice ACL_MEMCPY_DEVICE_TO_DEVICE
#define cudaMemcpyDefault wcudaMemcpyDefault

//------------ CUDA Runtime data-shape types ------------//
// These are cu2cce-owned source-compatibility containers, not CANN aliases.
#define cudaExtent wcceExtent
#define cudaPitchedPtr wccePitchedPtr
#define cudaPos wccePos
#define cudaPointerAttributes wccePointerAttributes
#define cudaMemoryType wcceMemoryType
#define cudaMemoryTypeUnregistered wcceMemoryTypeUnregistered
#define cudaMemoryTypeHost wcceMemoryTypeHost
#define cudaMemoryTypeDevice wcceMemoryTypeDevice
#define cudaMemoryTypeManaged wcceMemoryTypeManaged
#define cudaChannelFormatDesc wcceChannelFormatDesc
#define cudaChannelFormatKind wcceChannelFormatKind
#define cudaChannelFormatKindSigned wcceChannelFormatKindSigned
#define cudaChannelFormatKindUnsigned wcceChannelFormatKindUnsigned
#define cudaChannelFormatKindFloat wcceChannelFormatKindFloat
#define cudaChannelFormatKindNone wcceChannelFormatKindNone
#define cudaResourceType wcceResourceType
#define cudaResourceTypeArray wcceResourceTypeArray
#define cudaResourceTypeMipmappedArray wcceResourceTypeMipmappedArray
#define cudaResourceTypeLinear wcceResourceTypeLinear
#define cudaResourceTypePitch2D wcceResourceTypePitch2D
#define cudaTextureAddressMode wcceTextureAddressMode
#define cudaAddressModeWrap wcceAddressModeWrap
#define cudaAddressModeClamp wcceAddressModeClamp
#define cudaAddressModeMirror wcceAddressModeMirror
#define cudaAddressModeBorder wcceAddressModeBorder
#define cudaTextureFilterMode wcceTextureFilterMode
#define cudaFilterModePoint wcceFilterModePoint
#define cudaFilterModeLinear wcceFilterModeLinear
#define cudaTextureReadMode wcceTextureReadMode
#define cudaReadModeElementType wcceReadModeElementType
#define cudaReadModeNormalizedFloat wcceReadModeNormalizedFloat
#define cudaResourceViewFormat wcceResourceViewFormat
#define cudaResViewFormatNone wcceResourceViewFormatNone
#define cudaResViewFormatUnsignedChar1 wcceResourceViewFormatUnsignedChar1
#define cudaResViewFormatUnsignedChar2 wcceResourceViewFormatUnsignedChar2
#define cudaResViewFormatUnsignedChar4 wcceResourceViewFormatUnsignedChar4
#define cudaResViewFormatSignedChar1 wcceResourceViewFormatSignedChar1
#define cudaResViewFormatSignedChar2 wcceResourceViewFormatSignedChar2
#define cudaResViewFormatSignedChar4 wcceResourceViewFormatSignedChar4
#define cudaResViewFormatUnsignedShort1 wcceResourceViewFormatUnsignedShort1
#define cudaResViewFormatUnsignedShort2 wcceResourceViewFormatUnsignedShort2
#define cudaResViewFormatUnsignedShort4 wcceResourceViewFormatUnsignedShort4
#define cudaResViewFormatSignedShort1 wcceResourceViewFormatSignedShort1
#define cudaResViewFormatSignedShort2 wcceResourceViewFormatSignedShort2
#define cudaResViewFormatSignedShort4 wcceResourceViewFormatSignedShort4
#define cudaResViewFormatUnsignedInt1 wcceResourceViewFormatUnsignedInt1
#define cudaResViewFormatUnsignedInt2 wcceResourceViewFormatUnsignedInt2
#define cudaResViewFormatUnsignedInt4 wcceResourceViewFormatUnsignedInt4
#define cudaResViewFormatSignedInt1 wcceResourceViewFormatSignedInt1
#define cudaResViewFormatSignedInt2 wcceResourceViewFormatSignedInt2
#define cudaResViewFormatSignedInt4 wcceResourceViewFormatSignedInt4
#define cudaResViewFormatHalf1 wcceResourceViewFormatHalf1
#define cudaResViewFormatHalf2 wcceResourceViewFormatHalf2
#define cudaResViewFormatHalf4 wcceResourceViewFormatHalf4
#define cudaResViewFormatFloat1 wcceResourceViewFormatFloat1
#define cudaResViewFormatFloat2 wcceResourceViewFormatFloat2
#define cudaResViewFormatFloat4 wcceResourceViewFormatFloat4
#define cudaResourceDesc wcceResourceDesc
#define cudaTextureDesc wcceTextureDesc
#define cudaResourceViewDesc wcceResourceViewDesc
#define cudaArray_t wcceArray_t
#define cudaArray_const_t wcceArray_const_t
#define cudaMipmappedArray_t wcceMipmappedArray_t
#define cudaTextureObject_t wcceTextureObject_t
#define cudaSurfaceObject_t wcceSurfaceObject_t
#define cudaMemcpy3DParms wcceMemcpy3DParms
#define cudaMemcpy3DPeerParms wcceMemcpy3DPeerParms
#define cudaFuncAttributes wcceFuncAttributes
#define cudaMemPool_t wcceMemPool_t
#define cudaMemPoolProps wcceMemPoolProps
#define cudaMemPoolAttr wcceMemPoolAttr
#define cudaMemPoolReuseFollowEventDependencies wcceMemPoolReuseFollowEventDependencies
#define cudaMemPoolReuseAllowOpportunistic wcceMemPoolReuseAllowOpportunistic
#define cudaMemPoolReuseAllowInternalDependencies wcceMemPoolReuseAllowInternalDependencies
#define cudaMemPoolAttrReleaseThreshold wcceMemPoolAttrReleaseThreshold
#define cudaMemPoolAttrReservedMemCurrent wcceMemPoolAttrReservedMemCurrent
#define cudaMemPoolAttrReservedMemHigh wcceMemPoolAttrReservedMemHigh
#define cudaMemPoolAttrUsedMemCurrent wcceMemPoolAttrUsedMemCurrent
#define cudaMemPoolAttrUsedMemHigh wcceMemPoolAttrUsedMemHigh
#define cudaIpcMemHandle_t wcceIpcMemHandle_t
#define cudaIpcEventHandle_t wcceIpcEventHandle_t
#define cudaIpcMemLazyEnablePeerAccess 1U
#define cudaGraph_t wcceGraph_t
#define cudaGraphExec_t wcceGraphExec_t
#define cudaGraphNode_t wcceGraphNode_t
#define cudaStreamAttrID wcceStreamAttrID
#define cudaStreamAttrValue wcceStreamAttrValue
#define cudaStreamAttributeAccessPolicyWindow wcceStreamAttributeAccessPolicyWindow
#define cudaStreamAttributeSynchronizationPolicy wcceStreamAttributeSynchronizationPolicy
#define make_cudaExtent make_wcudaExtent
#define make_cudaPitchedPtr make_wcudaPitchedPtr
#define make_cudaPos make_wcudaPos

//------------ handle types ------------//
#define cudaStream_t aclrtStream
#define cudaEvent_t aclrtEvent

//------------ device properties ------------//
/* 昇腾无同形 DeviceProp 结构。由 cu2cce 自定义 _cceDeviceProp_t（见
 * include/bridge/runtime/cuda_wrapper_types.h）模拟 cudaDeviceProp 常用字段。 */
#define cudaDeviceProp _cceDeviceProp_t

// CUDA device scheduling/mapping flags. Ascend has no CUDA scheduling flag
// equivalent; the runtime wrapper accepts them as a compatibility no-op.
#define cudaDeviceScheduleAuto 0
#define cudaDeviceScheduleSpin 1
#define cudaDeviceScheduleYield 2
#define cudaDeviceScheduleBlockingSync 4
#define cudaDeviceBlockingSync cudaDeviceScheduleBlockingSync
#define cudaDeviceScheduleMask 7
#define cudaDeviceMapHost 8
#define cudaDeviceLmemResizeToMax 16
#define cudaDeviceMask 31

// CUDA event creation flags.
#define cudaEventDefault 0
#define cudaEventBlockingSync 1
#define cudaEventDisableTiming 2
#define cudaEventInterprocess 4

// CUDA pinned-host allocation flags (cudaHostAlloc). Values mirror the NVIDIA
// enum layout so host code storing them stays source-compatible.
#define cudaHostAllocDefault 0x00
#define cudaHostAllocPortable 0x01
#define cudaHostAllocMapped 0x02
#define cudaHostAllocWriteCombined 0x04

//--------------------- function replace --------------------//

// Device Management
#define cudaGetDeviceCount wcudaGetDeviceCount
#define cudaSetDevice wcudaSetDevice
#define cudaGetDevice wcudaGetDevice
#define cudaDeviceSynchronize wcudaDeviceSynchronize
#define cudaDeviceReset wcudaDeviceReset
#define cudaSetDeviceFlags wcudaSetDeviceFlags
#define cudaGetDeviceFlags wcudaGetDeviceFlags
#define cudaGetDeviceProperties wcudaGetDeviceProperties
#define cudaDeviceGetAttribute wcudaDeviceGetAttribute
/* Device 配置类（昇腾无对应/语义不同，声明保证可编译） */
#define cudaDeviceGetPCIBusId wcudaDeviceGetPCIBusId
#define cudaDeviceGetByPCIBusId wcudaDeviceGetByPCIBusId
#define cudaDeviceGetStreamPriorityRange wcudaDeviceGetStreamPriorityRange
#define cudaDeviceGetCacheConfig wcudaDeviceGetCacheConfig
#define cudaDeviceSetCacheConfig wcudaDeviceSetCacheConfig
#define cudaDeviceGetSharedMemConfig wcudaDeviceGetSharedMemConfig
#define cudaDeviceSetSharedMemConfig wcudaDeviceSetSharedMemConfig
#define cudaDeviceGetLimit wcudaDeviceGetLimit
#define cudaDeviceSetLimit wcudaDeviceSetLimit

/* cudaDeviceAttr 枚举常量（CUDA 标准 ABI 编号，昇腾侧用 int 简化，wrapper 内 switch）。
 * 常用值：MaxThreadsPerBlock=1..MaxGridDimZ=7, MaxSharedMemoryPerBlock=8, ClockRate=12,
 *         WarpSize=13, MaxPitch=14, MultiProcessorCount=16, KernelExecTimeout=19, Integrated=20 */
#define cudaDeviceAttr int
#define cudaDevAttrMaxThreadsPerBlock 1
#define cudaDevAttrMaxBlockDimX 2
#define cudaDevAttrMaxBlockDimY 3
#define cudaDevAttrMaxBlockDimZ 4
#define cudaDevAttrMaxGridDimX 5
#define cudaDevAttrMaxGridDimY 6
#define cudaDevAttrMaxGridDimZ 7
#define cudaDevAttrMaxSharedMemoryPerBlock 8
#define cudaDevAttrClockRate 12
#define cudaDevAttrWarpSize 13
#define cudaDevAttrMaxPitch 14
#define cudaDevAttrMultiProcessorCount 16
#define cudaDevAttrKernelExecTimeout 19
#define cudaDevAttrIntegrated 20
/* NVIDIA helper_cuda.h also queries these attributes during device selection.
 * They are bridge-private compatibility IDs; wcudaDeviceGetAttribute handles
 * them without claiming that Ascend has CUDA compute capability semantics. */
#define cudaDevAttrComputeMode 21
#define cudaDevAttrComputeCapabilityMajor 22
#define cudaDevAttrComputeCapabilityMinor 23
#define cudaComputeModeDefault 0
#define cudaComputeModeProhibited 2

// Peer Device Memory Access（昇腾有对应：aclrtDeviceCanAccessPeer 等）
#define cudaDeviceCanAccessPeer wcudaDeviceCanAccessPeer
#define cudaDeviceEnablePeerAccess wcudaDeviceEnablePeerAccess
#define cudaDeviceDisablePeerAccess wcudaDeviceDisablePeerAccess

// Context Management（昇腾有对应：aclrtCreateContext / SetCurrentContext 等；
// CUDA Runtime 的 cudaCtx* 在 driver 层，此处映射 wcuda* runtime 封装）
#define cudaCtxCreate wcudaCtxCreate
#define cudaCtxDestroy wcudaCtxDestroy
#define cudaCtxSetCurrent wcudaCtxSetCurrent
#define cudaCtxGetCurrent wcudaCtxGetCurrent
#define cudaCtxSynchronize wcudaCtxSynchronize

// Stream Management
#define cudaStreamCreate wcudaStreamCreate
#define cudaStreamCreateWithFlags wcudaStreamCreateWithFlags
#define cudaStreamCreateWithPriority wcudaStreamCreateWithPriority
#define cudaStreamGetFlags wcudaStreamGetFlags
#define cudaStreamGetPriority wcudaStreamGetPriority
#define cudaStreamGetDevice wcudaStreamGetDevice
#define cudaStreamGetId wcudaStreamGetId
#define cudaStreamCopyAttributes wcudaStreamCopyAttributes
#define cudaStreamGetAttribute wcudaStreamGetAttribute
#define cudaStreamSetAttribute wcudaStreamSetAttribute
#define cudaStreamAttachMemAsync wcudaStreamAttachMemAsync
/* CUDA stream flags are accepted by the official NVIDIA samples. Ascend's
 * wcudaStreamCreateWithFlags currently ignores the flag but preserves the
 * source-level CUDA ABI. */
#define cudaStreamDefault 0
#define cudaStreamNonBlocking 1
#define cudaStreamDestroy wcudaStreamDestroy
#define cudaStreamSynchronize wcudaStreamSynchronize
#define cudaStreamQuery wcudaStreamQuery
#define cudaStreamWaitEvent wcudaStreamWaitEvent

/* Stream capture（CUDA Graph 捕获）：昇腾无对应，占位返回 not-support。
 * 枚举值沿用 CUDA 标准 ABI。 */
#define cudaStreamCaptureMode int
#define cudaStreamCaptureModeGlobal 0
#define cudaStreamCaptureModeThreadLocal 1
#define cudaStreamCaptureModeRelaxed 2
#define cudaStreamCaptureStatus int
#define cudaStreamCaptureStatusNone 0
#define cudaStreamCaptureStatusActive 1
#define cudaStreamCaptureStatusInvalidated 2
#define cudaStreamBeginCapture wcudaStreamBeginCapture
#define cudaStreamEndCapture wcudaStreamEndCapture
#define cudaStreamIsCapturing wcudaStreamIsCapturing
#define cudaStreamGetCaptureInfo wcudaStreamGetCaptureInfo
#define cudaStreamGetCaptureInfo_v2 wcudaStreamGetCaptureInfo_v2

// Event Management
#define cudaEventCreate wcudaEventCreate
#define cudaEventCreateWithFlags wcudaEventCreateWithFlags
#define cudaEventDestroy wcudaEventDestroy
#define cudaEventRecord wcudaEventRecord
#define cudaEventRecordWithFlags wcudaEventRecordWithFlags
#define cudaEventSynchronize wcudaEventSynchronize
#define cudaEventQuery wcudaEventQuery
#define cudaEventElapsedTime wcudaEventElapsedTime
#define cudaStreamAddCallback wcudaStreamAddCallback

// Memory Management
#define cudaMalloc wcudaMalloc
#define cudaFree wcudaFree
#define cudaMemcpy wcudaMemcpy
#define cudaMemcpyAsync wcudaMemcpyAsync
#define cudaMemcpy2D wcudaMemcpy2D
#define cudaMemcpy2DAsync wcudaMemcpy2DAsync
#define cudaMemcpy3D wcudaMemcpy3D
#define cudaMemcpy3DAsync wcudaMemcpy3DAsync
#define cudaMemcpy3DPeer wcudaMemcpy3DPeer
#define cudaMemcpy3DPeerAsync wcudaMemcpy3DPeerAsync
#define cudaMemcpyPeer wcudaMemcpyPeer
#define cudaMemcpyPeerAsync wcudaMemcpyPeerAsync
#define cudaMemset wcudaMemset
#define cudaMemsetAsync wcudaMemsetAsync
#define cudaMemset2D wcudaMemset2D
#define cudaMemset2DAsync wcudaMemset2DAsync
#define cudaMemset3D wcudaMemset3D
#define cudaMemset3DAsync wcudaMemset3DAsync
#define cudaMallocPitch wcudaMallocPitch
#define cudaMalloc3D wcudaMalloc3D
#define cudaMallocArray wcudaMallocArray
#define cudaMalloc3DArray wcudaMalloc3DArray
#define cudaMallocMipmappedArray wcudaMallocMipmappedArray
#define cudaArrayGetInfo wcudaArrayGetInfo
#define cudaGetChannelDesc wcudaGetChannelDesc
#define cudaFreeArray wcudaFreeArray
#define cudaFreeMipmappedArray wcudaFreeMipmappedArray
#define cudaGetMipmappedArrayLevel wcudaGetMipmappedArrayLevel
#define cudaGetTextureObjectResourceDesc wcudaGetTextureObjectResourceDesc
#define cudaGetTextureObjectTextureDesc wcudaGetTextureObjectTextureDesc
#define cudaGetTextureObjectTextureDesc_v2 wcudaGetTextureObjectTextureDesc_v2
#define cudaGetTextureObjectResourceViewDesc wcudaGetTextureObjectResourceViewDesc
#define cudaGetSurfaceObjectResourceDesc wcudaGetSurfaceObjectResourceDesc
#define cudaMemcpyToArray wcudaMemcpyToArray
#define cudaMemcpyFromArray wcudaMemcpyFromArray
#define cudaMemcpy2DToArray wcudaMemcpy2DToArray
#define cudaMemcpy2DFromArray wcudaMemcpy2DFromArray
#define cudaMemcpyToArrayAsync wcudaMemcpyToArrayAsync
#define cudaMemcpyFromArrayAsync wcudaMemcpyFromArrayAsync
#define cudaMemcpy2DToArrayAsync wcudaMemcpy2DToArrayAsync
#define cudaMemcpy2DFromArrayAsync wcudaMemcpy2DFromArrayAsync
#define cudaMemcpyArrayToArray wcudaMemcpyArrayToArray
#define cudaMemcpy2DArrayToArray wcudaMemcpy2DArrayToArray
#define cudaGetSymbolAddress wcudaGetSymbolAddress
#define cudaGetSymbolSize wcudaGetSymbolSize
#define cudaMemcpyToSymbol wcudaMemcpyToSymbol
#define cudaMemcpyFromSymbol wcudaMemcpyFromSymbol
#define cudaMemcpyToSymbolAsync wcudaMemcpyToSymbolAsync
#define cudaMemcpyFromSymbolAsync wcudaMemcpyFromSymbolAsync
#define cudaMallocAsync wcudaMallocAsync
#define cudaFreeAsync wcudaFreeAsync
#define cudaMemPoolCreate wcudaMemPoolCreate
#define cudaMemPoolDestroy wcudaMemPoolDestroy
#define cudaDeviceGetMemPool wcudaDeviceGetMemPool
#define cudaMallocFromPoolAsync wcudaMallocFromPoolAsync
#define cudaMemPoolTrimTo wcudaMemPoolTrimTo
#define cudaMemPoolGetAttribute wcudaMemPoolGetAttribute
#define cudaMemPoolSetAttribute wcudaMemPoolSetAttribute
#define cudaIpcGetMemHandle wcudaIpcGetMemHandle
#define cudaIpcOpenMemHandle wcudaIpcOpenMemHandle
#define cudaIpcCloseMemHandle wcudaIpcCloseMemHandle
#define cudaIpcGetEventHandle wcudaIpcGetEventHandle
#define cudaIpcOpenEventHandle wcudaIpcOpenEventHandle
#define cudaDeviceSetMemPool wcudaDeviceSetMemPool
#define cudaMallocManaged wcudaMallocManaged
#define cudaMemAdvise wcudaMemAdvise
#define cudaMemPrefetchAsync wcudaMemPrefetchAsync
#define cudaPointerGetAttributes wcudaPointerGetAttributes
#define cudaMemGetInfo wcudaMemGetInfo
#define cudaMallocHost wcudaMallocHost
#define cudaFreeHost wcudaFreeHost
/* 昇腾部分支持/不支持：HostAlloc 等价转发；HostGetDevicePointer/Register 等运行时返回"不支持" */
#define cudaHostAlloc wcudaHostAlloc
#define cudaHostGetDevicePointer wcudaHostGetDevicePointer
#define cudaHostRegister wcudaHostRegister
#define cudaHostUnregister wcudaHostUnregister
#define cudaHostGetFlags wcudaHostGetFlags

// Execution Control
/* cudaLaunchKernel 显式 API：昇腾用 <<<>>> 语法糖（bisheng 原生），此 wrapper 当前返回 not-support */
#define cudaLaunchKernel wcudaLaunchKernel
#define cudaLaunchCooperativeKernel wcudaLaunchCooperativeKernel

// Occupancy
#define cudaOccupancyMaxActiveBlocksPerMultiprocessor wcudaOccupancyMaxActiveBlocksPerMultiprocessor
#define cudaOccupancyMaxPotentialBlockSize wcudaOccupancyMaxPotentialBlockSize
#define cudaOccupancyAvailableDynamicSMemPerBlock wcudaOccupancyAvailableDynamicSMemPerBlock
#define cudaOccupancyMaxActiveBlocksPerMultiprocessorWithFlags wcudaOccupancyMaxActiveBlocksPerMultiprocessorWithFlags
#define cudaOccupancyMaxPotentialBlockSizeWithFlags wcudaOccupancyMaxPotentialBlockSizeWithFlags

// Host Function & Func 属性
#define cudaLaunchHostFunc wcudaLaunchHostFunc
#define cudaHostFn_t wcceHostFn_t
#define cudaFuncGetAttributes wcudaFuncGetAttributes
#define cudaFuncSetAttribute wcudaFuncSetAttribute
#define cudaFuncSetCacheConfig wcudaFuncSetCacheConfig
#define cudaFuncSetSharedMemConfig wcudaFuncSetSharedMemConfig
#define cudaFuncGetName wcudaFuncGetName

// Error Handling
#define cudaGetLastError wcudaGetLastError
#define cudaPeekAtLastError wcudaPeekAtLastError
#define cudaGetErrorString wcudaGetErrorString
#define cudaGetErrorName wcudaGetErrorName

// External Memory（昇腾无对应，not-support）
#define cudaImportExternalMemory wcudaImportExternalMemory
#define cudaDestroyExternalMemory wcudaDestroyExternalMemory

// Profiler Control（no-op 占位，真机 profiling 走 aclprof）
#define cudaProfilerStart wcudaProfilerStart
#define cudaProfilerStop wcudaProfilerStop

// Graph（昇腾无 CUDA Graph，核心函数 not-support）
#define cudaGraphCreate wcudaGraphCreate
#define cudaGraphDestroy wcudaGraphDestroy
#define cudaGraphInstantiate wcudaGraphInstantiate
#define cudaGraphLaunch wcudaGraphLaunch
#define cudaGraphExecDestroy wcudaGraphExecDestroy
#define cudaGraphExecUpdate wcudaGraphExecUpdate
#define cudaGraphAddKernelNode wcudaGraphAddKernelNode
#define cudaGraphMemcpyNodeSetParams1D wcudaGraphMemcpyNodeSetParams1D
#define cudaGraphGetNodes wcudaGraphGetNodes
#define cudaGraphGetRootNodes wcudaGraphGetRootNodes
#define cudaGraphClone wcudaGraphClone
#define cudaGraphDestroyNode wcudaGraphDestroyNode

// Texture/Surface（昇腾无对应，全部 not-support）
#define cudaCreateChannelDesc wcudaCreateChannelDesc
#define cudaCreateTextureObject wcudaCreateTextureObject
#define cudaDestroyTextureObject wcudaDestroyTextureObject
#define cudaCreateSurfaceObject wcudaCreateSurfaceObject
#define cudaDestroySurfaceObject wcudaDestroySurfaceObject
#define cudaBindTexture wcudaBindTexture

// Thread Management（废弃 API）
#define cudaThreadSynchronize wcudaThreadSynchronize
#define cudaThreadExit wcudaThreadExit
#define cudaThreadGetCacheConfig wcudaThreadGetCacheConfig
#define cudaThreadGetLimit wcudaThreadGetLimit
#define cudaThreadSetLimit wcudaThreadSetLimit
#define cudaThreadSetCacheConfig wcudaThreadSetCacheConfig

// Version
#define cudaRuntimeGetVersion wcudaRuntimeGetVersion
#define cudaDriverGetVersion wcudaDriverGetVersion

//------------ error code 兜底映射（昇腾无一一对应，保证应用可编译） ------------//
/* 以下 cudaError* 在昇腾 aclError 大多无精确对应：语义明确的精确映射，其余兜底 ACL_ERROR_FAILURE。
 * 应用若需区分具体错误，真机上按 aclrtGetErrorInfo 实际返回处理。 */
#define cudaErrorAddressOfConstant ACL_ERROR_FAILURE
#define cudaErrorAlreadyAcquired ACL_ERROR_FAILURE
#define cudaErrorAlreadyMapped ACL_ERROR_FAILURE
#define cudaErrorArrayIsMapped ACL_ERROR_FAILURE
#define cudaErrorAssert ACL_ERROR_FAILURE
#define cudaErrorCallRequiresNewerDriver ACL_ERROR_FAILURE
#define cudaErrorCapturedEvent ACL_ERROR_FAILURE
#define cudaErrorCompatNotSupportedOnDevice ACL_ERROR_FAILURE
#define cudaErrorContextIsDestroyed ACL_ERROR_FAILURE
#define cudaErrorCooperativeLaunchTooLarge ACL_ERROR_FAILURE
#define cudaErrorCudartUnloading ACL_ERROR_RT_FAILURE
#define cudaErrorDeviceAlreadyInUse ACL_ERROR_RT_INVALID_DEVICEID
#define cudaErrorDeviceNotLicensed ACL_ERROR_FAILURE
#define cudaErrorDeviceUninitialized ACL_ERROR_RT_CONTEXT_NULL
#define cudaErrorDevicesUnavailable ACL_ERROR_RT_NO_DEVICE
#define cudaErrorDuplicateSurfaceName ACL_ERROR_FAILURE
#define cudaErrorDuplicateTextureName ACL_ERROR_FAILURE
#define cudaErrorDuplicateVariableName ACL_ERROR_FAILURE
#define cudaErrorECCUncorrectable ACL_ERROR_FAILURE
#define cudaErrorExternalDevice ACL_ERROR_FAILURE
#define cudaErrorFileNotFound ACL_ERROR_FAILURE
#define cudaErrorGraphExecUpdateFailure ACL_ERROR_FAILURE
#define cudaErrorHardwareStackError ACL_ERROR_FAILURE
#define cudaErrorHostMemoryAlreadyRegistered ACL_ERROR_FAILURE
#define cudaErrorHostMemoryNotRegistered ACL_ERROR_FAILURE
#define cudaErrorIllegalInstruction ACL_ERROR_FAILURE
#define cudaErrorIllegalState ACL_ERROR_FAILURE
#define cudaErrorIncompatibleDriverContext ACL_ERROR_FAILURE
#define cudaErrorInsufficientDriver ACL_ERROR_RT_INTERNAL_ERROR
#define cudaErrorInvalidAddressSpace ACL_ERROR_FAILURE
#define cudaErrorInvalidChannelDescriptor ACL_ERROR_FAILURE
#define cudaErrorInvalidDeviceFunction ACL_ERROR_INVALID_DEVICE
#define cudaErrorInvalidFilterSetting ACL_ERROR_FAILURE
#define cudaErrorInvalidGraphicsContext ACL_ERROR_FAILURE
#define cudaErrorInvalidHostPointer ACL_ERROR_INVALID_PARAM
#define cudaErrorInvalidKernelImage ACL_ERROR_RT_KERNEL_LOOKUP
#define cudaErrorInvalidMemcpyDirection ACL_ERROR_INVALID_PARAM
#define cudaErrorInvalidNormSetting ACL_ERROR_FAILURE
#define cudaErrorInvalidPc ACL_ERROR_FAILURE
#define cudaErrorInvalidPtx ACL_ERROR_RT_KERNEL_LOOKUP
#define cudaErrorInvalidResourceConfiguration ACL_ERROR_FAILURE
#define cudaErrorInvalidResourceType ACL_ERROR_FAILURE
#define cudaErrorInvalidSource ACL_ERROR_FAILURE
#define cudaErrorInvalidSurface ACL_ERROR_FAILURE
#define cudaErrorInvalidSymbol ACL_ERROR_FAILURE
#define cudaErrorInvalidTexture ACL_ERROR_FAILURE
#define cudaErrorInvalidTextureBinding ACL_ERROR_FAILURE
#define cudaErrorJitCompilationDisabled ACL_ERROR_FAILURE
#define cudaErrorJitCompilerNotFound ACL_ERROR_FAILURE
#define cudaErrorLaunchFailure ACL_ERROR_RT_INTERNAL_ERROR
#define cudaErrorLaunchFileScopedSurf ACL_ERROR_FAILURE
#define cudaErrorLaunchFileScopedTex ACL_ERROR_FAILURE
#define cudaErrorLaunchIncompatibleTexturing ACL_ERROR_FAILURE
#define cudaErrorLaunchMaxDepthExceeded ACL_ERROR_FAILURE
#define cudaErrorLaunchPendingCountExceeded ACL_ERROR_FAILURE
#define cudaErrorLaunchTimeout ACL_ERROR_RT_AICORE_TIMEOUT
#define cudaErrorMapBufferObjectFailed ACL_ERROR_FAILURE
#define cudaErrorMemoryValueTooLarge ACL_ERROR_FAILURE
#define cudaErrorMisalignedAddress ACL_ERROR_FAILURE
#define cudaErrorMissingConfiguration ACL_ERROR_FAILURE
#define cudaErrorMixedDeviceExecution ACL_ERROR_FAILURE
#define cudaErrorMpsConnectionFailed ACL_ERROR_FAILURE
#define cudaErrorMpsMaxClientsReached ACL_ERROR_FAILURE
#define cudaErrorMpsMaxConnectionsReached ACL_ERROR_FAILURE
#define cudaErrorMpsRpcFailure ACL_ERROR_FAILURE
#define cudaErrorMpsServerNotReady ACL_ERROR_FAILURE
#define cudaErrorNoKernelImageForDevice ACL_ERROR_RT_KERNEL_LOOKUP
#define cudaErrorNotMapped ACL_ERROR_FAILURE
#define cudaErrorNotMappedAsArray ACL_ERROR_FAILURE
#define cudaErrorNotMappedAsPointer ACL_ERROR_FAILURE
#define cudaErrorNotPermitted ACL_ERROR_FAILURE
#define cudaErrorNotSupported ACL_ERROR_FEATURE_UNSUPPORTED
#define cudaErrorNotYetImplemented ACL_ERROR_FAILURE
#define cudaErrorNvlinkUncorrectable ACL_ERROR_FAILURE
#define cudaErrorOperatingSystem ACL_ERROR_FAILURE
#define cudaErrorPeerAccessAlreadyEnabled ACL_ERROR_FAILURE
#define cudaErrorPeerAccessNotEnabled ACL_ERROR_FAILURE
#define cudaErrorPeerAccessUnsupported ACL_ERROR_FAILURE
#define cudaErrorPriorLaunchFailure ACL_ERROR_FAILURE
#define cudaErrorProfilerAlreadyStarted ACL_ERROR_FAILURE
#define cudaErrorProfilerAlreadyStopped ACL_ERROR_FAILURE
#define cudaErrorProfilerDisabled ACL_ERROR_FAILURE
#define cudaErrorProfilerNotInitialized ACL_ERROR_FAILURE
#define cudaErrorSetOnActiveProcess ACL_ERROR_FAILURE
#define cudaErrorSharedObjectInitFailed ACL_ERROR_FAILURE
#define cudaErrorSharedObjectSymbolNotFound ACL_ERROR_RT_KERNEL_LOOKUP
#define cudaErrorSoftwareValidityNotEstablished ACL_ERROR_FAILURE
#define cudaErrorStartupFailure ACL_ERROR_FAILURE
#define cudaErrorStreamCaptureImplicit ACL_ERROR_FAILURE
#define cudaErrorStreamCaptureInvalidated ACL_ERROR_FAILURE
#define cudaErrorStreamCaptureIsolation ACL_ERROR_FAILURE
#define cudaErrorStreamCaptureMerge ACL_ERROR_FAILURE
#define cudaErrorStreamCaptureUnjoined ACL_ERROR_FAILURE
#define cudaErrorStreamCaptureUnmatched ACL_ERROR_FAILURE
#define cudaErrorStreamCaptureUnsupported ACL_ERROR_FAILURE
#define cudaErrorStreamCaptureWrongThread ACL_ERROR_FAILURE
#define cudaErrorStubLibrary ACL_ERROR_FAILURE
#define cudaErrorSymbolNotFound ACL_ERROR_FAILURE
#define cudaErrorSyncDepthExceeded ACL_ERROR_FAILURE
#define cudaErrorSynchronizationError ACL_ERROR_RT_EVENT_NOT_COMPLETE
#define cudaErrorSystemDriverMismatch ACL_ERROR_FAILURE
#define cudaErrorSystemNotReady ACL_ERROR_FAILURE
#define cudaErrorTextureFetchFailed ACL_ERROR_FAILURE
#define cudaErrorTextureNotBound ACL_ERROR_FAILURE
#define cudaErrorTimeout ACL_ERROR_FAILURE
#define cudaErrorTooManyPeers ACL_ERROR_FAILURE
#define cudaErrorUnmapBufferObjectFailed ACL_ERROR_FAILURE
#define cudaErrorUnsupportedExecAffinity ACL_ERROR_FAILURE
#define cudaErrorUnsupportedLimit ACL_ERROR_FAILURE
#define cudaErrorUnsupportedPtxVersion ACL_ERROR_FAILURE
#define cudaError_enum ACL_ERROR_FAILURE
#endif //__CUDA_TO_CCE_ADAPTOR_H__
