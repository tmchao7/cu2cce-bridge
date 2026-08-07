/*
 * wcuda wrapper 实现（Phase A1 最小核心集）。
 *
 * 仿照 cu-bridge 的 src/bridge/runtime/src/cuda_runtime_wrapper.cpp：
 *   每个 wcuda 函数内部调用昇腾 ACL 原生 API（aclrt*）。
 *
 * ⚠️ 本文件在装有 CANN 的真机上编译（-I$ASCEND_TOOLKIT_HOME/include，-L.../lib64 -lascendcl
 *   或 -lacl_rt），本机（macOS）无 CANN，仅作代码阅读/移植参考，勿在本机构建。
 *
 * ACL 与 CUDA 的签名差异（记入 ASCEND_RESEARCH_NOTES.md §1.4，实现时逐条对齐）：
 *   1) aclrtMemcpy(dst, destMax, src, count, kind) —— CUDA 无 destMax，填 count 即可；
 *   2) aclrtMalloc(ptr, size, policy) —— 第三个参数是 aclrtMemMallocPolicy（大页/普通页）；
 *   3) ACL 需先 aclInit（在 bridge_init 中做一次），CUDA 无此要求；
 *   4) aclrtMemcpy 同步版的 kind 是"预留"（系统按指针判断方向）。
 */

#include "cuda_runtime_wrapper.h"
#include "cuda_wrapper_internal.h"
#include "cuda_wrapper_types.h"

#include <cstdlib>
#include <cstring>

/* ---------------- 内部状态 ---------------- */

/* 线程局部"上一次错误"。仿照 cu-bridge 的 g_last_error。 */
static thread_local aclError g_last_error = ACL_ERROR_NONE;

/* 全局是否已有 context（仿照 cu-bridge 的 g_hasContextBeenCreated） */
static thread_local bool g_has_context_created = false;

#define ACTIVE_CONTEXT() g_has_context_created = true;

/* ---------------- bridge 初始化 ---------------- */

namespace cubridge::internal {

void bridge_init() {
    /* 只跑一次（由 BRIDGE_INIT 的 std::call_once 保证）。
     * 昇腾 ACL 应用在首次调用运行时 API 前需 aclInit。
     * aclInit 参数传 nullptr 表示默认配置（对应 CUDA 的惰性初始化）。 */
    int32_t ret = aclInit(nullptr);
    (void)ret; /* 若返回非 0，后续 wcuda 调用会通过 WCUDA_RETURN 观察到错误 */
}

} // namespace cubridge::internal

/* ---------------- 返回宏 ---------------- */

/* 仿照 cu-bridge 的 WCUDA_RETURN：
 *   执行原生调用 -> 记录错误到 g_last_error -> 返回 aclError。
 * 昇腾侧没有 mcGetLaunchKernelError 这类"kernel 启动后置错误"查询，
 * 先简化为直接透传 aclError；后续如需对齐 CUDA 的 launch error 语义再加。 */
#define WCUDA_RETURN(ret)                                                                          \
    do {                                                                                           \
        BRIDGE_INIT();                                                                             \
        aclError error = (ret);                                                                    \
        if (error != ACL_ERROR_NONE) {                                                                \
            g_last_error = error;                                                                  \
        }                                                                                          \
        return error;                                                                              \
    } while (0);

/* ---------------- Device Management ---------------- */

