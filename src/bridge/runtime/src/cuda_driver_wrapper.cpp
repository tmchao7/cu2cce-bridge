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

extern "C" CUresult wcuCtxGetCurrent(CUcontext *context) {
    if (context != nullptr) {
        *context = nullptr;
    }
    return kUnsupported;
}

extern "C" CUresult wcuCtxSetCurrent(CUcontext context) {
    (void)context;
    return kUnsupported;
}

extern "C" CUresult wcuCtxSynchronize(void) {
    return kUnsupported;
}

extern "C" CUresult wcuModuleLoadData(CUmodule *module, const void *image) {
    (void)image;
    if (module != nullptr) {
        *module = nullptr;
    }
    return kUnsupported;
}

extern "C" CUresult wcuModuleUnload(CUmodule module) {
    (void)module;
    return kUnsupported;
}

extern "C" CUresult wcuModuleGetFunction(CUfunction *function, CUmodule module, const char *name) {
    (void)module;
    (void)name;
    if (function != nullptr) {
        *function = nullptr;
    }
    return kUnsupported;
}

extern "C" CUresult wcuLaunchKernel(CUfunction function, unsigned int gridDimX,
                                       unsigned int gridDimY, unsigned int gridDimZ,
                                       unsigned int blockDimX, unsigned int blockDimY,
                                       unsigned int blockDimZ, unsigned int sharedMemBytes,
                                       CUstream stream, void **kernelParams, void **extra) {
    (void)function;
    (void)gridDimX;
    (void)gridDimY;
    (void)gridDimZ;
    (void)blockDimX;
    (void)blockDimY;
    (void)blockDimZ;
    (void)sharedMemBytes;
    (void)stream;
    (void)kernelParams;
    (void)extra;
    return kUnsupported;
}
