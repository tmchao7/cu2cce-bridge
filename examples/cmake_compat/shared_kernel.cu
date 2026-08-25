#include <cuda_runtime.h>

__global__ void shared_mul_kernel(const int *input, int *output) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i == 0) output[0] = input[0] * 2;
}

int run_shared_kernel_test() {
    int host_input = 21;
    int host_output = 0;
    int *device_input = nullptr;
    int *device_output = nullptr;

    if (cudaMalloc(reinterpret_cast<void **>(&device_input), sizeof(host_input)) != cudaSuccess) return 1;
    if (cudaMalloc(reinterpret_cast<void **>(&device_output), sizeof(host_output)) != cudaSuccess) return 1;
    if (cudaMemcpy(device_input, &host_input, sizeof(host_input), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    shared_mul_kernel<<<1, 1>>>(device_input, device_output);
    if (cudaDeviceSynchronize() != cudaSuccess) return 1;
    if (cudaMemcpy(&host_output, device_output, sizeof(host_output), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    cudaFree(device_input);
    cudaFree(device_output);
    return host_output == 42 ? 0 : 1;
}
