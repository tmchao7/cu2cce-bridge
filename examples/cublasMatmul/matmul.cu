/*
 * matmul.cu —— cublas 桥真机验证（方向 2：cublasSgemm → aclblasGemmEx）
 *
 * 目标：验证 src/bridge/blas/src/cublas_v2_wrapper.cpp 的 Sgemm 映射在真机
 * （davinci0，CANN 9.2.0）能编译、链接、运行并算出正确 C = alpha*A*B + beta*C。
 * 这是 CURRENT_STATE 里"实现完成、从未真机运行"的最后一处（待确认项之外的 cublas）。
 *
 * 纯 CUDA 写法（零改写）：include <cublas_v2.h> → 落入 cu2cce 的 cublas_v2.h →
 * bridge/blas/cublas_v2_wrapper.h，宏替换 cublas* → wcublas*（内部 aclblasGemmEx）。
 *
 * 注意（已知差异，CLAUDE.md）：
 *   - CUDA 分配默认清零，昇腾不清零 → gemm 前手动 cudaMemset 初始化 C；
 *   - aclblasGemmEx 无 handle，直接取 handle 里存的 stream（本 example 不显式
 *     cublasSetStream，用默认 nullptr）。
 */
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <stdio.h>
#include <stdlib.h>

#define M 4
#define N 4
#define K 4

int main(void) {
    int devCount = 0;
    cudaError_t err = cudaGetDeviceCount(&devCount);
    if (err != cudaSuccess || devCount == 0) {
        printf("cudaGetDeviceCount failed: %d\n", (int)err);
        return 1;
    }

    cudaStream_t stream = nullptr;
    err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) {
        printf("cudaStreamCreate failed: %d (%s)\n", (int)err,
               cudaGetErrorString(err));
        return 1;
    }

    /* A = [[1,2,3,4],[5,6,7,8],...] KxK 满秩（每行 1..K 递增），
     * B = 单位阵；C = alpha*A*I + beta*C。
     * 选 alpha=1 beta=0 → C 应等于 A（无需转置）。 */
    float hA[M * K], hB[K * N], hC[M * N], hC_expect[M * N];
    for (int i = 0; i < M * K; ++i) hA[i] = (float)((i % K) + 1);
    for (int i = 0; i < K * N; ++i) hB[i] = (i % (N + 1) == 0) ? 1.0f : 0.0f;
    for (int i = 0; i < M * N; ++i) hC[i] = 0.0f;
    for (int r = 0; r < M; ++r)
        for (int c = 0; c < N; ++c) {
            float acc = 0.0f;
            for (int kk = 0; kk < K; ++kk) acc += hA[r * K + kk] * hB[kk * N + c];
            hC_expect[r * N + c] = acc;
        }

    float *dA = nullptr, *dB = nullptr, *dC = nullptr;
    err = cudaMalloc((void **)&dA, M * K * sizeof(float));
    if (err != cudaSuccess) return 1;
    err = cudaMalloc((void **)&dB, K * N * sizeof(float));
    if (err != cudaSuccess) return 1;
    err = cudaMalloc((void **)&dC, M * N * sizeof(float));
    if (err != cudaSuccess) return 1;

    err = cudaMemcpy(dA, hA, M * K * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(dB, hB, K * N * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return 1;
    /* 昇腾不清零：gemm 前手动初始化 C=0（已知差异） */
    err = cudaMemset(dC, 0, M * N * sizeof(float));
    if (err != cudaSuccess) return 1;

    cublasHandle_t handle = nullptr;
    cublasStatus_t st = cublasCreate(&handle);
    if (st != CUBLAS_STATUS_SUCCESS) {
        printf("cublasCreate failed: %d\n", (int)st);
        return 1;
    }
    /* 显式设置 stream：昇腾 aclblas 计算接口要求非空 aclrtStream，
     * 不能用 CUDA 的"nullptr 默认流"写法（wrapper 兜底也会自建，显式更稳）。 */
    st = cublasSetStream(handle, stream);
    if (st != CUBLAS_STATUS_SUCCESS) {
        printf("cublasSetStream failed: %d\n", (int)st);
        return 1;
    }

    float alpha = 1.0f, beta = 0.0f;
    st = cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                     M, N, K, &alpha, dA, M, dB, K, &beta, dC, M);
    if (st != CUBLAS_STATUS_SUCCESS) {
        printf("cublasSgemm failed: %d\n", (int)st);
        return 1;
    }

    /* gemm 是异步下发，等 stream + device 完成再拷回 */
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) {
        printf("cudaStreamSynchronize failed: %d (%s)\n",
               (int)err, cudaGetErrorString(err));
        return 1;
    }
    err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        printf("cudaDeviceSynchronize failed: %d (%s)\n",
               (int)err, cudaGetErrorString(err));
        return 1;
    }

    err = cudaMemcpy(hC, dC, M * N * sizeof(float), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return 1;

    int bad = 0;
    for (int i = 0; i < M * N; ++i) {
        if (hC[i] != hC_expect[i]) {
            if (bad < 4) printf("C[%d]=%.1f expected=%.1f\n",
                                i, hC[i], hC_expect[i]);
            bad++;
        }
    }
    if (bad != 0) {
        printf("cublas matmul FAILED (%d mismatch)\n", bad);
        return 1;
    }

    printf("cublas matmul OK (C = alpha*A*B + beta*C, %dx%dx%d)\n", M, N, K);

    cublasDestroy(handle);
    cudaStreamDestroy(stream);
    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    return 0;
}