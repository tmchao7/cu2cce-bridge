/*
 * vectorAdd.cu —— puresimt 形态的 vectorAdd（CUDA 原生写法，零改写）。
 *
 * 2026-08-21 导师方向：毕昇前端复用 CUDA 前端，`__global__` 与 CUDA 一致、
 * `__aicore__` 等价 `__device__`；除 `<<<>>>` 调用接口与新同构有差异外，
 * 其他 acl 接口一致。puresimt 验证成立后（examples/puresimt/puresimt_probe.asc：
 * -x asc --enable-simt --cce-aicore-arch=dav-c310-vec + ASCEND_HOME_PATH），
 * 本示例作为 puresimt 正式路线的最小样例：纯 CUDA kernel + host 侧 aclrt*。
 *
 * host 侧（2026-08-21 方向 A 修正）：改为全部走 bridge 宏/wrapper（cuda* →
 * wcuda* → aclrt*），include <cuda_runtime.h> 由 repo 的 include/cuda_runtime.h
 * 接住，用户源码零 acl 依赖。首调用换成 cudaStreamCreate：bridge_init 在
 * wcudaStreamCreate 内完成 init（aclInit→GetDeviceCount→SetDevice→CreateContext），
 * 首 API 即 cudaStreamCreate 验证初始化路径。
 *
 * 编译（真机，见 server_cmd）：
 *   export ASCEND_HOME_PATH=/usr/local/Ascend/latest
 *   ccec -x asc --cce-aicore-arch=dav-c310-vec --enable-simt -std=c++17 \
 *     -I<repo>/include -I/usr/local/Ascend/latest/include \
 *     -L/usr/local/Ascend/latest/lib64 \
 *     -lascendc_runtime -lascendcl -lruntime -lregister -lerror_manager \
 *     -lprofapi -lascendalog -lmmpa -lascend_dump -lc_sec -lstdc++ \
 *     examples/puresimt/pvectoradd.cu -o pvectoradd
 * 注意：__global__ kernel 使用 CUDA 线程索引（threadIdx/blockIdx/blockDim/gridDim），
 *   <<<blocks, threads, 0, stream>>> 是原生 CUDA 写法（puresimt 下可用）。
 */
#include "cuda_runtime.h"
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

    cudaStream_t stream = nullptr;
    cudaError_t err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) {
        printf("cudaStreamCreate failed: %d\n", (int)err);
        return 1;
    }

    float *h_a = nullptr, *h_b = nullptr, *h_c = nullptr;
    err = cudaMallocHost((void **)&h_a, byteSize);
    if (err != cudaSuccess) {
        printf("cudaMallocHost failed: %d\n", (int)err);
        return 1;
    }
    cudaMallocHost((void **)&h_b, byteSize);
    cudaMallocHost((void **)&h_c, byteSize);
    for (int i = 0; i < N; i++) {
        h_a[i] = (float)i;
        h_b[i] = 2.0f;
    }

    float *d_a = nullptr, *d_b = nullptr, *d_c = nullptr;
    err = cudaMalloc((void **)&d_a, byteSize);
    if (err != cudaSuccess) {
        printf("cudaMalloc failed: %d\n", (int)err);
        return 1;
    }
    cudaMalloc((void **)&d_b, byteSize);
    cudaMalloc((void **)&d_c, byteSize);
    cudaMemcpy(d_a, h_a, byteSize, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, h_b, byteSize, cudaMemcpyHostToDevice);
    cudaMemcpy(d_c, h_c, byteSize, cudaMemcpyHostToDevice);

    int blocks = (N + THREADS - 1) / THREADS;
    cudaLaunchKernel((const void *)vectorAddKernel, dim3(blocks), dim3(THREADS),
                     nullptr, 0, stream);
    cudaStreamSynchronize(stream);
    cudaMemcpy(h_c, d_c, byteSize, cudaMemcpyDeviceToHost);

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

    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
    cudaFreeHost(h_a);
    cudaFreeHost(h_b);
    cudaFreeHost(h_c);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    return 0;
}