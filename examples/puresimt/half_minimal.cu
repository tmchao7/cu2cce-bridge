/* Minimal CUDA half/half2 conversion coverage for Ascend puresimt. */
#include <cuda_runtime.h>
#include <cuda_fp16.h>
#include <stdio.h>

__global__ void halfOperators(float *output) {
    __half a = __float2half(1.5f);
    __half b = __float2half(2.25f);
    __half2 packed = __halves2half2(a, b);
    output[0] = __low2float(packed);
    output[1] = __high2float(packed);
}

int main(void) {
    float h[2] = {0.0f, 0.0f};
    float *d = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&d, sizeof(h)) != cudaSuccess) return 1;
    halfOperators<<<1, 1, 0, stream>>>(d);
    if (cudaGetLastError() != cudaSuccess || cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(h, d, sizeof(h), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (h[0] != 1.5f || h[1] != 2.25f) {
        printf("half_minimal FAILED (x=%.6f y=%.6f)\n", h[0], h[1]);
        return 1;
    }
    cudaFree(d); cudaStreamDestroy(stream); cudaDeviceReset();
    printf("half_minimal OK (half2 x=%.2f y=%.2f)\n", h[0], h[1]);
    return 0;
}
