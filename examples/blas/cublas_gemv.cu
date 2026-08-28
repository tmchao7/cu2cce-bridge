/*
 * Experimental CUDA cuBLAS Sgemv -> CANN aclblasGemvEx bridge test.
 *
 * y = alpha*A*x + beta*y with identity A (m=n): y must equal x elementwise
 * (alpha=1, beta=0). Column-major A, y length = m.
 */
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <stdio.h>

#define M 8
#define N 8

int main(void) {
    float hA[M * N], hx[N], hy[M];
    for (int c = 0; c < N; ++c) {
        for (int r = 0; r < M; ++r) {
            hA[c * M + r] = (r == c) ? 1.0f : 0.0f; /* column-major identity */
        }
    }
    for (int i = 0; i < N; ++i) hx[i] = (float)(i + 1);
    for (int i = 0; i < M; ++i) hy[i] = -1.0f;

    float *dA = nullptr, *dx = nullptr, *dy = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) {
        printf("cublas_gemv_experimental FAILED: stream\n");
        return 1;
    }
    if (cudaMalloc((void **)&dA, sizeof(hA)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dx, sizeof(hx)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dy, sizeof(hy)) != cudaSuccess) return 1;
    if (cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dx, hx, sizeof(hx), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dy, hy, sizeof(hy), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    cublasHandle_t handle = nullptr;
    if (cublasCreate(&handle) != CUBLAS_STATUS_SUCCESS) {
        printf("cublas_gemv_experimental FAILED: cublasCreate\n");
        return 1;
    }
    cublasSetStream(handle, stream);

    float alpha = 1.0f;
    float beta = 0.0f;
    cublasStatus_t status = cublasSgemv(handle, CUBLAS_OP_N, M, N, &alpha, dA, M, dx, 1, &beta, dy, 1);
    if (status != CUBLAS_STATUS_SUCCESS) {
        printf("cublas_gemv_experimental FAILED: cublasSgemv=%d\n", (int)status);
        return 1;
    }
    if (cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(hy, dy, sizeof(hy), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int i = 0; i < M; ++i) {
        if (hy[i] != hx[i]) {
            printf("cublas_gemv_experimental FAILED at %d: got %.1f expected %.1f\n",
                   i, hy[i], hx[i]);
            return 1;
        }
    }

    /* BLAS1 surface stays honestly not-supported, but validates parameters */
    float z = 0.0f;
    if (cublasSaxpy(handle, 0, &alpha, dx, 1, dy, 1) != (cublasStatus_t)ACL_ERROR_INVALID_PARAM) {
        printf("cublas_gemv_experimental FAILED: saxpy param validation\n");
        return 1;
    }
    if (cublasSaxpy(handle, 4, &alpha, dx, 1, &z, 1) != (cublasStatus_t)ACL_ERROR_API_NOT_SUPPORT) {
        printf("cublas_gemv_experimental FAILED: saxpy not-support mapping\n");
        return 1;
    }

    cublasDestroy(handle);
    cudaFree(dA);
    cudaFree(dx);
    cudaFree(dy);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    printf("cublas_gemv_experimental OK (FP32 Sgemv identity, %dx%d)\n", M, N);
    return 0;
}
