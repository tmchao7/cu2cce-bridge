/*
 * Minimal CUDA vectorAdd regression for cu2cce puresimt.
 *
 * This is intentionally a small, self-contained CUDA program rather than a
 * bridge-specific sample: one CUDA runtime header, one C stdio header, a
 * standard __global__ kernel, and the normal <<<>>> launch syntax.
 */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 50000
#define THREADS_PER_BLOCK 256

__global__ void vectorAdd(const float *A, const float *B, float *C, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        C[i] = A[i] + B[i];
    }
}

int main(void) {
    float hA[N], hB[N], hC[N];
    for (int i = 0; i < N; ++i) {
        hA[i] = (float)i;
        hB[i] = 2.0f;
        hC[i] = 0.0f;
    }

    float *dA = nullptr, *dB = nullptr, *dC = nullptr;
    cudaStream_t stream = nullptr;
    cudaError_t err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) {
        printf("vectorAdd_minimal: cudaStreamCreate failed: %d\n", (int)err);
        return 1;
    }
    err = cudaMalloc((void **)&dA, sizeof(hA));
    if (err != cudaSuccess) return 1;
    err = cudaMalloc((void **)&dB, sizeof(hB));
    if (err != cudaSuccess) return 1;
    err = cudaMalloc((void **)&dC, sizeof(hC));
    if (err != cudaSuccess) return 1;

    err = cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(dB, hB, sizeof(hB), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return 1;

    int blocks = (N + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    vectorAdd<<<blocks, THREADS_PER_BLOCK, 0, stream>>>(dA, dB, dC, N);
    err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("vectorAdd_minimal: kernel launch failed: %d\n", (int)err);
        return 1;
    }
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(hC, dC, sizeof(hC), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return 1;

    for (int i = 0; i < N; ++i) {
        if (hC[i] != (float)i + 2.0f) {
            printf("vectorAdd_minimal FAILED at %d: got %.1f expected %.1f\n",
                   i, hC[i], (float)i + 2.0f);
            return 1;
        }
    }

    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    printf("vectorAdd_minimal OK (N=%d)\n", N);
    return 0;
}
