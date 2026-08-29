#ifndef __CU2CCE_NVTX3_NVTIMER_H__
#define __CU2CCE_NVTX3_NVTIMER_H__

/*
 * NVTX v3 compatibility entry (cu-bridge ships the full NVIDIA nvtx3 header
 * set + a binary shim; NPU adaptation: this entry forwards to our no-op
 * stub because Ascend profiling annotations use aclprof, a different API
 * shape driven by the profiler, not in-source NVTX ranges).
 *
 * Include as <nvtx3/nvToolsExt.h> exactly like upstream projects do.
 * A project that ships its own real NVTX v3 header earlier on the include
 * path still wins (distinct guard names).
 */

#include "../bridge/tools_ext/nvtx_stub.h"

#endif /* __CU2CCE_NVTX3_NVTIMER_H__ */
