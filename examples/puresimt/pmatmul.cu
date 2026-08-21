/*
 * pmatmul.cu —— puresimt 形态的 cublas 桥矩阵乘真机首验（方向 2，fp16 版）。
 *
 * 目标：验证 src/bridge/blas/src/cublas_v2_wrapper.cpp 的 Sgemm 映射在 puresimt
 * 链路（-x asc --enable-simt --cce-aicore-arch=dav-c310-vec）编译/链接/运行，
 * 算出 C = A*B。当前真机确认（9.2.0 acl_cblas.h）：
 *   - 无 fp32 aclblasSgemm（grep 无）；
 *   - aclblasGemmEx 数据类型组合仅 fp16/int8（fp16×fp16×fp16、fp16×fp16×float、
 *     int8×int8×float、int8×int8×int32）。
 * → fp32 走 GemmEx 必 100000（历史 5 次失败根因）。所以本样例用 fp16 原生：
 *   host 显式把 A/B 转 half 输入，C 用 float 累加（fp16×fp16×float 组合）。
 *
 * 纯 CUDA 写法（零 acl 依赖）：include <cuda_runtime.h> + <cublas_v2.h> 均落入
 * repo include/；宏表 cublas* → wcublas*（内部 aclblasGemmEx）。host 侧首调
 * cudaStreamCreate（bridge_init）。
 *
 * fp16 精度说明：A/B 值 1..16 是整数，half 无损表示；C=A*B 最大 16*16=256，
 * half 累加可能引入舍入，因此 C 用 float（fp16×fp16×float 组合原生支持）。
 *
 * 尺寸：16×16×16（昇腾 Cube GEMM 临界小尺寸，沿用冻结值）。
 * 已知差异（昇腾）：CUDA 分配默认清零昇腾不清 → gemm 前 cudaMemset 初始化 C；
 * aclblasGemmEx 异步 → wcublasSgemm 内部已同步，调用方再双同步保险。
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

/* fp32 → half（IEEE 半精度，直接截断为 10 位尾数；A/B 全整数 1..16 无损） */
static unsigned short to_half(float x) {
    unsigned int u;
    float *p = &x;
    u = *(unsigned int *)p;
    unsigned int sign = (u >> 16) & 0x8000u;
    unsigned int exp = (u >> 23) & 0xffu;
    unsigned int mant = u & 0x7fffffu;
    if (exp == 0xffu) { /* Inf/NaN → 保持符号位的 Inf */
        return (unsigned short)(sign | 0x7c00u);
    }
    int e = (int)exp - 127 + 15;
    if (e >= 31) { /* overflow → Inf */
        return (unsigned short)(sign | 0x7c00u);
    }
    if (e <= 0) { /* subnormal/near zero */
        if (e < -10) return (unsigned short)sign;
        mant |= 0x800000u;
        unsigned int shift = (unsigned int)(14 - e);
        unsigned int round = (mant >> (shift - 1)) & 1u;
        unsigned int h = mant >> shift;
        if (round) h++;
        return (unsigned short)(sign | h);
    }
    unsigned int h = ((unsigned int)e << 10) | (mant >> 13);
    return (unsigned short)(sign | h);
}

int main(void) {
    cudaStream_t stream = nullptr;
    cudaError_t err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) {
        printf("cudaStreamCreate failed: %d\n", (int)err);
        return 1;
    }

    /* A 每行 [1..K]，B 单位阵 → C = A*I = A */
    unsigned short hA16[M * K], hB16[K * N];
    float hC[M * N], hC_expect[M * N];
    for (int i = 0; i < M * K; ++i) hA16[i] = to_half((float)((i % K) + 1));
    for (int i = 0; i < K * N; ++i) hB16[i] = to_half((i % (N + 1) == 0) ? 1.0f : 0.0f);
    for (int i = 0; i < M * N; ++i) hC[i] = 0.0f;
    for (int r = 0; r < M; ++r)
        for (int c = 0; c < N; ++c) {
            float acc = 0.0f;
            for (int kk = 0; kk < K; ++kk) {
                float av = (float)((r * K + kk) % K + 1);
                float bv = ((kk * N + c) % (N + 1) == 0) ? 1.0f : 0.0f;
                acc += av * bv;
            }
            hC_expect[r * N + c] = acc;
        }

    /* device 端全是 float 指针（cublas 桥的 C 是 fp32），A/B 输入用 fp16 half
     * 但 cudaMalloc 仍按 float 大小分配正好 16 字节（fp16 用不上 acl 库时）……
     * 不：C 是 fp32 输出，A/B 是 fp16 输入——这里用 fp16 的字节数分配 */
    size_t szA = M * K * sizeof(unsigned short);
    size_t szB = K * N * sizeof(unsigned short);
    size_t szC = M * N * sizeof(float);
    unsigned short *dA = nullptr, *dB = nullptr;
    float *dC = nullptr;
    err = cudaMalloc((void **)&dA, szA);
    if (err != cudaSuccess) { printf("cudaMalloc dA failed: %d\n", (int)err); return 1; }
    err = cudaMalloc((void **)&dB, szB);
    if (err != cudaSuccess) { printf("cudaMalloc dB failed: %d\n", (int)err); return 1; }
    err = cudaMalloc((void **)&dC, szC);
    if (err != cudaSuccess) { printf("cudaMalloc dC failed: %d\n", (int)err); return 1; }

    err = cudaMemcpy(dA, hA16, szA, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) { printf("cudaMemcpy A failed: %d\n", (int)err); return 1; }
    err = cudaMemcpy(dB, hB16, szB, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) { printf("cudaMemcpy B failed: %d\n", (int)err); return 1; }
    err = cudaMemset(dC, 0, szC);
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

    /* fp16 输入（half），fp32 输出（float）：aclblasGemmEx fp16×fp16×float 组合 */
    float alpha = 1.0f, beta = 0.0f;
    st = cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                     M, N, K, &alpha, dA, M, dB, K, &beta, dC, M);
    if (st != CUBLAS_STATUS_SUCCESS) {
        printf("cublasSgemm failed: %d\n", (int)st);
        return 1;
    }
    cudaStreamSynchronize(stream);
    cudaDeviceSynchronize();

    err = cudaMemcpy(hC, dC, szC, cudaMemcpyDeviceToHost);
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
    printf("pmatmul puresimt OK (C = A*B fp16xfp16->fp32, %dx%dx%d)\n", M, N, K);

    cublasDestroy(handle);
    cudaStreamDestroy(stream);
    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    cudaDeviceReset();
    return 0;
}