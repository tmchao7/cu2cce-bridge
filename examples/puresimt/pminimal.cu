/*
 * minimal.cu —— puresimt 形态的首调 cudaMalloc 最小样例（aclrt 版本）。
 *
 * 对照 examples/minimal/minimal.cu（新同构 B 主线）+ 107002 防复退：验证
 * puresimt 链路下 aclrtMalloc 首调用（aclInit→SetDevice→CreateContext→
 * CreateStream→aclrtMalloc→aclrtMemcpy→aclrtMemset→launch→Synchronize→
 * D2H→校验）整条 host 侧与 acl 接口一致（导师：除 <<<>>> 与新同构差异外
 * 其他 acl 接口一样）。
 */
#include "acl/acl.h"
#include <stdio.h>

#define N 256
#define THREADS 256

__global__ void addOneKernel(float *x, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        x[i] += 1.0f;
    }
}

int main(void) {
    size_t byteSize = N * sizeof(float);
    aclInit(nullptr);
    aclrtContext context = nullptr;
    int deviceId = 0;
    aclrtSetDevice(deviceId);
    aclrtCreateContext(&context, deviceId);
    aclrtStream stream = nullptr;
    aclrtCreateStream(&stream);

    float *h = nullptr;
    aclrtMallocHost((void **)&h, byteSize);
    for (int i = 0; i < N; i++) h[i] = (float)i;

    float *d = nullptr;
    aclError err = aclrtMalloc((void **)&d, byteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    if (err != ACL_SUCCESS) {
        printf("aclrtMalloc failed: %d\n", (int)err);
        return 1;
    }
    aclrtMemcpy(d, byteSize, h, byteSize, ACL_MEMCPY_HOST_TO_DEVICE);

    addOneKernel<<<1, THREADS, 0, stream>>>(d, N);
    aclrtSynchronizeStream(stream);
    aclrtMemcpy(h, byteSize, d, byteSize, ACL_MEMCPY_DEVICE_TO_HOST);

    int bad = 0;
    for (int i = 0; i < N; i++) {
        if (h[i] != (float)i + 1.0f) {
            if (bad < 4) printf("x[%d]=%.1f expected=%.1f\n", i, h[i], (float)i + 1.0f);
            bad++;
        }
    }
    if (bad != 0) {
        printf("minimal puresimt FAILED (%d mismatch)\n", bad);
        return 1;
    }
    printf("minimal puresimt OK (N=%d)\n", N);

    aclrtFree(d);
    aclrtFreeHost(h);
    aclrtDestroyStream(stream);
    aclrtDestroyContext(context);
    aclrtResetDevice(deviceId);
    aclFinalize();
    return 0;
}