aclError wcudaGetDeviceCount(int *count) {
    if (count == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    uint32_t cnt = 0;
    aclError ret = aclrtGetDeviceCount(&cnt);
    if (ret == ACL_ERROR_NONE) {
        *count = (int)cnt;
    }
    WCUDA_RETURN(ret);
}

aclError wcudaSetDevice(int device) {
    ACTIVE_CONTEXT();
    /* aclrtSetDevice 的 deviceId 范围 [0, 可用数-1]，与 cudaSetDevice 语义一致 */
    WCUDA_RETURN(aclrtSetDevice(device));
}

aclError wcudaGetDevice(int *device) {
    if (device == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    int32_t dev = 0;
    aclError ret = aclrtGetDevice(&dev);
    if (ret == ACL_ERROR_NONE) {
        *device = (int)dev;
    }
    WCUDA_RETURN(ret);
}

aclError wcudaDeviceSynchronize(void) {
    /* 阻塞当前线程直到所有 stream 完成（对应 CUDA cudaDeviceSynchronize） */
    WCUDA_RETURN(aclrtSynchronizeDevice());
}

aclError wcudaDeviceReset(void) {
    /* 释放当前 device 资源（对应 cudaDeviceReset）。
     * 昇腾侧用 aclrtResetDevice(int32_t deviceId)（引用计数式，与 aclrtSetDevice 配对）。
     * 需要先查当前 deviceId 再 reset。 */
    int32_t dev = 0;
    aclError ret = aclrtGetDevice(&dev);
    if (ret != ACL_ERROR_NONE) {
        WCUDA_RETURN(ret);
    }
    WCUDA_RETURN(aclrtResetDevice(dev));
}

aclError wcudaGetDeviceProperties(_cceDeviceProp_t *prop, int device) {
    if (prop == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    memset(prop, 0, sizeof(*prop));

    /* 查 SOC 型号填 name。注意 aclrtGetSocName() 签名是 const char*（无 aclError 返回），
     * 失败返回 nullptr（见 MegEngine acl_base.h 镜像）。 */
    const char *soc = aclrtGetSocName();
    if (soc != nullptr) {
        strncpy(prop->name, soc, _CCE_DEVICE_PROP_MAX_NAME_LEN - 1);
    }

    /* 查可用内存填 totalGlobalMem（free+total）。
     * aclrtGetMemInfo(aclrtMemAttr attr, size_t *free, size_t *total) */
    size_t freeMem = 0, totalMem = 0;
    aclError ret = aclrtGetMemInfo(ACL_HBM_MEM, &freeMem, &totalMem);
    if (ret == ACL_ERROR_NONE) {
        prop->totalGlobalMem = totalMem;
    }

    /* 昇腾 SIMT 语义：warp=32、block 内线程上限 2048、AIV 核数为 multiProcessorCount。
     * 具体核数建议真机上用 aclrtGetDeviceProperties 补齐（ACL 的 aclrtDeviceProperties
     * 结构与 cudaDeviceProp 字段不同，这里先用保守值，需要精确值时在真机核对）。 */
    prop->warpSize = 32;
    prop->maxThreadsPerBlock = 2048;
    prop->maxThreadsDim[0] = 2048;
    prop->maxThreadsDim[1] = 1;
    prop->maxThreadsDim[2] = 1;
    prop->maxGridSize[0] = 65535;
    prop->maxGridSize[1] = 65535;
    prop->maxGridSize[2] = 65535;
    prop->sharedMemPerBlock = 256 * 1024; /* SIMT 下 UB 容量约 256KB */
    prop->multiProcessorCount = 20;       /* 保守值，真机按 aclrtGetDeviceProperties 校准 */
    prop->major = 0;
    prop->minor = 0;

    WCUDA_RETURN(ACL_ERROR_NONE);
}

/* ---- Device 配置类（昇腾无对应或语义不同；返回默认/not-support） ---- */

aclError wcudaDeviceGetPCIBusId(char *pciBusId, int len, int device) {
    (void)device;
    /* 昇腾无 PCI 总线概念，用 SOC 型号近似填充（便于应用打印设备名）。 */
    if (pciBusId == nullptr || len <= 0) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    const char *soc = aclrtGetSocName();
    if (soc != nullptr) {
        strncpy(pciBusId, soc, (size_t)len - 1);
        pciBusId[len - 1] = '\0';
    } else {
        pciBusId[0] = '\0';
    }
    WCUDA_RETURN(ACL_ERROR_NONE);
}

aclError wcudaDeviceGetByPCIBusId(int *device, const char *pciBusId) {
    (void)pciBusId;
    if (device == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾无 PCI 标识定位设备，返回 device 0（若有多个设备需真机扩展）。 */
    *device = 0;
    WCUDA_RETURN(ACL_ERROR_NONE);
}

aclError wcudaDeviceGetStreamPriorityRange(int *leastPriority, int *greatestPriority) {
    /* 昇腾无 stream 优先级概念，返回 [0,0]（单优先级）。 */
    if (leastPriority != nullptr) {
        *leastPriority = 0;
    }
    if (greatestPriority != nullptr) {
        *greatestPriority = 0;
    }
    WCUDA_RETURN(ACL_ERROR_NONE);
}

aclError wcudaDeviceGetCacheConfig(int *pCacheConfig) {
    (void)pCacheConfig;
    /* 昇腾无 L1/L2 cache 配置概念，返回 not-support。 */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaDeviceSetCacheConfig(int cacheConfig) {
    (void)cacheConfig;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaDeviceGetSharedMemConfig(int *pConfig) {
    (void)pConfig;
    /* 昇腾无 shared memory 配置概念，返回 not-support。 */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaDeviceSetSharedMemConfig(int config) {
    (void)config;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaDeviceGetLimit(size_t *pValue, int limit) {
    (void)limit;
    if (pValue == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾无 cudaLimit 对应，返回 not-support（或按 limit 类型给默认值）。 */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaDeviceSetLimit(int limit, size_t value) {
    (void)limit;
    (void)value;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaDeviceGetAttribute(int *value, int attr, int device) {
    if (value == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    _cceDeviceProp_t prop;
    aclError err = wcudaGetDeviceProperties(&prop, device);
    if (err != ACL_ERROR_NONE) {
        WCUDA_RETURN(err);
    }
    switch (attr) {
    case 1: /* cudaDevAttrMaxThreadsPerBlock */
        *value = prop.maxThreadsPerBlock;
        break;
    case 2: /* MaxBlockDimX */
        *value = prop.maxThreadsDim[0];
        break;
    case 3:
        *value = prop.maxThreadsDim[1];
        break;
    case 4:
        *value = prop.maxThreadsDim[2];
        break;
    case 5: /* MaxGridDimX */
        *value = prop.maxGridSize[0];
        break;
    case 6:
        *value = prop.maxGridSize[1];
        break;
    case 7:
        *value = prop.maxGridSize[2];
        break;
    case 8: /* MaxSharedMemoryPerBlock */
        *value = (int)prop.sharedMemPerBlock;
        break;
    case 12: /* ClockRate */
        *value = prop.clockRate;
        break;
    case 13: /* WarpSize */
        *value = prop.warpSize;
        break;
    case 14: /* MaxPitch */
        *value = (int)prop.memPitch;
        break;
    case 16: /* MultiProcessorCount */
        *value = prop.multiProcessorCount;
        break;
    case 19: /* KernelExecTimeout */
        *value = 0; /* 昇腾无超时语义，填 0 表示不超时 */
        break;
    case 20: /* Integrated */
        *value = 0; /* 昇腾为独立加速卡，非集成 GPU */
        break;
    default:
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(ACL_ERROR_NONE);
}

/* ---------------- Stream Management ---------------- */

aclError wcudaStreamCreate(aclrtStream *pStream) {
    if (pStream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtCreateStream(pStream));
}

aclError wcudaStreamCreateWithFlags(aclrtStream *pStream, unsigned int flags) {
    /* 昇腾无 flags 变体，忽略 flags（cudaStreamNonBlocking 在昇腾无对应语义） */
    (void)flags;
    if (pStream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtCreateStream(pStream));
}

aclError wcudaStreamDestroy(aclrtStream stream) {
    if (stream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtDestroyStream(stream));
}

aclError wcudaStreamSynchronize(aclrtStream stream) {
    if (stream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtSynchronizeStream(stream));
}

aclError wcudaStreamQuery(aclrtStream stream) {
    if (stream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* aclrtStreamQuery(stream, aclrtStreamStatus *status)。
     * CUDA 语义：cudaStreamQuery 已完成返回 success，未完成返回 cudaErrorNotReady。
     * 昇腾用 status 输出判断：ACL_STREAM_STATUS_COMPLETE / NOT_READY。 */
    aclrtStreamStatus status = ACL_STREAM_STATUS_COMPLETE;
    aclError ret = aclrtStreamQuery(stream, &status);
    if (ret != ACL_ERROR_NONE) {
        WCUDA_RETURN(ret);
    }
    if (status != ACL_STREAM_STATUS_COMPLETE) {
        WCUDA_RETURN(ACL_ERROR_RT_STREAM_NOT_COMPLETE);
    }
    WCUDA_RETURN(ACL_ERROR_NONE);
}

aclError wcudaStreamWaitEvent(aclrtStream stream, aclrtEvent event, unsigned int flags) {
    (void)flags;
    if (stream == nullptr || event == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtStreamWaitEvent(stream, event));
}

/* ---- Stream capture（昇腾无 CUDA Graph 捕获，占位返回 not-support） ---- */

aclError wcudaStreamBeginCapture(aclrtStream stream, int mode) {
    (void)stream;
    (void)mode;
    /* 昇腾无 Graph capture；SIMT 下任务直接下发 stream，无 capture 语义。 */
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaStreamEndCapture(aclrtStream stream, void **graph) {
    (void)stream;
    if (graph != nullptr) {
        *graph = nullptr;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaStreamIsCapturing(aclrtStream stream, int *status) {
    (void)stream;
    if (status != nullptr) {
        *status = 0; /* cudaStreamCaptureStatusNone */
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaStreamGetCaptureInfo(aclrtStream stream, int *status, unsigned long long *id_out) {
    (void)stream;
    if (status != nullptr) {
        *status = 0;
    }
    if (id_out != nullptr) {
        *id_out = 0;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaStreamGetCaptureInfo_v2(aclrtStream stream, int *status, unsigned long long *id_out,
                                      void *graph_out, const void **dependencies_out,
                                      size_t *numDependencies_out) {
    (void)stream;
    if (status != nullptr) {
        *status = 0;
    }
    if (id_out != nullptr) {
        *id_out = 0;
    }
    if (graph_out != nullptr) {
        *(void **)graph_out = nullptr;
    }
    if (dependencies_out != nullptr) {
        *dependencies_out = nullptr;
    }
    if (numDependencies_out != nullptr) {
        *numDependencies_out = 0;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

/* ---------------- Event Management ---------------- */

aclError wcudaEventCreate(aclrtEvent *event) {
    if (event == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtCreateEvent(event));
}

aclError wcudaEventCreateWithFlags(aclrtEvent *event, unsigned int flags) {
    (void)flags;
    if (event == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾无 flags 变体，忽略 flags */
    WCUDA_RETURN(aclrtCreateEvent(event));
}

aclError wcudaEventDestroy(aclrtEvent event) {
    if (event == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtDestroyEvent(event));
}

aclError wcudaEventRecord(aclrtEvent event, aclrtStream stream) {
    if (event == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* CUDA 的 cudaEventRecord 在 stream 为空时用默认 stream。
     * 昇腾侧 aclrtRecordEvent 的 stream 参数语义待真机核对（有的版本 stream 可传 null）。 */
    if (stream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtRecordEvent(event, stream));
}

aclError wcudaEventRecordWithFlags(aclrtEvent event, aclrtStream stream, unsigned int flags) {
    /* 昇腾无 flags 变体，忽略 flags，等价于 wcudaEventRecord */
    (void)flags;
    return wcudaEventRecord(event, stream);
}

aclError wcudaEventSynchronize(aclrtEvent event) {
    if (event == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtSynchronizeEvent(event));
}

aclError wcudaEventQuery(aclrtEvent event) {
    if (event == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 9.x 推荐 aclrtQueryEventStatus(event, aclrtEventRecordedStatus *status)
     * （旧 aclrtQueryEvent 已废弃）。注意 aclrtEventRecordedStatus 取值：
     *   NOT_READY=0, COMPLETE=1（与 aclrtEventStatus 的 COMPLETE=0/NOT_READY=1 相反！）
     * CUDA 语义：已完成返回 success，未完成返回 cudaErrorNotReady。 */
    aclrtEventRecordedStatus status = ACL_EVENT_RECORDED_STATUS_NOT_READY;
    aclError ret = aclrtQueryEventStatus(event, &status);
    if (ret != ACL_ERROR_NONE) {
        WCUDA_RETURN(ret);
    }
    if (status != ACL_EVENT_RECORDED_STATUS_COMPLETE) {
        WCUDA_RETURN(ACL_ERROR_RT_EVENT_NOT_COMPLETE);
    }
    WCUDA_RETURN(ACL_ERROR_NONE);
}

aclError wcudaEventElapsedTime(float *ms, aclrtEvent start, aclrtEvent end) {
    if (ms == nullptr || start == nullptr || end == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtEventElapsedTime(ms, start, end));
}

/* ---------------- Memory Management ---------------- */

aclError wcudaMalloc(void **ptr, size_t sizeBytes) {
    if (ptr == nullptr || sizeBytes == 0) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* aclrtMalloc 第三参是内存分配规则（大页/普通页）。
     * 选大页优先（ACL_MEM_MALLOC_HUGE_FIRST），与 CUDA 大块分配的默认行为接近。 */
    void *devPtr = nullptr;
    aclError ret = aclrtMalloc(&devPtr, sizeBytes, ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret == ACL_ERROR_NONE) {
        *ptr = devPtr;
    }
    WCUDA_RETURN(ret);
}

aclError wcudaFree(void *ptr) {
    if (ptr == nullptr) {
        WCUDA_RETURN(ACL_ERROR_NONE); /* cudaFree(NULL) 是合法的 no-op */
    }
    WCUDA_RETURN(aclrtFree(ptr));
}

aclError wcudaMemcpy(void *dst, const void *src, size_t count, aclrtMemcpyKind kind) {
    if (dst == nullptr || src == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* aclrtMemcpy 比 cudaMemcpy 多一个 destMax（目的内存最大长度），此处填 count。 */
    WCUDA_RETURN(aclrtMemcpy(dst, count, src, count, kind));
}

aclError wcudaMemcpyAsync(void *dst, const void *src, size_t count, aclrtMemcpyKind kind,
                          aclrtStream stream) {
    if (dst == nullptr || src == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    if (stream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtMemcpyAsync(dst, count, src, count, kind, stream));
}

aclError wcudaMemcpy2D(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width,
                       size_t height, aclrtMemcpyKind kind) {
    if (dst == nullptr || src == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾 aclrtMemcpy2d 签名与 cudaMemcpy2D 完全一致（dst,dpitch,src,spitch,width,height,kind） */
    WCUDA_RETURN(aclrtMemcpy2d(dst, dpitch, src, spitch, width, height, kind));
}

aclError wcudaMemcpy2DAsync(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width,
                            size_t height, aclrtMemcpyKind kind, aclrtStream stream) {
    if (dst == nullptr || src == nullptr || stream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtMemcpy2dAsync(dst, dpitch, src, spitch, width, height, kind, stream));
}

aclError wcudaMemset(void *devPtr, int value, size_t count) {
    if (devPtr == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* aclrtMemset(devPtr, maxCount, value, count) —— 注意 maxCount 是目的内存最大长度，
     * CUDA 无此参数，此处填 count。 */
    WCUDA_RETURN(aclrtMemset(devPtr, count, value, count));
}

aclError wcudaMemset2D(void *devPtr, size_t pitch, int value, size_t width, size_t height) {
    if (devPtr == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾无 aclrtMemset2d，用逐行 aclrtMemset 模拟（对 2D 矩阵清零）。
     * 注意 height 行、每行 width 字节、行距 pitch。 */
    char *row = (char *)devPtr;
    for (size_t y = 0; y < height; ++y) {
        aclError ret = aclrtMemset(row + y * pitch, width, value, width);
        if (ret != ACL_ERROR_NONE) {
            WCUDA_RETURN(ret);
        }
    }
    WCUDA_RETURN(ACL_ERROR_NONE);
}

aclError wcudaMemset2DAsync(void *devPtr, size_t pitch, int value, size_t width, size_t height,
                            aclrtStream stream) {
    if (devPtr == nullptr || stream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾无 aclrtMemset2dAsync，用逐行 aclrtMemsetAsync 模拟。 */
    char *row = (char *)devPtr;
    for (size_t y = 0; y < height; ++y) {
        aclError ret = aclrtMemsetAsync(row + y * pitch, width, value, width, stream);
        if (ret != ACL_ERROR_NONE) {
            WCUDA_RETURN(ret);
        }
    }
    WCUDA_RETURN(ACL_ERROR_NONE);
}

aclError wcudaMallocPitch(void **devPtr, size_t *pitch, size_t width, size_t height) {
    if (devPtr == nullptr || pitch == nullptr || width == 0 || height == 0) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾无 aclrtMallocPitch。模拟 CUDA 的 pitch 语义：
     *   每行按 width 字节对齐（CUDA 默认 256B 对齐行），分配 height 行。
     * ⚠️ 简化实现：行对齐取 width 本身（无对齐填充），若应用依赖 pitch 对齐
     *   需真机按实际对齐规则调整。 */
    size_t rowPitch = width;
    size_t total = rowPitch * height;
    void *mem = nullptr;
    aclError ret = aclrtMalloc(&mem, total, ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret == ACL_ERROR_NONE) {
        *devPtr = mem;
        *pitch = rowPitch;
    }
    WCUDA_RETURN(ret);
}

/* ---- Array（昇腾无 array/mipmap 概念，全部 not-support） ---- */

static aclError array_not_supported() { return ACL_ERROR_API_NOT_SUPPORT; }

aclError wcudaMallocArray(void *array, const void *desc, size_t width, size_t height,
                          unsigned int flags) {
    (void)array; (void)desc; (void)width; (void)height; (void)flags;
    return array_not_supported();
}

aclError wcudaMalloc3DArray(void *array, const void *desc, void *extent, unsigned int flags) {
    (void)array; (void)desc; (void)extent; (void)flags;
    return array_not_supported();
}

aclError wcudaMallocMipmappedArray(void *mipmappedArray, const void *desc, void *extent,
                                   unsigned int numLevels, unsigned int flags) {
    (void)mipmappedArray; (void)desc; (void)extent; (void)numLevels; (void)flags;
    return array_not_supported();
}

aclError wcudaFreeArray(void *array) {
    (void)array;
    return array_not_supported();
}

aclError wcudaFreeMipmappedArray(void *mipmappedArray) {
    (void)mipmappedArray;
    return array_not_supported();
}

aclError wcudaMemcpyToArray(void *dst, size_t wOffset, size_t hOffset, const void *src,
                            size_t count, int kind) {
    (void)dst; (void)wOffset; (void)hOffset; (void)src; (void)count; (void)kind;
    return array_not_supported();
}

aclError wcudaMemcpyFromArray(void *dst, const void *src, size_t wOffset, size_t hOffset,
                              size_t count, int kind) {
    (void)dst; (void)src; (void)wOffset; (void)hOffset; (void)count; (void)kind;
    return array_not_supported();
}

aclError wcudaMemcpy2DToArray(void *dst, size_t wOffset, size_t hOffset, const void *src,
                              size_t spitch, size_t width, size_t height, int kind) {
    (void)dst; (void)wOffset; (void)hOffset; (void)src; (void)spitch;
    (void)width; (void)height; (void)kind;
    return array_not_supported();
}

aclError wcudaMemcpy2DFromArray(void *dst, size_t dpitch, const void *src, size_t wOffset,
                                size_t hOffset, size_t width, size_t height, int kind) {
    (void)dst; (void)dpitch; (void)src; (void)wOffset; (void)hOffset;
    (void)width; (void)height; (void)kind;
    return array_not_supported();
}

/* ---- Texture/Surface（昇腾无对应，全部 not-support） ---- */

unsigned long long wcudaCreateChannelDesc(int x, int y, int z, int w) {
    /* 承载 4 个 16bit 通道描述；昇腾无 texture 通道描述概念，仅保证类型可编译。 */
    return ((unsigned long long)(unsigned short)x) | ((unsigned long long)(unsigned short)y << 16) |
           ((unsigned long long)(unsigned short)z << 32) |
           ((unsigned long long)(unsigned short)w << 48);
}

aclError wcudaCreateTextureObject(void *pTexObject, const void *pResDesc, const void *pTexDesc,
                                  const void *pResViewDesc) {
    (void)pResDesc; (void)pTexDesc; (void)pResViewDesc;
    if (pTexObject != nullptr) {
        *(void **)pTexObject = nullptr;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaDestroyTextureObject(void *texObject) {
    (void)texObject;
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaCreateSurfaceObject(void *pSurfObject, const void *pResDesc) {
    (void)pResDesc;
    if (pSurfObject != nullptr) {
        *(void **)pSurfObject = nullptr;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaDestroySurfaceObject(void *surfObject) {
    (void)surfObject;
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaBindTexture(size_t *offset, const void *texref, const void *devPtr,
                          const void *desc, size_t size) {
    (void)texref; (void)devPtr; (void)desc; (void)size;
    if (offset != nullptr) {
        *offset = 0;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

/* ---- Graph（昇腾无 CUDA Graph，核心函数 not-support） ---- */

aclError wcudaGraphCreate(void *pGraph, unsigned int flags) {
    (void)flags;
    if (pGraph != nullptr) {
        *(void **)pGraph = nullptr;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaGraphDestroy(void *graph) {
    (void)graph;
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaGraphInstantiate(void *pGraphExec, void *graph, void *pErrorNode, char *pLogBuffer,
                               size_t bufferSize) {
    (void)graph; (void)pErrorNode; (void)pLogBuffer; (void)bufferSize;
    if (pGraphExec != nullptr) {
        *(void **)pGraphExec = nullptr;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaGraphLaunch(void *graphExec, aclrtStream stream) {
    (void)graphExec;
    (void)stream;
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaGraphExecDestroy(void *graphExec) {
    (void)graphExec;
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaGraphExecUpdate(void *hGraphExec, void *hGraph, void *hErrorNode_out,
                              int *updateResult_out) {
    (void)hGraphExec; (void)hGraph;
    if (hErrorNode_out != nullptr) {
        *(void **)hErrorNode_out = nullptr;
    }
    if (updateResult_out != nullptr) {
        *updateResult_out = 0;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaGraphAddKernelNode(void *pGraphNode, void *graph, const void *pDependencies,
                                 size_t numDependencies, const void *pNodeParams) {
    (void)graph; (void)pDependencies; (void)numDependencies; (void)pNodeParams;
    if (pGraphNode != nullptr) {
        *(void **)pGraphNode = nullptr;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaGraphMemcpyNodeSetParams1D(void *node, void *dst, const void *src, size_t count,
                                         int kind) {
    (void)node; (void)dst; (void)src; (void)count; (void)kind;
    return ACL_ERROR_API_NOT_SUPPORT;
}

/* ---- External Memory（昇腾无对应，not-support） ---- */

aclError wcudaImportExternalMemory(void *extMem_out, const void *memHandleDesc) {
    (void)memHandleDesc;
    if (extMem_out != nullptr) {
        *(void **)extMem_out = nullptr;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaDestroyExternalMemory(void *extMem) {
    (void)extMem;
    return ACL_ERROR_API_NOT_SUPPORT;
}

/* ---- Symbol 内存（昇腾无 device symbol 概念，返回 not-support） ---- */

aclError wcudaGetSymbolAddress(void **devPtr, const void *symbol) {
    (void)symbol;
    if (devPtr != nullptr) {
        *devPtr = nullptr;
    }
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaGetSymbolSize(size_t *size, const void *symbol) {
    (void)symbol;
    if (size != nullptr) {
        *size = 0;
    }
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaMemcpyToSymbol(const void *symbol, const void *src, size_t count, size_t offset,
                             int kind) {
    (void)symbol;
    (void)src;
    (void)count;
    (void)offset;
    (void)kind;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaMemcpyFromSymbol(void *dst, const void *symbol, size_t count, size_t offset,
                               int kind) {
    (void)dst;
    (void)symbol;
    (void)count;
    (void)offset;
    (void)kind;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

/* ---- 3D 拷贝（昇腾无 3D 参数结构，not-support） ---- */

aclError wcudaMemcpy3D(const void *p) {
    (void)p;
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaMemcpy3DAsync(const void *p, aclrtStream stream) {
    (void)p;
    (void)stream;
    return ACL_ERROR_API_NOT_SUPPORT;
}

/* ---- Peer 拷贝 ---- */

aclError wcudaMemcpyPeer(void *dst, int dstDevice, const void *src, int srcDevice, size_t count) {
    (void)dstDevice;
    (void)srcDevice;
    if (dst == nullptr || src == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾 peer 拷贝用 aclrtMemcpy（DeviceToDevice），需先 EnablePeerAccess。
     * 简化：直接用 D2D 拷贝（假设 peer access 已启用）。 */
    WCUDA_RETURN(aclrtMemcpy(dst, count, src, count, ACL_MEMCPY_DEVICE_TO_DEVICE));
}

aclError wcudaMemcpyPeerAsync(void *dst, int dstDevice, const void *src, int srcDevice,
                              size_t count, aclrtStream stream) {
    (void)dstDevice;
    (void)srcDevice;
    if (dst == nullptr || src == nullptr || stream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtMemcpyAsync(dst, count, src, count, ACL_MEMCPY_DEVICE_TO_DEVICE, stream));
}

/* ---- SOMA（Stream Ordered Memory Allocator） ----
 * 昇腾无 aclrtMallocAsync/MemPool。MallocAsync/FreeAsync fallback 到普通 device 内存；
 * MemPool 系列返回 not-support。 */

aclError wcudaMallocAsync(void **devPtr, size_t size, aclrtStream hStream) {
    (void)hStream;
    if (devPtr == nullptr || size == 0) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* ⚠️ 昇腾无 stream-ordered 内存，fallback 到普通 aclrtMalloc。
     * 无 pool 复用语义，应用若依赖该特性需真机评估。 */
    return wcudaMalloc(devPtr, size);
}

aclError wcudaFreeAsync(void *devPtr, aclrtStream hStream) {
    (void)hStream;
    if (devPtr == nullptr) {
        WCUDA_RETURN(ACL_ERROR_NONE);
    }
    return wcudaFree(devPtr);
}

aclError wcudaMemPoolCreate(void *memPool, const void *poolProps) {
    (void)memPool;
    (void)poolProps;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaMemPoolDestroy(void *memPool) {
    (void)memPool;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaDeviceGetMemPool(void *memPool, int device) {
    (void)memPool;
    (void)device;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaDeviceSetMemPool(int device, void *memPool) {
    (void)device;
    (void)memPool;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

/* ---- Unified Memory（昇腾无 managed memory，fallback device 分配） ---- */

aclError wcudaMallocManaged(void **devPtr, size_t size, unsigned int flags) {
    (void)flags;
    if (devPtr == nullptr || size == 0) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* ⚠️ 昇腾无 cudaMallocManaged 对应（无统一内存/自动迁移）。
     * 这里 fallback 到普通 device 内存，应用代码可编译运行，但没有 host/device 自动迁移语义。
     * 若应用依赖 managed 的自动拷贝，需真机评估替代方案。 */
    return wcudaMalloc(devPtr, size);
}

aclError wcudaMemAdvise(const void *devPtr, size_t count, int advice, int device) {
    (void)devPtr;
    (void)count;
    (void)advice;
    (void)device;
    /* 昇腾无内存建议接口，no-op 返回成功（不影响正确性）。 */
    WCUDA_RETURN(ACL_ERROR_NONE);
}

aclError wcudaMemPrefetchAsync(const void *devPtr, size_t count, int dstDevice, aclrtStream stream) {
    (void)devPtr;
    (void)count;
    (void)dstDevice;
    (void)stream;
    /* 昇腾无预取接口，no-op 返回成功。 */
    WCUDA_RETURN(ACL_ERROR_NONE);
}

aclError wcudaPointerGetAttributes(void *attributes, const void *ptr) {
    (void)attributes;
    (void)ptr;
    /* 昇腾无 cudaPointerGetAttributes 对应（需要 cudaPointerAttributes 结构）。
     * 简化返回 not-support；若应用依赖指针属性查询需真机实现。 */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaMemsetAsync(void *devPtr, int value, size_t count, aclrtStream stream) {
    if (devPtr == nullptr || stream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtMemsetAsync(devPtr, count, value, count, stream));
}

aclError wcudaMemGetInfo(size_t *free, size_t *total) {
    if (free == nullptr || total == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* ACL_HBM_MEM 为昇腾 HBM 显存类型 */
    WCUDA_RETURN(aclrtGetMemInfo(ACL_HBM_MEM, free, total));
}

aclError wcudaMallocHost(void **ptr, size_t size) {
    if (ptr == nullptr || size == 0) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    void *hostPtr = nullptr;
    aclError ret = aclrtMallocHost(&hostPtr, size);
    if (ret == ACL_ERROR_NONE) {
        *ptr = hostPtr;
    }
    WCUDA_RETURN(ret);
}

aclError wcudaMallocHostWithFlags(void **ptr, size_t size, unsigned int flags) {
    /* 昇腾无 flags 变体，忽略 flags（cudaHostAllocMapped 等无对应语义） */
    (void)flags;
    return wcudaMallocHost(ptr, size);
}

aclError wcudaFreeHost(void *ptr) {
    if (ptr == nullptr) {
        WCUDA_RETURN(ACL_ERROR_NONE);
    }
    WCUDA_RETURN(aclrtFreeHost(ptr));
}

/* ---- 昇腾无对应：返回"不支持"，保证应用能编译、运行时明确报错 ---- */

aclError wcudaHostAlloc(void **pHost, size_t size, unsigned int flags) {
    /* cudaHostAlloc 与 cudaMallocHost 语义接近（pinned memory）。
     * 昇腾 aclrtMallocHost 已是 pinned host 内存，此处等价转发。 */
    return wcudaMallocHostWithFlags(pHost, size, flags);
}

aclError wcudaHostGetDevicePointer(void **pDevice, void *pHost, unsigned int flags) {
    (void)pHost;
    (void)flags;
    /* 昇腾无"host 指针映射为 device 可访问指针"的等价接口（昇腾 host 内存
     * 由 aclrtMallocHost 分配后即可在 memcpy 中直接使用）。返回不支持。 */
    if (pDevice != nullptr) {
        *pDevice = nullptr;
    }
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaHostRegister(void *ptr, size_t size, unsigned int flags) {
    (void)ptr;
    (void)size;
    (void)flags;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaHostUnregister(void *ptr) {
    (void)ptr;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaHostGetFlags(unsigned int *pFlags, void *pHost) {
    (void)pHost;
    if (pFlags != nullptr) {
        *pFlags = 0;
    }
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

/* ---------------- Error Handling ---------------- */

aclError wcudaGetLastError(void) {
    aclError err = g_last_error;
    g_last_error = ACL_ERROR_NONE;
    return err;
}

aclError wcudaPeekAtLastError(void) { return g_last_error; }

const char *wcudaGetErrorString(aclError error) {
    /* 昇腾 aclGetRecentErrMsg()（const char*，无参）返回最近一次失败的错误描述。
     * ⚠️ 语义差异：CUDA 的 cudaGetErrorString(error) 按错误码查描述，可重复调用；
     *   昇腾的是"获取并清空"最近错误，且指针仅最后一次有效。
     * 这里优先返回最近错误描述，退化到通用错误码串。 */
    if (error != ACL_ERROR_NONE) {
        const char *msg = aclGetRecentErrMsg();
        if (msg != nullptr && msg[0] != '\0') {
            return msg;
        }
    }
    switch (error) {
    case ACL_ERROR_NONE:
        return "no error";
    case ACL_ERROR_INVALID_PARAM:
        return "invalid parameter";
    case ACL_ERROR_RT_MEMORY_ALLOCATION:
        return "memory allocation failed";
    case ACL_ERROR_RT_INTERNAL_ERROR:
        return "runtime internal error";
    case ACL_ERROR_INVALID_DEVICE:
        return "invalid device";
    case ACL_ERROR_RT_NO_DEVICE:
        return "no device";
    case ACL_ERROR_API_NOT_SUPPORT:
        return "operation not supported on this platform";
    default:
        return "unknown error";
    }
}

const char *wcudaGetErrorName(aclError error) {
    /* 昇腾无直接的错误名查询接口；先返回占位，真机核对后可用
     * aclrtGetErrorInfo 的字符串字段或自建 错误码->名字 表。 */
    static const char *unknown = "unknown";
    (void)error;
    return unknown;
}

/* ---------------- Peer Device Memory Access ---------------- */

aclError wcudaDeviceCanAccessPeer(int *canAccessPeer, int device, int peerDevice) {
    if (canAccessPeer == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    int32_t can = 0;
    aclError ret = aclrtDeviceCanAccessPeer(&can, device, peerDevice);
    if (ret == ACL_ERROR_NONE) {
        *canAccessPeer = (int)can;
    }
    WCUDA_RETURN(ret);
}

aclError wcudaDeviceEnablePeerAccess(int peerDevice, unsigned int flags) {
    /* 昇腾 aclrtDeviceEnablePeerAccess(int32_t peerDeviceId, uint32_t flags) */
    WCUDA_RETURN(aclrtDeviceEnablePeerAccess(peerDevice, flags));
}

aclError wcudaDeviceDisablePeerAccess(int peerDevice) {
    WCUDA_RETURN(aclrtDeviceDisablePeerAccess(peerDevice));
}

/* ---------------- Context Management ---------------- */

aclError wcudaCtxCreate(wcceContext_t *ctx, unsigned int flags, int device) {
    (void)flags; /* 昇腾无 flags 变体 */
    if (ctx == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtCreateContext(ctx, device));
}

aclError wcudaCtxDestroy(wcceContext_t ctx) {
    if (ctx == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtDestroyContext(ctx));
}

aclError wcudaCtxSetCurrent(wcceContext_t ctx) {
    if (ctx == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtSetCurrentContext(ctx));
}

aclError wcudaCtxGetCurrent(wcceContext_t *ctx) {
    if (ctx == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtGetCurrentContext(ctx));
}

aclError wcudaCtxSynchronize(void) {
    /* 阻塞直到当前 context 所有任务完成（对应 cudaCtxSynchronize）。
     * 昇腾无独立 context 同步接口，用 device 同步近似。 */
    WCUDA_RETURN(aclrtSynchronizeDevice());
}

/* ---------------- Occupancy ----------------
 * 昇腾无 occupancy 计算接口。给 SIMT 合理默认值：block 上限 2048。
 * 仅供应用据此选 blockSize；精确的硬件 occupancy 需真机用 aclrtGetDeviceProperties 校准。 */

aclError wcudaOccupancyMaxActiveBlocksPerMultiprocessor(int *numBlocks, const void *func,
                                                        int blockSize, size_t dynamicSMemSize) {
    (void)func;
    (void)dynamicSMemSize;
    if (numBlocks == nullptr || blockSize <= 0) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾 SIMT 单 block 最多 2048 线程；假设每核至少能放 1 个 block。 */
    *numBlocks = 1;
    WCUDA_RETURN(ACL_ERROR_NONE);
}

aclError wcudaOccupancyMaxPotentialBlockSize(int *minGridSize, int *blockSize, const void *func,
                                             size_t dynamicSMemSize, int blockSizeLimit) {
    (void)func;
    (void)dynamicSMemSize;
    if (minGridSize == nullptr || blockSize == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    int limit = (blockSizeLimit > 0) ? blockSizeLimit : 2048;
    if (limit > 2048) {
        limit = 2048; /* 昇腾 SIMT block 上限 */
    }
    *blockSize = limit;
    *minGridSize = 1;
    WCUDA_RETURN(ACL_ERROR_NONE);
}

/* ---------------- Execution Control ---------------- */

aclError wcudaLaunchKernel(const void *hostFunction, int gridDimX, int gridDimY, int gridDimZ,
                           int blockDimX, int blockDimY, int blockDimZ, void **args,
                           size_t sharedMemBytes, aclrtStream stream) {
    (void)hostFunction; (void)gridDimX; (void)gridDimY; (void)gridDimZ;
    (void)blockDimX; (void)blockDimY; (void)blockDimZ;
    (void)args; (void)sharedMemBytes; (void)stream;
    /* ⚠️ 昇腾 SIMT 请用 kernel<<<blocks, threads, stream>>>(...) 语法糖启动（bisheng 原生）。
     * 显式 cudaLaunchKernel 桥接需 aclrtLaunchKernelV2 + kernel stub（Phase A3 收尾）。 */
    return ACL_ERROR_API_NOT_SUPPORT;
}

/* ---------------- Profiler Control ----------------
 * 昇腾 profiling 用 aclprof（API 面不同）。此处 no-op 占位，
 * 真机如需 profiling 走 aclprof 单独启用。 */

aclError wcudaProfilerStart(void) { return ACL_ERROR_NONE; }

aclError wcudaProfilerStop(void) { return ACL_ERROR_NONE; }

/* ---------------- Thread Management（废弃 API） ---------------- */

aclError wcudaThreadSynchronize(void) {
    /* cudaThreadSynchronize 是 cudaDeviceSynchronize 的旧名 */
    return wcudaDeviceSynchronize();
}

aclError wcudaThreadExit(void) {
    /* cudaThreadExit 旧名，昇腾无线程退出概念，等价 device reset */
    return wcudaDeviceReset();
}

aclError wcudaThreadGetCacheConfig(int *cacheConfig) {
    /* 废弃 API，昇腾无 cache 配置，返回 not-support */
    (void)cacheConfig;
    return ACL_ERROR_API_NOT_SUPPORT;
}

/* ---------------- Host Function & Func 属性 ---------------- */

aclError wcudaLaunchHostFunc(aclrtStream stream, wcceHostFn_t fn, void *userData) {
    if (stream == nullptr || fn == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾 aclrtLaunchCallback(fn, userData, blockType, stream)。
     * blockType 用 ACL_CALLBACK_NO_BLOCK（不阻塞，语义接近 cudaLaunchHostFunc）。 */
    WCUDA_RETURN(aclrtLaunchCallback(fn, userData, ACL_CALLBACK_NO_BLOCK, stream));
}

aclError wcudaFuncGetAttributes(void *attr, const void *func) {
    (void)attr;
    (void)func;
    /* 昇腾无 kernel 属性查询接口，返回 not-support。 */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaFuncSetAttribute(const void *func, int attr, int value) {
    (void)func;
    (void)attr;
    (void)value;
    /* 昇腾无 func 属性设置接口，返回 not-support。 */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaFuncGetName(const char **name, const void *func) {
    (void)func;
    if (name != nullptr) {
        *name = "kernel";
    }
    WCUDA_RETURN(ACL_ERROR_NONE);
}

/* ---------------- Version ---------------- */

aclError wcudaRuntimeGetVersion(int *runtimeVersion) {
    if (runtimeVersion == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 返回模拟的 CUDA 版本（如 11060），与 cu-bridge 的版本模拟一致。
     * 注意：不依赖 aclrtGetVersion（其签名是 3 参 major/minor/patch 且返回 ACL 接口版本，
     * 不是 CUDA 版本；torch_npu 的 CANN 9.x 头中也不存在该接口）。
     * 应用按 CUDA 版本号判断的特性（#if CUDA_VERSION）由编译期宏 CUDA_VERSION 保证。 */
    *runtimeVersion = WCUDA_VERSION;
    WCUDA_RETURN(ACL_ERROR_NONE);
}

aclError wcudaDriverGetVersion(int *driverVersion) {
    /* 昇腾无 Driver/Runtime 分层，直接复用 runtime 版本号。 */
    return wcudaRuntimeGetVersion(driverVersion);
}
