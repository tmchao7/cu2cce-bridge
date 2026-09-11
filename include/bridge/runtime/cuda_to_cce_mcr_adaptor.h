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
/* Extended device attributes referenced by NVIDIA helper_cuda.h and modern
 * CUDA 11/12 headers. Values 24+ are bridge-private IDs; wcudaDeviceGetAttribute
 * returns 0/feature-unsupported for attributes with no Ascend equivalent. */
#define cudaDevAttrMaxTexture1DWidth 24
#define cudaDevAttrMaxTexture2DWidth 25
#define cudaDevAttrMaxTexture2DHeight 26
#define cudaDevAttrMaxTexture3DWidth 27
#define cudaDevAttrMaxTexture3DHeight 28
#define cudaDevAttrMaxTexture3DDepth 29
#define cudaDevAttrMaxTexture2DLayeredWidth 30
#define cudaDevAttrMaxTexture2DLayeredHeight 31
#define cudaDevAttrMaxTexture2DLayeredLayers 32
#define cudaDevAttrMaxTexture1DLayeredWidth 33
#define cudaDevAttrMaxTexture1DLayeredLayers 34
#define cudaDevAttrMaxTexture2DGatherWidth 35
#define cudaDevAttrMaxTexture2DGatherHeight 36
#define cudaDevAttrMaxTexture3DWidthAlt 37
#define cudaDevAttrMaxTexture3DHeightAlt 38
#define cudaDevAttrMaxTexture3DDepthAlt 39
#define cudaDevAttrMaxTextureCubemapWidth 40
#define cudaDevAttrMaxTextureCubemapLayeredWidth 41
#define cudaDevAttrMaxTextureCubemapLayeredLayers 42
#define cudaDevAttrMaxSurface1DWidth 43
#define cudaDevAttrMaxSurface2DWidth 44
#define cudaDevAttrMaxSurface2DHeight 45
#define cudaDevAttrMaxSurface3DWidth 46
#define cudaDevAttrMaxSurface3DHeight 47
#define cudaDevAttrMaxSurface3DDepth 48
#define cudaDevAttrMaxSurface1DLayeredWidth 49
#define cudaDevAttrMaxSurface1DLayeredLayers 50
#define cudaDevAttrMaxSurface2DLayeredWidth 51
#define cudaDevAttrMaxSurface2DLayeredHeight 52
#define cudaDevAttrMaxSurface2DLayeredLayers 53
#define cudaDevAttrMaxSurfaceCubemapWidth 54
#define cudaDevAttrMaxSurfaceCubemapLayeredWidth 55
#define cudaDevAttrMaxSurfaceCubemapLayeredLayers 56
#define cudaDevAttrMaxTexture1DLinearWidth 57
#define cudaDevAttrMaxTexture2DLinearWidth 58
#define cudaDevAttrMaxTexture2DLinearHeight 59
#define cudaDevAttrMaxTexture2DLinearPitch 60
#define cudaDevAttrMaxTexture1DMipmappedWidth 61
#define cudaDevAttrMaxTexture2DMipmappedWidth 62
#define cudaDevAttrMaxTexture2DMipmappedHeight 63
#define cudaDevAttrTotalConstantMemory 64
#define cudaDevAttrMaxRegistersPerBlock 65
#define cudaDevAttrTextureAlignment 66
#define cudaDevAttrTexturePitchAlignment 67
#define cudaDevAttrGpuOverlap 68
#define cudaDevAttrCanMapHostMemory 70
#define cudaDevAttrSurfaceAlignment 71
#define cudaDevAttrConcurrentKernels 72
#define cudaDevAttrEccEnabled 73
#define cudaDevAttrPciBusId 74
#define cudaDevAttrPciDeviceId 75
#define cudaDevAttrPciDomainId 76
#define cudaDevAttrTccDriver 77
#define cudaDevAttrMemoryClockRate 78
#define cudaDevAttrGlobalMemoryBusWidth 79
#define cudaDevAttrL2CacheSize 80
#define cudaDevAttrMaxThreadsPerMultiProcessor 81
#define cudaDevAttrAsyncEngineCount 82
#define cudaDevAttrUnifiedAddressing 83
#define cudaDevAttrMax 84
#define cudaDevAttrMaxSharedMemoryPerBlockOptin 85
#define cudaDevAttrMaxSharedMemoryPerMultiprocessor 86
#define cudaDevAttrMaxRegistersPerMultiprocessor 87
#define cudaDevAttrStreamPrioritiesSupported 88
#define cudaDevAttrGlobalL1CacheSupported 89
#define cudaDevAttrLocalL1CacheSupported 90
#define cudaDevAttrMaxBlocksPerMultiprocessor 91
#define cudaDevAttrMaxPersistingL2CacheSize 92
#define cudaDevAttrMaxAccessPolicyWindowSize 93
#define cudaDevAttrReservedSharedMemoryPerBlock 94
#define cudaDevAttrReserved92 95
#define cudaDevAttrReserved93 96
#define cudaDevAttrReserved94 97
#define cudaDevAttrManagedMemory 98
#define cudaDevAttrPageableMemoryAccess 99
#define cudaDevAttrPageableMemoryAccessUsesHostPageTables 100
#define cudaDevAttrDirectManagedMemAccessFromHost 101
#define cudaDevAttrConcurrentManagedAccess 102
#define cudaDevAttrMemoryPoolsSupported 103
#define cudaDevAttrMemoryPoolSupportedHandleTypes 104
#define cudaDevAttrClusterSchedulingPolicyDefault 105
#define cudaDevAttrComputePreemptionSupported 106
#define cudaDevAttrCooperativeLaunch 107
#define cudaDevAttrCooperativeMultiDeviceLaunch 108
#define cudaDevAttrHostNativeAtomicSupported 109
#define cudaDevAttrCanFlushRemoteWrites 110
#define cudaDevAttrCanUseHostPointerForRegisteredMem 111
#define cudaDevAttrHostRegisterSupported 112
#define cudaDevAttrHostRegisterReadOnlySupported 113
#define cudaDevAttrSparseCudaArraySupported 114
#define cudaDevAttrDeferredMappingCudaArraySupported 115
#define cudaDevAttrTimelineSemaphoreInteropSupported 116
#define cudaDevAttrMaxTimelineSemaphoreInteropSupported 117
#define cudaDevAttrIsMultiGpuBoard 118
#define cudaDevAttrMultiGpuBoardGroupID 119
#define cudaDevAttrSingleToDoublePrecisionPerfRatio 120
#define cudaDevAttrGPUDirectRDMAFlushWritesOptions 121
#define cudaDevAttrGPUDirectRDMASupported 122
#define cudaDevAttrGPUDirectRDMAWritesOrdering 123
#define cudaDevAttrMaxTexture1DLayeredLayers_RENAMED 9999

/* cudaComputeMode enum (CUDA standard values) */
#define cudaComputeMode int
#define cudaComputeModeDefault 0
#define cudaComputeModeExclusive 1
#define cudaComputeModeProhibited 2
#define cudaComputeModeExclusiveProcess 3

// Peer Device Memory Access（昇腾有对应：aclrtDeviceCanAccessPeer 等）
#define cudaDeviceCanAccessPeer wcudaDeviceCanAccessPeer
#define cudaDeviceEnablePeerAccess wcudaDeviceEnablePeerAccess
#define cudaDeviceDisablePeerAccess wcudaDeviceDisablePeerAccess
#define cudaPeerAccessDefault 0

/* P2P attribute enum (CUDA standard; wrapper returns unsupported) */
#define cudaDeviceP2PAttr int
#define cudaDevP2PAttrPerformanceRank 0
#define cudaDevP2PAttrAccessSupported 1
#define cudaDevP2PAttrNativeAtomicSupported 2
#define cudaDevP2PAttrCudaArrayAccessSupported 3
#define cudaDeviceGetP2PAttribute wcudaDeviceGetP2PAttribute

