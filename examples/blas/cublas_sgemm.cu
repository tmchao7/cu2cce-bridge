/*
 * Experimental CUDA cuBLAS -> CANN ACL CBLAS bridge test.
 *
 * This remains ordinary CUDA host code: cublas_v2.h is the repository's
 * compatibility header, while the bridge hides acl_cblas/GemmEx details.
 * The test uses an identity B so C must equal A.
 */
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <stdio.h>

#define M 32
#define N 32
#define K 32

int main(void) {
    float hA[M * K], hB[K * N], hC[M * N];
    for (int i = 0; i < M * K; ++i) hA[i] = (float)((i % K) + 1);
    for (int i = 0; i < K * N; ++i) hB[i] = (i % (N + 1) == 0) ? 1.0f : 0.0f;
    for (int i = 0; i < M * N; ++i) hC[i] = 0.0f;

    float *dA = nullptr, *dB = nullptr, *dC = nullptr;
    cudaStream_t stream = nullptr;
    cudaError_t err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) {
        printf("cublas_sgemm_experimental FAILED: stream=%d\n", (int)err);
        return 1;
    }
    if (cudaMalloc((void **)&dA, sizeof(hA)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dB, sizeof(hB)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dC, sizeof(hC)) != cudaSuccess) return 1;
    if (cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dB, hB, sizeof(hB), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemset(dC, 0, sizeof(hC)) != cudaSuccess) return 1;

    cublasHandle_t handle = nullptr;
    cublasStatus_t status = cublasCreate(&handle);
    if (status != CUBLAS_STATUS_SUCCESS) {
        printf("cublas_sgemm_experimental FAILED: cublasCreate=%d\n", (int)status);
        return 1;
    }
    status = cublasSetStream(handle, stream);
    if (status != CUBLAS_STATUS_SUCCESS) return 1;

    float alpha = 1.0f;
    float beta = 0.0f;
    status = cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                         M, N, K, &alpha, dA, M, dB, K, &beta, dC, M);
    if (status != CUBLAS_STATUS_SUCCESS) {
        printf("cublas_sgemm_experimental FAILED: cublasSgemm=%d\n", (int)status);
        return 1;
    }
    if (cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(hC, dC, sizeof(hC), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int i = 0; i < M * N; ++i) {
        if (hC[i] != hA[i]) {
            printf("cublas_sgemm_experimental FAILED at %d: got %.1f expected %.1f\n",
                   i, hC[i], hA[i]);
            return 1;
        }
    }

    cublasDestroy(handle);
    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    printf("cublas_sgemm_experimental OK (FP32 CUDA -> FP16 GemmEx, %dx%dx%d)\n", M, N, K);
    return 0;
}
