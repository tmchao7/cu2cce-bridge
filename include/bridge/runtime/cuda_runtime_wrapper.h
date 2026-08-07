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

/* Device 配置类（昇腾无对应或语义不同；声明保证应用可编译，实现返回默认/not-support） */
aclError CUBRIDGE_API wcudaDeviceGetPCIBusId(char *pciBusId, int len, int device);
aclError CUBRIDGE_API wcudaDeviceGetByPCIBusId(int *device, const char *pciBusId);
aclError CUBRIDGE_API wcudaDeviceGetStreamPriorityRange(int *leastPriority, int *greatestPriority);
aclError CUBRIDGE_API wcudaDeviceGetCacheConfig(int *pCacheConfig);
aclError CUBRIDGE_API wcudaDeviceSetCacheConfig(int cacheConfig);
aclError CUBRIDGE_API wcudaDeviceGetSharedMemConfig(int *pConfig);
aclError CUBRIDGE_API wcudaDeviceSetSharedMemConfig(int config);
aclError CUBRIDGE_API wcudaDeviceGetLimit(size_t *pValue, int limit);
aclError CUBRIDGE_API wcudaDeviceSetLimit(int limit, size_t value);

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
aclError CUBRIDGE_API wcudaEventRecordWithFlags(aclrtEvent event, aclrtStream stream /* =0 */,
                                                unsigned int flags /* =0 */);
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
/* Peer 拷贝（昇腾有 peer access：aclrtDeviceCanAccessPeer 等） */
aclError CUBRIDGE_API wcudaMemcpyPeer(void *dst, int dstDevice, const void *src, int srcDevice,
                                      size_t count);
