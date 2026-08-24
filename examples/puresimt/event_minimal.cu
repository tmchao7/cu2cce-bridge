/* Minimal CUDA event/stream-ordering regression for cu2cce puresimt. */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 1024

__global__ void fillValue(float *data, float value) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) data[i] = value;
}

int main(void) {
    float h[N] = {0};
    float *d = nullptr;
    cudaStream_t stream = nullptr;
    cudaEvent_t start = nullptr, stop = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaEventCreate(&start) != cudaSuccess) return 1;
    if (cudaEventCreate(&stop) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&d, sizeof(h)) != cudaSuccess) return 1;

    if (cudaEventRecord(start, stream) != cudaSuccess) return 1;
    fillValue<<<(N + 255) / 256, 256, 0, stream>>>(d, 7.0f);
    if (cudaGetLastError() != cudaSuccess) return 1;
    if (cudaEventRecord(stop, stream) != cudaSuccess) return 1;
    if (cudaEventSynchronize(stop) != cudaSuccess) return 1;
    if (cudaEventQuery(stop) != cudaSuccess) return 1;

    float elapsed = -1.0f;
    if (cudaEventElapsedTime(&elapsed, start, stop) != cudaSuccess) return 1;
    if (cudaMemcpy(h, d, sizeof(h), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    for (int i = 0; i < N; ++i) {
        if (h[i] != 7.0f) {
            printf("event_minimal FAILED at %d: got %.1f\n", i, h[i]);
            return 1;
        }
    }

    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    cudaFree(d);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    printf("event_minimal OK (N=%d, elapsed_ms=%.3f)\n", N, elapsed);
    return 0;
}
