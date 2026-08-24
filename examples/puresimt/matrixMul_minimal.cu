/*
 * Minimal CUDA matrix multiplication regression for cu2cce puresimt.
 *
 * Deliberately avoids NVIDIA helper headers, profiler headers, cublas, and
 * host-allocation helpers. The only source-level dependencies are CUDA runtime
 * and stdio; the kernel is ordinary CUDA shared-memory code.
 */
#include <cuda_runtime.h>
#include <stdio.h>

#define M 32
#define N 32
#define K 32
#define TILE 16

__global__ void matrixMul(const float *A, const float *B, float *C) {
    __shared__ float tileA[TILE][TILE];
    __shared__ float tileB[TILE][TILE];

    int row = blockIdx.y * TILE + threadIdx.y;
    int col = blockIdx.x * TILE + threadIdx.x;
    float sum = 0.0f;

    for (int tile = 0; tile < K / TILE; ++tile) {
        tileA[threadIdx.y][threadIdx.x] = A[row * K + tile * TILE + threadIdx.x];
        tileB[threadIdx.y][threadIdx.x] = B[(tile * TILE + threadIdx.y) * N + col];
        __syncthreads();

        for (int j = 0; j < TILE; ++j) {
            sum += tileA[threadIdx.y][j] * tileB[j][threadIdx.x];
        }
        __syncthreads();
    }

    C[row * N + col] = sum;
}

int main(void) {
    float hA[M * K], hB[K * N], hC[M * N];
    for (int i = 0; i < M * K; ++i) hA[i] = (float)((i % K) + 1);
    for (int i = 0; i < K * N; ++i) hB[i] = (i % (N + 1) == 0) ? 1.0f : 0.0f;
    for (int i = 0; i < M * N; ++i) hC[i] = 0.0f;

    float *dA = nullptr, *dB = nullptr, *dC = nullptr;
    cudaStream_t stream = nullptr;
    cudaError_t err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) {
        printf("matrixMul_minimal: cudaStreamCreate failed: %d\n", (int)err);
        return 1;
    }
    err = cudaMalloc((void **)&dA, sizeof(hA));
    if (err != cudaSuccess) return 1;
    err = cudaMalloc((void **)&dB, sizeof(hB));
    if (err != cudaSuccess) return 1;
    err = cudaMalloc((void **)&dC, sizeof(hC));
    if (err != cudaSuccess) return 1;

    cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice);
    cudaMemcpy(dB, hB, sizeof(hB), cudaMemcpyHostToDevice);
    cudaMemset(dC, 0, sizeof(hC));

    dim3 block(TILE, TILE, 1);
    dim3 grid((N + TILE - 1) / TILE, (M + TILE - 1) / TILE, 1);
    matrixMul<<<grid, block, 0, stream>>>(dA, dB, dC);
    err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("matrixMul_minimal: kernel launch failed: %d\n", (int)err);
        return 1;
    }
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(hC, dC, sizeof(hC), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return 1;

    for (int row = 0; row < M; ++row) {
        for (int col = 0; col < N; ++col) {
            float expected = hA[row * K + col];
            if (hC[row * N + col] != expected) {
                printf("matrixMul_minimal FAILED at (%d,%d): got %.1f expected %.1f\n",
                       row, col, hC[row * N + col], expected);
                return 1;
            }
        }
    }

    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    printf("matrixMul_minimal OK (C = A*B, %dx%dx%d)\n", M, N, K);
    return 0;
}
