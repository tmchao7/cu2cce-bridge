/*
 * preduction.cu —— puresimt 形态的块内 reduction（SIMT 原语真机验证）。
 *
 * 2026-08-21 puresimt 主线：probe/pvectoradd/pminimal 已真机全绿，但全部是
 * elementwise 模式。本样例首次在 puresimt（纯 CUDA 原生写法，零改写）下验证
 * 非 elementwise 的块内协作原语：`__shared__` 静态共享数组 + `__syncthreads()`
 * 树形块内归约 + `atomicAdd` 跨块累加。三个原语与 CUDA 语义一致时（导师：
 * `__global__` 与 CUDA 一致、`__aicore__`≡`__device__`），真实 reduction 类
 * kernel 即可零改写移植。
 *
 * 算法：N=1024 个 int（值 0..N-1），4 块 × 256 线程。
 *   1. 每线程把全局元素写入 __shared__ sdata[threadIdx.x]（越界置 0 防御）；
 *   2. __syncthreads() 两段树形归约，得到块内和 sdata[0]；
 *   3. tid==0 用 atomicAdd 把块内和累加到全局 result[0]（跨块串行安全）。
 * 注意：不提前 return（__syncthreads 需要整块线程都到，否则死锁）。
 * 校验：sum == 1024*1023/2 == 523776。
 *
 * 编译（真机，见 server_cmd）：
 *   export ASCEND_HOME_PATH=/usr/local/Ascend/latest
 *   ccec -x asc --cce-aicore-arch=dav-c310-vec --enable-simt -std=c++17 \
 *     -I/usr/local/Ascend/latest/include -L/usr/local/Ascend/latest/lib64 \
 *     -lascendc_runtime -lascendcl -lruntime -lregister -lerror_manager \
 *     -lprofapi -lascendalog -lmmpa -lascend_dump -lc_sec -lstdc++ \
 *     examples/puresimt/preduction.cu -o preduction
 */
#include "acl/acl.h"
#include <stdio.h>

#define N 1024
#define BLOCKS 4
#define THREADS 256

__global__ void reductionKernel(int *result, const int *data, int n) {
    __shared__ int sdata[THREADS];
    int tid = threadIdx.x;
    int i = blockIdx.x * blockDim.x + tid;

    sdata[tid] = (i < n) ? data[i] : 0;
    __syncthreads();

    for (int stride = THREADS / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            sdata[tid] += sdata[tid + stride];
        }
        __syncthreads();
    }

    if (tid == 0) {
        atomicAdd(&result[0], sdata[0]);
    }
}

int main(void) {
    size_t byteSize = N * sizeof(int);

    aclInit(nullptr);
    aclrtContext context = nullptr;
    int deviceId = 0;
    aclrtSetDevice(deviceId);
    aclrtCreateContext(&context, deviceId);
    aclrtStream stream = nullptr;
    aclrtCreateStream(&stream);

    int *h_data = nullptr, *h_result = nullptr;
    aclrtMallocHost((void **)&h_data, byteSize);
    aclrtMallocHost((void **)&h_result, sizeof(int));
    for (int i = 0; i < N; i++) h_data[i] = i;
    h_result[0] = 0;

    int *d_data = nullptr, *d_result = nullptr;
    aclrtMalloc((void **)&d_data, byteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&d_result, sizeof(int), ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMemcpy(d_data, byteSize, h_data, byteSize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(d_result, sizeof(int), h_result, sizeof(int),
                ACL_MEMCPY_HOST_TO_DEVICE);

    reductionKernel<<<BLOCKS, THREADS, 0, stream>>>(d_result, d_data, N);
    aclrtSynchronizeStream(stream);
    aclrtMemcpy(h_result, sizeof(int), d_result, sizeof(int),
                ACL_MEMCPY_DEVICE_TO_HOST);

    int expected = N * (N - 1) / 2;
    if (h_result[0] != expected) {
        printf("preduction puresimt FAILED (sum=%d expected=%d)\n",
               h_result[0], expected);
        aclrtFree(d_data);
        aclrtFree(d_result);
        aclrtFreeHost(h_data);
        aclrtFreeHost(h_result);
        aclrtDestroyStream(stream);
        aclrtDestroyContext(context);
        aclrtResetDevice(deviceId);
        aclFinalize();
        return 1;
    }
    printf("preduction puresimt OK (N=%d, sum=%d)\n", N, h_result[0]);

    aclrtFree(d_data);
    aclrtFree(d_result);
    aclrtFreeHost(h_data);
    aclrtFreeHost(h_result);
    aclrtDestroyStream(stream);
    aclrtDestroyContext(context);
    aclrtResetDevice(deviceId);
    aclFinalize();
    return 0;
}