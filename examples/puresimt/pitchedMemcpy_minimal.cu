/* Minimal CUDA pitched 2D-copy regression for cu2cce puresimt. */
#include <cuda_runtime.h>
#include <stdio.h>

#define WIDTH 37
#define HEIGHT 19

__global__ void addRows(char *data, size_t pitch, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x < width && y < height) {
        float *row = (float *)(data + (size_t)y * pitch);
        row[x] += (float)y;
    }
}

int main(void) {
    float hInput[HEIGHT][WIDTH], hOutput[HEIGHT][WIDTH];
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            hInput[y][x] = (float)x;
            hOutput[y][x] = 0.0f;
        }
    }

    char *d = nullptr;
    size_t pitch = 0;
    size_t rowBytes = WIDTH * sizeof(float);
    if (cudaMallocPitch((void **)&d, &pitch, rowBytes, HEIGHT) != cudaSuccess) return 1;
    if (cudaMemcpy2D(d, pitch, hInput, rowBytes, rowBytes, HEIGHT,
                     cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    dim3 block(16, 8, 1);
    dim3 grid((WIDTH + block.x - 1) / block.x, (HEIGHT + block.y - 1) / block.y, 1);
    addRows<<<grid, block>>>(d, pitch, WIDTH, HEIGHT);
    if (cudaGetLastError() != cudaSuccess || cudaDeviceSynchronize() != cudaSuccess) return 1;
    if (cudaMemcpy2D(hOutput, rowBytes, d, pitch, rowBytes, HEIGHT,
                     cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            float expected = (float)(x + y);
            if (hOutput[y][x] != expected) {
                printf("pitchedMemcpy_minimal FAILED at (%d,%d): got %.1f expected %.1f\n",
                       y, x, hOutput[y][x], expected);
                return 1;
            }
        }
    }

    cudaFree(d);
    cudaDeviceReset();
    printf("pitchedMemcpy_minimal OK (%dx%d, pitch=%zu)\n", WIDTH, HEIGHT, pitch);
    return 0;
}
