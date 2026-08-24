/* Minimal multi-kernel stream-ordering regression for cu2cce puresimt. */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 2048

__global__ void addOne(const int *input, int *output) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) output[i] = input[i] + 1;
}

__global__ void doubleValues(int *data) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) data[i] *= 2;
}

int main(void) {
    int hInput[N], hOutput[N];
    for (int i = 0; i < N; ++i) { hInput[i] = i; hOutput[i] = 0; }
    int *dInput = nullptr, *dOutput = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dInput, sizeof(hInput)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dOutput, sizeof(hOutput)) != cudaSuccess) return 1;
    if (cudaMemcpyAsync(dInput, hInput, sizeof(hInput), cudaMemcpyHostToDevice, stream) != cudaSuccess) return 1;

    addOne<<<(N + 255) / 256, 256, 0, stream>>>(dInput, dOutput);
    if (cudaGetLastError() != cudaSuccess) return 1;
    doubleValues<<<(N + 255) / 256, 256, 0, stream>>>(dOutput);
    if (cudaGetLastError() != cudaSuccess) return 1;
    if (cudaMemcpyAsync(hOutput, dOutput, sizeof(hOutput), cudaMemcpyDeviceToHost, stream) != cudaSuccess) return 1;
    if (cudaStreamSynchronize(stream) != cudaSuccess) return 1;

    for (int i = 0; i < N; ++i) {
        if (hOutput[i] != (i + 1) * 2) {
            printf("multiKernel_minimal FAILED at %d: got %d expected %d\n",
                   i, hOutput[i], (i + 1) * 2);
            return 1;
        }
    }

    cudaFree(dInput); cudaFree(dOutput); cudaStreamDestroy(stream); cudaDeviceReset();
    printf("multiKernel_minimal OK (N=%d, ordered_kernels=2)\n", N);
    return 0;
}
