/* B2：CCE 外层 launch 使用标量总线程数 256，隐藏参数传递 VF 形状 32x8x1。 */
#include <cuda_runtime.h>
#include <stdio.h>

__simt_vf__ [aicore] __attribute__((always_inline)) inline
void hidden_2d_vf(__gm__ float *x, int width, int height) {
    int x_index = blockIdx.x * blockDim.x + threadIdx.x;
    int y_index = blockIdx.y * blockDim.y + threadIdx.y;
    if (x_index < width && y_index < height) {
        x[y_index * width + x_index] = (float)(x_index + y_index);
    }
}

__global__[aicore] void hidden2D(__gm__ float *x, int width, int height,
                                 unsigned bx, unsigned by, unsigned bz) {
    cce::async_invoke<hidden_2d_vf>(cce::dim3{bx, by, bz}, x, width, height);
}

int main(void) {
    const int width = 32, height = 8, n = width * height;
    float h[n];
    for (int i = 0; i < n; ++i) h[i] = -1.0f;
    float *d = nullptr;
    cudaError_t err = cudaMalloc((void **)&d, n * sizeof(float));
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(d, h, n * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return 1;

    cudaStream_t stream = nullptr;
    err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) return 1;
    dim3 block2d(32, 8, 1);
    unsigned launch_threads = 256;
    hidden2D<<<1, launch_threads, 0, stream>>>(d, width, height,
                                               block2d.x, block2d.y, block2d.z);
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(h, d, n * sizeof(float), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return 1;

    int bad = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (h[y * width + x] != (float)(x + y)) ++bad;
        }
    }
    if (bad) {
        printf("hidden blockdim 2d FAILED (%d mismatch)\n", bad);
        return 1;
    }
    printf("hidden blockdim 2d OK (32x8x1)\n");
    cudaStreamDestroy(stream);
    cudaFree(d);
    return 0;
}
