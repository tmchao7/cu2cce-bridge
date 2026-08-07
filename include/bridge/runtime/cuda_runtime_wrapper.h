#ifndef __CUDA_RUNTIME_WRAPPER_H__
#define __CUDA_RUNTIME_WRAPPER_H__

/*
 * wcuda wrapper 声明（Phase A1 最小核心集）。
 *
 * 仿照 cu-bridge 的 include/bridge/runtime/cuda_runtime_wrapper.h：
 *   - 函数名 wcuda 与 CUDA 同名函数一一对应，签名保持 CUDA 形状；
 *   - 返回类型用 aclError（宏表已把 cudaError_t 替换为 aclError）；
 *   - 句柄类型用 aclrtStream / aclrtEvent（宏表已把 cudaStream_t / cudaEvent_t 替换）；
 *   - 实现在 src/bridge/runtime/src/cuda_runtime_wrapper.cpp，内部调用 aclrt* 原生 API。
 *
 * 本文件只列 Phase A1 需要的最小核心集（device/stream/event/memory/error/version）。
 * 其余 wcuda 函数（launch/graph/texture/occupancy 等）随 Phase A3+ 逐步补充，
 * 补充时照抄 cu-bridge 声明行，把 mc 类型换成对应 acl 类型即可。
 */

#include "cuda_benchmark_targets.h"
#include "cuda_wrapper_internal_types.h"
#include "cuda_wrapper_types.h"

#include <acl/acl_base.h>
#include <acl/acl_rt.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============ Device Management ============ */
aclError CUBRIDGE_API wcudaGetDeviceCount(int *count);
aclError CUBRIDGE_API wcudaSetDevice(int device);
aclError CUBRIDGE_API wcudaGetDevice(int *device);
aclError CUBRIDGE_API wcudaDeviceSynchronize(void);
aclError CUBRIDGE_API wcudaDeviceReset(void);
aclError CUBRIDGE_API wcudaGetDeviceProperties(_cceDeviceProp_t *prop, int device);
/* 常用设备属性查询（attr 取值见 cudaDeviceAttr 枚举，见 adaptor 宏表）
 * 昇腾无 aclrtDeviceGetAttribute，从 _cceDeviceProp_t 字段映射。 */
aclError CUBRIDGE_API wcudaDeviceGetAttribute(int *value, int attr, int device);

/* ============ Peer Device Memory Access ============ */
aclError CUBRIDGE_API wcudaDeviceCanAccessPeer(int *canAccessPeer, int device, int peerDevice);
aclError CUBRIDGE_API wcudaDeviceEnablePeerAccess(int peerDevice, unsigned int flags);
aclError CUBRIDGE_API wcudaDeviceDisablePeerAccess(int peerDevice);

/* ============ Context Management ============ */
/* 昇腾 Context 与 CUDA 语义相近（aclrtCreateContext / SetCurrentContext）。
 * CUDA 的 cudaCtx* 属于 Driver API 层，这里提供 Runtime 风格的 wcuda 封装。 */
typedef aclrtContext wcceContext_t; /* cudaCtx_t 的昇腾对应 */
aclError CUBRIDGE_API wcudaCtxCreate(wcceContext_t *ctx, unsigned int flags, int device);
aclError CUBRIDGE_API wcudaCtxDestroy(wcceContext_t ctx);
aclError CUBRIDGE_API wcudaCtxSetCurrent(wcceContext_t ctx);
aclError CUBRIDGE_API wcudaCtxGetCurrent(wcceContext_t *ctx);
aclError CUBRIDGE_API wcudaCtxSynchronize(void);

/* ============ Stream Management ============ */
aclError CUBRIDGE_API wcudaStreamCreate(aclrtStream *pStream);
aclError CUBRIDGE_API wcudaStreamCreateWithFlags(aclrtStream *pStream, unsigned int flags);
aclError CUBRIDGE_API wcudaStreamDestroy(aclrtStream stream);
aclError CUBRIDGE_API wcudaStreamSynchronize(aclrtStream stream);
aclError CUBRIDGE_API wcudaStreamQuery(aclrtStream stream);
aclError CUBRIDGE_API wcudaStreamWaitEvent(aclrtStream stream, aclrtEvent event,
                                           unsigned int flags /* =0 */);

/* Stream capture（CUDA Graph 捕获）：昇腾无对应，占位返回 not-support。
 * graph 参数用 void** 简化（真机若需要 CUDA Graph 桥再接 acl 对应物）。 */
aclError CUBRIDGE_API wcudaStreamBeginCapture(aclrtStream stream, int mode);
aclError CUBRIDGE_API wcudaStreamEndCapture(aclrtStream stream, void **graph);
aclError CUBRIDGE_API wcudaStreamIsCapturing(aclrtStream stream, int *status);
aclError CUBRIDGE_API wcudaStreamGetCaptureInfo(aclrtStream stream, int *status,
                                                unsigned long long *id_out /* =0 */);
