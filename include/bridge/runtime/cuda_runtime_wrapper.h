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
 * 本文件按 CUDA Runtime 能力族组织已暴露的 bridge 入口。每个声明都必须在
 * `cuda_to_cce_mcr_adaptor.h` 有对应 alias；没有可靠 Ascend 等价物的接口仍可
 * 为源码兼容保留声明，但实现必须返回明确的 `ACL_ERROR_API_NOT_SUPPORT`。
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
aclError CUBRIDGE_API wcudaSetDeviceFlags(unsigned int flags);
aclError CUBRIDGE_API wcudaGetDeviceFlags(unsigned int *flags);
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

/* Additional device management (stubs for compilation; see macro adaptor) */
aclError CUBRIDGE_API wcudaChooseDevice(int *device, const void *prop);
aclError CUBRIDGE_API wcudaSetValidDevices(int *device, int count);
aclError CUBRIDGE_API wcudaDeviceGetP2PAttribute(int *value, int attr, int device, int peerDevice);
aclError CUBRIDGE_API wcudaDeviceGetDefaultMemPool(wcceMemPool_t *pool, int device);
aclError CUBRIDGE_API wcudaDeviceFlushGPUDirectRDMAWrites(int target, int scope);
aclError CUBRIDGE_API wcudaDeviceGetGraphMemAttribute(int device, int attr, void *value);
aclError CUBRIDGE_API wcudaDeviceSetGraphMemAttribute(int device, int attr, const void *value);
aclError CUBRIDGE_API wcudaDeviceGraphMemTrim(int device);
aclError CUBRIDGE_API wcudaDeviceGetDevResource(void *resource, int device, int type);
aclError CUBRIDGE_API wcudaDeviceGetExecutionCtx(void *ctx, int device);
aclError CUBRIDGE_API wcudaDeviceGetNvSciSyncAttributes(void *attr, int device, unsigned int flags);
aclError CUBRIDGE_API wcudaDeviceGetTexture1DLinearMaxWidth(size_t *width, int device);
aclError CUBRIDGE_API wcudaCtxResetPersistingL2Cache(void);

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
/* CUDA stream priority/ID metadata has no complete public CANN equivalent.
 * These entries preserve source-level declarations and report unsupported
 * semantics instead of silently claiming priority ordering. */
aclError CUBRIDGE_API wcudaStreamCreateWithPriority(aclrtStream *pStream, unsigned int flags,
                                                    int priority);
aclError CUBRIDGE_API wcudaStreamGetFlags(aclrtStream stream, unsigned int *flags);
aclError CUBRIDGE_API wcudaStreamGetPriority(aclrtStream stream, int *priority);
aclError CUBRIDGE_API wcudaStreamGetDevice(aclrtStream stream, int *device);
aclError CUBRIDGE_API wcudaStreamGetId(aclrtStream stream, unsigned long long *streamId);
aclError CUBRIDGE_API wcudaStreamCopyAttributes(aclrtStream dst, aclrtStream src);
aclError CUBRIDGE_API wcudaStreamGetAttribute(aclrtStream stream, wcceStreamAttrID attr,
                                               wcceStreamAttrValue *value);
aclError CUBRIDGE_API wcudaStreamSetAttribute(aclrtStream stream, wcceStreamAttrID attr,
                                               const wcceStreamAttrValue *value);
aclError CUBRIDGE_API wcudaStreamAttachMemAsync(aclrtStream stream, void *devPtr,
                                                 size_t length = 0, unsigned int flags = 0);
aclError CUBRIDGE_API wcudaStreamDestroy(aclrtStream stream);
aclError CUBRIDGE_API wcudaStreamSynchronize(aclrtStream stream);
aclError CUBRIDGE_API wcudaStreamQuery(aclrtStream stream);
aclError CUBRIDGE_API wcudaStreamWaitEvent(aclrtStream stream, aclrtEvent event,
                                           unsigned int flags = 0);
aclError CUBRIDGE_API wcudaStreamAddCallback(aclrtStream stream,
                                             wcceStreamCallback_t callback,
                                             void *userData, unsigned int flags);

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
aclError CUBRIDGE_API wcudaStreamUpdateCaptureDependencies(aclrtStream stream, const void *deps,
                                                            size_t numDeps, unsigned int flags);
aclError CUBRIDGE_API wcudaStreamBeginCaptureToGraph(aclrtStream stream, void *graph,
                                                      const void *deps, size_t numDeps,
                                                      int captureMode);
aclError CUBRIDGE_API wcudaStreamGetDevResource(aclrtStream stream, void *resource, int type);
aclError CUBRIDGE_API wcudaThreadExchangeStreamCaptureMode(int *mode);

/* ============ Event Management ============ */
aclError CUBRIDGE_API wcudaEventCreate(aclrtEvent *event);
aclError CUBRIDGE_API wcudaEventCreateWithFlags(aclrtEvent *event, unsigned int flags);
aclError CUBRIDGE_API wcudaEventCreateFromEGLSync(aclrtEvent *event, void *eglsync,
                                                   unsigned int flags);
aclError CUBRIDGE_API wcudaEventDestroy(aclrtEvent event);
aclError CUBRIDGE_API wcudaEventRecord(aclrtEvent event, aclrtStream stream = 0);
aclError CUBRIDGE_API wcudaEventRecordWithFlags(aclrtEvent event, aclrtStream stream = 0,
                                                unsigned int flags = 0);
aclError CUBRIDGE_API wcudaEventSynchronize(aclrtEvent event);
aclError CUBRIDGE_API wcudaEventQuery(aclrtEvent event);
aclError CUBRIDGE_API wcudaEventElapsedTime(float *ms, aclrtEvent start, aclrtEvent end);

/* ============ Memory Management ============ */
aclError CUBRIDGE_API wcudaMalloc(void **ptr, size_t sizeBytes);
aclError CUBRIDGE_API wcudaFree(void *ptr);
aclError CUBRIDGE_API wcudaMemcpy(void *dst, const void *src, size_t count, aclrtMemcpyKind kind);
/* CUDA's Default direction requires pointer classification. CANN has no
 * public default-direction primitive; do not silently guess DEVICE_TO_DEVICE. */