/* Additional Device Management functions */
#define cudaChooseDevice wcudaChooseDevice
#define cudaSetValidDevices wcudaSetValidDevices
#define cudaDeviceGetDefaultMemPool wcudaDeviceGetDefaultMemPool
#define cudaDeviceFlushGPUDirectRDMAWrites wcudaDeviceFlushGPUDirectRDMAWrites
#define cudaDeviceGetGraphMemAttribute wcudaDeviceGetGraphMemAttribute
#define cudaDeviceSetGraphMemAttribute wcudaDeviceSetGraphMemAttribute
#define cudaDeviceGraphMemTrim wcudaDeviceGraphMemTrim
#define cudaCpuDeviceId (-1)
#define cudaInvalidDeviceId (-2)

/* GPUDirect RDMA enums (no Ascend equivalent; for compilation only) */
#define cudaFlushGPUDirectRDMAWritesOptions int
#define cudaFlushGPUDirectRDMAWritesOptionHost 0
#define cudaFlushGPUDirectRDMAWritesOptionMemOps 1
#define cudaFlushGPUDirectRDMAWritesScope int
#define cudaFlushGPUDirectRDMAWritesTarget int
#define cudaFlushGPUDirectRDMAWritesTargetCurrentDevice 0
#define cudaFlushGPUDirectRDMAWritesToAllDevices 1
#define cudaFlushGPUDirectRDMAWritesToOwner 2
#define cudaGPUDirectRDMAWritesOrdering int
#define cudaGPUDirectRDMAWritesOrderingNone 0
#define cudaGPUDirectRDMAWritesOrderingOwner 1
#define cudaGPUDirectRDMAWritesOrderingAllDevices 2

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
#define cudaStreamUpdateCaptureDependencies wcudaStreamUpdateCaptureDependencies
#define cudaStreamUpdateCaptureDependenciesFlags int
#define cudaStreamAddCaptureDependencies wcudaStreamUpdateCaptureDependencies
#define cudaStreamSetCaptureDependencies wcudaStreamUpdateCaptureDependencies
#define cudaStreamBeginCaptureToGraph wcudaStreamBeginCaptureToGraph
#define cudaStreamGetDevResource wcudaStreamGetDevResource
#define cudaThreadExchangeStreamCaptureMode wcudaThreadExchangeStreamCaptureMode

/* Legacy stream constants (CUDA default stream model) */
#define cudaStreamLegacy 0x1
#define cudaStreamPerThread 0x2
#define cudaEnableDefault 0
#define cudaEnableLegacyStream 0
#define cudaEnablePerThreadDefaultStream 1

/* Stream callback type alias */
#define cudaStreamCallback_t wcceStreamCallback_t

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

/* Event flags (CUDA standard values) */
#define cudaEventDefault 0
#define cudaEventBlockingSync 1
#define cudaEventDisableTiming 2
#define cudaEventInterprocess 4
#define cudaEventRecordDefault 0
#define cudaEventRecordExternal 32
#define cudaEventWaitDefault 0
#define cudaEventWaitExternal 32
#define cudaEventCreateFromEGLSync wcudaEventCreateFromEGLSync
#define cudaEventElapsedTime_v2 wcudaEventElapsedTime

/* Per-thread-stream (_ptsz) variants: map to same wrapper */
#define cudaEventRecord_ptsz wcudaEventRecord
#define cudaEventRecordWithFlags_ptsz wcudaEventRecordWithFlags

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

/* Host register flags (CUDA standard) */
#define cudaHostRegisterDefault 0
#define cudaHostRegisterPortable 1
#define cudaHostRegisterMapped 2
#define cudaHostRegisterIoMemory 4
#define cudaHostRegisterReadOnly 8

/* MemAdvise enum (CUDA standard; Ascend has no UM, wrapper returns not-support) */
#define cudaMemoryAdvise int
#define cudaMemAdviseSetReadMostly 1
#define cudaMemAdviseUnsetReadMostly 2
#define cudaMemAdviseSetPreferredLocation 3
#define cudaMemAdviseUnsetPreferredLocation 4
#define cudaMemAdviseSetAccessedBy 5
#define cudaMemAdviseUnsetAccessedBy 6

/* MemAttach flags */
#define cudaMemAttachGlobal 1
#define cudaMemAttachHost 2
#define cudaMemAttachSingle 4

/* MemRange attribute enum */
#define cudaMemRangeAttribute int
#define cudaMemRangeAttributeReadMostly 0
#define cudaMemRangeAttributePreferredLocation 1
#define cudaMemRangeAttributeAccessedBy 2
#define cudaMemRangeAttributeLastPrefetchLocation 3

/* MemPool extended functions */
#define cudaMemAdvise_v2 wcudaMemAdvise_v2
#define cudaMemRangeGetAttribute wcudaMemRangeGetAttribute
#define cudaMemRangeGetAttributes wcudaMemRangeGetAttributes
#define cudaMemPoolGetAccess wcudaMemPoolGetAccess
#define cudaMemPoolSetAccess wcudaMemPoolSetAccess
#define cudaMemPoolExportToShareableHandle wcudaMemPoolExportToShareableHandle
#define cudaMemPoolImportFromShareableHandle wcudaMemPoolImportFromShareableHandle
#define cudaMemPoolExportPointer wcudaMemPoolExportPointer
#define cudaMemPoolImportPointer wcudaMemPoolImportPointer

/* Mem allocation handle types (for compilation only) */
#define cudaMemAllocationHandleType int
#define cudaMemHandleTypeNone 0
#define cudaMemHandleTypePosixFileDescriptor 1
#define cudaMemHandleTypeWin32 2
#define cudaMemHandleTypeWin32Kmt 4
#define cudaMemHandleTypeFabric 8

/* Mem allocation types */
#define cudaMemAllocationType int
#define cudaMemAllocationTypeInvalid 0
#define cudaMemAllocationTypePinned 1
#define cudaMemAllocationTypeMax 2

/* Mem location types */
#define cudaMemLocationType int
#define cudaMemLocationTypeInvalid 0
#define cudaMemLocationTypeDevice 1
#define cudaMemLocationTypeHost 2
#define cudaMemLocationTypeHostNuma 3
#define cudaMemLocationTypeHostNumaCurrent 4
#define cudaMemLocationTypeNone 5

/* Mem access flags */
#define cudaMemAccessFlags int
#define cudaMemAccessFlagsProtNone 0
#define cudaMemAccessFlagsProtRead 1
#define cudaMemAccessFlagsProtReadWrite 3

/* Mem access desc / location structs (opaque for compilation) */
#define cudaMemAccessDesc wcceMemPoolProps
#define cudaMemLocation wcceMemPoolProps

/* Mem remap (no Ascend equivalent) */
#define cudaMemRemap wcudaMemRemap
#define cudaMemUnremap wcudaMemUnremap

/* Array extended functions */
#define cudaArrayGetPlane wcudaArrayGetPlane
#define cudaArrayGetMemoryRequirements wcudaArrayGetMemoryRequirements
#define cudaArrayGetSparseProperties wcudaArrayGetSparseProperties
#define cudaMipmappedArrayGetMemoryRequirements wcudaMipmappedArrayGetMemoryRequirements
#define cudaMipmappedArrayGetSparseProperties wcudaMipmappedArrayGetSparseProperties

/* Per-thread-stream (_ptsz) memcpy/memset variants */
#define cudaMemcpyAsync_ptsz wcudaMemcpyAsync
#define cudaMemcpy2DAsync_ptsz wcudaMemcpy2DAsync
#define cudaMemcpy3DAsync_ptsz wcudaMemcpy3DAsync
#define cudaMemsetAsync_ptsz wcudaMemsetAsync
#define cudaMemset2DAsync_ptsz wcudaMemset2DAsync
#define cudaMemset3DAsync_ptsz wcudaMemset3DAsync

/* Memset params struct (for graph nodes) */
#define cudaMemsetParams wcceExtent

