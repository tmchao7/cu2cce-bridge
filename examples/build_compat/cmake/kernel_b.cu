#include <cuda_runtime.h>

__global__ void double_kernel(const int *input, int *output) {
    if (threadIdx.x == 0) output[0] = input[0] * 2;
}

extern "C" int run_kernel_b() {
    int input = 21, output = 0;
    int *d_input = nullptr, *d_output = nullptr;
    if (cudaMalloc(reinterpret_cast<void **>(&d_input), sizeof(input)) != cudaSuccess) return 1;
    if (cudaMalloc(reinterpret_cast<void **>(&d_output), sizeof(output)) != cudaSuccess) return 1;
    if (cudaMemcpy(d_input, &input, sizeof(input), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    double_kernel<<<1, 1>>>(d_input, d_output);
    if (cudaDeviceSynchronize() != cudaSuccess) return 1;
    if (cudaMemcpy(&output, d_output, sizeof(output), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    cudaFree(d_input); cudaFree(d_output);
    return output == 42 ? 0 : 1;
}
