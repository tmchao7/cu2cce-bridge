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
#include <mutex>
#include <unordered_map>
#include <unistd.h> /* usleep */

/* ---------------- 内部状态 ---------------- */

/* 线程局部"上一次错误"。仿照 cu-bridge 的 g_last_error。 */
static thread_local aclError g_last_error = ACL_SUCCESS;

/* 全局是否已有 context（仿照 cu-bridge 的 g_hasContextBeenCreated） */
static thread_local bool g_has_context_created = false;

#define ACTIVE_CONTEXT() g_has_context_created = true;

/* 107002 排查压印（历史 runtime context 排障）：bridge_init 与 wcudaMalloc
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

/* ---------------- CUDA 默认流表示 ---------------- */

/* CUDA 的默认流（cudaStream_t 0，及 legacy/per-thread 变体）在昇腾没有对应
 * 句柄；aclrt* 家族把 nullptr 流一律判 ACL_ERROR_INVALID_PARAM（100000），
 * 上游 simpleStreams 的 cudaEventRecord(start_event, 0) 就是这样失败的
 * （真机日志 code=100000）。这里惰性创建一条桥内自有流作为 CUDA 三种默认流
 * 拼写的后端替身；用户显式创建的流原样透传。创建失败时持续返回同一错误，
 * 不做静默伪造。 */
/* Bridge-owned stand-in for the CUDA default stream (set by the resolver). */
static aclrtStream g_bridge_default_stream = nullptr;
static bool g_bridge_default_stream_ready = false;

static bool wcuda_is_bridge_default(aclrtStream s) {
    return g_bridge_default_stream_ready && s != nullptr && s == g_bridge_default_stream;
}

/* 记录在桥内默认流上的事件登记表（消费点语义，见 wcudaEventRecord 注释）。 */
static std::mutex g_event_mode_mu;
static std::unordered_map<void *, bool> g_event_on_default;

static void wcuda_event_mark_default(aclrtEvent event, bool on_default) {
    std::lock_guard<std::mutex> guard(g_event_mode_mu);
    g_event_on_default[(void *)event] = on_default;
}

static bool wcuda_event_is_default(aclrtEvent event) {
    std::lock_guard<std::mutex> guard(g_event_mode_mu);
    auto it = g_event_on_default.find((void *)event);
    return it != g_event_on_default.end() && it->second;
}