// Execution Control
/* cudaLaunchKernel 显式 API：昇腾用 <<<>>> 语法糖（bisheng 原生），此 wrapper 当前返回 not-support */
#define cudaLaunchKernel wcudaLaunchKernel
#define cudaLaunchCooperativeKernel wcudaLaunchCooperativeKernel
#define cudaLaunchKernelEx wcudaLaunchKernelExC
#define cudaLaunchKernelExC wcudaLaunchKernelExC
#define cudaLaunchCooperativeKernelMultiDevice wcudaLaunchCooperativeKernelMultiDevice
#define cudaFuncGetParamInfo wcudaFuncGetParamInfo
#define cudaGetFuncBySymbol wcudaGetFuncBySymbol
#define cudaGetExportTable wcudaGetExportTable
#define cudaGetDriverEntryPoint wcudaGetDriverEntryPoint

/* Launch attribute enums (for cudaLaunchKernelEx; no Ascend equivalent) */
#define cudaLaunchAttribute int
#define cudaLaunchAttributeIgnore 0
#define cudaLaunchAttributeAccessPolicyWindow 1
#define cudaLaunchAttributeCooperative 2
#define cudaLaunchAttributeSynchronizationPolicy 3
#define cudaLaunchAttributeClusterDimension 4
#define cudaLaunchAttributeClusterSchedulingPolicyPreference 5
#define cudaLaunchAttributeProgrammaticStreamSerialization 6
#define cudaLaunchAttributeProgrammaticEvent 7
#define cudaLaunchAttributePreferredSharedMemoryCarveout 8
#define cudaLaunchAttributeMemSyncDomainMap 9
#define cudaLaunchAttributeMemSyncDomain 10
#define cudaLaunchAttributeLaunchCompletionEvent 11
#define cudaLaunchAttributeDeviceUpdatableKernelNode 12
#define cudaLaunchAttributePreferredClusterDimension 13

/* Launch config / params types (opaque for compilation) */
#define cudaLaunchConfig_t wcceExtent
#define cudaLaunchParams wcceExtent
#define cudaFunction_t void*

/* Synchronization policy enum */
#define cudaSynchronizationPolicy int
#define cudaSyncPolicyAuto 1
#define cudaSyncPolicySpin 2
#define cudaSyncPolicyYield 3
#define cudaSyncPolicyBlockingSync 4

/* Mem sync domain */
#define cudaLaunchMemSyncDomainDefault 0
#define cudaLaunchMemSyncDomainRemote 1
#define cudaLaunchMemSyncDomainMap wcceExtent

/* Cluster scheduling */
#define cudaClusterSchedulingPolicyDefault 0
#define cudaClusterSchedulingPolicySpread 1
#define cudaClusterSchedulingPolicyLoadBalancing 2

/* Cooperative launch flags */
#define cudaCooperativeLaunchMultiDeviceNoPreSync 1
#define cudaCooperativeLaunchMultiDeviceNoPostSync 2

/* Per-thread-stream launch variants */
#define cudaStreamWaitEvent_ptsz wcudaStreamWaitEvent
#define cudaLaunchDevice wcudaLaunchKernel
#define cudaLaunchDevice_ptsz wcudaLaunchKernel

/* Func attribute enum */
#define cudaFuncAttribute int
#define cudaFuncAttributeMaxDynamicSharedMemorySize 1
#define cudaFuncAttributePreferredSharedMemoryCarveout 2
#define cudaFuncAttributeMax 3

// Occupancy
#define cudaOccupancyMaxActiveBlocksPerMultiprocessor wcudaOccupancyMaxActiveBlocksPerMultiprocessor
#define cudaOccupancyMaxPotentialBlockSize wcudaOccupancyMaxPotentialBlockSize
#define cudaOccupancyAvailableDynamicSMemPerBlock wcudaOccupancyAvailableDynamicSMemPerBlock
#define cudaOccupancyMaxActiveBlocksPerMultiprocessorWithFlags wcudaOccupancyMaxActiveBlocksPerMultiprocessorWithFlags
#define cudaOccupancyMaxPotentialBlockSizeWithFlags wcudaOccupancyMaxPotentialBlockSizeWithFlags
#define cudaOccupancyMaxPotentialClusterSize wcudaOccupancyMaxPotentialClusterSize
#define cudaOccupancyMaxActiveClusters wcudaOccupancyMaxActiveClusters
#define cudaOccupancyMaxPotentialBlockSizeVariableSMem wcudaOccupancyMaxPotentialBlockSizeVariableSMem
#define cudaOccupancyMaxPotentialBlockSizeVariableSMemWithFlags wcudaOccupancyMaxPotentialBlockSizeVariableSMemWithFlags
#define cudaOccupancyDefault 0
#define cudaOccupancyDisableCachingOverride 1

/* Func cache enum (map to wcce types) */
#define cudaFuncCache wcceFuncCache
#define cudaFuncCachePreferNone wcceFuncCachePreferNone
#define cudaFuncCachePreferShared wcceFuncCachePreferShared
#define cudaFuncCachePreferL1 wcceFuncCachePreferL1
#define cudaFuncCachePreferEqual wcceFuncCachePreferEqual

/* Limit enum (map to wcce types, extend with missing values) */
#define cudaLimit wcceLimit
#define cudaLimitStackSize wcceLimitStackSize
#define cudaLimitPrintfFifoSize wcceLimitPrintfFifoSize
#define cudaLimitMallocHeapSize wcceLimitMallocHeapSize
#define cudaLimitDevRuntimeSyncDepth wcceLimitDevRuntimeSyncDepth
#define cudaLimitDevRuntimePendingLaunchCount wcceLimitDevRuntimePendingLaunchCount
#define cudaLimitMaxL2FetchGranularity 5
#define cudaLimitPersistingL2CacheSize 6

/* Shared mem config enum (map to wcce types) */
#define cudaSharedMemConfig wcceSharedMemConfig
#define cudaSharedMemBankSizeDefault wcceSharedMemBankSizeDefault
#define cudaSharedMemBankSizeFourByte wcceSharedMemBankSizeFourByte
#define cudaSharedMemBankSizeEightByte wcceSharedMemBankSizeEightByte

/* Shared mem carveout */
#define cudaSharedCarveout int
#define cudaSharedmemCarveoutDefault -1
#define cudaSharedmemCarveoutMaxShared 100
#define cudaSharedmemCarveoutMaxL1 0

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

// External Memory & Semaphore（昇腾无对应，not-support）
#define cudaImportExternalMemory wcudaImportExternalMemory
#define cudaDestroyExternalMemory wcudaDestroyExternalMemory
#define cudaImportExternalSemaphore wcudaImportExternalSemaphore
#define cudaDestroyExternalSemaphore wcudaDestroyExternalSemaphore
#define cudaSignalExternalSemaphoresAsync wcudaSignalExternalSemaphoresAsync
#define cudaSignalExternalSemaphoresAsync_v2 wcudaSignalExternalSemaphoresAsync
#define cudaWaitExternalSemaphoresAsync wcudaWaitExternalSemaphoresAsync
#define cudaWaitExternalSemaphoresAsync_v2 wcudaWaitExternalSemaphoresAsync
#define cudaExternalMemoryGetMappedBuffer wcudaExternalMemoryGetMappedBuffer
#define cudaExternalMemoryGetMappedMipmappedArray wcudaExternalMemoryGetMappedMipmappedArray

/* External memory/semaphore types (opaque for compilation) */
#define cudaExternalMemory_t void*
#define cudaExternalSemaphore_t void*
#define cudaExternalMemoryHandleDesc wcceExtent
#define cudaExternalMemoryBufferDesc wcceExtent
#define cudaExternalMemoryMipmappedArrayDesc wcceExtent
#define cudaExternalSemaphoreHandleDesc wcceExtent
#define cudaExternalSemaphoreSignalParams wcceExtent
#define cudaExternalSemaphoreWaitParams wcceExtent
#define cudaExternalSemaphoreSignalParams_v1 wcceExtent
#define cudaExternalSemaphoreWaitParams_v1 wcceExtent
#define cudaExternalSemaphoreSignalNodeParams wcceExtent
#define cudaExternalSemaphoreWaitNodeParams wcceExtent
#define cudaExternalSemaphoreSignalSkipNvSciBufMemSync 1
#define cudaExternalSemaphoreWaitSkipNvSciBufMemSync 1
#define cudaExternalMemoryDedicated 1

