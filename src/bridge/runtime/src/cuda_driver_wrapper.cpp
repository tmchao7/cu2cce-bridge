#include "cuda_driver_wrapper.h"

namespace {
constexpr CUresult kUnsupported = static_cast<CUresult>(ACL_ERROR_API_NOT_SUPPORT);
}

extern "C" CUresult wcuInit(unsigned int flags) {
    (void)flags;
    return kUnsupported;
}

extern "C" CUresult wcuDriverGetVersion(int *driverVersion) {
    if (driverVersion != nullptr) {
        *driverVersion = 0;
    }
    return kUnsupported;
}

extern "C" CUresult wcuDeviceGetCount(int *count) {
    if (count != nullptr) {
        *count = 0;
    }
    return kUnsupported;
}

extern "C" CUresult wcuDeviceGet(CUdevice *device, int ordinal) {
    (void)ordinal;
    if (device != nullptr) {
        *device = 0;
    }
    return kUnsupported;
}
