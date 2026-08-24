/*
 * Minimal CUDA warp-shuffle regression for cu2cce puresimt.
 *
 * Based on the warp/shuffle reduction pattern in NVIDIA CUDA Samples
 * (shfl_scan). Uses only the CUDA runtime and stdio headers.
 */
#include <cuda_runtime.h>
#include <stdio.h>

#define WARPS 8
#define THREADS (WARPS * 32)

__global__ void warpReduce(const int *input, int *output) {
    int lane = threadIdx.x & 31;
    int warp = threadIdx.x >> 5;
    int value = input[threadIdx.x];

    for (int offset = 16; offset > 0; offset >>= 1) {
        value += __shfl_down_sync(0xffffffffu, value, offset);
    }
    if (lane == 0) {
        output[warp] = value;
    }
}

int main(void) {
    int hInput[THREADS];
    int hOutput[WARPS] = {0};
    for (int i = 0; i < THREADS; ++i) hInput[i] = 1;

    int *dInput = nullptr, *dOutput = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dInput, sizeof(hInput)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dOutput, sizeof(hOutput)) != cudaSuccess) return 1;
    if (cudaMemcpy(dInput, hInput, sizeof(hInput), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    warpReduce<<<1, THREADS, 0, stream>>>(dInput, dOutput);
    if (cudaGetLastError() != cudaSuccess) return 1;
    if (cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(hOutput, dOutput, sizeof(hOutput), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int i = 0; i < WARPS; ++i) {
        if (hOutput[i] != 32) {
            printf("warpShuffle_minimal FAILED at warp %d: got %d expected 32\n", i, hOutput[i]);
            return 1;
        }
    }

    cudaFree(dInput);
    cudaFree(dOutput);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    printf("warpShuffle_minimal OK (warps=%d, warp_sum=32)\n", WARPS);
    return 0;
}