/* External memory handle type enum */
#define cudaExternalMemoryHandleType int
#define cudaExternalMemoryHandleTypeOpaqueFd 1
#define cudaExternalMemoryHandleTypeOpaqueWin32 2
#define cudaExternalMemoryHandleTypeOpaqueWin32Kmt 3
#define cudaExternalMemoryHandleTypeD3D12Heap 4
#define cudaExternalMemoryHandleTypeD3D12Resource 5
#define cudaExternalMemoryHandleTypeD3D11Resource 6
#define cudaExternalMemoryHandleTypeD3D11ResourceKmt 7
#define cudaExternalMemoryHandleTypeNvSciBuf 8

/* External semaphore handle type enum */
#define cudaExternalSemaphoreHandleType int
#define cudaExternalSemaphoreHandleTypeOpaqueFd 1
#define cudaExternalSemaphoreHandleTypeOpaqueWin32 2
#define cudaExternalSemaphoreHandleTypeOpaqueWin32Kmt 3
#define cudaExternalSemaphoreHandleTypeD3D12Fence 4
#define cudaExternalSemaphoreHandleTypeD3D11Fence 5
#define cudaExternalSemaphoreHandleTypeNvSciSync 6
#define cudaExternalSemaphoreHandleTypeKeyedMutex 7
#define cudaExternalSemaphoreHandleTypeKeyedMutexKmt 8
#define cudaExternalSemaphoreHandleTypeTimelineSemaphoreFd 9
#define cudaExternalSemaphoreHandleTypeTimelineSemaphoreWin32 10

// Profiler Control（no-op 占位，真机 profiling 走 aclprof）
#define cudaProfilerStart wcudaProfilerStart
#define cudaProfilerStop wcudaProfilerStop
#define cudaProfilerInitialize wcudaProfilerInitialize

/* Output mode (for nvprof/ncu compatibility) */
#define cudaOutputMode int
#define cudaOutputMode_t int
#define cudaCSV 1
#define cudaKeyValuePair 0

/* Driver entry point enum */
#define cudaDriverEntryPointQueryResult int
#define cudaDriverEntryPointSuccess 0
#define cudaDriverEntryPointSymbolNotFound 1
#define cudaDriverEntryPointVersionNotSufficent 2
#define cudaGetDriverEntryPointFlags int

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
/* Extended graph functions (all not-support on Ascend) */
#define cudaGraphAddChildGraphNode wcudaGraphAddChildGraphNode
#define cudaGraphAddDependencies wcudaGraphAddDependencies
#define cudaGraphAddDependencies_v2 wcudaGraphAddDependencies
#define cudaGraphAddEmptyNode wcudaGraphAddEmptyNode
#define cudaGraphAddEventRecordNode wcudaGraphAddEventRecordNode
#define cudaGraphAddEventWaitNode wcudaGraphAddEventWaitNode
#define cudaGraphAddExternalSemaphoresSignalNode wcudaGraphAddExternalSemaphoresSignalNode
#define cudaGraphAddExternalSemaphoresWaitNode wcudaGraphAddExternalSemaphoresWaitNode
#define cudaGraphAddHostNode wcudaGraphAddHostNode
#define cudaGraphAddMemAllocNode wcudaGraphAddMemAllocNode
#define cudaGraphAddMemcpyNode wcudaGraphAddMemcpyNode
#define cudaGraphAddMemcpyNode1D wcudaGraphAddMemcpyNode1D
#define cudaGraphAddMemcpyNodeFromSymbol wcudaGraphAddMemcpyNodeFromSymbol
#define cudaGraphAddMemcpyNodeToSymbol wcudaGraphAddMemcpyNodeToSymbol
#define cudaGraphAddMemFreeNode wcudaGraphAddMemFreeNode
#define cudaGraphAddMemsetNode wcudaGraphAddMemsetNode
#define cudaGraphAddNode wcudaGraphAddNode
#define cudaGraphAddNode_v2 wcudaGraphAddNode
#define cudaGraphChildGraphNodeGetGraph wcudaGraphChildGraphNodeGetGraph
#define cudaGraphDebugDotPrint wcudaGraphDebugDotPrint
#define cudaGraphEventRecordNodeGetEvent wcudaGraphEventRecordNodeGetEvent
#define cudaGraphEventRecordNodeSetEvent wcudaGraphEventRecordNodeSetEvent
#define cudaGraphEventWaitNodeGetEvent wcudaGraphEventWaitNodeGetEvent
#define cudaGraphEventWaitNodeSetEvent wcudaGraphEventWaitNodeSetEvent
#define cudaGraphExecChildGraphNodeSetParams wcudaGraphExecChildGraphNodeSetParams
#define cudaGraphExecEventRecordNodeSetEvent wcudaGraphExecEventRecordNodeSetEvent
#define cudaGraphExecEventWaitNodeSetEvent wcudaGraphExecEventWaitNodeSetEvent
#define cudaGraphExecExternalSemaphoresSignalNodeSetParams wcudaGraphExecExternalSemaphoresSignalNodeSetParams
#define cudaGraphExecExternalSemaphoresWaitNodeSetParams wcudaGraphExecExternalSemaphoresWaitNodeSetParams
#define cudaGraphExecGetFlags wcudaGraphExecGetFlags
#define cudaGraphExecHostNodeSetParams wcudaGraphExecHostNodeSetParams
#define cudaGraphExecKernelNodeSetParams wcudaGraphExecKernelNodeSetParams
#define cudaGraphExecMemcpyNodeSetParams wcudaGraphExecMemcpyNodeSetParams
#define cudaGraphExecMemcpyNodeSetParams1D wcudaGraphExecMemcpyNodeSetParams1D
#define cudaGraphExecMemcpyNodeSetParamsFromSymbol wcudaGraphExecMemcpyNodeSetParamsFromSymbol
#define cudaGraphExecMemcpyNodeSetParamsToSymbol wcudaGraphExecMemcpyNodeSetParamsToSymbol
#define cudaGraphExecMemsetNodeSetParams wcudaGraphExecMemsetNodeSetParams
#define cudaGraphExternalSemaphoresSignalNodeGetParams wcudaGraphExternalSemaphoresSignalNodeGetParams
#define cudaGraphExternalSemaphoresSignalNodeSetParams wcudaGraphExternalSemaphoresSignalNodeSetParams
#define cudaGraphExternalSemaphoresWaitNodeGetParams wcudaGraphExternalSemaphoresWaitNodeGetParams
#define cudaGraphExternalSemaphoresWaitNodeSetParams wcudaGraphExternalSemaphoresWaitNodeSetParams
#define cudaGraphGetEdges wcudaGraphGetEdges
#define cudaGraphGetEdges_v2 wcudaGraphGetEdges
#define cudaGraphHostNodeGetParams wcudaGraphHostNodeGetParams
#define cudaGraphHostNodeSetParams wcudaGraphHostNodeSetParams
#define cudaGraphInstantiateWithFlags wcudaGraphInstantiateWithFlags
#define cudaGraphInstantiateWithParams wcudaGraphInstantiateWithParams
#define cudaGraphKernelNodeCopyAttributes wcudaGraphKernelNodeCopyAttributes
#define cudaGraphKernelNodeGetAttribute wcudaGraphKernelNodeGetAttribute
#define cudaGraphKernelNodeGetParams wcudaGraphKernelNodeGetParams
#define cudaGraphKernelNodeSetAttribute wcudaGraphKernelNodeSetAttribute
#define cudaGraphKernelNodeSetParams wcudaGraphKernelNodeSetParams
#define cudaGraphMemAllocNodeGetParams wcudaGraphMemAllocNodeGetParams
#define cudaGraphMemcpyNodeGetParams wcudaGraphMemcpyNodeGetParams
#define cudaGraphMemcpyNodeSetParams wcudaGraphMemcpyNodeSetParams
#define cudaGraphMemcpyNodeSetParamsFromSymbol wcudaGraphMemcpyNodeSetParamsFromSymbol
#define cudaGraphMemcpyNodeSetParamsToSymbol wcudaGraphMemcpyNodeSetParamsToSymbol
#define cudaGraphMemFreeNodeGetParams wcudaGraphMemFreeNodeGetParams
#define cudaGraphMemsetNodeGetParams wcudaGraphMemsetNodeGetParams
#define cudaGraphMemsetNodeSetParams wcudaGraphMemsetNodeSetParams
#define cudaGraphNodeFindInClone wcudaGraphNodeFindInClone
#define cudaGraphNodeGetDependencies wcudaGraphNodeGetDependencies
#define cudaGraphNodeGetDependencies_v2 wcudaGraphNodeGetDependencies
#define cudaGraphNodeGetDependentNodes wcudaGraphNodeGetDependentNodes
#define cudaGraphNodeGetDependentNodes_v2 wcudaGraphNodeGetDependentNodes
#define cudaGraphNodeGetEnabled wcudaGraphNodeGetEnabled
#define cudaGraphNodeGetType wcudaGraphNodeGetType
#define cudaGraphNodeSetEnabled wcudaGraphNodeSetEnabled
#define cudaGraphReleaseUserObject wcudaGraphReleaseUserObject
#define cudaGraphRemoveDependencies wcudaGraphRemoveDependencies
#define cudaGraphRemoveDependencies_v2 wcudaGraphRemoveDependencies
#define cudaGraphRetainUserObject wcudaGraphRetainUserObject
#define cudaGraphUpload wcudaGraphUpload

