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
int aclrtStreamWaitEvent(aclrtStream stream, aclrtEvent event);
int aclrtCreateEvent(aclrtEvent *event);
int aclrtDestroyEvent(aclrtEvent event);
int aclrtRecordEvent(aclrtEvent event, aclrtStream stream);
int aclrtSynchronizeEvent(aclrtEvent event);
int aclrtQueryEvent(aclrtEvent event, aclrtEventStatus *status);
int aclrtEventElapsedTime(float *ms, aclrtEvent start, aclrtEvent end);
int aclrtMalloc(void **devPtr, size_t size, int policy);
int aclrtFree(void *devPtr);
int aclrtMallocHost(void **hostPtr, size_t size);
int aclrtFreeHost(void *hostPtr);
int aclrtMemcpy(void *dst, size_t destMax, const void *src, size_t count, int kind);
int aclrtMemcpyAsync(void *dst, size_t destMax, const void *src, size_t count, int kind, aclrtStream stream);
int aclrtMemset(void *devPtr, size_t maxCount, int32_t value, size_t count);
int aclrtMemsetAsync(void *devPtr, size_t maxCount, int32_t value, size_t count, aclrtStream stream);
int aclrtCreateContext(aclrtContext *context, int32_t deviceId);
int aclrtDestroyContext(aclrtContext context);
int aclrtSetCurrentContext(aclrtContext context);
int aclrtGetCurrentContext(aclrtContext *context);
int aclrtDeviceCanAccessPeer(int32_t *canAccessPeer, int32_t deviceId, int32_t peerDeviceId);
int aclrtDeviceEnablePeerAccess(int32_t peerDeviceId, uint32_t flags);
int aclrtDeviceDisablePeerAccess(int32_t peerDeviceId);
int aclrtGetVersion(int32_t *version);
#ifdef __cplusplus
}
#endif
#endif
