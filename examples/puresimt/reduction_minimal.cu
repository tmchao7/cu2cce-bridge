/*
 * Minimal CUDA reduction regression for cu2cce puresimt.
 *
 * This follows the shared-memory/tree-reduction idea in NVIDIA's reduction
 * sample while using only CUDA runtime + stdio headers. Every thread reaches
 * both barriers; no early return is used before __syncthreads().
 */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 4096
#define THREADS_PER_BLOCK 256
#define BLOCKS ((N + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK)

__global__ void reduceSum(const int *input, int *output, int n) {
    __shared__ int partial[THREADS_PER_BLOCK];
    int tid = threadIdx.x;
    int i = blockIdx.x * blockDim.x + tid;
    partial[tid] = (i < n) ? input[i] : 0;
    __syncthreads();

    for (int stride = THREADS_PER_BLOCK / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            partial[tid] += partial[tid + stride];
        }
        __syncthreads();
    }

    if (tid == 0) {
        atomicAdd(output, partial[0]);
    }
}

int main(void) {
    int hInput[N];
    int hOutput = 0;
    for (int i = 0; i < N; ++i) hInput[i] = i;

    int *dInput = nullptr, *dOutput = nullptr;
    cudaStream_t stream = nullptr;
    cudaError_t err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dInput, sizeof(hInput)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dOutput, sizeof(int)) != cudaSuccess) return 1;
    if (cudaMemcpy(dInput, hInput, sizeof(hInput), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemset(dOutput, 0, sizeof(int)) != cudaSuccess) return 1;

    reduceSum<<<BLOCKS, THREADS_PER_BLOCK, 0, stream>>>(dInput, dOutput, N);
    err = cudaGetLastError();
    if (err != cudaSuccess) return 1;
    if (cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(&hOutput, dOutput, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    int expected = N * (N - 1) / 2;
    if (hOutput != expected) {
        printf("reduction_minimal FAILED (sum=%d expected=%d)\n", hOutput, expected);
        return 1;
    }

    cudaFree(dInput);
    cudaFree(dOutput);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    printf("reduction_minimal OK (N=%d, sum=%d)\n", N, hOutput);
    return 0;
}
