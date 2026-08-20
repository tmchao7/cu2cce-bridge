/*
 * dynamic_blockdim.cu —— 纯 CUDA 的 B 方案端到端样例。
 * 同一 kernel 分别以 256/512 threads 启动，并以 dim3(32,8,1) 启动 2D kernel。
 * 用户源码不包含任何 CCE 隐藏参数；全部由 kernel_convert.py 自动生成。
 */
#include <cuda_runtime.h>
#include <stdio.h>

__global__ void flexibleAdd(float *x, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) x[i] += 1.0f;
}

__global__ void fill2D(float *x, int width, int height) {
    int x_index = blockIdx.x * blockDim.x + threadIdx.x;
    int y_index = blockIdx.y * blockDim.y + threadIdx.y;
    if (x_index < width && y_index < height) {
        x[y_index * width + x_index] = (float)(x_index + y_index);
    }
}

static int check_add(const float *values, int n) {
    for (int i = 0; i < n; ++i) {
        if (values[i] != (float)i + 1.0f) return i + 1;
    }
    return 0;
}

int main(void) {
    const int n256 = 256, n512 = 512;
    const int width = 32, height = 8, n2d = width * height;
    float h256[n256], h512[n512], h2d[n2d];
    for (int i = 0; i < n256; ++i) h256[i] = (float)i;
    for (int i = 0; i < n512; ++i) h512[i] = (float)i;
    for (int i = 0; i < n2d; ++i) h2d[i] = -1.0f;

    float *d256 = nullptr, *d512 = nullptr, *d2d = nullptr;
    cudaError_t err = cudaMalloc((void **)&d256, n256 * sizeof(float));
    if (err != cudaSuccess) return 1;
    err = cudaMalloc((void **)&d512, n512 * sizeof(float));
    if (err != cudaSuccess) return 1;
    err = cudaMalloc((void **)&d2d, n2d * sizeof(float));
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(d256, h256, n256 * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(d512, h512, n512 * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(d2d, h2d, n2d * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return 1;

    cudaStream_t stream = nullptr;
    err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) return 1;
    unsigned threads256 = 256, threads512 = 512;
    flexibleAdd<<<1, threads256, 0, stream>>>(d256, n256);
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) return 1;
    flexibleAdd<<<1, threads512, 0, stream>>>(d512, n512);
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) return 1;
    dim3 block2d(32, 8, 1);
    fill2D<<<1, block2d, 0, stream>>>(d2d, width, height);
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) return 1;

    err = cudaMemcpy(h256, d256, n256 * sizeof(float), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(h512, d512, n512 * sizeof(float), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(h2d, d2d, n2d * sizeof(float), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return 1;

    int bad256 = check_add(h256, n256);
    int bad512 = check_add(h512, n512);
    int bad2d = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (h2d[y * width + x] != (float)(x + y)) ++bad2d;
        }
    }
    if (bad256 || bad512 || bad2d) {
        printf("dynamic blockdim FAILED (256=%d 512=%d 2d=%d)\n",
               bad256, bad512, bad2d);
        return 1;
    }
    printf("dynamic blockdim OK (256,512,32x8x1)\n");
    cudaStreamDestroy(stream);
    cudaFree(d256);
    cudaFree(d512);
    cudaFree(d2d);
    return 0;
}
