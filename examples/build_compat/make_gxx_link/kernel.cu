/*
 * Device side of the make_gxx_link carrier. The kernel is compiled through
 * cucc (ccec --enable-simt); the host entry is exposed with C linkage so
 * main.cpp (compiled by g++) can call it without C++ ABI coupling.
 */
#include <cuda_runtime.h>

__global__ void scale_kernel(const float *in, float *out, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) out[i] = in[i] * 2.f;
}

extern "C" int run_scale(const float *in, float *out, int n) {
    scale_kernel<<<(n + 255) / 256, 256>>>(in, out, n);
    return cudaDeviceSynchronize() == cudaSuccess ? 0 : 1;
}