/* Graph types and enums */
#define cudaGraphNodeType int
#define cudaGraphNodeTypeKernel 0
#define cudaGraphNodeTypeMemcpy 1
#define cudaGraphNodeTypeMemset 2
#define cudaGraphNodeTypeHost 3
#define cudaGraphNodeTypeGraph 4
#define cudaGraphNodeTypeEmpty 5
#define cudaGraphNodeTypeWaitEvent 6
#define cudaGraphNodeTypeEventRecord 7
#define cudaGraphNodeTypeExtSemaphoreSignal 8
#define cudaGraphNodeTypeExtSemaphoreWait 9
#define cudaGraphNodeTypeMemAlloc 10
#define cudaGraphNodeTypeMemFree 11
#define cudaGraphNodeTypeConditional 12
#define cudaGraphNodeTypeCount 13
#define cudaGraphDependencyType int
#define cudaGraphDependencyTypeDefault 0
#define cudaGraphDependencyTypeProgrammatic 1
#define cudaGraphNodeParams wcceExtent
#define cudaHostNodeParams wcceExtent
#define cudaKernelNodeParams wcceExtent
#define cudaKernelNodeAttrID int
#define cudaKernelNodeAttrValue wcceExtent
#define cudaKernelNodeAttributeAccessPolicyWindow 1
#define cudaKernelNodeAttributeCooperative 2
#define cudaKernelNodeAttributePriority 3
#define cudaGraphExecUpdateResult int
#define cudaGraphExecUpdateSuccess 0
#define cudaGraphExecUpdateError wcceExtent
#define cudaGraphExecUpdateResultInfo wcceExtent
#define cudaGraphExecUpdateErrorTopologyChanged 1
#define cudaGraphExecUpdateErrorNodeTypeChanged 2
#define cudaGraphExecUpdateErrorFunctionChanged 3
#define cudaGraphExecUpdateErrorParametersChanged 4
#define cudaGraphExecUpdateErrorNotSupported 5
#define cudaGraphExecUpdateErrorUnsupportedFunctionChange 6
#define cudaGraphExecUpdateErrorAttributesChanged 7
#define cudaGraphInstantiateResult int
#define cudaGraphInstantiateSuccess 0
#define cudaGraphInstantiateError wcceExtent
#define cudaGraphInstantiateParams wcceExtent
#define cudaGraphInstantiateFlags int
#define cudaGraphInstantiateFlagAutoFreeOnLaunch 1
#define cudaGraphInstantiateFlagUpload 2
#define cudaGraphInstantiateFlagDeviceLaunch 4
#define cudaGraphInstantiateFlagUseNodePriority 8
#define cudaGraphInstantiateInvalidStructure 1
#define cudaGraphInstantiateMultipleDevicesNotSupported 2
#define cudaGraphInstantiateNodeOperationNotSupported 3
#define cudaGraphMemAttributeType int
#define cudaGraphMemAttrUsedMemCurrent 0
#define cudaGraphMemAttrUsedMemHigh 1
#define cudaGraphMemAttrReservedMemCurrent 2
#define cudaGraphMemAttrReservedMemHigh 3
#define cudaGraphDebugDotFlags int
#define cudaGraphDebugDotFlagsVerbose 1
#define cudaGraphDebugDotFlagsHandles 2
#define cudaGraphDebugDotFlagsKernelNodeParams 4
#define cudaGraphDebugDotFlagsMemcpyNodeParams 8
#define cudaGraphDebugDotFlagsMemsetNodeParams 16
#define cudaGraphDebugDotFlagsHostNodeParams 32
#define cudaGraphDebugDotFlagsEventNodeParams 64
#define cudaGraphDebugDotFlagsExtSemasSignalNodeParams 128
#define cudaGraphDebugDotFlagsExtSemasWaitNodeParams 256
#define cudaGraphDebugDotFlagsKernelNodeAttributes 512
#define cudaGraphDebugDotFlagsConditionalNodeParams 1024
#define cudaGraphConditionalHandle void*
#define cudaGraphConditionalHandleFlags int
#define cudaGraphConditionalHandleCreate wcudaGraphConditionalHandleCreate
#define cudaGraphCondTypeIf 0
#define cudaGraphCondTypeWhile 1
#define cudaGraphCondTypeSwitch 2
#define cudaGraphCondAssignDefault 0
#define cudaGraphSetConditional wcudaGraphSetConditional
#define cudaGraphEdgeData wcceExtent
#define cudaGraphKernelNodePortDefault 0
#define cudaGraphKernelNodePortLaunchCompletion 1
#define cudaGraphKernelNodePortProgrammatic 2
#define cudaGraphDeviceNode_t wcceGraphNode_t
#define cudaGraphUserObjectMove wcudaGraphRetainUserObject
#define cudaUserObject_t void*
#define cudaUserObjectFlags int
#define cudaUserObjectNoDestructorSync 1
#define cudaUserObjectCreate wcudaUserObjectCreate
#define cudaUserObjectRelease wcudaUserObjectRelease
#define cudaUserObjectRetain wcudaUserObjectRetain
#define cudaUserObjectRetainFlags wcudaUserObjectRetain

/* Access policy window (for L2 cache hints) */
#define cudaAccessPolicyWindow wcceExtent
#define cudaAccessProperty int
#define cudaAccessPropertyNormal 0
#define cudaAccessPropertyStreaming 1
#define cudaAccessPropertyPersisting 2

// Texture/Surface（昇腾无对应，全部 not-support）
#define cudaCreateChannelDesc wcudaCreateChannelDesc
#define cudaCreateTextureObject wcudaCreateTextureObject
#define cudaCreateTextureObject_v2 wcudaCreateTextureObject
#define cudaDestroyTextureObject wcudaDestroyTextureObject
#define cudaCreateSurfaceObject wcudaCreateSurfaceObject
#define cudaDestroySurfaceObject wcudaDestroySurfaceObject
#define cudaBindTexture wcudaBindTexture
#define cudaUnbindTexture wcudaUnbindTexture
#define cudaBindTexture2D wcudaBindTexture2D
#define cudaBindTextureToArray wcudaBindTextureToArray
#define cudaBindTextureToMipmappedArray wcudaBindTextureToMipmappedArray
#define cudaBindSurfaceToArray wcudaBindSurfaceToArray
#define cudaGetTextureReference wcudaGetTextureReference
#define cudaGetSurfaceReference wcudaGetSurfaceReference
#define cudaGetTextureAlignmentOffset wcudaGetTextureAlignmentOffset