static aclError wcuda_resolve_stream(aclrtStream request, aclrtStream *resolved) {
    if (resolved == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    if (request != nullptr) {
        *resolved = request;
        return ACL_SUCCESS;
    }
    BRIDGE_INIT(); /* 默认流可能是进程内的首个 runtime API */
    static aclError create_status = ACL_SUCCESS;
    static std::once_flag once;
    std::call_once(once, []() {
        aclrtStream handle = nullptr;
        create_status = aclrtCreateStream(&handle);
        if (create_status != ACL_SUCCESS || handle == nullptr) {
            g_bridge_default_stream = nullptr;
            if (create_status == ACL_SUCCESS) {
                create_status = ACL_ERROR_RT_INTERNAL_ERROR;
            }
        } else {
            g_bridge_default_stream = handle;
            g_bridge_default_stream_ready = true;
        }
    });
    if (g_bridge_default_stream == nullptr) {
        return create_status;
    }
    *resolved = g_bridge_default_stream;
    return ACL_SUCCESS;
}

/* ---------------- bridge 初始化 ---------------- */

namespace cubridge::internal {

/* 107002 排查（历史 runtime context 排障）：初始化各步返回码全部打印，
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

    /* 对齐 vectorAdd 的调用顺序（历史 runtime context 排障）：
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

    /* 107002 根因修复（历史 runtime context 排障结论，2026-08-19 定案）：
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

aclError wcudaSetDeviceFlags(unsigned int flags) {
    /* CUDA scheduling/map-host flags have no direct Ascend runtime equivalent.
     * Keep the call source-compatible and let cudaDeviceProp::canMapHostMemory
     * report whether generic host mapping is available. */
    (void)flags;
    BRIDGE_INIT();
    WCUDA_RETURN(ACL_SUCCESS);
}

aclError wcudaGetDeviceFlags(unsigned int *flags) {
    if (flags == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* Ascend has no scheduling-flag state; report the default (spin/yield
     * autonomous scheduling, no map-host), consistent with SetDeviceFlags
     * being a compatibility no-op and canMapHostMemory reporting 0. */
    BRIDGE_INIT();
    *flags = 0;
    WCUDA_RETURN(ACL_SUCCESS);
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
    /* Generic host-memory mapping is not exposed by this bridge path. */
    prop->canMapHostMemory = 0;
    /* No overlapping copy engines are exposed through ACL (see struct comment). */
    prop->deviceOverlap = 0;
    prop->asyncEngineCount = 0;

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

aclError wcudaStreamCreateWithPriority(aclrtStream *pStream, unsigned int flags,
                                         int priority) {
    (void)flags;
    (void)priority;
    if (pStream != nullptr) {
        *pStream = nullptr;
    }
    /* CANN aclrtCreateStream has no CUDA priority parameter. Creating a
     * normal stream here would silently weaken ordering guarantees. */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaStreamGetFlags(aclrtStream stream, unsigned int *flags) {
    (void)stream;
    if (flags == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    *flags = 0; /* CUDA stream default flag; keep backend wrapper independent. */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaStreamGetPriority(aclrtStream stream, int *priority) {
    (void)stream;
    if (priority == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    *priority = 0;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaStreamGetDevice(aclrtStream stream, int *device) {
    aclrtStream resolved = nullptr;
    if (device == nullptr || wcuda_resolve_stream(stream, &resolved) != ACL_SUCCESS) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    (void)resolved;
    /* ACL streams are created in the current context; this is an adapted
     * query, not a claim that arbitrary foreign stream handles are portable. */
    WCUDA_RETURN(wcudaGetDevice(device));
}

aclError wcudaStreamGetId(aclrtStream stream, unsigned long long *streamId) {
    (void)stream;
    if (streamId == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    *streamId = 0;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaStreamAttachMemAsync(aclrtStream stream, void *devPtr, size_t length,
                                   unsigned int flags) {
    (void)stream;
    (void)devPtr;
    (void)length;
    (void)flags;
    /* This API controls CUDA managed-memory association; Ascend has no
     * stream-ordered managed-memory equivalent. */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
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
    aclrtStream resolved = nullptr;
    if (wcuda_resolve_stream(stream, &resolved) != ACL_SUCCESS) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtSynchronizeStream(resolved));
}

aclError wcudaStreamQuery(aclrtStream stream) {
    aclrtStream resolved = nullptr;
    if (wcuda_resolve_stream(stream, &resolved) != ACL_SUCCESS) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* aclrtStreamQuery(stream, aclrtStreamStatus *status)。
     * CUDA 语义：cudaStreamQuery 已完成返回 success，未完成返回 cudaErrorNotReady。
     * 昇腾用 status 输出判断：ACL_STREAM_STATUS_COMPLETE / NOT_READY。 */
    aclrtStreamStatus status = ACL_STREAM_STATUS_COMPLETE;
    aclError ret = aclrtStreamQuery(resolved, &status);
    if (ret != ACL_SUCCESS) {
        WCUDA_RETURN(ret);
    }
    if (status != ACL_STREAM_STATUS_COMPLETE) {
        WCUDA_RETURN(ACL_ERROR_RT_STREAM_NOT_COMPLETE);
    }
    WCUDA_RETURN(ACL_SUCCESS);
}

aclError wcudaStreamAddCallback(aclrtStream stream, wcceStreamCallback_t callback,
                                void *userData, unsigned int flags) {
    (void)stream;
    (void)callback;
    (void)userData;
    (void)flags;
    /* CANN callback registration/report processing is not CUDA callback ABI
     * compatible; do not silently schedule a callback with weaker ordering. */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaStreamWaitEvent(aclrtStream stream, aclrtEvent event, unsigned int flags) {
    (void)flags;
    aclrtStream resolved = nullptr;
    if (event == nullptr || wcuda_resolve_stream(stream, &resolved) != ACL_SUCCESS) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtStreamWaitEvent(resolved, event));
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
    /* CUDA 的 cudaEventRecord(event, 0) 用默认流。桥内用惰性创建的自有流
     * 表示三种 CUDA 默认流拼写（见 wcuda_resolve_stream 注释）。 */
    aclrtStream resolved = nullptr;
    if (wcuda_resolve_stream(stream, &resolved) != ACL_SUCCESS) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* Legacy 默认流语义（simpleStreams 议题定案，2026-08-28）：stream-0 上的
     * 工作与所有阻塞流隐式同步。同步点放在"消费时"（EventSynchronize/Query/
     * ElapsedTime）而非"记录时"——记录时全设备同步会破坏 asyncAPI 这类
     * 中途向默认流记事件的流水线（真机回归：data 25→0、spin 4800→33）。 */
    wcuda_event_mark_default(event, wcuda_is_bridge_default(resolved));
    WCUDA_RETURN(aclrtRecordEvent(event, resolved));
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
    /* legacy 默认流语义的消费点：默认流事件完成 ⇒ 全部流工作完成。 */
    if (wcuda_event_is_default(event)) {
        BRIDGE_INIT();
        aclError dev_ret = aclrtSynchronizeDevice();
        if (dev_ret != ACL_SUCCESS) {
            WCUDA_RETURN(dev_ret);
        }
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
    /* legacy 默认流语义的消费点：轮询到 COMPLETE 时，若事件记录在默认流上，
     * 必须确保全部流的工作（含创建流上在途的异步拷贝/内核）真正落地后才
     * 返回 success——否则 asyncAPI 式"query 忙等后读 host"会读到在途数据。 */
    if (wcuda_event_is_default(event)) {
        aclError dev_ret = aclrtSynchronizeDevice();
        if (dev_ret != ACL_SUCCESS) {
            WCUDA_RETURN(dev_ret);
        }
    }
    WCUDA_RETURN(ACL_SUCCESS);
}

aclError wcudaEventElapsedTime(float *ms, aclrtEvent start, aclrtEvent end) {
    if (ms == nullptr || start == nullptr || end == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    aclError ret = ACL_SUCCESS;
    /* simpleStreams 议题取证：跨流计时值是否可信（现场观察到 streamed 段
     * elapsed≈0 且数据大部分未生效）。默认流事件对 elapsed 时先同步设备，
     * 保证读取瞬间所有相关工作已落地。 */
    if (wcuda_event_is_default(start) || wcuda_event_is_default(end)) {
        ret = aclrtSynchronizeDevice();
        if (wcuda_dbg()) {
            fprintf(stderr, "[wcuda] wcudaEventElapsedTime default-sync ret=%d\n", (int)ret);
        }
        if (ret != ACL_SUCCESS) {
            return ret;
        }
    }
    ret = aclrtEventElapsedTime(ms, start, end);
    if (wcuda_dbg()) {
        fprintf(stderr, "[wcuda] wcudaEventElapsedTime ret=%d ms=%.6f start=%p end=%p\n",
                (int)ret, (double)*ms, (void *)start, (void *)end);
    }
    return ret;
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

aclError wcudaMemcpyDefault(void *dst, const void *src, size_t count) {
    (void)dst;
    (void)src;
    (void)count;
    /* CUDA infers H2H/H2D/D2H/D2D from pointer attributes. The public CANN
     * Runtime API does not expose an equivalent default-direction operation,
     * so guessing DEVICE_TO_DEVICE would corrupt host/device transfers. */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaMemcpyAsync(void *dst, const void *src, size_t count, aclrtMemcpyKind kind,
                          aclrtStream stream) {
    if (dst == nullptr || src == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    aclrtStream resolved = nullptr;
    if (wcuda_resolve_stream(stream, &resolved) != ACL_SUCCESS) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    aclError ret = aclrtMemcpyAsync(dst, count, src, count, kind, resolved);
    /* simpleStreams 议题取证：流水线里的每个异步拷贝都要留下去向。 */
    if (wcuda_dbg()) {
        fprintf(stderr, "[wcuda] wcudaMemcpyAsync ret=%d n=%zu kind=%d stream=%p dst=%p src=%p\n",
                (int)ret, count, (int)kind, (void *)resolved, dst, src);
    }
    return ret;
}

aclError wcudaMemcpy2D(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width,
                       size_t height, aclrtMemcpyKind kind) {
    if (dst == nullptr || src == nullptr || width == 0 || height == 0 || dpitch < width ||
        spitch < width) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* CANN documents aclrtMemcpy2d as a separate platform-dependent API and only
     * guarantees H2D/D2H on supported products. Implement CUDA's synchronous 2D
     * copy as row-wise aclrtMemcpy instead: this preserves CUDA pitch semantics
     * and also works on Ascend products where aclrtMemcpy2d is unavailable or
     * returns ACL_ERROR_API_NOT_SUPPORT. */
    char *dstBytes = (char *)dst;
    const char *srcBytes = (const char *)src;
    for (size_t row = 0; row < height; ++row) {
        aclError ret = aclrtMemcpy(dstBytes + row * dpitch, width, srcBytes + row * spitch, width, kind);
        if (ret != ACL_SUCCESS) {
            WCUDA_RETURN(ret);
        }
    }
    WCUDA_RETURN(ACL_SUCCESS);
}

aclError wcudaMemcpy2DAsync(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width,
                            size_t height, aclrtMemcpyKind kind, aclrtStream stream) {
    aclrtStream resolved = nullptr;
    if (dst == nullptr || src == nullptr || width == 0 || height == 0 || dpitch < width ||
        spitch < width || wcuda_resolve_stream(stream, &resolved) != ACL_SUCCESS) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    char *dstBytes = (char *)dst;
    const char *srcBytes = (const char *)src;
    for (size_t row = 0; row < height; ++row) {
        aclError ret = aclrtMemcpyAsync(dstBytes + row * dpitch, width, srcBytes + row * spitch,
                                        width, kind, resolved);
        if (ret != ACL_SUCCESS) {
            WCUDA_RETURN(ret);
        }
    }
    WCUDA_RETURN(ACL_SUCCESS);
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
    aclrtStream resolved = nullptr;
    if (devPtr == nullptr || wcuda_resolve_stream(stream, &resolved) != ACL_SUCCESS) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾无 aclrtMemset2dAsync，用逐行 aclrtMemsetAsync 模拟。 */
    char *row = (char *)devPtr;
    for (size_t y = 0; y < height; ++y) {
        aclError ret = aclrtMemsetAsync(row + y * pitch, width, value, width, resolved);
        if (ret != ACL_SUCCESS) {
            WCUDA_RETURN(ret);
        }
    }
    WCUDA_RETURN(ACL_SUCCESS);
}

aclError wcudaMalloc3D(wccePitchedPtr *pitchedDevPtr, wcceExtent extent) {
    if (pitchedDevPtr == nullptr || extent.width == 0 || extent.height == 0 ||
        extent.depth == 0) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* CANN has no CUDA 3D allocation object. A linear allocation with a
     * row-shaped descriptor is useful only as a source-compatibility bridge;
     * cudaMemcpy3D remains explicitly unsupported. */
    size_t row_pitch = extent.width;
    if (extent.height > SIZE_MAX / extent.depth) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    size_t rows = extent.height * extent.depth;
    if (row_pitch > SIZE_MAX / rows) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    void *memory = nullptr;
    aclError ret = wcudaMalloc(&memory, row_pitch * rows);
    if (ret == ACL_SUCCESS) {
        pitchedDevPtr->ptr = memory;
        pitchedDevPtr->pitch = row_pitch;
        pitchedDevPtr->xsize = extent.width;
        pitchedDevPtr->ysize = extent.height;
    }
    return ret;
}

aclError wcudaMemset3D(wccePitchedPtr pitchedDevPtr, int value, wcceExtent extent) {
    (void)pitchedDevPtr;
    (void)value;
    (void)extent;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaMemset3DAsync(wccePitchedPtr pitchedDevPtr, int value, wcceExtent extent,
                            aclrtStream stream) {
    (void)pitchedDevPtr;
    (void)value;
    (void)extent;
    (void)stream;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaMallocPitch(void **devPtr, size_t *pitch, size_t width, size_t height) {
    /* cudaMallocPitch can be the first CUDA API in a process. The underlying
     * aclrtMalloc requires a bound ACL context, so initialize before calling it
     * rather than relying on WCUDA_RETURN after the allocation attempt. */
    BRIDGE_INIT();
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

aclError wcudaMallocArray(wcceArray_t *array, const wcceChannelFormatDesc *desc, size_t width, size_t height,
                          unsigned int flags) {
    (void)array; (void)desc; (void)width; (void)height; (void)flags;
    return array_not_supported();
}

aclError wcudaMalloc3DArray(wcceArray_t *array, const wcceChannelFormatDesc *desc, wcceExtent extent, unsigned int flags) {
    (void)array; (void)desc; (void)extent; (void)flags;
    return array_not_supported();
}

aclError wcudaMallocMipmappedArray(wcceMipmappedArray_t *mipmappedArray, const wcceChannelFormatDesc *desc, wcceExtent extent,
                                   unsigned int numLevels, unsigned int flags) {
    (void)mipmappedArray; (void)desc; (void)extent; (void)numLevels; (void)flags;
    return array_not_supported();
}

aclError wcudaFreeArray(wcceArray_t array) {
    (void)array;
    return array_not_supported();
}

aclError wcudaFreeMipmappedArray(wcceMipmappedArray_t mipmappedArray) {
    (void)mipmappedArray;
    return array_not_supported();
}

aclError wcudaMemcpyToArray(wcceArray_t dst, size_t wOffset, size_t hOffset, const void *src,
                            size_t count, int kind) {
    (void)dst; (void)wOffset; (void)hOffset; (void)src; (void)count; (void)kind;
    return array_not_supported();
}

aclError wcudaMemcpyFromArray(void *dst, wcceArray_const_t src, size_t wOffset, size_t hOffset,
                              size_t count, int kind) {
    (void)dst; (void)src; (void)wOffset; (void)hOffset; (void)count; (void)kind;
    return array_not_supported();
}

aclError wcudaMemcpy2DToArray(wcceArray_t dst, size_t wOffset, size_t hOffset, const void *src,
                              size_t spitch, size_t width, size_t height, int kind) {
    (void)dst; (void)wOffset; (void)hOffset; (void)src; (void)spitch;
    (void)width; (void)height; (void)kind;
    return array_not_supported();
}

aclError wcudaMemcpy2DFromArray(void *dst, size_t dpitch, wcceArray_const_t src, size_t wOffset,
                                size_t hOffset, size_t width, size_t height, int kind) {
    (void)dst; (void)dpitch; (void)src; (void)wOffset; (void)hOffset;
    (void)width; (void)height; (void)kind;
    return array_not_supported();
}

aclError wcudaArrayGetInfo(wcceChannelFormatDesc *desc, wcceExtent *extent,
                           unsigned int *flags, wcceArray_const_t array) {
    (void)array;
    if (desc != nullptr) {
        *desc = wcceChannelFormatDesc{0, 0, 0, 0, wcceChannelFormatKindNone};
    }
    if (extent != nullptr) {
        *extent = wcceExtent{0, 0, 0};
    }
    if (flags != nullptr) {
        *flags = 0;
    }
    return array_not_supported();
}

aclError wcudaGetChannelDesc(wcceChannelFormatDesc *desc, wcceArray_const_t array) {
    (void)array;
    if (desc != nullptr) {
        *desc = wcceChannelFormatDesc{0, 0, 0, 0, wcceChannelFormatKindNone};
    }
    return array_not_supported();
}

/* ---- CUDA Runtime data-shape factories ---- */

wcceExtent make_wcudaExtent(size_t width, size_t height, size_t depth) {
    wcceExtent value = {width, height, depth};
    return value;
}

wccePitchedPtr make_wcudaPitchedPtr(void *ptr, size_t pitch, size_t xsize, size_t ysize) {
    wccePitchedPtr value = {ptr, pitch, xsize, ysize};
    return value;
}

wccePos make_wcudaPos(size_t x, size_t y, size_t z) {
    wccePos value = {x, y, z};
    return value;
}

/* ---- Texture/Surface（昇腾无对应，全部 not-support） ---- */

wcceChannelFormatDesc wcudaCreateChannelDesc(int x, int y, int z, int w) {
    /* Keep the CUDA data shape available to callers; Ascend has no texture
     * channel execution path, so the format kind is intentionally None. */
    wcceChannelFormatDesc value = {x, y, z, w, wcceChannelFormatKindNone};
    return value;
}

aclError wcudaCreateTextureObject(wcceTextureObject_t *pTexObject, const wcceResourceDesc *pResDesc,
                                  const wcceTextureDesc *pTexDesc,
                                  const wcceResourceViewDesc *pResViewDesc) {
    (void)pResDesc; (void)pTexDesc; (void)pResViewDesc;
    if (pTexObject != nullptr) {
        *pTexObject = 0;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaDestroyTextureObject(wcceTextureObject_t texObject) {
    (void)texObject;
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaCreateSurfaceObject(wcceSurfaceObject_t *pSurfObject, const wcceResourceDesc *pResDesc) {
    (void)pResDesc;
    if (pSurfObject != nullptr) {
        *pSurfObject = 0;
    }
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaDestroySurfaceObject(wcceSurfaceObject_t surfObject) {
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

aclError wcudaMemcpy3D(const wcceMemcpy3DParms *p) {
    (void)p;
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaMemcpy3DAsync(const wcceMemcpy3DParms *p, aclrtStream stream) {
    (void)p;
    (void)stream;
    return ACL_ERROR_API_NOT_SUPPORT;
}

aclError wcudaMemcpy3DPeer(const wcceMemcpy3DPeerParms *p) {
    (void)p;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaMemcpy3DPeerAsync(const wcceMemcpy3DPeerParms *p, aclrtStream stream) {
    (void)p;
    (void)stream;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
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
    aclrtStream resolved = nullptr;
    if (dst == nullptr || src == nullptr ||
        wcuda_resolve_stream(stream, &resolved) != ACL_SUCCESS) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtMemcpyAsync(dst, count, src, count, ACL_MEMCPY_DEVICE_TO_DEVICE, resolved));
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

aclError wcudaMemPoolCreate(wcceMemPool_t *memPool,
                              const wcceMemPoolProps *poolProps) {
    (void)memPool;
    (void)poolProps;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaMemPoolDestroy(wcceMemPool_t memPool) {
    (void)memPool;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaDeviceGetMemPool(wcceMemPool_t *memPool, int device) {
    (void)memPool;
    (void)device;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaDeviceSetMemPool(int device, wcceMemPool_t memPool) {
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

aclError wcudaPointerGetAttributes(wccePointerAttributes *attributes, const void *ptr) {
    (void)attributes;
    (void)ptr;
    /* 昇腾无 cudaPointerGetAttributes 对应（需要 cudaPointerAttributes 结构）。
     * 简化返回 not-support；若应用依赖指针属性查询需真机实现。 */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaMemsetAsync(void *devPtr, int value, size_t count, aclrtStream stream) {
    aclrtStream resolved = nullptr;
    if (devPtr == nullptr || wcuda_resolve_stream(stream, &resolved) != ACL_SUCCESS) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    WCUDA_RETURN(aclrtMemsetAsync(devPtr, count, value, count, resolved));
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
    /* CUDA 契约：无状态、按错误码返回 cudaErrorXxx 风格名字（与
     * GetErrorString 的"最近一次错误文本"语义不同）。本表覆盖桥内会产生
     * 的全部 ACL 错误码；未列出的码回落 "unknown"。 */
    static const struct {
        aclError code;
        const char *name;
    } k_names[] = {
        {ACL_SUCCESS, "cudaSuccess"},
        {ACL_ERROR_INVALID_PARAM, "cudaErrorInvalidValue"},
        {ACL_ERROR_RT_PARAM_INVALID, "cudaErrorInvalidValue"},
        {ACL_ERROR_UNINITIALIZE, "cudaErrorInitializationError"},
        {ACL_ERROR_RT_MEMORY_ALLOCATION, "cudaErrorMemoryAllocation"},
        {ACL_ERROR_BAD_ALLOC, "cudaErrorMemoryAllocation"},
        {ACL_ERROR_RT_INTERNAL_ERROR, "cudaErrorUnknown"},
        {ACL_ERROR_FAILURE, "cudaErrorUnknown"},
        {ACL_ERROR_RT_EVENT_NOT_COMPLETE, "cudaErrorNotReady"},
        {ACL_ERROR_RT_STREAM_NOT_COMPLETE, "cudaErrorNotReady"},
        {ACL_ERROR_API_NOT_SUPPORT, "cudaErrorNotSupported"},
        {ACL_ERROR_FEATURE_UNSUPPORTED, "cudaErrorNotSupported"},
        {ACL_ERROR_INVALID_DEVICE, "cudaErrorInvalidDevice"},
        {ACL_ERROR_RT_NO_DEVICE, "cudaErrorNoDevice"},
        {ACL_ERROR_RT_NO_STREAM_RESOURCE, "cudaErrorStreamCreationFailed"},
        {ACL_ERROR_RT_AICORE_EXCEPTION, "cudaErrorKernelExecutionFailed"},
    };
    for (const auto &entry : k_names) {
        if (entry.code == error) {
            return entry.name;
        }
    }
    static const char *unknown = "unknown";
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

aclError wcudaLaunchCooperativeKernel(const void *hostFunction, dim3 gridDim, dim3 blockDim,
                                      void **args, size_t sharedMemBytes, aclrtStream stream) {
    (void)hostFunction;
    (void)gridDim;
    (void)blockDim;
    (void)args;
    (void)sharedMemBytes;
    (void)stream;
    /* CANN SIMT launch does not expose CUDA cooperative-grid synchronization
     * semantics. A normal launch would be a semantic downgrade. */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaLaunchKernel(const void *hostFunction, dim3 gridDim, dim3 blockDim, void **args,
                           size_t sharedMemBytes, aclrtStream stream) {
    if (hostFunction == nullptr) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* 昇腾 aclrtLaunchSIMTKernelWithArgsArray(func, gridDim, blockDim, dynUbufSize, stream, cfg, args)
     * 官方文档确认：Ascend 950PR/950DT 支持（A2/A3/910B 不支持）。
     * cfg 传 nullptr（默认配置）。dynUbufSize = CUDA 的 sharedMemBytes。 */
    aclError ret = aclrtLaunchSIMTKernelWithArgsArray((void *)hostFunction, gridDim, blockDim,
                                                      sharedMemBytes, stream, nullptr, args);
    /* simpleStreams 议题取证：多流流水线的每次 launch 都要留下 return 码
     * 与网格形状（现场现象：流水线工作大面积未生效但返回全 0）。 */
    if (wcuda_dbg()) {
        fprintf(stderr,
                "[wcuda] wcudaLaunchKernel ret=%d grid=(%u,%u,%u) block=(%u,%u,%u) stream=%p "
                "func=%p args=%p\n",
                (int)ret, gridDim.x, gridDim.y, gridDim.z, blockDim.x, blockDim.y, blockDim.z,
                (void *)stream, (void *)hostFunction, (void *)args);
    }
    return ret;
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

aclError wcudaThreadGetLimit(size_t *pValue, int limit) {
    return wcudaDeviceGetLimit(pValue, limit);
}

aclError wcudaThreadSetLimit(int limit, size_t value) {
    return wcudaDeviceSetLimit(limit, value);
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
    aclrtStream resolved = nullptr;
    if (fn == nullptr || wcuda_resolve_stream(stream, &resolved) != ACL_SUCCESS) {
        WCUDA_RETURN(ACL_ERROR_INVALID_PARAM);
    }
    /* blockType 用 ACL_CALLBACK_NO_BLOCK（不阻塞 stream 后续任务，语义接近 cudaLaunchHostFunc）。
     * ⚠️ 待真机：需先 aclrtSubscribeReport 注册回调线程，否则回调可能不触发。 */
    WCUDA_RETURN(aclrtLaunchCallback(fn, userData, ACL_CALLBACK_NO_BLOCK, resolved));
}

aclError wcudaFuncGetAttributes(wcceFuncAttributes *attr, const void *func) {
    (void)attr;
    (void)func;
    /* 昇腾无 kernel 属性查询接口，返回 not-support。 */
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaFuncSetCacheConfig(const void *func, wcceFuncCache cacheConfig) {
    (void)func;
    (void)cacheConfig;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaFuncSetSharedMemConfig(const void *func, wcceSharedMemConfig config) {
    (void)func;
    (void)config;
    WCUDA_RETURN(ACL_ERROR_API_NOT_SUPPORT);
}

aclError wcudaThreadSetCacheConfig(int cacheConfig) {
    (void)cacheConfig;
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
