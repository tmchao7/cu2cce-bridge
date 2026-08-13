/**
 * vectorAdd —— cu2cce-bridge 迁移示例（仿照 cu-bridge 文档的 vectorAdd）。
 *
 * 目标：用户 .cu 源码保持 CUDA 写法，经 cucc（bisheng）编译后跑在昇腾 NPU 上。
 *
 * ⚠️ 本机（macOS）无昇腾硬件，此文件是"真机验证的测试资产"，勿在本机编译。
 * 真机编译：
 *   export CUCC_PATH=<cu2cce-bridge 安装目录>; export CUDA_PATH=${CUCC_PATH}
 *   export ASCEND_TOOLKIT_HOME=/usr/local/Ascend/ascend-toolkit/latest
 *   cucc vectorAdd.cu -o vectorAdd
 *
 * 本示例验证 host 侧 API 替换 + kernel 直通端到端链路（Phase A3）：
 *   cudaMalloc/cudaMemcpy/cudaDeviceSynchronize/cudaGetDeviceCount
 *   → wcudaMalloc/aclrtMalloc 等；kernel 经 bisheng 直编后校验 c = a + b。
 */
#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* SIMT kernel：昇腾 950 系列（dav-310）由 bisheng 直编。
 * __global__ 同名；同步原语由 simt_adaptor.h 映射（__syncthreads→asc_syncthreads）。 */
__global__ void vectorAddKernel(float *c, const float *a, const float *b, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        c[i] = a[i] + b[i];
    }
}

int main(void) {
    int devCount = 0;
    cudaError_t err = cudaGetDeviceCount(&devCount);
    if (err != cudaSuccess) {
        printf("cudaGetDeviceCount failed: %d\n", (int)err);
        return 1;
    }
    printf("device count = %d\n", devCount);
    if (devCount == 0) {
        return 1;
    }

    /* host 侧内存 */
    float *h_a = (float *)malloc(N * sizeof(float));
    float *h_b = (float *)malloc(N * sizeof(float));
    float *h_c = (float *)malloc(N * sizeof(float));
    for (int i = 0; i < N; ++i) {
        h_a[i] = 1.0f;
        h_b[i] = 2.0f;
    }

    /* device 侧内存（cudaMalloc → wcudaMalloc → aclrtMalloc） */
    float *d_a = nullptr, *d_b = nullptr, *d_c = nullptr;
    err = cudaMalloc((void **)&d_a, N * sizeof(float));
    if (err != cudaSuccess) {
        printf("cudaMalloc d_a failed: %d\n", (int)err);
        return 1;
    }
    err = cudaMalloc((void **)&d_b, N * sizeof(float));
    err = cudaMalloc((void **)&d_c, N * sizeof(float));
    if (err != cudaSuccess) {
        printf("cudaMalloc d_c failed: %d\n", (int)err);
        return 1;
    }

    /* 拷贝（cudaMemcpy → wcudaMemcpy → aclrtMemcpy） */
    err = cudaMemcpy(d_a, h_a, N * sizeof(float), cudaMemcpyHostToDevice);
    err = cudaMemcpy(d_b, h_b, N * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        printf("cudaMemcpy H2D failed: %d\n", (int)err);
        return 1;
    }

    /* kernel 启动（昇腾 SIMT 语法糖：<<<blocks, threads, dyn_ubuf_size, stream>>>，4 参，
     * 与 CUDA 的 <<<grid, block, sharedMem, stream>>> 一致，由 bisheng 原生识别。
     * 依据：asc-devkit hello_world_simt 示例。
     * 注意：CUDA 里 cudaStream_t stream = 0 表示默认 stream；昇腾需显式创建。 */
    int threads = 256;
    int blocks = (N + threads - 1) / threads;
    cudaStream_t stream = nullptr;
    err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) {
        printf("cudaStreamCreate failed: %d\n", (int)err);
        return 1;
    }
    vectorAddKernel<<<blocks, threads, 0, stream>>>(d_c, d_a, d_b, N);
    cudaStreamSynchronize(stream);
    cudaStreamDestroy(stream);

    cudaDeviceSynchronize();

    /* 拷回（cudaMemcpy → wcudaMemcpy → aclrtMemcpy） */
    err = cudaMemcpy(h_c, d_c, N * sizeof(float), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        printf("cudaMemcpy D2H failed: %d\n", (int)err);
        return 1;
    }

    /* 数值校验：kernel 直通跑通后 c 应全为 1+2=3 */
    int mismatch = 0;
    for (int i = 0; i < N; ++i) {
        if (h_c[i] != 3.0f) {
            mismatch++;
        }
    }
    if (mismatch != 0) {
        printf("vectorAdd FAILED: %d/%d mismatch\n", mismatch, N);
        return 1;
    }
    printf("vectorAdd OK (N=%d)\n", N);

    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
    free(h_a);
    free(h_b);
    free(h_c);
    return 0;
}