/* Array flags (CUDA standard) */
#define cudaArrayDefault 0
#define cudaArrayLayered 1
#define cudaArraySurfaceLoadStore 2
#define cudaArrayCubemap 4
#define cudaArrayTextureGather 8
#define cudaArrayColorAttachment 0x20
#define cudaArraySparse 0x40
#define cudaArrayDeferredMapping 0x80

/* Array/memory requirements types */
#define cudaArray wcceArray_t
#define cudaMipmappedArray wcceMipmappedArray_t
#define cudaMipmappedArray_const_t wcceMipmappedArray_t
#define cudaArrayMemoryRequirements wcceExtent
#define cudaArraySparseProperties wcceExtent
#define cudaArraySparsePropertiesSingleMipTail 1

/* Texture type enum */
#define cudaTextureType1D 1
#define cudaTextureType2D 2
#define cudaTextureType3D 3
#define cudaTextureTypeCubemap 4
#define cudaTextureType1DLayered 5
#define cudaTextureType2DLayered 6
#define cudaTextureTypeCubemapLayered 7

/* Surface type enum */
#define cudaSurfaceType1D 1
#define cudaSurfaceType2D 2
#define cudaSurfaceType3D 3
#define cudaSurfaceTypeCubemap 4
#define cudaSurfaceType1DLayered 5
#define cudaSurfaceType2DLayered 6
#define cudaSurfaceTypeCubemapLayered 7

/* Surface/texture modes */
#define cudaSurfaceBoundaryMode int
#define cudaSurfaceFormatMode int
#define cudaFormatModeAuto 0
#define cudaFormatModeForced 1
#define cudaBoundaryModeZero 0
#define cudaBoundaryModeClamp 1
#define cudaBoundaryModeTrap 2

/* Channel format kind extensions (for block compressed formats) */
#define cudaChannelFormatKindNV12 0x10

/* IPC handle types */
#define cudaIpcMemHandle_st wcceIpcMemHandle_t
#define cudaIpcEventHandle_st wcceIpcEventHandle_t

/* UUID type */
#define cudaUUID_t wcceExtent

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

// Legacy / deprecated APIs
#define cudaSetDoubleForDevice wcudaSetDoubleForDevice
#define cudaSetDoubleForHost wcudaSetDoubleForHost

// Graphics Interop（昇腾无对应，not-support stubs for compilation）
#define cudaGraphicsResource void*
#define cudaGraphicsResource_t void*
#define cudaGraphicsMapFlags int
#define cudaGraphicsMapFlagsNone 0
#define cudaGraphicsMapFlagsReadOnly 1
#define cudaGraphicsMapFlagsWriteDiscard 2
#define cudaGraphicsRegisterFlags int
#define cudaGraphicsRegisterFlagsNone 0
#define cudaGraphicsRegisterFlagsReadOnly 1
#define cudaGraphicsRegisterFlagsWriteDiscard 2
#define cudaGraphicsRegisterFlagsSurfaceLoadStore 4
#define cudaGraphicsRegisterFlagsTextureGather 8
#define cudaGraphicsMapResources wcudaGraphicsMapResources
#define cudaGraphicsUnmapResources wcudaGraphicsUnmapResources
#define cudaGraphicsUnregisterResource wcudaGraphicsUnregisterResource
#define cudaGraphicsResourceGetMappedPointer wcudaGraphicsResourceGetMappedPointer
#define cudaGraphicsResourceGetMappedMipmappedArray wcudaGraphicsResourceGetMappedMipmappedArray
#define cudaGraphicsResourceSetMapFlags wcudaGraphicsResourceSetMapFlags
#define cudaGraphicsSubResourceGetMappedArray wcudaGraphicsSubResourceGetMappedArray
#define cudaGraphicsCubeFace int
#define cudaGraphicsCubeFacePositiveX 0
#define cudaGraphicsCubeFaceNegativeX 1
#define cudaGraphicsCubeFacePositiveY 2
#define cudaGraphicsCubeFaceNegativeY 3
#define cudaGraphicsCubeFacePositiveZ 4
#define cudaGraphicsCubeFaceNegativeZ 5

// GL Interop (stubs)
#define cudaGLDeviceList int
#define cudaGLDeviceListAll 0
#define cudaGLDeviceListCurrentFrame 1
#define cudaGLDeviceListNextFrame 2
#define cudaGLGetDevices wcudaGLGetDevices
#define cudaGLMapBufferObject wcudaGLMapBufferObject
#define cudaGLMapBufferObjectAsync wcudaGLMapBufferObjectAsync
#define cudaGLMapFlags int
#define cudaGLMapFlagsNone 0
#define cudaGLMapFlagsReadOnly 1
#define cudaGLMapFlagsWriteDiscard 2
#define cudaGLRegisterBufferObject wcudaGLRegisterBufferObject
#define cudaGLSetBufferObjectMapFlags wcudaGLSetBufferObjectMapFlags
#define cudaGLSetGLDevice wcudaGLSetGLDevice
#define cudaGLUnmapBufferObject wcudaGLUnmapBufferObject
#define cudaGLUnmapBufferObjectAsync wcudaGLUnmapBufferObjectAsync
#define cudaGLUnregisterBufferObject wcudaGLUnregisterBufferObject
#define cudaGraphicsGLRegisterBuffer wcudaGraphicsGLRegisterBuffer
#define cudaGraphicsGLRegisterImage wcudaGraphicsGLRegisterImage

// EGL Interop (stubs)
#define cudaEglColorFormat int
#define cudaEglColorFormat_enum int
#define cudaEglColorFormatY 0
#define cudaEglColorFormatY_ER 1
#define cudaEglFrame wcceExtent
#define cudaEglFrame_st wcceExtent
#define cudaEglFrameType int
#define cudaEglFrameType_enum int
#define cudaEglFrameTypeArray 0
#define cudaEglFrameTypePitch 1
#define cudaEglPlaneDesc wcceExtent
#define cudaEglPlaneDesc_st wcceExtent
#define cudaEglResourceLocationFlags int
#define cudaEglResourceLocationFlags_enum int
#define cudaEglResourceLocationSysmem 0
#define cudaEglResourceLocationVidmem 1
#define cudaEglStreamConnection void*
#define cudaEGLStreamConsumerAcquireFrame wcudaEGLStreamConsumerAcquireFrame
#define cudaEGLStreamConsumerConnect wcudaEGLStreamConsumerConnect
#define cudaEGLStreamConsumerConnectWithFlags wcudaEGLStreamConsumerConnectWithFlags
#define cudaEGLStreamConsumerDisconnect wcudaEGLStreamConsumerDisconnect
#define cudaEGLStreamConsumerReleaseFrame wcudaEGLStreamConsumerReleaseFrame
#define cudaEGLStreamProducerConnect wcudaEGLStreamProducerConnect
#define cudaEGLStreamProducerDisconnect wcudaEGLStreamProducerDisconnect
#define cudaEGLStreamProducerPresentFrame wcudaEGLStreamProducerPresentFrame
#define cudaEGLStreamProducerReturnFrame wcudaEGLStreamProducerReturnFrame
#define cudaGraphicsEGLRegisterImage wcudaGraphicsEGLRegisterImage
#define cudaGraphicsResourceGetMappedEglFrame wcudaGraphicsResourceGetMappedEglFrame
#define cudaEventCreateFromEGLSync wcudaEventCreateFromEGLSync

// VDPAU Interop (stubs)
#define cudaVDPAUGetDevice wcudaVDPAUGetDevice
#define cudaVDPAUSetVDPAUDevice wcudaVDPAUSetVDPAUDevice
#define cudaGraphicsVDPAURegisterOutputSurface wcudaGraphicsVDPAURegisterOutputSurface
#define cudaWGLGetDevice wcudaWGLGetDevice

