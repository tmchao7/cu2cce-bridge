#ifndef __CU2CCE_BRIDGE_TOOLS_EXT_NVTX_STUB_H__
#define __CU2CCE_BRIDGE_TOOLS_EXT_NVTX_STUB_H__

/*
 * Minimal NVTX compatibility shim (cu-bridge parity: libToolsExt_cu +
 * nvtx3/nvToolsExt.h; NPU adaptation: annotation calls are no-ops because
 * Ascend's profiling annotation surface, aclprof, has a different API shape
 * and is normally driven through the profiler rather than in-source NVTX).
 *
 * Contract: every entry point is a real symbol-free inline that succeeds, so
 * performance-instrumented projects compile and run unchanged while range
 * annotations are simply not recorded. No fabricated profiling data.
 *
 * Covered surface (the subset real projects use most):
 *   nvtxRangePushA / nvtxRangePop / nvtxMarkA /
 *   nvtxRangeStartA / nvtxRangeEnd / nvtxRangePushEx(nvtxEventAttributes_t*)
 */

#include <cstddef>

typedef unsigned long long nvtxRangeId_t;

struct nvtxEventAttributes_v2;
typedef struct nvtxEventAttributes_v2 nvtxEventAttributes_t;

#ifdef __cplusplus
extern "C" {
#endif

static inline int wnvtxRangePushA(const char *message) {
    (void)message;
    return 0;
}

static inline int wnvtxRangePushEx(const nvtxEventAttributes_t *eventAttrib) {
    (void)eventAttrib;
    return 0;
}

static inline int wnvtxRangePop(void) { return 0; }

static inline void wnvtxMarkA(const char *message) { (void)message; }

static inline nvtxRangeId_t wnvtxRangeStartA(const char *message) {
    (void)message;
    return 0;
}

static inline void wnvtxRangeEnd(nvtxRangeId_t id) { (void)id; }

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NVIDIA spelling -> shim (mirrors cu-bridge's nvtx_to_mctx_adaptor.h). */
#define nvtxRangePushA(message) wnvtxRangePushA(message)
#define nvtxRangePushEx(eventAttrib) wnvtxRangePushEx(eventAttrib)
#define nvtxRangePop() wnvtxRangePop()
#define nvtxMarkA(message) wnvtxMarkA(message)
#define nvtxRangeStartA(message) wnvtxRangeStartA(message)
#define nvtxRangeEnd(id) wnvtxRangeEnd(id)

#endif /* __CU2CCE_BRIDGE_TOOLS_EXT_NVTX_STUB_H__ */
