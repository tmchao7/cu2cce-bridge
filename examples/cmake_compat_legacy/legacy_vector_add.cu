#include <cuda_runtime.h>
#include <cstdio>

__global__ void legacy_add_kernel(const int *input, int *output, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) output[i] = input[i] + 1;
}

int main() {
    constexpr int N = 64;
    int host_input[N]{};
    int host_output[N]{};
    int *device_input = nullptr;
    int *device_output = nullptr;
    for (int i = 0; i < N; ++i) host_input[i] = i;

    if (cudaMalloc(reinterpret_cast<void **>(&device_input), sizeof(host_input)) != cudaSuccess ||
        cudaMalloc(reinterpret_cast<void **>(&device_output), sizeof(host_output)) != cudaSuccess ||
        cudaMemcpy(device_input, host_input, sizeof(host_input), cudaMemcpyHostToDevice) != cudaSuccess) {
        return 1;
    }
    legacy_add_kernel<<<1, N>>>(device_input, device_output, N);
    if (cudaDeviceSynchronize() != cudaSuccess ||
        cudaMemcpy(host_output, device_output, sizeof(host_output), cudaMemcpyDeviceToHost) != cudaSuccess) {
        return 1;
    }
    cudaFree(device_input);
    cudaFree(device_output);
    for (int i = 0; i < N; ++i) {
        if (host_output[i] != i + 1) return 1;
    }
    std::printf("legacy CUDA OK (N=%d)\n", N);
    return 0;
}
