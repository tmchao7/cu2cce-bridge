/*
 * Minimal CUDA tiled-transpose regression for cu2cce puresimt.
 *
 * Based on NVIDIA CUDA Samples' transpose shared-memory tile pattern. The
 * source intentionally uses only CUDA runtime and stdio.
 */
#include <cuda_runtime.h>
#include <stdio.h>

#define M 32
#define N 32
#define TILE 16

__global__ void transpose(const float *input, float *output) {
    __shared__ float tile[TILE][TILE + 1];
    int x = blockIdx.x * TILE + threadIdx.x;
    int y = blockIdx.y * TILE + threadIdx.y;

    tile[threadIdx.y][threadIdx.x] = input[y * N + x];
    __syncthreads();

    int outX = blockIdx.y * TILE + threadIdx.x;
    int outY = blockIdx.x * TILE + threadIdx.y;
    output[outY * M + outX] = tile[threadIdx.x][threadIdx.y];
}

int main(void) {
    float hInput[M * N], hOutput[M * N];
    for (int y = 0; y < M; ++y) {
        for (int x = 0; x < N; ++x) {
            hInput[y * N + x] = (float)(y * N + x);
            hOutput[y * M + x] = 0.0f;
        }
    }

    float *dInput = nullptr, *dOutput = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dInput, sizeof(hInput)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dOutput, sizeof(hOutput)) != cudaSuccess) return 1;
    if (cudaMemcpy(dInput, hInput, sizeof(hInput), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    dim3 block(TILE, TILE, 1);
    dim3 grid((N + TILE - 1) / TILE, (M + TILE - 1) / TILE, 1);
    transpose<<<grid, block, 0, stream>>>(dInput, dOutput);
    if (cudaGetLastError() != cudaSuccess) return 1;
    if (cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(hOutput, dOutput, sizeof(hOutput), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int y = 0; y < M; ++y) {
        for (int x = 0; x < N; ++x) {
            float expected = hInput[x * N + y];
            if (hOutput[y * M + x] != expected) {
                printf("transpose_minimal FAILED at (%d,%d): got %.1f expected %.1f\n",
                       y, x, hOutput[y * M + x], expected);
                return 1;
            }
        }
    }

    cudaFree(dInput);
    cudaFree(dOutput);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    printf("transpose_minimal OK (%dx%d)\n", M, N);
    return 0;
}
