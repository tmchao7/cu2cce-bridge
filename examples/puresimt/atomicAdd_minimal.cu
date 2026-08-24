/*
 * Minimal CUDA atomic-operator regression for cu2cce puresimt.
 *
 * The test follows the global-memory atomic pattern used by NVIDIA's
 * simpleAtomicIntrinsics sample, but keeps only CUDA runtime + stdio headers so
 * bridge/header failures remain isolated from NVIDIA helper utilities.
 */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 4096
#define THREADS_PER_BLOCK 256

__global__ void atomicOperators(int *sum, int *minimum, int *maximum, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        atomicAdd(sum, 1);
        atomicMin(minimum, i);
        atomicMax(maximum, i);
    }
}

int main(void) {
    int hSum = 0;
    int hMin = N;
    int hMax = -1;
    int *dSum = nullptr, *dMin = nullptr, *dMax = nullptr;
    cudaStream_t stream = nullptr;
    cudaError_t err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) return 1;

    if (cudaMalloc((void **)&dSum, sizeof(int)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dMin, sizeof(int)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dMax, sizeof(int)) != cudaSuccess) return 1;
    if (cudaMemcpy(dSum, &hSum, sizeof(int), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dMin, &hMin, sizeof(int), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dMax, &hMax, sizeof(int), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    int blocks = (N + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    atomicOperators<<<blocks, THREADS_PER_BLOCK, 0, stream>>>(dSum, dMin, dMax, N);
    err = cudaGetLastError();
    if (err != cudaSuccess) return 1;
    if (cudaStreamSynchronize(stream) != cudaSuccess) return 1;

    if (cudaMemcpy(&hSum, dSum, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (cudaMemcpy(&hMin, dMin, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (cudaMemcpy(&hMax, dMax, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    if (hSum != N || hMin != 0 || hMax != N - 1) {
        printf("atomicAdd_minimal FAILED (sum=%d min=%d max=%d)\n", hSum, hMin, hMax);
        return 1;
    }

    cudaFree(dSum);
    cudaFree(dMin);
    cudaFree(dMax);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    printf("atomicAdd_minimal OK (N=%d, sum=%d, min=%d, max=%d)\n", N, hSum, hMin, hMax);
    return 0;
}
