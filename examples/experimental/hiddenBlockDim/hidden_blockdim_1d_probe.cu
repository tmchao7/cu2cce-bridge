/* B1：普通 bx/by/bz 参数控制同一 VF wrapper 的 256/512 线程。 */
#include <cuda_runtime.h>
#include <stdio.h>

__simt_vf__ [aicore] __attribute__((always_inline)) inline
void hidden_add_vf(__gm__ float *x, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) x[i] += 1.0f;
}

__global__[aicore] void hiddenAdd(__gm__ float *x, int n,
                                  unsigned bx, unsigned by, unsigned bz) {
    cce::async_invoke<hidden_add_vf>(cce::dim3{bx, by, bz}, x, n);
}

static int check_add(const float *values, int n) {
    for (int i = 0; i < n; ++i) {
        if (values[i] != (float)i + 1.0f) return i + 1;
    }
    return 0;
}

int main(void) {
    const int n256 = 256, n512 = 512;
    float h256[n256], h512[n512];
    for (int i = 0; i < n256; ++i) h256[i] = (float)i;
    for (int i = 0; i < n512; ++i) h512[i] = (float)i;

    float *d256 = nullptr, *d512 = nullptr;
    cudaError_t err = cudaMalloc((void **)&d256, n256 * sizeof(float));
    if (err != cudaSuccess) return 1;
    err = cudaMalloc((void **)&d512, n512 * sizeof(float));
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(d256, h256, n256 * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(d512, h512, n512 * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return 1;

    cudaStream_t stream = nullptr;
    err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) return 1;
    unsigned threads256 = 256, threads512 = 512;
    hiddenAdd<<<1, threads256, 0, stream>>>(d256, n256, threads256, 1, 1);
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) return 1;
    hiddenAdd<<<1, threads512, 0, stream>>>(d512, n512, threads512, 1, 1);
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) return 1;

    err = cudaMemcpy(h256, d256, n256 * sizeof(float), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(h512, d512, n512 * sizeof(float), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return 1;
    int bad256 = check_add(h256, n256), bad512 = check_add(h512, n512);
    if (bad256 || bad512) {
        printf("hidden blockdim 1d FAILED (256=%d 512=%d)\n", bad256, bad512);
        return 1;
    }
    printf("hidden blockdim 1d OK (256,512)\n");
    cudaStreamDestroy(stream);
    cudaFree(d256);
    cudaFree(d512);
    return 0;
}
