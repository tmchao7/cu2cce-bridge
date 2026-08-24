/* Minimal CUDA 1D stencil regression, based on NVIDIA shared-memory patterns. */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 1024
#define THREADS 256

__global__ void stencil(const float *input, float *output) {
    __shared__ float tile[THREADS + 2];
    int tid = threadIdx.x;
    int i = blockIdx.x * blockDim.x + tid;
    tile[tid + 1] = (i < N) ? input[i] : 0.0f;
    if (tid == 0) tile[0] = (i > 0) ? input[i - 1] : 0.0f;
    if (tid == THREADS - 1) tile[THREADS + 1] = (i + 1 < N) ? input[i + 1] : 0.0f;
    __syncthreads();
    if (i < N) output[i] = tile[tid] + tile[tid + 1] + tile[tid + 2];
}

int main(void) {
    float hInput[N], hOutput[N];
    for (int i = 0; i < N; ++i) { hInput[i] = 1.0f; hOutput[i] = 0.0f; }
    float *dInput = nullptr, *dOutput = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dInput, sizeof(hInput)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dOutput, sizeof(hOutput)) != cudaSuccess) return 1;
    if (cudaMemcpy(dInput, hInput, sizeof(hInput), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    stencil<<<(N + THREADS - 1) / THREADS, THREADS, 0, stream>>>(dInput, dOutput);
    if (cudaGetLastError() != cudaSuccess || cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(hOutput, dOutput, sizeof(hOutput), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    for (int i = 0; i < N; ++i) {
        float expected = (i == 0 || i == N - 1) ? 2.0f : 3.0f;
        if (hOutput[i] != expected) {
            printf("stencil_minimal FAILED at %d: got %.1f expected %.1f\n", i, hOutput[i], expected);
            return 1;
        }
    }
    cudaFree(dInput); cudaFree(dOutput); cudaStreamDestroy(stream); cudaDeviceReset();
    printf("stencil_minimal OK (N=%d)\n", N);
    return 0;
}
