/*
 * hidden_blockdim_probe.cu —— B 方案最小真机探针
 *
 * 目的：验证 __global__[aicore] wrapper 是否可以把普通 kernel 参数
 *       bx/by/bz 传给 cce::async_invoke，而不读取 wrapper 内的
 *       blockDim.x/y/z。
 *
 * 这不是正式 CUDA 零改写样例：它显式使用了已验证的 CCE SIMT VF 模板，
 * 只用于隔离 Bisheng 编译器行为。kernel_convert.py 仍会把 CUDA 四参启动
 * 转成 CCE 三参启动。
 */
#include <cuda_runtime.h>
#include <stdio.h>

__simt_vf__ [aicore] __attribute__((always_inline)) inline
void hidden_add_vf(__gm__ float *x, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        x[i] += 1.0f;
    }
}

__global__[aicore] void hiddenAdd(__gm__ float *x, int n,
                                  unsigned bx, unsigned by, unsigned bz) {
    cce::async_invoke<hidden_add_vf>(cce::dim3{bx, by, bz}, x, n);
}

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

static int check_add(const float *values, int n) {
    for (int i = 0; i < n; ++i) {
        if (values[i] != (float)i + 1.0f) {
            return i + 1;
        }
    }
    return 0;
}

static int check_2d(const float *values, int width, int height) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (values[y * width + x] != (float)(x + y)) {
                return y * width + x + 1;
            }
        }
    }
    return 0;
}

int main(void) {
    const int n256 = 256;
    const int n512 = 512;
    const int width = 32;
    const int height = 8;
    const int n2d = width * height;
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

    unsigned threads256 = 256;
    unsigned threads512 = 512;
    hiddenAdd<<<1, threads256, 0, stream>>>(d256, n256, threads256, 1, 1);
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) return 1;
    hiddenAdd<<<1, threads512, 0, stream>>>(d512, n512, threads512, 1, 1);
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) return 1;

    dim3 block2d(32, 8, 1);
    hidden2D<<<1, block2d, 0, stream>>>(d2d, width, height,
                                        block2d.x, block2d.y, block2d.z);
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
    int bad2d = check_2d(h2d, width, height);
    if (bad256 || bad512 || bad2d) {
        printf("hidden blockdim FAILED (256=%d 512=%d 2d=%d)\n",
               bad256, bad512, bad2d);
        return 1;
    }

    printf("hidden blockdim OK (256,512,2d)\n");
    cudaStreamDestroy(stream);
    cudaFree(d256);
    cudaFree(d512);
    cudaFree(d2d);
    return 0;
}
