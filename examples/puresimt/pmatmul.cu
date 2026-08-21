/*
 * pmatmul.cu —— puresimt 形态的 cublas 桥矩阵乘真机首验（方向 2）。
 *
 * 目标：验证 src/bridge/blas/src/cublas_v2_wrapper.cpp 的 Sgemm 映射
 * （cublasSgemm → aclblasGemmEx）在 puresimt 链路（-x asc --enable-simt
 * --cce-aicore-arch=dav-c310-vec）下编译、链接、运行并算出 C = A*B。
 * 这是 CURRENT_STATE"实现完成、从未真机运行"的最后一处（方向 2）。
 *
 * 纯 CUDA 写法（零 acl 依赖）：include <cuda_runtime.h> + <cublas_v2.h> → 均落入
 * repo include/；宏表 cublas* → wcublas*（内部 aclblasGemmEx）。host 侧与
 * puresimt 其他样例一致：首调 cudaStreamCreate（bridge_init），cuda* 全走
 * wcuda wrapper。
 *
 * 为什么必选 aclbp（acl cblas）：
 *   - 昇腾 SIMT 只做向量（AIV），不做 Cube，cuda 没有"kernel 矩阵乘"可用；
 *   - cublas 桥是方向 2，Sgemm 已有实现、缺一次真机运行。
 *
 * 已知差异（CLAUDE.md / wrapper 注释）：
 *   - CUDA 分配默认清零，昇腾不清 → gemm 前手动 cudaMemset 初始化 C；
 *   - aclblasGemmEx 异步下发 → wcublasSgemm 内部已 aclrtSynchronizeStream，
 *     调用方再 cudaStreamSynchronize/cudaDeviceSynchronize 保险；
 *   - stream 非空：cublasSetStream(handle, stream) 显式传。
 *
 * 尺寸：16×16×16（昇腾 Cube GEMM 临界小尺寸，16 是旧 cublasMatmul 定稿值）。
 * 数据：A = 行 i 为 [1..K]，B = 单位阵，C = A*I = A。校验逐元素相等。
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

#define M 16
#define N 16
#define K 16

int main(void) {
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
    st = cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                     M, N, K, &alpha, dA, M, dB, K, &beta, dC, M);
    if (st != CUBLAS_STATUS_SUCCESS) {
        printf("cublasSgemm failed: %d\n", (int)st);
        return 1;
    }
    cudaStreamSynchronize(stream);
    cudaDeviceSynchronize();

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

    cublasDestroy(handle);
    cudaStreamDestroy(stream);
    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    cudaDeviceReset();
    return 0;
}