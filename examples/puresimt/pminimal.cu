/*
 * minimal.cu —— puresimt 形态的首调 cudaMalloc 最小样例（cuda* 版本）。
 *
 * 对照 examples/minimal/minimal.cu（新同构 B 主线）+ 107002 防复退：验证
 * puresimt 链路下整条 host 侧与 CUDA 接口一致（导师：除 <<<>>> 与新同构差异外
 * 其他 acl 接口一样）——cudaStreamCreate→cudaMalloc→cudaMemcpy→
 * cudaLaunchKernel→cudaStreamSynchronize→D2H→校验，全部走 bridge 宏/wrapper
 * （cuda* → wcuda* → aclrt*），include <cuda_runtime.h> 由 repo include 接住。
 * 首调用 cudaStreamCreate：bridge_init 完成 aclInit→GetDeviceCount→SetDevice→
 * CreateContext，若初始化链有任何一环失败会直接暴露。
 */
#include "cuda_runtime.h"
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

    cudaStream_t stream = nullptr;
    cudaError_t err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) {
        printf("cudaStreamCreate failed: %d\n", (int)err);
        return 1;
    }

    float *h = nullptr;
    cudaMallocHost((void **)&h, byteSize);
    for (int i = 0; i < N; i++) h[i] = (float)i;

    float *d = nullptr;
    err = cudaMalloc((void **)&d, byteSize);
    if (err != cudaSuccess) {
        printf("cudaMalloc failed: %d\n", (int)err);
        return 1;
    }
    cudaMemcpy(d, h, byteSize, cudaMemcpyHostToDevice);

    cudaLaunchKernel((const void *)addOneKernel, dim3(1), dim3(THREADS),
                     nullptr, 0, stream);
    cudaStreamSynchronize(stream);
    cudaMemcpy(h, d, byteSize, cudaMemcpyDeviceToHost);

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

    cudaFree(d);
    cudaFreeHost(h);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    return 0;
}