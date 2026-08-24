/* Minimal CUDA dot-product regression, based on NVIDIA reduction patterns. */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 1024
#define THREADS 256

__global__ void dotProduct(const float *a, const float *b, float *out) {
    __shared__ float partial[THREADS];
    int tid = threadIdx.x;
    int i = blockIdx.x * blockDim.x + tid;
    partial[tid] = (i < N) ? a[i] * b[i] : 0.0f;
    __syncthreads();
    for (int stride = THREADS / 2; stride > 0; stride >>= 1) {
        if (tid < stride) partial[tid] += partial[tid + stride];
        __syncthreads();
    }
    if (tid == 0) atomicAdd(out, partial[0]);
}

int main(void) {
    float hA[N], hB[N], hOut = 0.0f;
    for (int i = 0; i < N; ++i) { hA[i] = 1.0f; hB[i] = 2.0f; }
    float *dA = nullptr, *dB = nullptr, *dOut = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dA, sizeof(hA)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dB, sizeof(hB)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dOut, sizeof(float)) != cudaSuccess) return 1;
    if (cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dB, hB, sizeof(hB), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemset(dOut, 0, sizeof(float)) != cudaSuccess) return 1;
    dotProduct<<<(N + THREADS - 1) / THREADS, THREADS, 0, stream>>>(dA, dB, dOut);
    if (cudaGetLastError() != cudaSuccess || cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(&hOut, dOut, sizeof(float), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (hOut != 2.0f * N) {
        printf("dot_minimal FAILED (dot=%.1f expected=%.1f)\n", hOut, 2.0f * N);
        return 1;
    }
    cudaFree(dA); cudaFree(dB); cudaFree(dOut); cudaStreamDestroy(stream); cudaDeviceReset();
    printf("dot_minimal OK (N=%d, dot=%.1f)\n", N, hOut);
    return 0;
}
