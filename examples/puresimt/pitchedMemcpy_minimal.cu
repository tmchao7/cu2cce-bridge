/* Minimal CUDA pitched 2D-copy regression for cu2cce puresimt. */
#include <cuda_runtime.h>
#include <stdio.h>

#define WIDTH 37
#define HEIGHT 19

static int requireCuda(const char *stage, cudaError_t err) {
    if (err != cudaSuccess) {
        printf("pitchedMemcpy_minimal ERROR at %s: code=%d (%s)\n",
               stage, (int)err, cudaGetErrorString(err));
        return 0;
    }
    printf("pitchedMemcpy_minimal: %s OK\n", stage);
    return 1;
}

__global__ void addRows(char *data, size_t pitch, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x < width && y < height) {
        float *row = (float *)(data + (size_t)y * pitch);
        row[x] += (float)y;
    }
}

int main(void) {
    setbuf(stdout, nullptr);
    printf("pitchedMemcpy_minimal: start\n");
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
    if (!requireCuda("cudaMallocPitch", cudaMallocPitch((void **)&d, &pitch, rowBytes, HEIGHT))) return 1;
    printf("pitchedMemcpy_minimal: d=%p pitch=%zu rowBytes=%zu total=%zu\n",
           (void *)d, pitch, rowBytes, pitch * HEIGHT);
    if (!requireCuda("cudaMemcpy2D H2D",
                     cudaMemcpy2D(d, pitch, hInput, rowBytes, rowBytes, HEIGHT,
                                  cudaMemcpyHostToDevice))) return 1;

    dim3 block(16, 8, 1);
    dim3 grid((WIDTH + block.x - 1) / block.x, (HEIGHT + block.y - 1) / block.y, 1);
    printf("pitchedMemcpy_minimal: launch grid=(%u,%u,%u) block=(%u,%u,%u)\n",
           grid.x, grid.y, grid.z, block.x, block.y, block.z);
    addRows<<<grid, block>>>(d, pitch, WIDTH, HEIGHT);
    if (!requireCuda("cudaGetLastError", cudaGetLastError())) return 1;
    if (!requireCuda("cudaDeviceSynchronize", cudaDeviceSynchronize())) return 1;
    if (!requireCuda("cudaMemcpy2D D2H",
                     cudaMemcpy2D(hOutput, rowBytes, d, pitch, rowBytes, HEIGHT,
                                  cudaMemcpyDeviceToHost))) return 1;

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

    if (!requireCuda("cudaFree", cudaFree(d))) return 1;
    if (!requireCuda("cudaDeviceReset", cudaDeviceReset())) return 1;
    printf("pitchedMemcpy_minimal OK (%dx%d, pitch=%zu)\n", WIDTH, HEIGHT, pitch);
    return 0;
}
