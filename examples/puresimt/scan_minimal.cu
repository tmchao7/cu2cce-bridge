/*
 * Minimal CUDA inclusive-scan regression for cu2cce puresimt.
 *
 * Based on the block scan pattern in NVIDIA CUDA Samples (scan), reduced to a
 * single self-contained CUDA source so helper headers do not hide bridge issues.
 */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 256

__global__ void inclusiveScan(const int *input, int *output) {
    __shared__ int values[N];
    int tid = threadIdx.x;
    values[tid] = input[tid];
    __syncthreads();

    for (int offset = 1; offset < N; offset <<= 1) {
        int add = (tid >= offset) ? values[tid - offset] : 0;
        __syncthreads();
        values[tid] += add;
        __syncthreads();
    }
    output[tid] = values[tid];
}

int main(void) {
    int hInput[N], hOutput[N];
    for (int i = 0; i < N; ++i) {
        hInput[i] = 1;
        hOutput[i] = 0;
    }

    int *dInput = nullptr, *dOutput = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dInput, sizeof(hInput)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dOutput, sizeof(hOutput)) != cudaSuccess) return 1;
    if (cudaMemcpy(dInput, hInput, sizeof(hInput), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    inclusiveScan<<<1, N, 0, stream>>>(dInput, dOutput);
    if (cudaGetLastError() != cudaSuccess) return 1;
    if (cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(hOutput, dOutput, sizeof(hOutput), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int i = 0; i < N; ++i) {
        if (hOutput[i] != i + 1) {
            printf("scan_minimal FAILED at %d: got %d expected %d\n", i, hOutput[i], i + 1);
            return 1;
        }
    }

    cudaFree(dInput);
    cudaFree(dOutput);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    printf("scan_minimal OK (N=%d, final=%d)\n", N, hOutput[N - 1]);
    return 0;
}
