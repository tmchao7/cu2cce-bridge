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
    /* 昇腾 ACL 无 aclrtStreamQuery（真实头文件中只有 aclrtSynchronizeStream）。
     * CUDA 语义：cudaStreamQuery 非阻塞查询，已完成返回 success，未完成返回 cudaErrorNotReady。
     * 昇腾无直接对应，此处用阻塞同步模拟并返回 success；
     * ⚠️ 语义不完全等价，待真机核对是否有其它查询手段。 */
    WCUDA_RETURN(aclrtSynchronizeStream(stream));
}

aclError wcudaStreamWaitEvent(aclrtStream stream, aclrtEvent event, unsigned int flags) {
    (void)flags;
    if (stream == nullptr || event == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtStreamWaitEvent(stream, event));
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
    /* aclrtQueryEvent(event, aclrtEventStatus *status)。
     * CUDA 语义：已完成返回 success，未完成返回 cudaErrorNotReady。
     * 昇腾用 status 输出判断：ACL_EVENT_STATUS_COMPLETE / NOT_COMPLETE。 */
    aclrtEventStatus status = ACL_EVENT_STATUS_COMPLETE;
    aclError ret = aclrtQueryEvent(event, &status);
    if (ret != ACL_ERROR_NONE) {
        WCUDA_RETURN(ret);
    }
    if (status != ACL_EVENT_STATUS_COMPLETE) { /* ACL_EVENT_STATUS_NOT_READY=1 */
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
    /* 昇腾 aclrtGetErrorInfo 可查错误描述；此处先返回固定串，真机核对后完善。 */
    static const char *unknown = "unknown error";
    (void)error;
    return unknown;
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

/* ---------------- Version ---------------- */

aclError wcudaRuntimeGetVersion(int *runtimeVersion) {
    if (runtimeVersion == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    int32_t ver = 0;
    aclError ret = aclrtGetVersion(&ver);
    if (ret == ACL_ERROR_NONE) {
        /* 昇腾返回的版本号格式与 CUDA 的 12090 不同，此处原样透传，
         * 应用若按 CUDA 版本号判断请谨慎。 */
        *runtimeVersion = (int)ver;
    }
    WCUDA_RETURN(ret);
}

aclError wcudaDriverGetVersion(int *driverVersion) {
    /* 昇腾无 Driver/Runtime 分层，直接复用 runtime 版本号。 */
    return wcudaRuntimeGetVersion(driverVersion);
}
