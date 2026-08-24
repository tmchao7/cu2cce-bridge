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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h> /* usleep */

/* ---------------- 内部状态 ---------------- */

/* 线程局部"上一次错误"。仿照 cu-bridge 的 g_last_error。 */
static thread_local aclError g_last_error = ACL_SUCCESS;

/* 全局是否已有 context（仿照 cu-bridge 的 g_hasContextBeenCreated） */
static thread_local bool g_has_context_created = false;

#define ACTIVE_CONTEXT() g_has_context_created = true;

/* 107002 排查压印（INVESTIGATION_107002.md §5.6）：bridge_init 与 wcudaMalloc
 * 逐行打时间戳，先定位"minimal 进程内 aclInit/SetDevice 全 OK 却紧跟 107002"
 * 的第三个动作在哪个进程空间发生。走 WCUDA_DEBUG（无 env 不输出）。 */
static bool wcuda_dbg() {
    const char *env = getenv("WCUDA_DEBUG");
    return env != nullptr && env[0] != '\0';
}
static void wcuda_mark(const char *label, aclError e) {
    if (wcuda_dbg()) {
        fprintf(stderr, "[wcuda] %-26s ret=%d (%s)\n", label, (int)e,
                e == ACL_SUCCESS ? "OK" : "FAIL");
    }
}

/* ---------------- bridge 初始化 ---------------- */

