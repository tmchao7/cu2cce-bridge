/*
 * vectorAdd.cu —— puresimt 形态的 vectorAdd（CUDA 原生写法，零改写）。
 *
 * 2026-08-21 导师方向：毕昇前端复用 CUDA 前端，`__global__` 与 CUDA 一致、
 * `__aicore__` 等价 `__device__`；除 `<<<>>>` 调用接口与新同构有差异外，
 * 其他 acl 接口一致。puresimt 验证成立后（examples/puresimt/puresimt_probe.asc：
 * -x asc --enable-simt --cce-aicore-arch=dav-c310-vec + ASCEND_HOME_PATH），
 * 本示例作为 puresimt 正式路线的最小样例：纯 CUDA kernel + host 侧 aclrt*。
 *
 * 编译（真机，见 server_cmd）：
 *   export ASCEND_HOME_PATH=/usr/local/Ascend/latest
 *   ccec -x asc --cce-aicore-arch=dav-c310-vec --enable-simt -std=c++17 \
 *     -I/usr/local/Ascend/latest/include -L/usr/local/Ascend/latest/lib64 \
 *     -lascendc_runtime -lascendcl -lruntime -lregister -lerror_manager \
 *     -lprofapi -lascendalog -lmmpa -lascend_dump -lc_sec -lstdc++ \
 *     examples/puresimt/pvectoradd.cu -o pvectoradd
 * 注意：__global__ kernel 使用 CUDA 线程索引（threadIdx/blockIdx/blockDim/gridDim），
 *   <<<blocks, threads, 0, stream>>> 是原生 CUDA 写法（puresimt 下可用）。
 */
#include "acl/acl.h"
#include <stdio.h>

#define N 1024
#define THREADS 256

__global__ void vectorAddKernel(float *c, const float *a, const float *b, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        c[i] = a[i] + b[i];
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

    float *h_a = nullptr, *h_b = nullptr, *h_c = nullptr;
    aclrtMallocHost((void **)&h_a, byteSize);
    aclrtMallocHost((void **)&h_b, byteSize);
    aclrtMallocHost((void **)&h_c, byteSize);
    for (int i = 0; i < N; i++) {
        h_a[i] = (float)i;
        h_b[i] = 2.0f;
    }

    float *d_a = nullptr, *d_b = nullptr, *d_c = nullptr;
    aclrtMalloc((void **)&d_a, byteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&d_b, byteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&d_c, byteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMemcpy(d_a, byteSize, h_a, byteSize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(d_b, byteSize, h_b, byteSize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(d_c, byteSize, h_c, byteSize, ACL_MEMCPY_HOST_TO_DEVICE);

    int blocks = (N + THREADS - 1) / THREADS;
    vectorAddKernel<<<blocks, THREADS, 0, stream>>>(d_c, d_a, d_b, N);
    aclrtSynchronizeStream(stream);
    aclrtMemcpy(h_c, byteSize, d_c, byteSize, ACL_MEMCPY_DEVICE_TO_HOST);

    int bad = 0;
    for (int i = 0; i < N; i++) {
        if (h_c[i] != (float)i + 2.0f) {
            if (bad < 4) printf("c[%d]=%.1f expected=%.1f\n", i, h_c[i], (float)i + 2.0f);
            bad++;
        }
    }
    if (bad != 0) {
        printf("vectorAdd puresimt FAILED (%d mismatch)\n", bad);
        return 1;
    }
    printf("vectorAdd puresimt OK (N=%d)\n", N);

    aclrtFree(d_a);
    aclrtFree(d_b);
    aclrtFree(d_c);
    aclrtFreeHost(h_a);
    aclrtFreeHost(h_b);
    aclrtFreeHost(h_c);
    aclrtDestroyStream(stream);
    aclrtDestroyContext(context);
    aclrtResetDevice(deviceId);
    aclFinalize();
    return 0;
}