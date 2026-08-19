/* 纯 host 侧 API 桥冒烟测试（本机 mock 可验证，不含 kernel 语法）。
 * 真机/含 kernel 的完整链路见 examples/vectorAdd/vectorAdd.cu（需 bisheng）。
 * ⚠️ 不 include 系统标准库头（ccec+glibc 2.39 兼容问题，真机实测）；
 *    printf/malloc 等由 __cuda_runtime.h 提供最小声明。 */
#include <cuda_runtime.h>
#include <cublas_v2.h>

int main(void) {
    int devCount = 0;
    cudaError_t err = cudaGetDeviceCount(&devCount);
    if (err != cudaSuccess) {
        printf("getDeviceCount failed: %d\n", (int)err);
        return 1;
    }
    if (devCount > 0) {
        cudaSetDevice(0);
    }

    float *d = nullptr;
    err = cudaMalloc((void **)&d, 1024 * sizeof(float));
    if (err != cudaSuccess) {
        printf("cudaMalloc failed: %d\n", (int)err);
        return 1;
    }
    err = cudaMemset(d, 0, 1024 * sizeof(float));
    if (err != cudaSuccess) {
        printf("cudaMemset failed: %d\n", (int)err);
        return 1;
    }

    float h[4] = {1, 2, 3, 4};
    err = cudaMemcpy(d, h, sizeof(h), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        printf("cudaMemcpy H2D failed: %d\n", (int)err);
        return 1;
    }
    err = cudaMemcpy(h, d, sizeof(h), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        printf("cudaMemcpy D2H failed: %d\n", (int)err);
        return 1;
    }

    cudaDeviceSynchronize();
    cudaFree(d);

    int attr = 0;
    err = cudaDeviceGetAttribute(&attr, cudaDevAttrWarpSize, 0);
    if (err != cudaSuccess) {
        printf("cudaDeviceGetAttribute failed: %d\n", (int)err);
        return 1;
    }

    /* cublas 桥 */
    cublasHandle_t hb = nullptr;
    cublasStatus_t s = cublasCreate(&hb);
    if (s == CUBLAS_STATUS_SUCCESS && hb != nullptr) {
        cublasDestroy(hb);
    }

    printf("host bridge smoke OK\n");
    return 0;
}
