#include <cuda_runtime.h>
#include <stdio.h>

#define N 1024

__global__ void vectorAdd(const float *a, const float *b, float *c, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) c[i] = a[i] + b[i];
}

int main(void) {
    float hA[N], hB[N], hC[N];
    for (int i = 0; i < N; ++i) {
        hA[i] = (float)i;
        hB[i] = 2.0f;
        hC[i] = 0.0f;
    }
    float *dA = nullptr, *dB = nullptr, *dC = nullptr;
    if (cudaMalloc((void **)&dA, sizeof(hA)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dB, sizeof(hB)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dC, sizeof(hC)) != cudaSuccess) return 1;
    if (cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dB, hB, sizeof(hB), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    vectorAdd<<<(N + 255) / 256, 256>>>(dA, dB, dC, N);
    if (cudaGetLastError() != cudaSuccess || cudaDeviceSynchronize() != cudaSuccess) return 1;
    if (cudaMemcpy(hC, dC, sizeof(hC), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    for (int i = 0; i < N; ++i) {
        if (hC[i] != (float)i + 2.0f) return 1;
    }
    cudaFree(dA); cudaFree(dB); cudaFree(dC); cudaDeviceReset();
    printf("cmake vectorAdd OK (N=%d)\n", N);
    return 0;
}
