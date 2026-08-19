/**
 * minimal —— 最小可跑示例：验证"随便写个简单 CUDA kernel 就能在昇腾上跑"。
 *
 * 纯 CUDA 代码（零改写）：host 侧 cudaMalloc/Memcpy/Stream，kernel 一个
 * __global__ 简单函数。cucc 编译时自动转换（kernel_convert.py）→ ccec → davinci0。
 *
 * 编译运行（真机）：
 *   export CUCC_PATH=$PWD/build/real_test/install
 *   export CUDA_PATH=$CUCC_PATH
 *   export PATH=$CUCC_PATH/bin:$PATH
 *   cucc -c examples/minimal/minimal.cu -o /tmp/minimal.o
 *   cucc /tmp/minimal.o -o /tmp/minimal
 *   /tmp/minimal
 *
 * 预期输出：minimal OK (N=256)
 */
#include <cuda_runtime.h>
#include <stdio.h>

__global__ void addOne(float *x, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        x[i] += 1.0f;
    }
}

int main(void) {
    const int n = 256;
    float h[n];
    for (int i = 0; i < n; ++i) {
        h[i] = (float)i;
    }

    float *d = nullptr;
    cudaError_t err = cudaMalloc((void **)&d, n * sizeof(float));
    if (err != cudaSuccess) {
        printf("cudaMalloc failed: %d\n", (int)err);
        return 1;
    }
    err = cudaMemcpy(d, h, n * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        printf("cudaMemcpy H2D failed: %d\n", (int)err);
        return 1;
    }

    cudaStream_t s = nullptr;
    cudaStreamCreate(&s);
    addOne<<<1, 256, 0, s>>>(d, n);
    cudaStreamSynchronize(s);
    cudaStreamDestroy(s);

    err = cudaMemcpy(h, d, n * sizeof(float), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        printf("cudaMemcpy D2H failed: %d\n", (int)err);
        return 1;
    }

    int bad = 0;
    for (int i = 0; i < n; ++i) {
        if (h[i] != (float)i + 1.0f) {
            bad++;
        }
    }
    if (bad != 0) {
        printf("minimal FAILED (%d mismatch)\n", bad);
        return 1;
    }
    printf("minimal OK (N=%d)\n", n);

    cudaFree(d);
    return 0;
}