// Cooperative Groups (stubs)
#define cudaCGScope int
#define cudaCGScopeInvalid 0
#define cudaCGScopeGrid 1
#define cudaCGScopeMultiGrid 2
#define cudaCGSynchronize wcudaCGSynchronize
#define cudaCGSynchronizeGrid wcudaCGSynchronizeGrid
#define cudaCGGetIntrinsicHandle wcudaCGGetIntrinsicHandle
#define cudaCGGetRank wcudaCGGetRank
#define cudaCGGetSize wcudaCGGetSize

// Execution Context / Green Context (stubs)
#define cudaExecutionContext_t wcceExtent
#define cudaExecutionContext_st wcceExtent
#define cudaExecutionCtxDestroy wcudaExecutionCtxDestroy
#define cudaExecutionCtxGetDevice wcudaExecutionCtxGetDevice
#define cudaExecutionCtxGetDevResource wcudaExecutionCtxGetDevResource
#define cudaExecutionCtxGetId wcudaExecutionCtxGetId
#define cudaExecutionCtxRecordEvent wcudaExecutionCtxRecordEvent
#define cudaExecutionCtxStreamCreate wcudaExecutionCtxStreamCreate
#define cudaExecutionCtxSynchronize wcudaExecutionCtxSynchronize
#define cudaExecutionCtxWaitEvent wcudaExecutionCtxWaitEvent
#define cudaGreenCtxCreate wcudaGreenCtxCreate

// Device Resource (stubs)
#define cudaDevResource wcceExtent
#define cudaDevResourceDesc_t wcceExtent
#define cudaDevResourceType int
#define cudaDevResourceTypeInvalid 0
#define cudaDevResourceTypeSm 1
#define cudaDevResourceTypeWorkqueue 2
#define cudaDevResourceTypeWorkqueueConfig 3
#define cudaDevSmResource wcceExtent
#define cudaDevSmResourceGroupParams wcceExtent
#define cudaDevSmResourceSplit wcudaDevSmResourceSplit
#define cudaDevSmResourceSplitByCount wcudaDevSmResourceSplitByCount
#define cudaDevWorkqueueConfigResource wcceExtent
#define cudaDevWorkqueueConfigScope int
#define cudaDevWorkqueueConfigScopeDeviceCtx 0
#define cudaDevWorkqueueConfigScopeGreenCtxBalanced 1
#define cudaDevWorkqueueResource wcceExtent
#define cudaDeviceGetDevResource wcudaDeviceGetDevResource
#define cudaDeviceGetExecutionCtx wcudaDeviceGetExecutionCtx
#define cudaDeviceGetNvSciSyncAttributes wcudaDeviceGetNvSciSyncAttributes
#define cudaDeviceGetTexture1DLinearMaxWidth wcudaDeviceGetTexture1DLinearMaxWidth
#define cudaDevicePropDontCare wcceExtent
#define cudaDevResourceGenerateDesc wcudaDevResourceGenerateDesc
#define cudaNvSciSyncAttrSignal 1
#define cudaNvSciSyncAttrWait 2

// Canny Ext types (for CUDAArray interop)
#define cudaXtDesc wcceExtent
#define cudaXtDesc_t wcceExtent
#define cudaLibXtDesc wcceExtent
#define cudaLibXtDesc_t wcceExtent
#define cudaLibXtCopyType int
#define cudaXtCopyType_t int

// cudaError type alias (for code that references the enum tag)
#define cudaError int

// cudaOcc internal types (referenced by occupancy_calculator.h in CUDA samples)
#define cudaOccError int
#define cudaOccError_enum int
#define cudaOccResult wcceExtent
#define cudaOccDeviceProp wcceExtent
#define cudaOccDevicePropCheck wcceExtent
#define cudaOccDeviceState wcceExtent
#define cudaOccDeviceStateCheck wcceExtent
#define cudaOccFuncAttributes wcceExtent
#define cudaOccFuncAttributesCheck wcceExtent
#define cudaOccFuncShmemConfig int
#define cudaOccFuncShmemConfig_enum int
#define cudaOccCacheConfig int
#define cudaOccCacheConfig_enum int
#define cudaOccCarveoutConfig int
#define cudaOccCarveoutConfig_enum int
#define cudaOccPartitionedGCConfig int
#define cudaOccPartitionedGCConfig_enum int
#define cudaOccPartitionedGCExpected int
#define cudaOccPartitionedGCSupport int
#define cudaOccPartitionedGCSupport_enum int
#define cudaOccPartitionedGlobalCachingModeSupport int
#define cudaOccLimitingFactor int
#define cudaOccLimitingFactor_enum int
#define cudaOccInputCheck wcceExtent
#define cudaOccMaxActiveBlocksPerMultiprocessor int
#define cudaOccMaxBlocksPerMultiprocessor int
#define cudaOccMaxBlocksPerSMRegsLimit int
#define cudaOccMaxBlocksPerSMSmemLimit int
#define cudaOccMaxBlocksPerSMWarpsLimit int
#define cudaOccMaxPotentialOccupancyBlockSize int
#define cudaOccMaxPotentialOccupancyBlockSizeVariableSMem int
#define cudaOccRegAllocationGranularity int
#define cudaOccRegAllocationMaxPerThread int
#define cudaOccSMemAllocationGranularity int
#define cudaOccSMemPerBlock int
#define cudaOccSMemPerMultiprocessor int
#define cudaOccSMemPreference int
#define cudaOccSMemPreferenceVoltaPlus int
#define cudaOccSubPartitionsPerMultiprocessor int
#define cudaOccAlignUpShmemSizeVoltaPlus int
#define cudaOccAvailableDynamicSMemPerBlock int
#define cudaOccMaxBlocksPerSM int

/* Additional missing functions */
#define cudaCtxResetPersistingL2Cache wcudaCtxResetPersistingL2Cache
#define cudaGetParameterBuffer wcudaGetParameterBuffer
#define cudaGetParameterBufferV2 wcudaGetParameterBuffer
#define cudaLaunchAttributePriority 14
#define cudaLaunchDeviceV2 wcudaLaunchKernel
#define cudaLaunchDeviceV2_ptsz wcudaLaunchKernel
#define cudaMemAllocNodeParams wcceExtent
#define cudaMemPoolPtrExportData wcceIpcMemHandle_t

/* Block compressed channel format kinds (extended enum values) */
#define cudaChannelFormatKindSignedBlockCompressed4 0x20
#define cudaChannelFormatKindSignedBlockCompressed5 0x21
#define cudaChannelFormatKindSignedBlockCompressed6H 0x22
#define cudaChannelFormatKindSignedNormalized16X1 0x23
#define cudaChannelFormatKindSignedNormalized16X2 0x24
#define cudaChannelFormatKindSignedNormalized16X4 0x25
#define cudaChannelFormatKindSignedNormalized8X1 0x26
#define cudaChannelFormatKindSignedNormalized8X2 0x27
#define cudaChannelFormatKindSignedNormalized8X4 0x28
#define cudaChannelFormatKindUnsignedBlockCompressed1 0x30
#define cudaChannelFormatKindUnsignedBlockCompressed1SRGB 0x31
#define cudaChannelFormatKindUnsignedBlockCompressed2 0x32
#define cudaChannelFormatKindUnsignedBlockCompressed2SRGB 0x33
#define cudaChannelFormatKindUnsignedBlockCompressed3 0x34
#define cudaChannelFormatKindUnsignedBlockCompressed3SRGB 0x35
#define cudaChannelFormatKindUnsignedBlockCompressed4 0x36
#define cudaChannelFormatKindUnsignedBlockCompressed5 0x37
#define cudaChannelFormatKindUnsignedBlockCompressed6H 0x38
#define cudaChannelFormatKindUnsignedBlockCompressed7 0x39
#define cudaChannelFormatKindUnsignedBlockCompressed7SRGB 0x3A
#define cudaChannelFormatKindUnsignedNormalized16X1 0x3B
#define cudaChannelFormatKindUnsignedNormalized16X2 0x3C
#define cudaChannelFormatKindUnsignedNormalized16X4 0x3D
#define cudaChannelFormatKindUnsignedNormalized8X1 0x3E
#define cudaChannelFormatKindUnsignedNormalized8X2 0x3F
#define cudaChannelFormatKindUnsignedNormalized8X4 0x40

