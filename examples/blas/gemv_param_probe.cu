/*
 * Diagnostic-only aclblasGemvEx parameter-matrix probe.
 *
 * Server evidence so far: with device scalars and lda=-1 (the convention
 * aclblasGemmEx requires for its reserved leading dimensions), GemvEx still
 * returns 100000 (INVALID_PARAM) — so its parameter contract differs from
 * GemmEx's and the public Huawei doc site renders no API content (SPA).
 *
 * This probe enumerates the plausible contract dimensions in ONE run:
 *   scalar location (host vs device) x leading dimension (real vs -1)
 *   x transpose spelling x compute precision.
 * Every attempt prints its combo and return code; nothing is retried
 * silently. The winning combo becomes the documented mapping in the
 * wcublasSgemv wrapper; a table of all-100000 means the contract lives in
 * another dimension (e.g. m/n meaning), which the printout will bound.
 * DIAGNOSTIC-ONLY: never part of required gates.
 */
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <acl/ops/acl_cblas.h>
#include <stdio.h>
#include <cstring>

#define M 8
#define N 8

static cudaStream_t g_stream = nullptr;

static int try_combo(const char *tag, int device_scalar, int lda, aclTransType trans,
                     aclComputeType compute, float *dA, float *dx, float *dy,
                     const float *hostAlpha, const float *hostBeta,
                     const void *devAlpha, const void *devBeta) {
    const void *a = device_scalar ? devAlpha : (const void *)hostAlpha;
    const void *b = device_scalar ? devBeta : (const void *)hostBeta;
    /* GemmEx 真机教训：昇腾计算接口要求非空 stream，空 stream 会把整个矩阵
     * 污染成 100000——这里统一传已创建的流。 */
    aclError ret = aclblasGemvEx(trans, M, N, a, dA, lda, ACL_FLOAT, dx, 1, ACL_FLOAT, b, dy, 1,
                                 ACL_FLOAT, compute, (aclrtStream)g_stream);
    printf("[probe] %-42s ret=%d\n", tag, (int)ret);
    return (int)ret;
}

int main(void) {
    float hA[M * N], hx[N], hy[M];
    for (int c = 0; c < N; ++c)
        for (int r = 0; r < M; ++r) hA[c * M + r] = (r == c) ? 1.0f : 0.0f;
    for (int i = 0; i < N; ++i) hx[i] = (float)(i + 1);
    for (int i = 0; i < M; ++i) hy[i] = -1.0f;

    float *dA = nullptr, *dx = nullptr, *dy = nullptr;
    void *devAlpha = nullptr, *devBeta = nullptr;
    if (cudaStreamCreate(&g_stream) != cudaSuccess) {
        printf("gemv_param_probe FAILED: stream\n");
        return 1;
    }
    if (cudaMalloc((void **)&dA, sizeof(hA)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dx, sizeof(hx)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dy, sizeof(hy)) != cudaSuccess) return 1;
    if (cudaMalloc(&devAlpha, sizeof(float)) != cudaSuccess) return 1;
    if (cudaMalloc(&devBeta, sizeof(float)) != cudaSuccess) return 1;
    if (cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dx, hx, sizeof(hx), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    float one = 1.0f, zero = 0.0f;
    if (cudaMemcpy(devAlpha, &one, sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(devBeta, &zero, sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    printf("===== aclblasGemvEx parameter matrix (M=N=%d, identity A) =====\n", M);
    try_combo("host scalar + lda=8      + N + HIGH", 0, 8, ACL_TRANS_N, ACL_COMPUTE_HIGH_PRECISION,
              dA, dx, dy, &one, &zero, devAlpha, devBeta);
    try_combo("host scalar + lda=-1     + N + HIGH", 0, -1, ACL_TRANS_N, ACL_COMPUTE_HIGH_PRECISION,
              dA, dx, dy, &one, &zero, devAlpha, devBeta);
    try_combo("dev  scalar + lda=8      + N + HIGH", 1, 8, ACL_TRANS_N, ACL_COMPUTE_HIGH_PRECISION,
              dA, dx, dy, &one, &zero, devAlpha, devBeta);
    try_combo("dev  scalar + lda=-1     + N + HIGH", 1, -1, ACL_TRANS_N, ACL_COMPUTE_HIGH_PRECISION,
              dA, dx, dy, &one, &zero, devAlpha, devBeta);
    try_combo("host scalar + lda=8      + T + HIGH", 0, 8, ACL_TRANS_T, ACL_COMPUTE_HIGH_PRECISION,
              dA, dx, dy, &one, &zero, devAlpha, devBeta);
    try_combo("host scalar + lda=8      + N + LOW ", 0, 8, ACL_TRANS_N, ACL_COMPUTE_LOW_PRECISION,
              dA, dx, dy, &one, &zero, devAlpha, devBeta);
    try_combo("host scalar + lda=m*n=64 + N + HIGH", 0, 64, ACL_TRANS_N, ACL_COMPUTE_HIGH_PRECISION,
              dA, dx, dy, &one, &zero, devAlpha, devBeta);

    printf("===== matrix done (ret=0 rows are viable contracts) =====\n");
    cudaFree(dA);
    cudaFree(dx);
    cudaFree(dy);
    cudaFree(devAlpha);
    cudaFree(devBeta);
    printf("gemv_param_probe OK (see table above)\n");
    return 0;
}
