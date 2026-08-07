#ifndef ACL_RT_MOCK
#define ACL_RT_MOCK
#include <stdint.h>
#include <stddef.h>
/* 模拟 aclrt* 运行时接口签名（真机以 CANN include 为准） */
typedef void *aclrtStream;
typedef void *aclrtEvent;
typedef void *aclrtContext;
#ifdef __cplusplus
extern "C" {
#endif
int aclInit(const char *config);
int aclFinalize(void);
const char *aclGetRecentErrMsg(void);
int aclrtGetDeviceCount(uint32_t *count);
int aclrtSetDevice(int32_t deviceId);
int aclrtResetDevice(int32_t deviceId);
int aclrtGetDevice(int32_t *deviceId);
int aclrtSynchronizeDevice(void);
const char *aclrtGetSocName(void);
int aclrtGetMemInfo(int attr, size_t *free, size_t *total);
int aclrtCreateStream(aclrtStream *stream);
int aclrtDestroyStream(aclrtStream stream);
int aclrtSynchronizeStream(aclrtStream stream);
typedef enum { ACL_STREAM_STATUS_COMPLETE=0, ACL_STREAM_STATUS_NOT_READY=1 } aclrtStreamStatus;
int aclrtStreamQuery(aclrtStream stream, aclrtStreamStatus *status);
int aclrtStreamWaitEvent(aclrtStream stream, aclrtEvent event);
int aclrtCreateEvent(aclrtEvent *event);
int aclrtDestroyEvent(aclrtEvent event);
int aclrtRecordEvent(aclrtEvent event, aclrtStream stream);
int aclrtSynchronizeEvent(aclrtEvent event);
int aclrtQueryEvent(aclrtEvent event, aclrtEventStatus *status);
int aclrtQueryEventStatus(aclrtEvent event, aclrtEventRecordedStatus *status);
int aclrtEventElapsedTime(float *ms, aclrtEvent start, aclrtEvent end);
int aclrtMalloc(void **devPtr, size_t size, int policy);
int aclrtFree(void *devPtr);
int aclrtMallocHost(void **hostPtr, size_t size);
int aclrtFreeHost(void *hostPtr);
int aclrtMemcpy(void *dst, size_t destMax, const void *src, size_t count, int kind);
int aclrtMemcpyAsync(void *dst, size_t destMax, const void *src, size_t count, int kind, aclrtStream stream);
int aclrtMemcpy2d(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width, size_t height, int kind);
int aclrtMemcpy2dAsync(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width, size_t height, int kind, aclrtStream stream);
int aclrtMemset(void *devPtr, size_t maxCount, int32_t value, size_t count);
int aclrtMemsetAsync(void *devPtr, size_t maxCount, int32_t value, size_t count, aclrtStream stream);
int aclrtCreateContext(aclrtContext *context, int32_t deviceId);
int aclrtDestroyContext(aclrtContext context);
int aclrtSetCurrentContext(aclrtContext context);
int aclrtGetCurrentContext(aclrtContext *context);
int aclrtDeviceCanAccessPeer(int32_t *canAccessPeer, int32_t deviceId, int32_t peerDeviceId);
int aclrtDeviceEnablePeerAccess(int32_t peerDeviceId, uint32_t flags);
int aclrtDeviceDisablePeerAccess(int32_t peerDeviceId);
typedef void (*aclrtCallback)(void *userData);
typedef enum { ACL_CALLBACK_NO_BLOCK=0, ACL_CALLBACK_BLOCK=1 } aclrtCallbackBlockType;
int aclrtLaunchCallback(aclrtCallback fn, void *userData, aclrtCallbackBlockType blockType, aclrtStream stream);
struct dim3 { unsigned int x, y, z; dim3(unsigned int _x=1,unsigned int _y=1,unsigned int _z=1):x(_x),y(_y),z(_z){} };
typedef void *aclrtLaunchKernelCfg;
int aclrtLaunchSIMTKernelWithArgsArray(void *func, dim3 gridDim, dim3 blockDim, size_t dynUbufSize, aclrtStream stream, aclrtLaunchKernelCfg *cfg, void **args);
int aclrtGetVersion(int32_t *version);
#ifdef __cplusplus
}
#endif
#endif
