/* Minimal CUDA dynamic shared-memory coverage for cu2cce puresimt. */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 256

__global__ void dynamicShared(const int *input, int *output, int n) {
    extern __shared__ int shared[];
    int tid = threadIdx.x;
    shared[tid] = input[tid];
    __syncthreads();
    output[tid] = shared[(tid + 1) % n] + 1;
}

int main(void) {
    int hInput[N], hOutput[N];
    for (int i = 0; i < N; ++i) { hInput[i] = i; hOutput[i] = 0; }
    int *dInput = nullptr, *dOutput = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dInput, sizeof(hInput)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dOutput, sizeof(hOutput)) != cudaSuccess) return 1;
    if (cudaMemcpy(dInput, hInput, sizeof(hInput), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    dynamicShared<<<1, N, N * sizeof(int), stream>>>(dInput, dOutput, N);
    if (cudaGetLastError() != cudaSuccess || cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(hOutput, dOutput, sizeof(hOutput), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    for (int i = 0; i < N; ++i) {
        int expected = ((i + 1) % N) + 1;
        if (hOutput[i] != expected) {
            printf("dynamicShared_minimal FAILED at %d: got %d expected %d\n", i, hOutput[i], expected);
            return 1;
        }
    }
    cudaFree(dInput); cudaFree(dOutput); cudaStreamDestroy(stream); cudaDeviceReset();
    printf("dynamicShared_minimal OK (N=%d, shared_bytes=%zu)\n", N, sizeof(int) * (size_t)N);
    return 0;
}