namespace cubridge::internal {

/* 107002 排查（INVESTIGATION_107002.md §5.1）：初始化各步返回码全部打印，
 * 不再静默吞掉——minimal 路径首次 API 即 cudaMalloc，上游是 aclInit、
 * aclrtGetDeviceCount 还是 aclrtSetDevice 失败，从输出一眼看出。 */
static bool bridge_debug_enabled() {
    const char *env = getenv("WCUDA_DEBUG");
    return env != nullptr && env[0] != '\0';
}

static void bridge_log(const char *step, int32_t ret) {
    if (bridge_debug_enabled()) {
        fprintf(stderr, "[bridge_init] %-24s ret=%d (%s)\n", step, (int)ret,
                ret == ACL_SUCCESS ? "OK" : "FAIL");
    }
}

void bridge_init() {
    /* 只跑一次（由 BRIDGE_INIT 的 std::call_once 保证）。
     * 昇腾 ACL 应用在首次调用运行时 API 前需 aclInit。
     * aclInit 参数传 nullptr 表示默认配置（对应 CUDA 的惰性初始化）。 */
    int32_t ret = aclInit(nullptr);
    bridge_log("aclInit", ret);

    /* 对齐 vectorAdd 的调用顺序（INVESTIGATION_107002.md §5.2 候选根因 1）：
     * 先 aclrtGetDeviceCount 完成设备枚举，再 aclrtSetDevice(0)。vectorAdd
     * 能跑正是因为 main 第一个 API 是 cudaGetDeviceCount（先枚举后设设备）。 */
    uint32_t devCnt = 0;
    ret = aclrtGetDeviceCount(&devCnt);
    bridge_log("aclrtGetDeviceCount", ret);

    /* CUDA 语义：默认 device 0（首次 API 调用隐式设置，用户可不写 cudaSetDevice）。
     * 昇腾 aclrtMalloc 等必须先 aclrtSetDevice。失败时连续重试几次
     * （ACL 初始化/驱动握手需要时间），并打印每次结果。 */
    for (int i = 0; i < 5; ++i) {
        aclError dr = aclrtSetDevice(0);
        char step[32];
        snprintf(step, sizeof(step), "aclrtSetDevice(0) #%d", i);
        bridge_log(step, (int32_t)dr);
        if (dr == ACL_SUCCESS) {
            break;
        }
        usleep(100 * 1000);  /* 100ms */
    }

    /* 107002 根因修复（INVESTIGATION_107002.md 结论节，2026-08-19 定案）：
     * 仅 SetDevice(0) 还不足以让后续 aclrtMalloc 成功——需显式创建 context
     * 并设为当前 context。
     * 证据（ROUND13）：同一台机、同一 wrapper：
     *   - ctor_alloc（aclInit→GetDeviceCount→SetDevice→aclrtMalloc）malloc ret=0
     *   - prog_b/c/d（main 里先 cudaGetDeviceCount/GetDevice/SetDevice 任一前置
     *     API，触发 bridge_init 后再 cudaMalloc）全部 success
     *   - prog_a/minimal/echo（首个 API 即 cudaMalloc，bridge_init 在
     *     '0 前置'下 SetDevice 完立刻 aclrtMalloc）稳定 107002
     * 区别：前置 API（wcudaGetDeviceCount/wcudaGetDevice/wcudaSetDevice）内部
     * 的 aclrtGetDeviceCount/aclrtGetDevice 调用会隐式建立 ACL 的当前上下文；
     * 而 minimal 路径在 aclrtMalloc 前没有任何一次会隐式建上下文的 API。
     * ROUND14 第一版（仅 aclrtCreateContext）不生效：CreateContext 创建的
     * context 并不自动成为当前 context，aclrtMalloc 仍需线程的当前 context。
     * 修复：CreateContext 后显式 aclrtSetCurrentContext(ctx)（对齐 ctor_alloc
     * 的完整成功路径：SetDevice→CreateContext→SetCurrentContext→分配）。 */
    aclrtContext ctx = nullptr;
    aclError cc = aclrtCreateContext(&ctx, 0);
    bridge_log("aclrtCreateContext(0)", (int32_t)cc);
    aclError cs = ACL_ERROR_INVALID_PARAM;
    if (ctx != nullptr) {
        cs = aclrtSetCurrentContext(ctx);
    }
    bridge_log("aclrtSetCurrentContext", (int32_t)cs);
    /* context 保持当前/默认（不主动 destroy：CUDA 默认 context 生命周期为进程，
     * 应用后续 API 需在固定当前 context 上工作；aclFinalize 在无 res 时统一收尾） */
    (void)ctx;
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
        if (error != ACL_SUCCESS) {                                                                \
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
    if (ret == ACL_SUCCESS) {
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
    if (ret == ACL_SUCCESS) {
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
     * 昇腾官方 ACL GEMM sample 的进程收尾顺序是 aclrtResetDevice -> aclFinalize。
     * 官方 GEMM/ATC 路径已验证 reset 返回 0、main 打印 done 后仍 SIGSEGV，说明
     * CANN runtime 的进程退出清理还未完成；对已调用 aclopSetModelDir 的进程，补上
     * aclFinalize，避免 CANN 静态析构阶段再次清理同一 runtime 状态。 */
    int32_t dev = 0;
    BRIDGE_INIT();
    aclError ret = aclrtGetDevice(&dev);
    if (ret != ACL_SUCCESS) {
        g_last_error = ret;
        return ret;
    }
    ret = aclrtResetDevice(dev);
    if (ret != ACL_SUCCESS) {
        g_last_error = ret;
        return ret;
    }
    aclError final_ret = aclFinalize();
    if (wcuda_dbg()) {
        fprintf(stderr, "[wcuda][device_reset] reset=0 finalize=%d recent=%s\n",
                (int)final_ret, aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
    }
    if (final_ret != ACL_SUCCESS) {
        g_last_error = final_ret;
    }
    return final_ret;
}

aclError wcudaGetDeviceProperties(_cceDeviceProp_t *prop, int device) {
    BRIDGE_INIT(); /* 首次调用即本函数时先完成 ACL 初始化（107002 教训） */
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
    if (ret == ACL_SUCCESS) {
        prop->totalGlobalMem = totalMem;
    }

    /* 昇腾 SIMT 语义（官方确认）：warp=32、block 上限 2048、gridDim 乘积 ≤65535。
     * 具体核数建议真机上用 aclrtGetDeviceInfo(deviceId, attr, &value) 补齐
     * （昇腾无 aclrtGetDeviceProperties，正确查询 API 是 aclrtGetDeviceInfo；
     *   这里先用保守值，需要精确值时在真机核对）。 */
    prop->warpSize = 32;
    prop->maxThreadsPerBlock = 2048;
    prop->maxThreadsDim[0] = 2048;
    prop->maxThreadsDim[1] = 1;
    prop->maxThreadsDim[2] = 1;
    /* ⚠️ 昇腾 gridDim 约束是 乘积 ≤65535（非每维）；此处按 CUDA 语义填每维上限，
     * 应用若算出超出乘积的 grid 需真机调整。 */
    prop->maxGridSize[0] = 65535;
    prop->maxGridSize[1] = 65535;
    prop->maxGridSize[2] = 65535;
    prop->sharedMemPerBlock = 256 * 1024; /* SIMT 下 UB 容量约 256KB */
    prop->multiProcessorCount = 20;       /* 保守值，真机按 aclrtGetDeviceInfo 校准 */
    prop->clockRate = 0;                  /* 昇腾无时钟频率语义，填 0 */
    /* CUDA Samples use these only to select/print a CUDA device. Ascend has no
     * CUDA SM capability; expose a stable compatibility value so NVIDIA's
     * helper_cuda.h can remain unmodified. */
    prop->major = 9;
    prop->minor = 0;

    WCUDA_RETURN(ACL_SUCCESS);
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
    WCUDA_RETURN(ACL_SUCCESS);
}

aclError wcudaDeviceGetByPCIBusId(int *device, const char *pciBusId) {
    (void)pciBusId;
    if (device == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾无 PCI 标识定位设备，返回 device 0（若有多个设备需真机扩展）。 */
    *device = 0;
    WCUDA_RETURN(ACL_SUCCESS);
}

aclError wcudaDeviceGetStreamPriorityRange(int *leastPriority, int *greatestPriority) {
    /* 昇腾无 stream 优先级概念，返回 [0,0]（单优先级）。 */
    if (leastPriority != nullptr) {
        *leastPriority = 0;
    }
    if (greatestPriority != nullptr) {
        *greatestPriority = 0;
    }
    WCUDA_RETURN(ACL_SUCCESS);
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
    if (err != ACL_SUCCESS) {
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
    case 21: /* ComputeMode */
        *value = 0; /* CUDA compute mode default；Ascend 无 prohibited mode */
        break;
    case 22: /* ComputeCapabilityMajor */
        *value = prop.major;
        break;
    case 23: /* ComputeCapabilityMinor */
        *value = prop.minor;
        break;
    default:
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(ACL_SUCCESS);
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
        g_last_error = ACL_ERROR_INVALID_PARAM;
        return ACL_ERROR_INVALID_PARAM;
    }
    /* Destroy is deliberately not wrapped in BRIDGE_INIT/WCUDA_RETURN. The stream was
     * created only after bridge_init, and re-entering the once-only ACL initializer here
     * is unsafe after aclopSetModelDir + aclblasGemmEx. Official ACL samples destroy the
     * already-synchronized stream directly with aclrtDestroyStream. */
    if (wcuda_dbg()) {
        fprintf(stderr, "[wcuda][stream_destroy] before stream=%p\n", (void *)stream);
    }
    aclError ret = aclrtDestroyStream(stream);
    if (wcuda_dbg()) {
        fprintf(stderr, "[wcuda][stream_destroy] after ret=%d recent=%s\n", (int)ret,
                aclGetRecentErrMsg() != nullptr ? aclGetRecentErrMsg() : "(null)");
    }
    if (ret != ACL_SUCCESS) {
        g_last_error = ret;
    }
    return ret;
}

aclError wcudaStreamSynchronize(aclrtStream stream) {
    if (stream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtSynchronizeStream(stream));
}

aclError wcudaStreamQuery(aclrtStream stream) {
    BRIDGE_INIT(); /* 首次调用即本函数时先完成 ACL 初始化（107002 教训） */
    if (stream == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* aclrtStreamQuery(stream, aclrtStreamStatus *status)。
     * CUDA 语义：cudaStreamQuery 已完成返回 success，未完成返回 cudaErrorNotReady。
     * 昇腾用 status 输出判断：ACL_STREAM_STATUS_COMPLETE / NOT_READY。 */
    aclrtStreamStatus status = ACL_STREAM_STATUS_COMPLETE;
    aclError ret = aclrtStreamQuery(stream, &status);
    if (ret != ACL_SUCCESS) {
        WCUDA_RETURN(ret);
    }
    if (status != ACL_STREAM_STATUS_COMPLETE) {
        WCUDA_RETURN(ACL_ERROR_RT_STREAM_NOT_COMPLETE);
    }
    WCUDA_RETURN(ACL_SUCCESS);
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
    BRIDGE_INIT(); /* 首次调用即本函数时先完成 ACL 初始化（107002 教训） */
    if (event == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 9.x 推荐 aclrtQueryEventStatus(event, aclrtEventRecordedStatus *status)
     * （旧 aclrtQueryEvent 已废弃）。注意 aclrtEventRecordedStatus 取值：
     *   NOT_READY=0, COMPLETE=1（与 aclrtEventStatus 的 COMPLETE=0/NOT_READY=1 相反！）
     * CUDA 语义：已完成返回 success，未完成返回 cudaErrorNotReady。 */
    aclrtEventRecordedStatus status = ACL_EVENT_RECORDED_STATUS_NOT_READY;
    aclError ret = aclrtQueryEventStatus(event, &status);
    if (ret != ACL_SUCCESS) {
        WCUDA_RETURN(ret);
    }
    if (status != ACL_EVENT_RECORDED_STATUS_COMPLETE) {
        WCUDA_RETURN(ACL_ERROR_RT_EVENT_NOT_COMPLETE);
    }
    WCUDA_RETURN(ACL_SUCCESS);
}

aclError wcudaEventElapsedTime(float *ms, aclrtEvent start, aclrtEvent end) {
    if (ms == nullptr || start == nullptr || end == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtEventElapsedTime(ms, start, end));
}

/* ---------------- Memory Management ---------------- */

aclError wcudaMalloc(void **ptr, size_t sizeBytes) {
    /* 107002 根因（ROUND17 定案）：BRIDGE_INIT 只在 WCUDA_RETURN 宏里触发，
     * 而 wcudaMalloc 的 aclrtMalloc 调用写在 WCUDA_RETURN 之前——首个用户
     * API 即 cudaMalloc 时，aclrtMalloc 在 aclInit 之前就执行，稳定 107002。
     * 压印顺序铁证：每次输出都是 [wcuda] aclrtMalloc call → 之后才
     * [bridge_init] aclInit。前置 API（GetDeviceCount 等）成功是因为
     * aclrtGetDeviceCount 会隐式完成 ACL 初始化，恰好在 aclrtMalloc 之前
     * 建立了上下文。修复：函数体开头显式 BRIDGE_INIT()（call_once 幂等）。 */
    BRIDGE_INIT();
    if (ptr == nullptr || sizeBytes == 0) {
        wcuda_mark("wcudaMalloc(invalid)", ACL_ERROR_INVALID_PARAM);
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    wcuda_mark("wcudaMalloc enter", ACL_SUCCESS);
    /* 107002 排查（ROUND16）：把传给 aclrtMalloc 的 size/policy 原值打出来，
     * 确认 wrapper 是否有参数被改/硬编码（sizeBytes 应为 n*sizeof(float)，
     * policy 应为 ACL_MEM_MALLOC_HUGE_FIRST）。 */
    if (wcuda_dbg()) {
        fprintf(stderr, "[wcuda] wcudaMalloc size=%zu policy=%d\n", sizeBytes,
                (int)ACL_MEM_MALLOC_HUGE_FIRST);
    }
    /* aclrtMalloc 第三参是内存分配规则（大页/普通页）。
     * 选大页优先（ACL_MEM_MALLOC_HUGE_FIRST），与 CUDA 大块分配的默认行为接近。 */
    void *devPtr = nullptr;
    aclError ret = aclrtMalloc(&devPtr, sizeBytes, ACL_MEM_MALLOC_HUGE_FIRST);
    wcuda_mark("aclrtMalloc call", ret);
    if (ret == ACL_SUCCESS) {
        *ptr = devPtr;
    }
    wcuda_mark("wcudaMalloc exit", ret);
    WCUDA_RETURN(ret);
}

aclError wcudaFree(void *ptr) {
    if (ptr == nullptr) {
        WCUDA_RETURN(ACL_SUCCESS); /* cudaFree(NULL) 是合法的 no-op */
    }
    WCUDA_RETURN(aclrtFree(ptr));
}

aclError wcudaMemcpy(void *dst, const void *src, size_t count, aclrtMemcpyKind kind) {
    if (dst == nullptr || src == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* CUDA synchronous D2H copies order after preceding work in the default stream.
     * NVIDIA's official vectorAdd sample intentionally uses a 2-argument launch and
     * then cudaMemcpy without an explicit cudaStreamSynchronize. On Ascend, the CCE
     * default-stream path can still have pending SIMT work when aclrtMemcpy is called;
     * synchronize the device before a synchronous device-originating copy to preserve
     * CUDA's blocking-copy contract. Explicit-stream samples already synchronized, so
     * this is a no-op from a correctness perspective. */
    if (kind == ACL_MEMCPY_DEVICE_TO_HOST || kind == ACL_MEMCPY_DEVICE_TO_DEVICE) {
        aclError sync = aclrtSynchronizeDevice();
        if (sync != ACL_SUCCESS) {
            WCUDA_RETURN(sync);
        }
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
        if (ret != ACL_SUCCESS) {
            WCUDA_RETURN(ret);
        }
    }
    WCUDA_RETURN(ACL_SUCCESS);
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
        if (ret != ACL_SUCCESS) {
            WCUDA_RETURN(ret);
        }
    }
    WCUDA_RETURN(ACL_SUCCESS);
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
    if (ret == ACL_SUCCESS) {
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
        WCUDA_RETURN(ACL_SUCCESS);
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
    WCUDA_RETURN(ACL_SUCCESS);
}

aclError wcudaMemPrefetchAsync(const void *devPtr, size_t count, int dstDevice, aclrtStream stream) {
    (void)devPtr;
    (void)count;
    (void)dstDevice;
    (void)stream;
    /* 昇腾无预取接口，no-op 返回成功。 */
    WCUDA_RETURN(ACL_SUCCESS);
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
    if (ret == ACL_SUCCESS) {
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
        WCUDA_RETURN(ACL_SUCCESS);
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
    g_last_error = ACL_SUCCESS;
    return err;
}

aclError wcudaPeekAtLastError(void) { return g_last_error; }

const char *wcudaGetErrorString(aclError error) {
    /* 昇腾 aclGetRecentErrMsg()（const char*，无参）返回最近一次失败的错误描述。
     * ⚠️ 语义差异：CUDA 的 cudaGetErrorString(error) 按错误码查描述，可重复调用；
     *   昇腾的是"获取并清空"最近错误，且指针仅最后一次有效。
     * 这里优先返回最近错误描述，退化到通用错误码串。 */
    if (error != ACL_SUCCESS) {
        const char *msg = aclGetRecentErrMsg();
        if (msg != nullptr && msg[0] != '\0') {
            return msg;
        }
    }
    switch (error) {
    case ACL_SUCCESS:
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
    if (ret == ACL_SUCCESS) {
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
    WCUDA_RETURN(ACL_SUCCESS);
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
    WCUDA_RETURN(ACL_SUCCESS);
}

/* ---------------- Execution Control ---------------- */

aclError wcudaLaunchKernel(const void *hostFunction, dim3 gridDim, dim3 blockDim, void **args,
                           size_t sharedMemBytes, aclrtStream stream) {
    if (hostFunction == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾 aclrtLaunchSIMTKernelWithArgsArray(func, gridDim, blockDim, dynUbufSize, stream, cfg, args)
     * 官方文档确认：Ascend 950PR/950DT 支持（A2/A3/910B 不支持）。
     * cfg 传 nullptr（默认配置）。dynUbufSize = CUDA 的 sharedMemBytes。 */
    WCUDA_RETURN(aclrtLaunchSIMTKernelWithArgsArray((void *)hostFunction, gridDim, blockDim,
                                                    sharedMemBytes, stream, nullptr, args));
}

/* ---------------- Profiler Control ----------------
 * 昇腾 profiling 用 aclprof（API 面不同）。此处 no-op 占位，
 * 真机如需 profiling 走 aclprof 单独启用。 */

aclError wcudaProfilerStart(void) { return ACL_SUCCESS; }

aclError wcudaProfilerStop(void) { return ACL_SUCCESS; }

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

/* ⚠️ 昇腾回调机制（官方 12_execution_control.md 确认）与 cudaLaunchHostFunc 有实质差异：
 *   1. 需先创建线程并 aclrtSubscribeReport(threadId, stream) 注册；
 *   2. 再 aclrtLaunchCallback(fn, userData, blockType, stream) 下发；
 *   3. 回调在注册线程中执行（配合 aclrtProcessReport 等待）。
 * CUDA 无此注册步骤。当前只做 aclrtLaunchCallback 调用（签名正确），
 * 完整的线程注册流程需真机验证 aclrtSubscribeReport 的线程-流绑定语义后补全。 */

aclError wcudaLaunchHostFunc(aclrtStream stream, wcceHostFn_t fn, void *userData) {
    if (stream == nullptr || fn == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* blockType 用 ACL_CALLBACK_NO_BLOCK（不阻塞 stream 后续任务，语义接近 cudaLaunchHostFunc）。
     * ⚠️ 待真机：需先 aclrtSubscribeReport 注册回调线程，否则回调可能不触发。 */
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
    WCUDA_RETURN(ACL_SUCCESS);
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
    WCUDA_RETURN(ACL_SUCCESS);
}

aclError wcudaDriverGetVersion(int *driverVersion) {
    /* 昇腾无 Driver/Runtime 分层，直接复用 runtime 版本号。 */
    return wcudaRuntimeGetVersion(driverVersion);
}