aclError CUBRIDGE_API wcudaMemcpyDefault(void *dst, const void *src, size_t count);
aclError CUBRIDGE_API wcudaMemcpyAsync(void *dst, const void *src, size_t count,
                                       aclrtMemcpyKind kind, aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemcpy2D(void *dst, size_t dpitch, const void *src, size_t spitch,
                                    size_t width, size_t height, aclrtMemcpyKind kind);
/* 3D 拷贝（结构可见，但 Ascend bridge 当前无 CUDA 3D 执行路径） */
aclError CUBRIDGE_API wcudaMemcpy3D(const wcceMemcpy3DParms *p);
aclError CUBRIDGE_API wcudaMemcpy3DAsync(const wcceMemcpy3DParms *p, aclrtStream stream);
aclError CUBRIDGE_API wcudaMemcpy3DPeer(const wcceMemcpy3DPeerParms *p);
aclError CUBRIDGE_API wcudaMemcpy3DPeerAsync(const wcceMemcpy3DPeerParms *p,
                                             aclrtStream stream /* =0 */);

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
aclError CUBRIDGE_API wcudaMallocArray(wcceArray_t *array, const wcceChannelFormatDesc *desc,
                                       size_t width, size_t height, unsigned int flags);
aclError CUBRIDGE_API wcudaMalloc3DArray(wcceArray_t *array, const wcceChannelFormatDesc *desc,
                                         wcceExtent extent, unsigned int flags);
aclError CUBRIDGE_API wcudaMallocMipmappedArray(wcceMipmappedArray_t *mipmappedArray,
                                                const wcceChannelFormatDesc *desc,
                                                wcceExtent extent, unsigned int numLevels,
                                                unsigned int flags);
aclError CUBRIDGE_API wcudaFreeArray(wcceArray_t array);
aclError CUBRIDGE_API wcudaFreeMipmappedArray(wcceMipmappedArray_t mipmappedArray);
aclError CUBRIDGE_API wcudaMemcpyToArray(wcceArray_t dst, size_t wOffset, size_t hOffset,
                                         const void *src, size_t count, int kind);
aclError CUBRIDGE_API wcudaMemcpyFromArray(void *dst, wcceArray_const_t src, size_t wOffset,
                                           size_t hOffset, size_t count, int kind);
aclError CUBRIDGE_API wcudaMemcpy2DToArray(wcceArray_t dst, size_t wOffset, size_t hOffset,
                                           const void *src, size_t spitch, size_t width,
                                           size_t height, int kind);
aclError CUBRIDGE_API wcudaMemcpy2DFromArray(void *dst, size_t dpitch, wcceArray_const_t src,
                                             size_t wOffset, size_t hOffset, size_t width,
                                             size_t height, int kind);
aclError CUBRIDGE_API wcudaMemcpyToArrayAsync(wcceArray_t dst, size_t wOffset,
                                                  size_t hOffset, const void *src, size_t count,
                                                  int kind, aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemcpyFromArrayAsync(void *dst, wcceArray_const_t src, size_t wOffset,
                                                size_t hOffset, size_t count, int kind,
                                                aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemcpy2DToArrayAsync(wcceArray_t dst, size_t wOffset,
                                                 size_t hOffset, const void *src, size_t spitch,
                                                 size_t width, size_t height, int kind,
                                                 aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemcpy2DFromArrayAsync(void *dst, size_t dpitch, wcceArray_const_t src,
                                                   size_t wOffset, size_t hOffset, size_t width,
                                                   size_t height, int kind,
                                                   aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemcpyArrayToArray(wcceArray_t dst, size_t wOffsetDst,
                                              size_t hOffsetDst, wcceArray_const_t src,
                                              size_t wOffsetSrc, size_t hOffsetSrc, size_t width,
                                              size_t height, int kind);
aclError CUBRIDGE_API wcudaMemcpy2DArrayToArray(wcceArray_t dst, size_t wOffsetDst,
                                                size_t hOffsetDst, wcceArray_const_t src,
                                                size_t wOffsetSrc, size_t hOffsetSrc, size_t width,
                                                size_t height, int kind);
aclError CUBRIDGE_API wcudaArrayGetInfo(wcceChannelFormatDesc *desc, wcceExtent *extent,
                                        unsigned int *flags, wcceArray_const_t array);
aclError CUBRIDGE_API wcudaGetChannelDesc(wcceChannelFormatDesc *desc, wcceArray_const_t array);

aclError CUBRIDGE_API wcudaGetSymbolAddress(void **devPtr, const void *symbol);
aclError CUBRIDGE_API wcudaGetSymbolSize(size_t *size, const void *symbol);
aclError CUBRIDGE_API wcudaMemcpyToSymbol(const void *symbol, const void *src, size_t count,
                                          size_t offset /* =0 */, int kind /* =H2D */);
aclError CUBRIDGE_API wcudaMemcpyFromSymbol(void *dst, const void *symbol, size_t count,
                                            size_t offset /* =0 */, int kind /* =D2H */);
aclError CUBRIDGE_API wcudaMemcpyToSymbolAsync(const void *symbol, const void *src,
                                                 size_t count, size_t offset, int kind,
                                                 aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemcpyFromSymbolAsync(void *dst, const void *symbol, size_t count,
                                                 size_t offset, int kind,
                                                 aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemset2DAsync(void *devPtr, size_t pitch, int value, size_t width,
                                         size_t height, aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemset3D(wccePitchedPtr pitchedDevPtr, int value,
                                    wcceExtent extent);
aclError CUBRIDGE_API wcudaMemset3DAsync(wccePitchedPtr pitchedDevPtr, int value,
                                         wcceExtent extent, aclrtStream stream /* =0 */);
aclError CUBRIDGE_API wcudaMemGetInfo(size_t *free, size_t *total);
aclError CUBRIDGE_API wcudaThreadGetLimit(size_t *pValue, int limit);
aclError CUBRIDGE_API wcudaThreadSetLimit(int limit, size_t value);
/* 昇腾无 pitch 分配接口，wcudaMallocPitch 用 aclrtMalloc 模拟（宽度对齐，见实现） */
aclError CUBRIDGE_API wcudaMallocPitch(void **devPtr, size_t *pitch, size_t width, size_t height);
aclError CUBRIDGE_API wcudaMalloc3D(wccePitchedPtr *pitchedDevPtr, wcceExtent extent);

/* Stream Ordered Memory Allocator（SOMA）：
 * cudaMallocAsync/FreeAsync fallback 到 aclrtMalloc/Free（无 pool 语义）；
 * MemPool 系列昇腾无对应，not-support。 */
aclError CUBRIDGE_API wcudaMallocAsync(void **devPtr, size_t size, aclrtStream hStream);
aclError CUBRIDGE_API wcudaFreeAsync(void *devPtr, aclrtStream hStream);
aclError CUBRIDGE_API wcudaMemPoolCreate(wcceMemPool_t *memPool,
                                         const wcceMemPoolProps *poolProps);
aclError CUBRIDGE_API wcudaMemPoolDestroy(wcceMemPool_t memPool);
aclError CUBRIDGE_API wcudaDeviceGetMemPool(wcceMemPool_t *memPool, int device);
aclError CUBRIDGE_API wcudaDeviceSetMemPool(int device, wcceMemPool_t memPool);
aclError CUBRIDGE_API wcudaMallocFromPoolAsync(void **ptr, size_t size, wcceMemPool_t memPool,
                                                aclrtStream stream);
aclError CUBRIDGE_API wcudaMemPoolTrimTo(wcceMemPool_t memPool, size_t minBytesToKeep);
aclError CUBRIDGE_API wcudaMemPoolGetAttribute(wcceMemPool_t memPool, wcceMemPoolAttr attr,
                                               void *value);
aclError CUBRIDGE_API wcudaMemPoolSetAttribute(wcceMemPool_t memPool, wcceMemPoolAttr attr,
                                               const void *value);
aclError CUBRIDGE_API wcudaIpcGetMemHandle(wcceIpcMemHandle_t *handle, void *devPtr);
aclError CUBRIDGE_API wcudaIpcOpenMemHandle(void **devPtr, wcceIpcMemHandle_t handle,
                                             unsigned int flags);
aclError CUBRIDGE_API wcudaIpcCloseMemHandle(void *devPtr);
aclError CUBRIDGE_API wcudaIpcGetEventHandle(wcceIpcEventHandle_t *handle, aclrtEvent event);
aclError CUBRIDGE_API wcudaIpcOpenEventHandle(aclrtEvent *event, wcceIpcEventHandle_t handle);

/* Unified Memory：昇腾无 managed memory，fallback 到 device 分配（无自动迁移语义，标注差异）。
 * MemAdvise/Prefetch 为 no-op/not-support（昇腾无统一寻址）。 */
aclError CUBRIDGE_API wcudaMallocManaged(void **devPtr, size_t size, unsigned int flags = 0);
aclError CUBRIDGE_API wcudaMemAdvise(const void *devPtr, size_t count, int advice, int device);
aclError CUBRIDGE_API wcudaMemPrefetchAsync(const void *devPtr, size_t count, int dstDevice,
                                            aclrtStream stream = 0);
/* PointerGetAttributes：昇腾无统一寻址，简化返回 device 指针属性 */
aclError CUBRIDGE_API wcudaPointerGetAttributes(wccePointerAttributes *attributes,
                                                 const void *ptr);
aclError CUBRIDGE_API wcudaMallocHost(void **ptr, size_t size);
aclError CUBRIDGE_API wcudaMallocHostWithFlags(void **ptr, size_t size, unsigned int flags);
aclError CUBRIDGE_API wcudaFreeHost(void *ptr);
/* 昇腾无对应或语义不同，声明以保证宏表可映射、编译通过；运行时返回"不支持" */
aclError CUBRIDGE_API wcudaHostGetDevicePointer(void **pDevice, void *pHost, unsigned int flags);
aclError CUBRIDGE_API wcudaHostAlloc(void **pHost, size_t size, unsigned int flags);
aclError CUBRIDGE_API wcudaHostRegister(void *ptr, size_t size, unsigned int flags);
aclError CUBRIDGE_API wcudaHostUnregister(void *ptr);
aclError CUBRIDGE_API wcudaHostGetFlags(unsigned int *pFlags, void *pHost);

/* Extended memory management (stubs for compilation) */
aclError CUBRIDGE_API wcudaMemAdvise_v2(const void *devPtr, size_t count, int advice, int device);
aclError CUBRIDGE_API wcudaMemRangeGetAttribute(void *data, size_t dataSize, int attr,
                                                 const void *devPtr, size_t count);
aclError CUBRIDGE_API wcudaMemRangeGetAttributes(void **data, size_t *dataSizes, int *attrs,
                                                  size_t numAttrs, const void *devPtr, size_t count);
aclError CUBRIDGE_API wcudaMemPoolGetAccess(int *access, wcceMemPool_t pool, const void *ptr);
aclError CUBRIDGE_API wcudaMemPoolSetAccess(wcceMemPool_t pool, const void *maps, size_t numMaps);
aclError CUBRIDGE_API wcudaMemPoolExportToShareableHandle(void *handle, wcceMemPool_t pool,
                                                          unsigned int handleType, unsigned int flags);
aclError CUBRIDGE_API wcudaMemPoolImportFromShareableHandle(wcceMemPool_t *pool, void *handle,
                                                             unsigned int handleType);
aclError CUBRIDGE_API wcudaMemPoolExportPointer(wcceIpcMemHandle_t *ptrData, void *ptr);
aclError CUBRIDGE_API wcudaMemPoolImportPointer(void **ptr, wcceMemPool_t pool,
                                                 const wcceIpcMemHandle_t *ptrData);
aclError CUBRIDGE_API wcudaMemRemap(void *ptr, size_t size, int device, int node);
aclError CUBRIDGE_API wcudaMemUnremap(void *ptr, size_t size);
aclError CUBRIDGE_API wcudaArrayGetPlane(wcceArray_t *pPlane, wcceArray_t array, unsigned int planeIdx);
aclError CUBRIDGE_API wcudaArrayGetMemoryRequirements(void *req, wcceArray_t array);
aclError CUBRIDGE_API wcudaArrayGetSparseProperties(void *props, wcceArray_t array);
aclError CUBRIDGE_API wcudaMipmappedArrayGetMemoryRequirements(void *req, wcceMipmappedArray_t mipmap);
aclError CUBRIDGE_API wcudaMipmappedArrayGetSparseProperties(void *props, wcceMipmappedArray_t mipmap);

/* ============ Error Handling ============ */
aclError CUBRIDGE_API wcudaGetLastError(void);
aclError CUBRIDGE_API wcudaPeekAtLastError(void);
const char CUBRIDGE_API *wcudaGetErrorString(aclError error);
const char CUBRIDGE_API *wcudaGetErrorName(aclError error);

/* ============ Func 属性 ============ */
/* 昇腾无 kernel 属性查询，返回 not-support */
aclError CUBRIDGE_API wcudaFuncGetAttributes(wcceFuncAttributes *attr, const void *func);
aclError CUBRIDGE_API wcudaFuncGetParamInfo(const void *func, size_t paramIndex,
                                             size_t *paramOffset, size_t *paramSize,
                                             const char **paramName);

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
aclError CUBRIDGE_API wcudaOccupancyAvailableDynamicSMemPerBlock(
    size_t *dynamicSmemSize, const void *func, int numBlocks, int blockSize);
aclError CUBRIDGE_API wcudaOccupancyMaxActiveBlocksPerMultiprocessorWithFlags(
    int *numBlocks, const void *func, int blockSize, size_t dynamicSMemSize, unsigned int flags);
aclError CUBRIDGE_API wcudaOccupancyMaxPotentialBlockSizeWithFlags(
    int *minGridSize, int *blockSize, const void *func, size_t dynamicSMemSize,
    int blockSizeLimit, unsigned int flags);
aclError CUBRIDGE_API wcudaOccupancyMaxPotentialClusterSize(int *clusterSize, const void *func,
                                                             dim3 gridDim, dim3 blockDim,
                                                             size_t dynamicSMemSize,
                                                             aclrtStream stream);
aclError CUBRIDGE_API wcudaOccupancyMaxActiveClusters(int *numClusters, const void *func,
                                                       dim3 gridDim, dim3 blockDim,
                                                       size_t dynamicSMemSize,
                                                       aclrtStream stream);
aclError CUBRIDGE_API wcudaOccupancyMaxPotentialBlockSizeVariableSMem(
    int *minGridSize, int *blockSize, const void *func,
    void (*userFunc)(size_t, size_t *), size_t dynamicSMemSize, int blockSizeLimit);
aclError CUBRIDGE_API wcudaOccupancyMaxPotentialBlockSizeVariableSMemWithFlags(
    int *minGridSize, int *blockSize, const void *func,
    void (*userFunc)(size_t, size_t *), size_t dynamicSMemSize, int blockSizeLimit,
    unsigned int flags);

/* ============ Execution Control ============ */
/* cudaLaunchKernel(func, gridDim, blockDim, args, sharedMem, stream)
 * 昇腾映射 aclrtLaunchSIMTKernelWithArgsArray(func, gridDim, blockDim, dynUbufSize, stream, cfg, args)
 * （官方 14_kerne_loading_and_execution.md 确认，Ascend 950PR/950DT 支持）。
 * dim3 由 bisheng 提供（CUDA 同构）。 */
aclError CUBRIDGE_API wcudaLaunchKernel(const void *hostFunction, dim3 gridDim, dim3 blockDim,
                                        void **args, size_t sharedMemBytes, aclrtStream stream);
aclError CUBRIDGE_API wcudaLaunchCooperativeKernel(const void *hostFunction, dim3 gridDim,
                                                     dim3 blockDim, void **args,
                                                     size_t sharedMemBytes, aclrtStream stream);
aclError CUBRIDGE_API wcudaLaunchKernelExC(const void *hostFunction, dim3 gridDim, dim3 blockDim,
                                            void **args, size_t sharedMemBytes, aclrtStream stream,
                                            const void *config);
aclError CUBRIDGE_API wcudaLaunchCooperativeKernelMultiDevice(const void *hostFunction,
                                                               dim3 gridDim, dim3 blockDim,
                                                               void **args, size_t sharedMemBytes,
                                                               aclrtStream stream);
aclError CUBRIDGE_API wcudaGetFuncBySymbol(void **func, const void *symbol);
aclError CUBRIDGE_API wcudaGetExportTable(const void **table, const void *id);
aclError CUBRIDGE_API wcudaGetDriverEntryPoint(const void **func, const char *name,
                                                unsigned int flags, int *status);
aclError CUBRIDGE_API wcudaGetParameterBuffer(void *buf, size_t size);

/* ============ Profiler Control ============ */
/* 昇腾 profiling 用 aclprof（API 面不同），此处 no-op 占位 */
aclError CUBRIDGE_API wcudaProfilerStart(void);
aclError CUBRIDGE_API wcudaProfilerStop(void);
aclError CUBRIDGE_API wcudaProfilerInitialize(const char *configFile, const char *outputFile,
                                                unsigned int outputMode);

/* ============ Legacy / deprecated ============ */
aclError CUBRIDGE_API wcudaSetDoubleForDevice(double *value);
aclError CUBRIDGE_API wcudaSetDoubleForHost(double *value);

/* ============ CUDA Runtime data-shape factories ============ */
wcceExtent CUBRIDGE_API make_wcudaExtent(size_t width, size_t height, size_t depth);
wccePitchedPtr CUBRIDGE_API make_wcudaPitchedPtr(void *ptr, size_t pitch,
                                                 size_t xsize, size_t ysize);
wccePos CUBRIDGE_API make_wcudaPos(size_t x, size_t y, size_t z);

/* ============ Texture/Surface（昇腾无对应，全部 not-support；句柄用 void* 简化） ============ */
/* Channel descriptor is a source-compatibility value; texture execution is
 * still unsupported on Ascend. */
wcceChannelFormatDesc CUBRIDGE_API wcudaCreateChannelDesc(int x, int y, int z, int w);
aclError CUBRIDGE_API wcudaCreateTextureObject(wcceTextureObject_t *pTexObject,
                                               const wcceResourceDesc *pResDesc,
                                               const wcceTextureDesc *pTexDesc,
                                               const wcceResourceViewDesc *pResViewDesc);
aclError CUBRIDGE_API wcudaDestroyTextureObject(wcceTextureObject_t texObject);
aclError CUBRIDGE_API wcudaCreateSurfaceObject(wcceSurfaceObject_t *pSurfObject,
                                               const wcceResourceDesc *pResDesc);
aclError CUBRIDGE_API wcudaDestroySurfaceObject(wcceSurfaceObject_t surfObject);
aclError CUBRIDGE_API wcudaBindTexture(size_t *offset, const void *texref, const void *devPtr,
                                       const void *desc, size_t size);
aclError CUBRIDGE_API wcudaGetMipmappedArrayLevel(wcceArray_t *levelArray,
                                                   wcceMipmappedArray_t mipmappedArray,
                                                   unsigned int level);
aclError CUBRIDGE_API wcudaGetTextureObjectResourceDesc(wcceResourceDesc *pResDesc,
                                                         wcceTextureObject_t texObject);
aclError CUBRIDGE_API wcudaGetTextureObjectTextureDesc(wcceTextureDesc *pTexDesc,
                                                        wcceTextureObject_t texObject);
aclError CUBRIDGE_API wcudaGetTextureObjectTextureDesc_v2(wcceTextureDesc *pTexDesc,
                                                           wcceTextureObject_t texObject);
aclError CUBRIDGE_API wcudaGetTextureObjectResourceViewDesc(wcceResourceViewDesc *pResViewDesc,
                                                             wcceTextureObject_t texObject);
aclError CUBRIDGE_API wcudaGetSurfaceObjectResourceDesc(wcceResourceDesc *pResDesc,
                                                         wcceSurfaceObject_t surfObject);

/* ============ External Memory & Semaphore（昇腾无对应，not-support） ============ */
aclError CUBRIDGE_API wcudaImportExternalMemory(void *extMem_out, const void *memHandleDesc);
aclError CUBRIDGE_API wcudaDestroyExternalMemory(void *extMem);
aclError CUBRIDGE_API wcudaImportExternalSemaphore(void *extSem_out, const void *semHandleDesc);
aclError CUBRIDGE_API wcudaDestroyExternalSemaphore(void *extSem);
aclError CUBRIDGE_API wcudaSignalExternalSemaphoresAsync(const void *extSems, const void *params,
                                                          unsigned int numExtSems,
                                                          aclrtStream stream);
aclError CUBRIDGE_API wcudaWaitExternalSemaphoresAsync(const void *extSems, const void *params,
                                                        unsigned int numExtSems,
                                                        aclrtStream stream);
aclError CUBRIDGE_API wcudaExternalMemoryGetMappedBuffer(void **devPtr, void *extMem,
                                                          const void *bufferDesc);
aclError CUBRIDGE_API wcudaExternalMemoryGetMappedMipmappedArray(void *mipmap_out, void *extMem,
                                                                  const void *mipmapDesc);

/* ============ Extended Texture/Surface（not-support stubs） ============ */
aclError CUBRIDGE_API wcudaUnbindTexture(const void *texref);
aclError CUBRIDGE_API wcudaBindTexture2D(size_t *offset, const void *texref, const void *devPtr,
                                          const void *desc, size_t width, size_t height,
                                          size_t pitch);
aclError CUBRIDGE_API wcudaBindTextureToArray(const void *texref, wcceArray_t array,
                                               const void *desc);
aclError CUBRIDGE_API wcudaBindTextureToMipmappedArray(const void *texref,
                                                        wcceMipmappedArray_t mipmap,
                                                        const void *desc);
aclError CUBRIDGE_API wcudaBindSurfaceToArray(const void *surfref, wcceArray_t array);
aclError CUBRIDGE_API wcudaGetTextureReference(const void **ref, const void *texref);
aclError CUBRIDGE_API wcudaGetSurfaceReference(const void **ref, const void *surfref);
aclError CUBRIDGE_API wcudaGetTextureAlignmentOffset(size_t *offset, const void *texref);

/* ============ Extended Graph（not-support stubs） ============ */
aclError CUBRIDGE_API wcudaGraphAddChildGraphNode(void *pGraphNode, void *graph,
                                                   const void *deps, size_t numDeps, void *childGraph);
aclError CUBRIDGE_API wcudaGraphAddDependencies(void *graph, const void *from, const void *to,
                                                 size_t numDeps);
aclError CUBRIDGE_API wcudaGraphAddEmptyNode(void *pGraphNode, void *graph, const void *deps,
                                              size_t numDeps);
aclError CUBRIDGE_API wcudaGraphAddEventRecordNode(void *pGraphNode, void *graph, const void *deps,
                                                    size_t numDeps, aclrtEvent event);
aclError CUBRIDGE_API wcudaGraphAddEventWaitNode(void *pGraphNode, void *graph, const void *deps,
                                                  size_t numDeps, aclrtEvent event);
aclError CUBRIDGE_API wcudaGraphAddExternalSemaphoresSignalNode(void *pGraphNode, void *graph,
                                                                 const void *deps, size_t numDeps,
                                                                 const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphAddExternalSemaphoresWaitNode(void *pGraphNode, void *graph,
                                                               const void *deps, size_t numDeps,
                                                               const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphAddHostNode(void *pGraphNode, void *graph, const void *deps,
                                             size_t numDeps, const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphAddMemAllocNode(void *pGraphNode, void *graph, const void *deps,
                                                 size_t numDeps, const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphAddMemcpyNode(void *pGraphNode, void *graph, const void *deps,
                                               size_t numDeps, const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphAddMemcpyNode1D(void *pGraphNode, void *graph, const void *deps,
                                                 size_t numDeps, void *dst, const void *src,
                                                 size_t count, int kind);
aclError CUBRIDGE_API wcudaGraphAddMemcpyNodeFromSymbol(void *pGraphNode, void *graph,
                                                         const void *deps, size_t numDeps,
                                                         void *dst, const void *symbol,
                                                         size_t count, size_t offset, int kind);
aclError CUBRIDGE_API wcudaGraphAddMemcpyNodeToSymbol(void *pGraphNode, void *graph,
                                                       const void *deps, size_t numDeps,
                                                       const void *symbol, const void *src,
                                                       size_t count, size_t offset, int kind);
aclError CUBRIDGE_API wcudaGraphAddMemFreeNode(void *pGraphNode, void *graph, const void *deps,
                                                size_t numDeps, void *devPtr);
aclError CUBRIDGE_API wcudaGraphAddMemsetNode(void *pGraphNode, void *graph, const void *deps,
                                               size_t numDeps, const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphAddNode(void *pGraphNode, void *graph, const void *deps,
                                         size_t numDeps, const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphChildGraphNodeGetGraph(void *node, void *childGraph);
aclError CUBRIDGE_API wcudaGraphDebugDotPrint(void *graph, const char *path, unsigned int flags);
aclError CUBRIDGE_API wcudaGraphEventRecordNodeGetEvent(void *node, aclrtEvent *event);
aclError CUBRIDGE_API wcudaGraphEventRecordNodeSetEvent(void *node, aclrtEvent event);
aclError CUBRIDGE_API wcudaGraphEventWaitNodeGetEvent(void *node, aclrtEvent *event);
aclError CUBRIDGE_API wcudaGraphEventWaitNodeSetEvent(void *node, aclrtEvent event);
aclError CUBRIDGE_API wcudaGraphExecChildGraphNodeSetParams(void *hGraphExec, void *hNode,
                                                              void *childGraph);
aclError CUBRIDGE_API wcudaGraphExecEventRecordNodeSetEvent(void *hGraphExec, void *hNode,
                                                              aclrtEvent event);
aclError CUBRIDGE_API wcudaGraphExecEventWaitNodeSetEvent(void *hGraphExec, void *hNode,
                                                            aclrtEvent event);
aclError CUBRIDGE_API wcudaGraphExecExternalSemaphoresSignalNodeSetParams(void *hGraphExec,
                                                                           void *hNode,
                                                                           const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphExecExternalSemaphoresWaitNodeSetParams(void *hGraphExec,
                                                                          void *hNode,
                                                                          const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphExecGetFlags(void *hGraphExec, unsigned int *flags);
aclError CUBRIDGE_API wcudaGraphExecHostNodeSetParams(void *hGraphExec, void *hNode,
                                                        const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphExecKernelNodeSetParams(void *hGraphExec, void *hNode,
                                                          const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphExecMemcpyNodeSetParams(void *hGraphExec, void *hNode,
                                                          const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphExecMemcpyNodeSetParams1D(void *hGraphExec, void *hNode,
                                                             void *dst, const void *src,
                                                             size_t count, int kind);
aclError CUBRIDGE_API wcudaGraphExecMemcpyNodeSetParamsFromSymbol(void *hGraphExec, void *hNode,
                                                                    void *dst, const void *symbol,
                                                                    size_t count, size_t offset,
                                                                    int kind);
aclError CUBRIDGE_API wcudaGraphExecMemcpyNodeSetParamsToSymbol(void *hGraphExec, void *hNode,
                                                                  const void *symbol,
                                                                  const void *src, size_t count,
                                                                  size_t offset, int kind);
aclError CUBRIDGE_API wcudaGraphExecMemsetNodeSetParams(void *hGraphExec, void *hNode,
                                                          const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphExternalSemaphoresSignalNodeGetParams(void *node, void *params);
aclError CUBRIDGE_API wcudaGraphExternalSemaphoresSignalNodeSetParams(void *node, const void *params);
aclError CUBRIDGE_API wcudaGraphExternalSemaphoresWaitNodeGetParams(void *node, void *params);
aclError CUBRIDGE_API wcudaGraphExternalSemaphoresWaitNodeSetParams(void *node, const void *params);
aclError CUBRIDGE_API wcudaGraphGetEdges(void *graph, void *from, void *to, size_t *numEdges);
aclError CUBRIDGE_API wcudaGraphHostNodeGetParams(void *node, void *nodeParams);
aclError CUBRIDGE_API wcudaGraphHostNodeSetParams(void *node, const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphInstantiateWithFlags(void *pGraphExec, void *graph,
                                                       unsigned long long flags);
aclError CUBRIDGE_API wcudaGraphInstantiateWithParams(void *pGraphExec, void *graph,
                                                        const void *params);
aclError CUBRIDGE_API wcudaGraphKernelNodeCopyAttributes(void *dst, const void *src);
aclError CUBRIDGE_API wcudaGraphKernelNodeGetAttribute(void *node, int attr, void *value);
aclError CUBRIDGE_API wcudaGraphKernelNodeGetParams(void *node, void *nodeParams);
aclError CUBRIDGE_API wcudaGraphKernelNodeSetAttribute(void *node, int attr, const void *value);
aclError CUBRIDGE_API wcudaGraphKernelNodeSetParams(void *node, const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphMemAllocNodeGetParams(void *node, void *params);
aclError CUBRIDGE_API wcudaGraphMemcpyNodeGetParams(void *node, void *nodeParams);
aclError CUBRIDGE_API wcudaGraphMemcpyNodeSetParams(void *node, const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphMemcpyNodeSetParamsFromSymbol(void *node, void *dst,
                                                                const void *symbol, size_t count,
                                                                size_t offset, int kind);
aclError CUBRIDGE_API wcudaGraphMemcpyNodeSetParamsToSymbol(void *node, const void *symbol,
                                                              const void *src, size_t count,
                                                              size_t offset, int kind);
aclError CUBRIDGE_API wcudaGraphMemFreeNodeGetParams(void *node, void *params);
aclError CUBRIDGE_API wcudaGraphMemsetNodeGetParams(void *node, void *nodeParams);
aclError CUBRIDGE_API wcudaGraphMemsetNodeSetParams(void *node, const void *nodeParams);
aclError CUBRIDGE_API wcudaGraphNodeFindInClone(void *pNode, void *originalNode, void *clonedGraph);
aclError CUBRIDGE_API wcudaGraphNodeGetDependencies(void *node, void *deps, size_t *numDeps);
aclError CUBRIDGE_API wcudaGraphNodeGetDependentNodes(void *node, void *deps, size_t *numDeps);
aclError CUBRIDGE_API wcudaGraphNodeGetEnabled(void *node, unsigned int *isEnabled);
aclError CUBRIDGE_API wcudaGraphNodeGetType(void *node, int *nodeType);
aclError CUBRIDGE_API wcudaGraphNodeSetEnabled(void *node, unsigned int isEnabled);
aclError CUBRIDGE_API wcudaGraphReleaseUserObject(void *graph, void *userObject,
                                                   unsigned int count);
aclError CUBRIDGE_API wcudaGraphRemoveDependencies(void *graph, const void *from, const void *to,
                                                    size_t numDeps);
aclError CUBRIDGE_API wcudaGraphRetainUserObject(void *graph, void *userObject, unsigned int count,
                                                  unsigned int flags);
aclError CUBRIDGE_API wcudaGraphUpload(void *graph, aclrtStream stream);
aclError CUBRIDGE_API wcudaGraphConditionalHandleCreate(void *handle, void *graph,
                                                         unsigned int flags, int type);
aclError CUBRIDGE_API wcudaGraphSetConditional(void *handle, unsigned int value, unsigned int flags);

/* ============ Graphics Interop（not-support stubs） ============ */
aclError CUBRIDGE_API wcudaGraphicsMapResources(unsigned int count, void **resources,
                                                 aclrtStream stream);
aclError CUBRIDGE_API wcudaGraphicsUnmapResources(unsigned int count, void **resources,
                                                   aclrtStream stream);
aclError CUBRIDGE_API wcudaGraphicsUnregisterResource(void *resource);
aclError CUBRIDGE_API wcudaGraphicsResourceGetMappedPointer(void **devPtr, size_t *size,
                                                              void *resource);
aclError CUBRIDGE_API wcudaGraphicsResourceGetMappedMipmappedArray(wcceMipmappedArray_t *mipmap,
                                                                     void *resource);
aclError CUBRIDGE_API wcudaGraphicsResourceSetMapFlags(void *resource, unsigned int flags);
aclError CUBRIDGE_API wcudaGraphicsSubResourceGetMappedArray(wcceArray_t *array, void *resource,
                                                               unsigned int index,
                                                               unsigned int level);
aclError CUBRIDGE_API wcudaGraphicsGLRegisterBuffer(void *resource, unsigned int buffer,
                                                      unsigned int flags);
aclError CUBRIDGE_API wcudaGraphicsGLRegisterImage(void *resource, unsigned int image,
                                                     unsigned int target, unsigned int flags);
aclError CUBRIDGE_API wcudaGLGetDevices(unsigned int *deviceCount, int *devices,
                                         unsigned int flags, int deviceList);
aclError CUBRIDGE_API wcudaGLMapBufferObject(void **devPtr, unsigned int buffer);
aclError CUBRIDGE_API wcudaGLMapBufferObjectAsync(void **devPtr, unsigned int buffer,
                                                    aclrtStream stream);
aclError CUBRIDGE_API wcudaGLRegisterBufferObject(unsigned int buffer);
aclError CUBRIDGE_API wcudaGLSetBufferObjectMapFlags(unsigned int buffer, unsigned int flags);
aclError CUBRIDGE_API wcudaGLSetGLDevice(int device);
aclError CUBRIDGE_API wcudaGLUnmapBufferObject(unsigned int buffer);
aclError CUBRIDGE_API wcudaGLUnmapBufferObjectAsync(unsigned int buffer, aclrtStream stream);
aclError CUBRIDGE_API wcudaGLUnregisterBufferObject(unsigned int buffer);
aclError CUBRIDGE_API wcudaGraphicsEGLRegisterImage(void *resource, void *image,
                                                      unsigned int flags);
aclError CUBRIDGE_API wcudaGraphicsResourceGetMappedEglFrame(void *eglFrame, void *resource,
                                                                unsigned int index,
                                                                unsigned int level);
aclError CUBRIDGE_API wcudaEGLStreamConsumerAcquireFrame(void *conn, void *resource,
                                                           aclrtStream stream);
aclError CUBRIDGE_API wcudaEGLStreamConsumerConnect(void *conn, void *stream);
aclError CUBRIDGE_API wcudaEGLStreamConsumerConnectWithFlags(void *conn, void *stream,
                                                                unsigned int flags);
aclError CUBRIDGE_API wcudaEGLStreamConsumerDisconnect(void *conn);
aclError CUBRIDGE_API wcudaEGLStreamConsumerReleaseFrame(void *conn, void *resource,
                                                           aclrtStream stream);
aclError CUBRIDGE_API wcudaEGLStreamProducerConnect(void *conn, void *stream,
                                                      unsigned int flags);
aclError CUBRIDGE_API wcudaEGLStreamProducerDisconnect(void *conn);
aclError CUBRIDGE_API wcudaEGLStreamProducerPresentFrame(void *conn, void *frame,
                                                           aclrtStream stream);
aclError CUBRIDGE_API wcudaEGLStreamProducerReturnFrame(void *conn, void *resource,
                                                          aclrtStream stream);
aclError CUBRIDGE_API wcudaVDPAUGetDevice(int *device, void *vdpDevice, void *vdpGetProcAddress);
aclError CUBRIDGE_API wcudaVDPAUSetVDPAUDevice(int device, void *vdpDevice,
                                                  void *vdpGetProcAddress);
aclError CUBRIDGE_API wcudaGraphicsVDPAURegisterOutputSurface(void *resource, void *vdpSurface,
                                                                unsigned int flags);
aclError CUBRIDGE_API wcudaWGLGetDevice(int *device, void *hDC);

/* ============ Cooperative Groups（not-support stubs） ============ */
aclError CUBRIDGE_API wcudaCGSynchronize(void *handle, int scope);
aclError CUBRIDGE_API wcudaCGSynchronizeGrid(void *handle);
aclError CUBRIDGE_API wcudaCGGetIntrinsicHandle(void *handle);
aclError CUBRIDGE_API wcudaCGGetRank(void *handle);
aclError CUBRIDGE_API wcudaCGGetSize(void *handle);

/* ============ Execution Context / Green Context（not-support stubs） ============ */
aclError CUBRIDGE_API wcudaExecutionCtxDestroy(void *ctx);
aclError CUBRIDGE_API wcudaExecutionCtxGetDevice(int *device, void *ctx);
aclError CUBRIDGE_API wcudaExecutionCtxGetDevResource(void *resource, void *ctx, int type);
aclError CUBRIDGE_API wcudaExecutionCtxGetId(unsigned long long *id, void *ctx);
aclError CUBRIDGE_API wcudaExecutionCtxRecordEvent(void *ctx, aclrtEvent event);
aclError CUBRIDGE_API wcudaExecutionCtxStreamCreate(aclrtStream *stream, void *ctx);
aclError CUBRIDGE_API wcudaExecutionCtxSynchronize(void *ctx);
aclError CUBRIDGE_API wcudaExecutionCtxWaitEvent(void *ctx, aclrtEvent event);
aclError CUBRIDGE_API wcudaGreenCtxCreate(void *ctx, int device, const void *resource);

/* ============ Device Resource（not-support stubs） ============ */
aclError CUBRIDGE_API wcudaDevSmResourceSplit(void *result, const void *resource,
                                                unsigned int count, unsigned int flags);
aclError CUBRIDGE_API wcudaDevSmResourceSplitByCount(void *result, const void *resource,
                                                       unsigned int count, unsigned int flags);

/* ============ User Objects（not-support stubs） ============ */
aclError CUBRIDGE_API wcudaUserObjectCreate(void *userObject, void *graph, void *data,
                                              void *destroy, unsigned int weight,
                                              unsigned int flags);
aclError CUBRIDGE_API wcudaUserObjectRelease(void *userObject, unsigned int count);
aclError CUBRIDGE_API wcudaUserObjectRetain(void *userObject, unsigned int count);

/* ============ Thread Management（废弃 API，映射到等价运行时调用） ============ */
aclError CUBRIDGE_API wcudaThreadSynchronize(void);
aclError CUBRIDGE_API wcudaThreadExit(void);
aclError CUBRIDGE_API wcudaThreadGetCacheConfig(int *cacheConfig);

/* ============ Basic Graph（not-support；句柄用 void* 简化） ============ */
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
aclError CUBRIDGE_API wcudaGraphGetNodes(wcceGraph_t graph, wcceGraphNode_t *nodes,
                                          size_t *numNodes);
aclError CUBRIDGE_API wcudaGraphGetRootNodes(wcceGraph_t graph, wcceGraphNode_t *nodes,
                                               size_t *numNodes);
aclError CUBRIDGE_API wcudaGraphClone(wcceGraph_t *pGraphClone, wcceGraph_t originalGraph);
aclError CUBRIDGE_API wcudaGraphDestroyNode(wcceGraphNode_t node);

/* ============ Host Function & Func 属性 ============ */
/* cudaLaunchHostFunc(stream, fn, userData)：昇腾有 aclrtLaunchCallback 对应。
 * cudaHostFn_t 签名 void(*)(void*) 与 aclrtCallback 一致。 */
typedef void (*wcceHostFn_t)(void *);
aclError CUBRIDGE_API wcudaLaunchHostFunc(aclrtStream stream, wcceHostFn_t fn, void *userData = 0);
/* 昇腾无 func 属性/名字接口，声明保证编译 */
aclError CUBRIDGE_API wcudaFuncSetAttribute(const void *func, int attr, int value);
aclError CUBRIDGE_API wcudaFuncSetCacheConfig(const void *func, wcceFuncCache cacheConfig);
aclError CUBRIDGE_API wcudaFuncSetSharedMemConfig(const void *func,
                                                   wcceSharedMemConfig config);
aclError CUBRIDGE_API wcudaThreadSetCacheConfig(int cacheConfig);
aclError CUBRIDGE_API wcudaFuncGetName(const char **name, const void *func);

/* ============ Version ============ */
aclError CUBRIDGE_API wcudaRuntimeGetVersion(int *runtimeVersion);
aclError CUBRIDGE_API wcudaDriverGetVersion(int *driverVersion);

/* ============ v2/alias variants ============ */
#define wcudaCreateTextureObject_v2 wcudaCreateTextureObject
#define wcudaGetDriverEntryPoint_V2 wcudaGetDriverEntryPoint
#define wcudaGraphExecUpdate_V2 wcudaGraphExecUpdate
#define wcudaGraphInstantiate_V2 wcudaGraphInstantiate
#define wcudaGraphAddDependencies_v2 wcudaGraphAddDependencies
#define wcudaGraphAddNode_v2 wcudaGraphAddNode
#define wcudaGraphGetEdges_v2 wcudaGraphGetEdges
#define wcudaGraphNodeGetDependencies_v2 wcudaGraphNodeGetDependencies
#define wcudaGraphNodeGetDependentNodes_v2 wcudaGraphNodeGetDependentNodes
#define wcudaGraphRemoveDependencies_v2 wcudaGraphRemoveDependencies

#ifdef __cplusplus
} /* extern "C" */

/* CUDA also exposes a five-argument channel descriptor helper. Keep the
 * four-argument C ABI stable and provide the kind-bearing form as a C++
 * source-compatibility overload. Texture execution remains unsupported. */
inline wcceChannelFormatDesc wcudaCreateChannelDesc(int x, int y, int z, int w,
                                                    wcceChannelFormatKind f) {
    wcceChannelFormatDesc value = wcudaCreateChannelDesc(x, y, z, w);
    value.f = f;
    return value;
}

/* CUDA Runtime provides typed C++ overloads for allocation APIs. NVIDIA's
 * official matrixMul sample uses cudaMallocHost(&float_ptr, size) without a
 * cast; keep that source unchanged while forwarding to the C ABI wrapper. */
template <typename T>
inline aclError wcudaMalloc(T **ptr, size_t size) {
    return wcudaMalloc(reinterpret_cast<void **>(ptr), size);
}

template <typename T>
inline aclError wcudaMallocHost(T **ptr, size_t size) {
    return wcudaMallocHost(reinterpret_cast<void **>(ptr), size);
}

template <typename T>
inline aclError wcudaMallocHostWithFlags(T **ptr, size_t size, unsigned int flags) {
    return wcudaMallocHostWithFlags(reinterpret_cast<void **>(ptr), size, flags);
}

/* simpleMultiCopy and similar samples call cudaHostAlloc(&float_ptr, ...) with
 * a typed pointer; NVIDIA exposes the same overload family. */
template <typename T>
inline aclError wcudaHostAlloc(T **ptr, size_t size, unsigned int flags) {
    return wcudaHostAlloc(reinterpret_cast<void **>(ptr), size, flags);
}

/*
 * CUDA Runtime exposes the same typed-pointer C++ overload family for the rest
 * of the allocation APIs. Upstream sources call e.g.
 *   cudaMallocManaged(&float_ptr, bytes)
 *   cudaMallocPitch(&float_ptr, &pitch, w, h)
 *   cudaMallocAsync(&float_ptr, bytes, stream)
 *   cudaMallocFromPoolAsync(&float_ptr, bytes, pool, stream)
 * without a (void**) cast; standard C++ forbids the implicit T** -> void**
 * conversion, so without these templates the call fails with "no known
 * conversion from 'float **' to 'void **'". Forward to the C ABI wrapper via
 * reinterpret_cast, mirroring the wcudaMalloc/wcudaMallocHost pattern above.
 * Defaults on the underlying declarations (flags=0, stream=0) are repeated
 * here so the typed overload accepts the same reduced-argument call shapes.
 */
template <typename T>
inline aclError wcudaMallocManaged(T **ptr, size_t size, unsigned int flags = 0) {
    return wcudaMallocManaged(reinterpret_cast<void **>(ptr), size, flags);
}

template <typename T>
inline aclError wcudaMallocPitch(T **ptr, size_t *pitch, size_t width, size_t height) {
    return wcudaMallocPitch(reinterpret_cast<void **>(ptr), pitch, width, height);
}

template <typename T>
inline aclError wcudaMallocAsync(T **ptr, size_t size, aclrtStream hStream) {
    return wcudaMallocAsync(reinterpret_cast<void **>(ptr), size, hStream);
}

template <typename T>
inline aclError wcudaMallocFromPoolAsync(T **ptr, size_t size, wcceMemPool_t memPool,
                                         aclrtStream stream) {
    return wcudaMallocFromPoolAsync(reinterpret_cast<void **>(ptr), size, memPool, stream);
}
#endif

#endif //__CUDA_RUNTIME_WRAPPER_H__
