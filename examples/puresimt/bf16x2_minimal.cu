/* Minimal CUDA BF16/BF16x2 conversion coverage for Ascend puresimt. */
#include <cuda_runtime.h>
#include <cuda_bf16.h>
#include <stdio.h>

__global__ void bf16Operators(float *output) {
    __nv_bfloat162 packed = __float2bfloat162_rn(1.5f);
    float2 unpacked = __bfloat1622float2(packed);
    output[0] = unpacked.x;
    output[1] = unpacked.y;
}

int main(void) {
    float h[2] = {0.0f, 0.0f};
    float *d = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&d, sizeof(h)) != cudaSuccess) return 1;
    bf16Operators<<<1, 1, 0, stream>>>(d);
    if (cudaGetLastError() != cudaSuccess || cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(h, d, sizeof(h), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (h[0] != 1.5f || h[1] != 1.5f) {
        printf("bf16x2_minimal FAILED (x=%.6f y=%.6f)\n", h[0], h[1]);
        return 1;
    }
    cudaFree(d); cudaStreamDestroy(stream); cudaDeviceReset();
    printf("bf16x2_minimal OK (bf16x2 x=%.2f y=%.2f)\n", h[0], h[1]);
    return 0;
}