/* Resource view format block compressed (extended enum values) */
#define cudaResViewFormatSignedBlockCompressed4 0x50
#define cudaResViewFormatSignedBlockCompressed5 0x51
#define cudaResViewFormatSignedBlockCompressed6H 0x52
#define cudaResViewFormatUnsignedBlockCompressed1 0x53
#define cudaResViewFormatUnsignedBlockCompressed2 0x54
#define cudaResViewFormatUnsignedBlockCompressed3 0x55
#define cudaResViewFormatUnsignedBlockCompressed4 0x56
#define cudaResViewFormatUnsignedBlockCompressed5 0x57
#define cudaResViewFormatUnsignedBlockCompressed6H 0x58
#define cudaResViewFormatUnsignedBlockCompressed7 0x59

/* EGL color format enum values (sequential integers for compilation only) */
#define cudaEglColorFormatA 0
#define cudaEglColorFormatABGR 1
#define cudaEglColorFormatARGB 2
#define cudaEglColorFormatAYUV 3
#define cudaEglColorFormatAYUV_ER 4
#define cudaEglColorFormatBayer10BGGR 5
#define cudaEglColorFormatBayer10CCCC 6
#define cudaEglColorFormatBayer10GBRG 7
#define cudaEglColorFormatBayer10GRBG 8
#define cudaEglColorFormatBayer10RGGB 9
#define cudaEglColorFormatBayer12BCCR 10
#define cudaEglColorFormatBayer12BGGR 11
#define cudaEglColorFormatBayer12CBRC 12
#define cudaEglColorFormatBayer12CCCC 13
#define cudaEglColorFormatBayer12CRBC 14
#define cudaEglColorFormatBayer12GBRG 15
#define cudaEglColorFormatBayer12GRBG 16
#define cudaEglColorFormatBayer12RCCB 17
#define cudaEglColorFormatBayer12RGGB 18
#define cudaEglColorFormatBayer14BGGR 19
#define cudaEglColorFormatBayer14GBRG 20
#define cudaEglColorFormatBayer14GRBG 21
#define cudaEglColorFormatBayer14RGGB 22
#define cudaEglColorFormatBayer20BGGR 23
#define cudaEglColorFormatBayer20GBRG 24
#define cudaEglColorFormatBayer20GRBG 25
#define cudaEglColorFormatBayer20RGGB 26
#define cudaEglColorFormatBayerBCCR 27
#define cudaEglColorFormatBayerBGGR 28
#define cudaEglColorFormatBayerCBRC 29
#define cudaEglColorFormatBayerCRBC 30
#define cudaEglColorFormatBayerGBRG 31
#define cudaEglColorFormatBayerGRBG 32
#define cudaEglColorFormatBayerIspBGGR 33
#define cudaEglColorFormatBayerIspGBRG 34
#define cudaEglColorFormatBayerIspGRBG 35
#define cudaEglColorFormatBayerIspRGGB 36
#define cudaEglColorFormatBayerRCCB 37
#define cudaEglColorFormatBayerRGGB 38
#define cudaEglColorFormatBGRA 39
#define cudaEglColorFormatL 40
#define cudaEglColorFormatR 41
#define cudaEglColorFormatRG 42
#define cudaEglColorFormatRGBA 43
#define cudaEglColorFormatUYVY422 44
#define cudaEglColorFormatUYVY_ER 45
#define cudaEglColorFormatVYUY 46
#define cudaEglColorFormatVYUY_ER 47
#define cudaEglColorFormatY10_709_ER 48
#define cudaEglColorFormatY10_ER 49
#define cudaEglColorFormatY10V10U10_420SemiPlanar 50
#define cudaEglColorFormatY10V10U10_420SemiPlanar_2020 51
#define cudaEglColorFormatY10V10U10_420SemiPlanar_709 52
#define cudaEglColorFormatY10V10U10_420SemiPlanar_709_ER 53
#define cudaEglColorFormatY10V10U10_420SemiPlanar_ER 54
#define cudaEglColorFormatY10V10U10_422SemiPlanar 55
#define cudaEglColorFormatY10V10U10_422SemiPlanar_2020 56
#define cudaEglColorFormatY10V10U10_422SemiPlanar_709 57
#define cudaEglColorFormatY10V10U10_444SemiPlanar 58
#define cudaEglColorFormatY10V10U10_444SemiPlanar_709_ER 59
#define cudaEglColorFormatY10V10U10_444SemiPlanar_ER 60
#define cudaEglColorFormatY12_709_ER 61
#define cudaEglColorFormatY12_ER 62
#define cudaEglColorFormatY12V12U12_420SemiPlanar 63
#define cudaEglColorFormatY12V12U12_420SemiPlanar_709_ER 64
#define cudaEglColorFormatY12V12U12_420SemiPlanar_ER 65
#define cudaEglColorFormatY12V12U12_444SemiPlanar 66
#define cudaEglColorFormatY12V12U12_444SemiPlanar_709_ER 67
#define cudaEglColorFormatY12V12U12_444SemiPlanar_ER 68
#define cudaEglColorFormatY_709_ER 69
#define cudaEglColorFormatYUV420Planar 70
#define cudaEglColorFormatYUV420Planar_2020 71
#define cudaEglColorFormatYUV420Planar_709 72
#define cudaEglColorFormatYUV420Planar_ER 73
#define cudaEglColorFormatYUV420SemiPlanar 74
#define cudaEglColorFormatYUV420SemiPlanar_2020 75
#define cudaEglColorFormatYUV420SemiPlanar_709 76
#define cudaEglColorFormatYUV420SemiPlanar_ER 77
#define cudaEglColorFormatYUV422Planar 78
#define cudaEglColorFormatYUV422Planar_ER 79
#define cudaEglColorFormatYUV422SemiPlanar 80
#define cudaEglColorFormatYUV422SemiPlanar_ER 81
#define cudaEglColorFormatYUV444Planar 82
#define cudaEglColorFormatYUV444Planar_ER 83
#define cudaEglColorFormatYUV444SemiPlanar 84
#define cudaEglColorFormatYUV444SemiPlanar_ER 85
#define cudaEglColorFormatYUVA 86
#define cudaEglColorFormatYUVA_ER 87
#define cudaEglColorFormatYUYV422 88
#define cudaEglColorFormatYUYV_ER 89
#define cudaEglColorFormatYVU420Planar 90
#define cudaEglColorFormatYVU420Planar_2020 91
#define cudaEglColorFormatYVU420Planar_709 92
#define cudaEglColorFormatYVU420Planar_ER 93
#define cudaEglColorFormatYVU420SemiPlanar 94
#define cudaEglColorFormatYVU420SemiPlanar_2020 95
#define cudaEglColorFormatYVU420SemiPlanar_709 96
#define cudaEglColorFormatYVU420SemiPlanar_ER 97
#define cudaEglColorFormatYVU422Planar 98
#define cudaEglColorFormatYVU422Planar_ER 99
#define cudaEglColorFormatYVU422SemiPlanar 100
#define cudaEglColorFormatYVU422SemiPlanar_ER 101
#define cudaEglColorFormatYVU444Planar 102
#define cudaEglColorFormatYVU444Planar_ER 103
#define cudaEglColorFormatYVU444SemiPlanar 104
#define cudaEglColorFormatYVU444SemiPlanar_ER 105
#define cudaEglColorFormatYVYU 106
#define cudaEglColorFormatYVYU_ER 107

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
