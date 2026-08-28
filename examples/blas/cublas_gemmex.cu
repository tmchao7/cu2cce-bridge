/*
 * Experimental CUDA cuBLAS GemmEx -> CANN aclblasGemmEx bridge test.
 *
 * dtype-explicit path: FP16 A/B with FP32 C and CUBLAS_COMPUTE_32F, the
 * combination CANN aclblasGemmEx supports natively (no bridge-side
 * restaging). Identity B keeps C == A (fp16 values round-trip exactly for
 * small integers).
 */
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <stdio.h>

#define M 16
#define N 16
#define K 16

int main(void) {
    __fp16 hA[M * K], hB[K * N];
    float hC[M * N];
    for (int i = 0; i < M * K; ++i) hA[i] = (__fp16)((float)((i % K) + 1));
    for (int i = 0; i < K * N; ++i) hB[i] = (i % (N + 1) == 0) ? (__fp16)1.0f : (__fp16)0.0f;

    __fp16 *dA = nullptr, *dB = nullptr;
    float *dC = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) {
        printf("cublas_gemmex_experimental FAILED: stream\n");
        return 1;
    }
    if (cudaMalloc((void **)&dA, sizeof(hA)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dB, sizeof(hB)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dC, sizeof(hC)) != cudaSuccess) return 1;
    if (cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dB, hB, sizeof(hB), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemset(dC, 0, sizeof(hC)) != cudaSuccess) return 1;

    cublasHandle_t handle = nullptr;
    if (cublasCreate(&handle) != CUBLAS_STATUS_SUCCESS) {
        printf("cublas_gemmex_experimental FAILED: cublasCreate\n");
        return 1;
    }
    cublasSetMathMode(handle, CUBLAS_DEFAULT_MATH);
    cublasPointerMode_t pm = CUBLAS_POINTER_MODE_DEVICE;
    cublasGetPointerMode(handle, &pm);
    if (pm != CUBLAS_POINTER_MODE_HOST) {
        printf("cublas_gemmex_experimental FAILED: default pointer mode\n");
        return 1;
    }

    float alpha = 1.0f;
    float beta = 0.0f;
    cublasStatus_t status = cublasGemmEx(handle, CUBLAS_OP_N, CUBLAS_OP_N, M, N, K,
                                         &alpha, dA, CUDA_R_16F, M,
                                         dB, CUDA_R_16F, K,
                                         &beta, dC, CUDA_R_32F, M,
                                         CUBLAS_COMPUTE_32F);
    if (status != CUBLAS_STATUS_SUCCESS) {
        printf("cublas_gemmex_experimental FAILED: cublasGemmEx=%d\n", (int)status);
        return 1;
    }
    if (cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(hC, dC, sizeof(hC), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int i = 0; i < M * N; ++i) {
        float expect = (float)(unsigned short)hA[i];
        if (hC[i] != expect) {
            printf("cublas_gemmex_experimental FAILED at %d: got %.1f expected %.1f\n",
                   i, hC[i], expect);
            return 1;
        }
    }

    /* dtype-explicit rejection must stay honest: fp64 A-type is NOT_SUPPORT */
    double alpha64 = 1.0, beta64 = 0.0;
    status = cublasGemmEx(handle, CUBLAS_OP_N, CUBLAS_OP_N, M, N, K,
                          &alpha64, dA, CUDA_R_64F, M,
                          dB, CUDA_R_64F, K,
                          &beta64, dC, CUDA_R_64F, M,
                          CUBLAS_COMPUTE_32F);
    if (status == CUBLAS_STATUS_SUCCESS) {
        printf("cublas_gemmex_experimental FAILED: fp64 combo unexpectedly accepted\n");
        return 1;
    }

    cublasDestroy(handle);
    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    printf("cublas_gemmex_experimental OK (FP16xFP16->FP32 GemmEx, %dx%dx%d)\n", M, N, K);
    return 0;
}
