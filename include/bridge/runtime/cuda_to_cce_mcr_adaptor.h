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
 *   - cudaSuccess  -> ACL_ERROR_NONE   （原生 ACL 错误码）
 *
 * ⚠️ 说明：以下宏值/枚举值依据 CANN 文档（见 ASCEND_RESEARCH_NOTES.md §1-§2）编写，
 *    具体 ACL 枚举名（ACL_MEMCPY_HOST_TO_DEVICE 等）以真机 acl_base.h / acl_rt.h 为准，
 *    若命名不符，在真机上核对后批量替换即可，宏表结构不变。
 */

#include "acl/acl_base.h"
#include "acl/acl_rt.h"

//============================CUDA Runtime API====================//

//------------ error code ------------//
/* cudaError_t 是枚举，aclError 是 int32。宏替换后 wcuda 返回 aclError，
 * 用户代码里 `cudaError_t err = cudaMalloc(...)` 编译为 `aclError err = wcudaMalloc(...)`，
 * 类型一致，无需强转。 */
#define cudaError_t aclError
#define cudaSuccess ACL_ERROR_NONE

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
#define cudaMemcpyDefault ACL_MEMCPY_DEVICE_TO_DEVICE

//------------ handle types ------------//
#define cudaStream_t aclrtStream
#define cudaEvent_t aclrtEvent

//------------ device properties ------------//
/* 昇腾无同形 DeviceProp 结构。由 cu2cce 自定义 _cceDeviceProp_t（见
 * include/bridge/runtime/cuda_wrapper_types.h）模拟 cudaDeviceProp 常用字段。 */
#define cudaDeviceProp _cceDeviceProp_t

//--------------------- function replace --------------------//

// Device Management
#define cudaGetDeviceCount wcudaGetDeviceCount
#define cudaSetDevice wcudaSetDevice
#define cudaGetDevice wcudaGetDevice
#define cudaDeviceSynchronize wcudaDeviceSynchronize
#define cudaDeviceReset wcudaDeviceReset
#define cudaGetDeviceProperties wcudaGetDeviceProperties
#define cudaDeviceGetAttribute wcudaDeviceGetAttribute

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
#define cudaStreamDestroy wcudaStreamDestroy
#define cudaStreamSynchronize wcudaStreamSynchronize
#define cudaStreamQuery wcudaStreamQuery
#define cudaStreamWaitEvent wcudaStreamWaitEvent

// Event Management
#define cudaEventCreate wcudaEventCreate
#define cudaEventCreateWithFlags wcudaEventCreateWithFlags
#define cudaEventDestroy wcudaEventDestroy
#define cudaEventRecord wcudaEventRecord
#define cudaEventSynchronize wcudaEventSynchronize
#define cudaEventQuery wcudaEventQuery
#define cudaEventElapsedTime wcudaEventElapsedTime

// Memory Management
#define cudaMalloc wcudaMalloc
#define cudaFree wcudaFree
#define cudaMemcpy wcudaMemcpy
#define cudaMemcpyAsync wcudaMemcpyAsync
#define cudaMemcpy2D wcudaMemcpy2D
#define cudaMemcpy2DAsync wcudaMemcpy2DAsync
#define cudaMemset wcudaMemset
#define cudaMemsetAsync wcudaMemsetAsync
#define cudaMemset2D wcudaMemset2D
#define cudaMemset2DAsync wcudaMemset2DAsync
#define cudaMallocPitch wcudaMallocPitch
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

// Error Handling
#define cudaGetLastError wcudaGetLastError
#define cudaPeekAtLastError wcudaPeekAtLastError
#define cudaGetErrorString wcudaGetErrorString
#define cudaGetErrorName wcudaGetErrorName

// Version
#define cudaRuntimeGetVersion wcudaRuntimeGetVersion
#define cudaDriverGetVersion wcudaDriverGetVersion

#endif //__CUDA_TO_CCE_ADAPTOR_H__
