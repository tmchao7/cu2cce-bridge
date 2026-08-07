#ifndef __CUDA_TO_CCE_ADAPTOR_H__
#define __CUDA_TO_CCE_ADAPTOR_H__

/*
 * 核心宏替换表（Phase A1 最小核心集）。
 *
 * 仿照 cu-bridge 的 include/bridge/runtime/cuda_to_maca_mcr_adaptor.h：
 *   宏把用户代码里的 CUDA 符号（函数名、枚举值、错误码、类型）替换为
 *   cu2cce-bridge 的 wcuda* wrapper / acl* 原生符号。
 *
 * 本文件是"编译期宏替换"这一迁移线的核心。目标平台从 MXMACA(mc*) 换成昇腾(acl*)：
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

//============================CUDA Runtime API====================//

//------------ error code ------------//
/* cudaError_t 是枚举，aclError 是 int32。宏替换后 wcuda* 返回 aclError，
 * 用户代码里 `cudaError_t err = cudaMalloc(...)` 编译为 `aclError err = wcudaMalloc(...)`，
 * 类型一致，无需强转。 */
#define cudaError_t aclError
#define cudaSuccess ACL_SUCCESS

// 常用错误码（部分映射，非全量；全量映射表见 docs/01_Design/CUDA_TO_CCE_PORTING_PLAN.md §Phase A1）
#define cudaErrorInvalidValue ACL_ERROR_INVALID_PARAM
#define cudaErrorMemoryAllocation ACL_ERROR_RT_MEMORY_ALLOCATION
#define cudaErrorInitializationError ACL_ERROR_RT_INTERNAL_ERROR
#define cudaErrorInvalidDevice ACL_ERROR_INVALID_DEVICE
#define cudaErrorNoDevice ACL_ERROR_RT_INVALID_DEVICEID
#define cudaErrorInvalidResourceHandle ACL_ERROR_INVALID_RESOURCE_HANDLE
#define cudaErrorNotReady ACL_ERROR_RT_EVENT_STATUS_ERROR

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
#define cudaMemset wcudaMemset
#define cudaMemsetAsync wcudaMemsetAsync
#define cudaMemGetInfo wcudaMemGetInfo
#define cudaMallocHost wcudaMallocHost
#define cudaFreeHost wcudaFreeHost

// Error Handling
#define cudaGetLastError wcudaGetLastError
#define cudaPeekAtLastError wcudaPeekAtLastError
#define cudaGetErrorString wcudaGetErrorString
#define cudaGetErrorName wcudaGetErrorName

// Version
#define cudaRuntimeGetVersion wcudaRuntimeGetVersion
#define cudaDriverGetVersion wcudaDriverGetVersion

#endif //__CUDA_TO_CCE_ADAPTOR_H__