aclError CUBRIDGE_API wcudaMemcpyPeerAsync(void *dst, int dstDevice, const void *src,
                                           int srcDevice, size_t count, aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemcpy2DAsync(void *dst, size_t dpitch, const void *src, size_t spitch,
                                         size_t width, size_t height, aclrtMemcpyKind kind,
                                         aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemset(void *devPtr, int value, size_t count);
aclError CUBRIDGE_API wcudaMemsetAsync(void *devPtr, int value, size_t count,
                                       aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemset2D(void *devPtr, size_t pitch, int value, size_t width,
                                    size_t height);
/* Symbol 内存（昇腾无 device symbol 概念，返回 not-support） */
/* Array（昇腾无 array/mipmap 概念，全部 not-support；用 void* 简化句柄类型保证可编译） */
aclError CUBRIDGE_API wcudaMallocArray(void *array, const void *desc, size_t width,
                                       size_t height, unsigned int flags);
aclError CUBRIDGE_API wcudaMalloc3DArray(void *array, const void *desc, void *extent,
                                         unsigned int flags);
aclError CUBRIDGE_API wcudaMallocMipmappedArray(void *mipmappedArray, const void *desc,
                                                void *extent, unsigned int numLevels,
                                                unsigned int flags);
aclError CUBRIDGE_API wcudaFreeArray(void *array);
aclError CUBRIDGE_API wcudaFreeMipmappedArray(void *mipmappedArray);
aclError CUBRIDGE_API wcudaMemcpyToArray(void *dst, size_t wOffset, size_t hOffset,
                                         const void *src, size_t count, int kind);
aclError CUBRIDGE_API wcudaMemcpyFromArray(void *dst, const void *src, size_t wOffset,
                                           size_t hOffset, size_t count, int kind);
aclError CUBRIDGE_API wcudaMemcpy2DToArray(void *dst, size_t wOffset, size_t hOffset,
                                           const void *src, size_t spitch, size_t width,
                                           size_t height, int kind);
aclError CUBRIDGE_API wcudaMemcpy2DFromArray(void *dst, size_t dpitch, const void *src,
                                             size_t wOffset, size_t hOffset, size_t width,
                                             size_t height, int kind);

aclError CUBRIDGE_API wcudaGetSymbolAddress(void **devPtr, const void *symbol);
aclError CUBRIDGE_API wcudaGetSymbolSize(size_t *size, const void *symbol);
aclError CUBRIDGE_API wcudaMemcpyToSymbol(const void *symbol, const void *src, size_t count,
                                          size_t offset /* =0 */, int kind /* =H2D */);
aclError CUBRIDGE_API wcudaMemcpyFromSymbol(void *dst, const void *symbol, size_t count,
                                            size_t offset /* =0 */, int kind /* =D2H */);
aclError CUBRIDGE_API wcudaMemset2DAsync(void *devPtr, size_t pitch, int value, size_t width,
                                         size_t height, aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemGetInfo(size_t *free, size_t *total);
/* 昇腾无 pitch 分配接口，wcudaMallocPitch 用 aclrtMalloc 模拟（宽度对齐，见实现） */
aclError CUBRIDGE_API wcudaMallocPitch(void **devPtr, size_t *pitch, size_t width, size_t height);

/* Stream Ordered Memory Allocator（SOMA）：
 * cudaMallocAsync/FreeAsync fallback 到 aclrtMalloc/Free（无 pool 语义）；
 * MemPool 系列昇腾无对应，not-support。 */
aclError CUBRIDGE_API wcudaMallocAsync(void **devPtr, size_t size, aclrtStream hStream);
aclError CUBRIDGE_API wcudaFreeAsync(void *devPtr, aclrtStream hStream);
aclError CUBRIDGE_API wcudaMemPoolCreate(void *memPool, const void *poolProps);
aclError CUBRIDGE_API wcudaMemPoolDestroy(void *memPool);
aclError CUBRIDGE_API wcudaDeviceGetMemPool(void *memPool, int device);
aclError CUBRIDGE_API wcudaDeviceSetMemPool(int device, void *memPool);

/* Unified Memory：昇腾无 managed memory，fallback 到 device 分配（无自动迁移语义，标注差异）。
 * MemAdvise/Prefetch 为 no-op/not-support（昇腾无统一寻址）。 */
aclError CUBRIDGE_API wcudaMallocManaged(void **devPtr, size_t size, unsigned int flags);
aclError CUBRIDGE_API wcudaMemAdvise(const void *devPtr, size_t count, int advice, int device);
aclError CUBRIDGE_API wcudaMemPrefetchAsync(const void *devPtr, size_t count, int dstDevice,
                                            aclrtStream stream /* =0 */);
/* PointerGetAttributes：昇腾无统一寻址，简化返回 device 指针属性 */
aclError CUBRIDGE_API wcudaPointerGetAttributes(void *attributes, const void *ptr);
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

/* ============ Func 属性 ============ */
/* 昇腾无 kernel 属性查询，返回 not-support */
aclError CUBRIDGE_API wcudaFuncGetAttributes(void *attr, const void *func);

/* ============ Occupancy ============ */
/* 昇腾无 occupancy 计算接口，给 SIMT 合理默认值（block 上限 2048）。 */
aclError CUBRIDGE_API wcudaOccupancyMaxActiveBlocksPerMultiprocessor(int *numBlocks,
                                                                    const void *func,
                                                                    int blockSize,
                                                                    size_t dynamicSMemSize);
aclError CUBRIDGE_API wcudaOccupancyMaxPotentialBlockSize(int *minGridSize, int *blockSize,
                                                         const void *func,
                                                         size_t dynamicSMemSize /* =0 */,
                                                         int blockSizeLimit /* =0 */);

/* ============ Execution Control ============ */
/* CUDA 的 cudaLaunchKernel(hostFunction, grid, block, args, smem, stream) 需要函数指针→kernel stub。
 * 昇腾 SIMT 的主流启动是 kernel<<<blocks, threads, stream>>>(...) 语法糖（bisheng 原生处理），
 * 显式 cudaLaunchKernel 的完整桥接需 aclrtLaunchKernelV2 + kernel stub 注册（Phase A3 收尾，真机验证）。
 * 当前返回 not-support，应用请用 <<<>>> 启动。 */
aclError CUBRIDGE_API wcudaLaunchKernel(const void *hostFunction, int gridDimX, int gridDimY,
                                        int gridDimZ, int blockDimX, int blockDimY, int blockDimZ,
                                        void **args, size_t sharedMemBytes, aclrtStream stream);

/* ============ Profiler Control ============ */
/* 昇腾 profiling 用 aclprof（API 面不同），此处 no-op 占位 */
aclError CUBRIDGE_API wcudaProfilerStart(void);
aclError CUBRIDGE_API wcudaProfilerStop(void);

/* ============ Texture/Surface（昇腾无对应，全部 not-support；句柄用 void* 简化） ============ */
/* cudaCreateChannelDesc 返回 4-int 结构，用 unsigned long long 承载（x,y,z,w 各 16bit） */
unsigned long long CUBRIDGE_API wcudaCreateChannelDesc(int x, int y, int z, int w);
aclError CUBRIDGE_API wcudaCreateTextureObject(void *pTexObject, const void *pResDesc,
                                               const void *pTexDesc, const void *pResViewDesc);
aclError CUBRIDGE_API wcudaDestroyTextureObject(void *texObject);
aclError CUBRIDGE_API wcudaCreateSurfaceObject(void *pSurfObject, const void *pResDesc);
aclError CUBRIDGE_API wcudaDestroySurfaceObject(void *surfObject);
aclError CUBRIDGE_API wcudaBindTexture(size_t *offset, const void *texref, const void *devPtr,
                                       const void *desc, size_t size);

/* ============ Thread Management（废弃 API，映射到等价运行时调用） ============ */
aclError CUBRIDGE_API wcudaThreadSynchronize(void);
aclError CUBRIDGE_API wcudaThreadExit(void);
aclError CUBRIDGE_API wcudaThreadGetCacheConfig(int *cacheConfig);

/* ============ Graph（昇腾无 CUDA Graph 对应，核心函数 not-support；句柄用 void* 简化） ============ */
aclError CUBRIDGE_API wcudaGraphCreate(void *pGraph, unsigned int flags);
aclError CUBRIDGE_API wcudaGraphDestroy(void *graph);
aclError CUBRIDGE_API wcudaGraphInstantiate(void *pGraphExec, void *graph, void *pErrorNode,
                                            char *pLogBuffer, size_t bufferSize);
aclError CUBRIDGE_API wcudaGraphLaunch(void *graphExec, aclrtStream stream);
aclError CUBRIDGE_API wcudaGraphExecDestroy(void *graphExec);
aclError CUBRIDGE_API wcudaGraphExecUpdate(void *hGraphExec, void *hGraph, void *hErrorNode_out,
                                           int *updateResult_out);
aclError CUBRIDGE_API wcudaGraphAddKernelNode(void *pGraphNode, void *graph,
                                             const void *pDependencies, size_t numDependencies,
                                             const void *pNodeParams);
aclError CUBRIDGE_API wcudaGraphMemcpyNodeSetParams1D(void *node, void *dst, const void *src,
                                                      size_t count, int kind);

/* ============ Host Function & Func 属性 ============ */
/* cudaLaunchHostFunc(stream, fn, userData)：昇腾有 aclrtLaunchCallback 对应。
 * cudaHostFn_t 签名 void(*)(void*) 与 aclrtCallback 一致。 */
typedef void (*wcceHostFn_t)(void *);
aclError CUBRIDGE_API wcudaLaunchHostFunc(aclrtStream stream, wcceHostFn_t fn, void *userData);
/* 昇腾无 func 属性/名字接口，声明保证编译 */
aclError CUBRIDGE_API wcudaFuncSetAttribute(const void *func, int attr, int value);
aclError CUBRIDGE_API wcudaFuncGetName(const char **name, const void *func);

/* ============ Version ============ */
aclError CUBRIDGE_API wcudaRuntimeGetVersion(int *runtimeVersion);
aclError CUBRIDGE_API wcudaDriverGetVersion(int *driverVersion);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //__CUDA_RUNTIME_WRAPPER_H__
