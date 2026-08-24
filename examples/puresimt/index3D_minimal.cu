/* Minimal CUDA 3D grid/block indexing coverage for cu2cce puresimt. */
#include <cuda_runtime.h>
#include <stdio.h>

#define X 4
#define Y 3
#define Z 2
#define BX 2
#define BY 2
#define BZ 1
#define TOTAL (X * Y * Z)

__global__ void index3D(int *output) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x < X && y < Y && z < Z) output[(z * Y + y) * X + x] = x + 10 * y + 100 * z;
}

int main(void) {
    int hOutput[TOTAL] = {0};
    int *dOutput = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dOutput, sizeof(hOutput)) != cudaSuccess) return 1;
    if (cudaMemset(dOutput, 0, sizeof(hOutput)) != cudaSuccess) return 1;
    dim3 block(BX, BY, BZ);
    dim3 grid((X + BX - 1) / BX, (Y + BY - 1) / BY, (Z + BZ - 1) / BZ);
    index3D<<<grid, block, 0, stream>>>(dOutput);
    if (cudaGetLastError() != cudaSuccess || cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(hOutput, dOutput, sizeof(hOutput), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    for (int z = 0; z < Z; ++z) for (int y = 0; y < Y; ++y) for (int x = 0; x < X; ++x) {
        int index = (z * Y + y) * X + x;
        int expected = x + 10 * y + 100 * z;
        if (hOutput[index] != expected) {
            printf("index3D_minimal FAILED at (%d,%d,%d): got %d expected %d\n",
                   x, y, z, hOutput[index], expected);
            return 1;
        }
    }
    cudaFree(dOutput); cudaStreamDestroy(stream); cudaDeviceReset();
    printf("index3D_minimal OK (%dx%dx%d)\n", X, Y, Z);
    return 0;
}
