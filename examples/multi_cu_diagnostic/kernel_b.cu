#include <cuda_runtime.h>

__global__ void add1_kernel(const float *in, float *out, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) out[i] = in[i] + 1.0f;
}

extern "C" int run_add1(const float *in, float *out, int n) {
    float *d_in = nullptr;
    float *d_out = nullptr;
    if (cudaMalloc(&d_in, sizeof(float) * n) != cudaSuccess) return 1;
    if (cudaMalloc(&d_out, sizeof(float) * n) != cudaSuccess) return 1;
    if (cudaMemcpy(d_in, in, sizeof(float) * n, cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    add1_kernel<<<1, 128>>>(d_in, d_out, n);
    if (cudaDeviceSynchronize() != cudaSuccess) return 1;
    if (cudaMemcpy(out, d_out, sizeof(float) * n, cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    cudaFree(d_in);
    cudaFree(d_out);
    return 0;
}