aclError CUBRIDGE_API wcudaStreamGetCaptureInfo_v2(aclrtStream stream, int *status,
                                                   unsigned long long *id_out /* =0 */,
                                                   void *graph_out /* =0 */,
                                                   const void **dependencies_out /* =0 */,
                                                   size_t *numDependencies_out /* =0 */);

/* ============ Event Management ============ */
aclError CUBRIDGE_API wcudaEventCreate(aclrtEvent *event);
aclError CUBRIDGE_API wcudaEventCreateWithFlags(aclrtEvent *event, unsigned int flags);
aclError CUBRIDGE_API wcudaEventDestroy(aclrtEvent event);
aclError CUBRIDGE_API wcudaEventRecord(aclrtEvent event, aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaEventSynchronize(aclrtEvent event);
aclError CUBRIDGE_API wcudaEventQuery(aclrtEvent event);
aclError CUBRIDGE_API wcudaEventElapsedTime(float *ms, aclrtEvent start, aclrtEvent end);

/* ============ Memory Management ============ */
aclError CUBRIDGE_API wcudaMalloc(void **ptr, size_t sizeBytes);
aclError CUBRIDGE_API wcudaFree(void *ptr);
aclError CUBRIDGE_API wcudaMemcpy(void *dst, const void *src, size_t count, aclrtMemcpyKind kind);
aclError CUBRIDGE_API wcudaMemcpyAsync(void *dst, const void *src, size_t count,
                                       aclrtMemcpyKind kind, aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemcpy2D(void *dst, size_t dpitch, const void *src, size_t spitch,
                                    size_t width, size_t height, aclrtMemcpyKind kind);
aclError CUBRIDGE_API wcudaMemcpy2DAsync(void *dst, size_t dpitch, const void *src, size_t spitch,
                                         size_t width, size_t height, aclrtMemcpyKind kind,
                                         aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemset(void *devPtr, int value, size_t count);
aclError CUBRIDGE_API wcudaMemsetAsync(void *devPtr, int value, size_t count,
                                       aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemset2D(void *devPtr, size_t pitch, int value, size_t width,
                                    size_t height);
aclError CUBRIDGE_API wcudaMemset2DAsync(void *devPtr, size_t pitch, int value, size_t width,
                                         size_t height, aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemGetInfo(size_t *free, size_t *total);
/* 昇腾无 pitch 分配接口，wcudaMallocPitch 用 aclrtMalloc 模拟（宽度对齐，见实现） */
aclError CUBRIDGE_API wcudaMallocPitch(void **devPtr, size_t *pitch, size_t width, size_t height);
aclError CUBRIDGE_API wcudaMallocHost(void **ptr, size_t size);
aclError CUBRIDGE_API wcudaMallocHostWithFlags(void **ptr, size_t size, unsigned int flags);
aclError CUBRIDGE_API wcudaFreeHost(void *ptr);
/* 昇腾无对应或语义不同，声明以保证宏表可映射、编译通过；运行时返回"不支持" */
aclError CUBRIDGE_API wcudaHostGetDevicePointer(void **pDevice, void *pHost, unsigned int flags);
aclError CUBRIDGE_API wcudaHostAlloc(void **pHost, size_t size, unsigned int flags);
aclError CUBRIDGE_API wcudaHostRegister(void *ptr, size_t size, unsigned int flags);
aclError CUBRIDGE_API wcudaHostUnregister(void *ptr);
aclError CUBRIDGE_API wcudaHostGetFlags(unsigned int *pFlags, void *pHost);

/* ============ Error Handling ============ */
aclError CUBRIDGE_API wcudaGetLastError(void);
aclError CUBRIDGE_API wcudaPeekAtLastError(void);
const char CUBRIDGE_API *wcudaGetErrorString(aclError error);
const char CUBRIDGE_API *wcudaGetErrorName(aclError error);

/* ============ Execution Control ============ */
/* CUDA 的 cudaLaunchKernel(hostFunction, grid, block, args, smem, stream) 需要函数指针→kernel stub。
 * 昇腾 SIMT 的主流启动是 kernel<<<blocks, threads, stream>>>(...) 语法糖（bisheng 原生处理），
 * 显式 cudaLaunchKernel 的完整桥接需 aclrtLaunchKernelV2 + kernel stub 注册（Phase A3 收尾，真机验证）。
 * 当前返回 not-support，应用请用 <<<>>> 启动。 */
aclError CUBRIDGE_API wcudaLaunchKernel(const void *hostFunction, int gridDimX, int gridDimY,
                                        int gridDimZ, int blockDimX, int blockDimY, int blockDimZ,
                                        void **args, size_t sharedMemBytes, aclrtStream stream);

/* ============ Version ============ */
aclError CUBRIDGE_API wcudaRuntimeGetVersion(int *runtimeVersion);
aclError CUBRIDGE_API wcudaDriverGetVersion(int *driverVersion);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //__CUDA_RUNTIME_WRAPPER_H__
