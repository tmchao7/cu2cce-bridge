/* Minimal CUDA thread-fence/flag ordering coverage for cu2cce puresimt. */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 256

__global__ void publish(const int *input, int *payload, int *ready) {
    int tid = threadIdx.x;
    payload[tid] = input[tid] * 2;
    __threadfence();
    atomicAdd(ready, 1);
}

int main(void) {
    int hInput[N], hPayload[N], hReady = 0;
    for (int i = 0; i < N; ++i) { hInput[i] = i; hPayload[i] = 0; }
    int *dInput = nullptr, *dPayload = nullptr, *dReady = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dInput, sizeof(hInput)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dPayload, sizeof(hPayload)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dReady, sizeof(int)) != cudaSuccess) return 1;
    if (cudaMemcpy(dInput, hInput, sizeof(hInput), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemset(dReady, 0, sizeof(int)) != cudaSuccess) return 1;
    publish<<<1, N, 0, stream>>>(dInput, dPayload, dReady);
    if (cudaGetLastError() != cudaSuccess || cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(&hReady, dReady, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (cudaMemcpy(hPayload, dPayload, sizeof(hPayload), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (hReady != N) {
        printf("threadFence_minimal FAILED (ready=%d expected=%d)\n", hReady, N);
        return 1;
    }
    for (int i = 0; i < N; ++i) if (hPayload[i] != i * 2) {
        printf("threadFence_minimal FAILED payload at %d: got %d expected %d\n", i, hPayload[i], i * 2);
        return 1;
    }
    cudaFree(dInput); cudaFree(dPayload); cudaFree(dReady);
    cudaStreamDestroy(stream); cudaDeviceReset();
    printf("threadFence_minimal OK (N=%d, ready=%d)\n", N, hReady);
    return 0;
}
