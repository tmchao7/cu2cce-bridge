/*
 * pmatmul.cu —— puresimt 形态的 cublas 桥矩阵乘真机首验（方向 2）。
 *
 * 目标：验证 src/bridge/blas/src/cublas_v2_wrapper.cpp 的 Sgemm 映射
 * （cublasSgemm → wcublasSgemm → aclblasGemmEx）在 puresimt 链路（-x asc
 * --enable-simt --cce-aicore-arch=dav-c310-vec）编译、链接、运行，算出
 * C = A*B。纯 CUDA 写法（零 acl 依赖、零手写 half）：include <cuda_runtime.h>
 * + <cublas_v2.h> 均落入 repo include/；host 首调 cudaStreamCreate（bridge_init）。
 *
 * fp32 自动降级说明（昇腾硬件限制，wrapper 内部处理）：真机 9.2.0 acl_cblas.h
 * 确认无 fp32 aclblasSgemm，aclblasGemmEx 仅 fp16/int8 组合 → wcublasSgemm
 * 内部把 fp32 输入降为 half 计算、C 保持 fp32（fp16×fp16×float 组合）。本样例
 * A/B 是整数 1..32，half 无损，校验值与纯 fp32 一致。
 *
 * 尺寸：32×32×32（用于排查 16 对齐/最小尺寸约束；本轮只改 shape）。
 * 已知差异（昇腾）：CUDA 分配默认清零昇腾不清 → gemm 前 cudaMemset 初始化 C；
 * aclblasGemmEx 异步 → wcublasSgemm 内部同步，调用方再双同步保险。
 *
 * 编译（真机，见 server_cmd）：
 *   export ASCEND_HOME_PATH=/usr/local/Ascend/latest
 *   ccec -x asc --cce-aicore-arch=dav-c310-vec --enable-simt -std=c++17 \
 *     -I<repo>/include -I/usr/local/Ascend/latest/include \
 *     -L<repo>/build/real_test/install/lib -L/usr/local/Ascend/latest/lib64 \
 *     -lascendc_runtime -lascendcl -lruntime -lregister -lerror_manager \
 *     -lprofapi -lascendalog -lmmpa -lascend_dump -lc_sec -lstdc++ \
 *     -lcce_bridge_runtime -lacl_cblas \
 *     examples/puresimt/pmatmul.cu -o pmatmul
 */
#include "cuda_runtime.h"
#include "cublas_v2.h"
#include <stdio.h>
#include <stdlib.h>

#define M 32
#define N 32
#define K 32

int main(void) {
    /* 真机回归通过文件重定向 stdout；无缓冲便于区分 GemmEx 成功后 cleanup
     * 崩溃，还是结果校验/后续 CUDA bridge 崩溃。 */
    setbuf(stdout, nullptr);
    printf("pmatmul: start\n");
    cudaStream_t stream = nullptr;
    cudaError_t err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) {
        printf("cudaStreamCreate failed: %d\n", (int)err);
        return 1;
    }

    float hA[M * K], hB[K * N], hC[M * N], hC_expect[M * N];
    for (int i = 0; i < M * K; ++i) hA[i] = (float)((i % K) + 1);
    for (int i = 0; i < K * N; ++i) hB[i] = (i % (N + 1) == 0) ? 1.0f : 0.0f;
    for (int i = 0; i < M * N; ++i) hC[i] = 0.0f;
    for (int r = 0; r < M; ++r)
        for (int c = 0; c < N; ++c) {
            float acc = 0.0f;
            for (int kk = 0; kk < K; ++kk) acc += hA[r * K + kk] * hB[kk * N + c];
            hC_expect[r * N + c] = acc;
        }

    float *dA = nullptr, *dB = nullptr, *dC = nullptr;
    err = cudaMalloc((void **)&dA, M * K * sizeof(float));
    if (err != cudaSuccess) { printf("cudaMalloc dA failed: %d\n", (int)err); return 1; }
    err = cudaMalloc((void **)&dB, K * N * sizeof(float));
    if (err != cudaSuccess) { printf("cudaMalloc dB failed: %d\n", (int)err); return 1; }
    err = cudaMalloc((void **)&dC, M * N * sizeof(float));
    if (err != cudaSuccess) { printf("cudaMalloc dC failed: %d\n", (int)err); return 1; }

    err = cudaMemcpy(dA, hA, M * K * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) { printf("cudaMemcpy A failed: %d\n", (int)err); return 1; }
    err = cudaMemcpy(dB, hB, K * N * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) { printf("cudaMemcpy B failed: %d\n", (int)err); return 1; }
    err = cudaMemset(dC, 0, M * N * sizeof(float));
    if (err != cudaSuccess) { printf("cudaMemset C failed: %d\n", (int)err); return 1; }

    cublasHandle_t handle = nullptr;
    cublasStatus_t st = cublasCreate(&handle);
    if (st != CUBLAS_STATUS_SUCCESS) {
        printf("cublasCreate failed: %d\n", (int)st);
        return 1;
    }
    st = cublasSetStream(handle, stream);
    if (st != CUBLAS_STATUS_SUCCESS) {
        printf("cublasSetStream failed: %d\n", (int)st);
        return 1;
    }

    float alpha = 1.0f, beta = 0.0f;
    printf("pmatmul: before cublasSgemm\n");
    st = cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                     M, N, K, &alpha, dA, M, dB, K, &beta, dC, M);
    printf("pmatmul: after cublasSgemm status=%d\n", (int)st);
    if (st != CUBLAS_STATUS_SUCCESS) {
        printf("cublasSgemm failed: %d\n", (int)st);
        return 1;
    }
    printf("pmatmul: before cudaStreamSynchronize\n");
    err = cudaStreamSynchronize(stream);
    printf("pmatmul: after cudaStreamSynchronize status=%d\n", (int)err);
    err = cudaDeviceSynchronize();
    printf("pmatmul: after cudaDeviceSynchronize status=%d\n", (int)err);

    printf("pmatmul: before cudaMemcpy C\n");
    err = cudaMemcpy(hC, dC, M * N * sizeof(float), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) { printf("cudaMemcpy C failed: %d\n", (int)err); return 1; }

    int bad = 0;
    for (int i = 0; i < M * N; ++i) {
        if (hC[i] != hC_expect[i]) {
            if (bad < 4) printf("C[%d]=%.1f expected=%.1f\n",
                                i, hC[i], hC_expect[i]);
            bad++;
        }
    }
    if (bad != 0) {
        printf("pmatmul puresimt FAILED (%d mismatch)\n", bad);
        return 1;
    }
    printf("pmatmul puresimt OK (C = A*B, %dx%dx%d)\n", M, N, K);

    printf("pmatmul: before cublasDestroy\n");
    cublasDestroy(handle);
    printf("pmatmul: before cudaStreamDestroy\n");
    err = cudaStreamDestroy(stream);
    printf("pmatmul: after cudaStreamDestroy status=%d\n", (int)err);
    printf("pmatmul: before cudaFree A\n");
    err = cudaFree(dA);
    printf("pmatmul: after cudaFree A status=%d\n", (int)err);
    printf("pmatmul: before cudaFree B\n");
    err = cudaFree(dB);
    printf("pmatmul: after cudaFree B status=%d\n", (int)err);
    printf("pmatmul: before cudaFree C\n");
    err = cudaFree(dC);
    printf("pmatmul: after cudaFree C status=%d\n", (int)err);
    printf("pmatmul: before cudaDeviceReset\n");
    err = cudaDeviceReset();
    printf("pmatmul: after cudaDeviceReset status=%d\n", (int)err);
    printf("pmatmul: done\n");
    return 0;
}