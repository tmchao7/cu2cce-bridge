/*
 * Diagnostic-only aclblasGemvEx parameter-matrix probe, round 2.
 *
 * Round-2 evidence: all 7 parameter combos returned 100000 even though the
 * installed acl_cblas.h documents a textbook contract (m/n = rows/columns,
 * lda = leading dimension, alpha/beta scalars of dataTypeC). The one
 * systematic difference from the WORKING aclblasGemmEx path: gemm_impl
 * always runs aclopSetModelDir() (ATC-generated single-op model) first,
 * while neither the Sgemv wrapper nor this probe ever initialized the
 * operator-model environment.
 *
 * Round-3 probe therefore:
 *   1. sets the model dir from CU2CCE_GEMM_MODEL_DIR (same as GemmEx),
 *   2. re-runs the key Ex combos,
 *   3. adds the NATIVE aclblasHgemv (FP16) row — the header exposes
 *      specialized variants (Hgemv/S8gemv) which may have built-in ops
 *      even where the Ex generic path needs a model.
 * DIAGNOSTIC-ONLY: never part of required gates.
 */
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <acl/ops/acl_cblas.h>
#include <acl/acl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define M 8
#define N 8

/* float32 -> IEEE fp16 bits (probe-local, mirrors the wrapper converter). */
static unsigned short probe_f32_to_f16(float x) {
    unsigned int u;
    std::memcpy(&u, &x, sizeof(u));
    unsigned int sign = (u >> 16) & 0x8000u;
    int e = (int)((u >> 23) & 0xffu) - 127 + 15;
    unsigned int mant = u & 0x7fffffu;
    if (((u >> 23) & 0xffu) == 0xffu) return (unsigned short)(sign | 0x7c00u);
    if (e >= 31) return (unsigned short)(sign | 0x7c00u);
    if (e <= 0) {
        if (e < -10) return (unsigned short)sign;
        mant |= 0x800000u;
        unsigned int shift = (unsigned int)(14 - e);
        unsigned int h = mant >> shift;
        if ((mant >> (shift - 1)) & 1u) h++;
        return (unsigned short)(sign | h);
    }
    return (unsigned short)(sign | ((unsigned int)e << 10) | (mant >> 13));
}

static cudaStream_t g_stream = nullptr;

static void report(const char *tag, aclError ret) {
    const char *msg = aclGetRecentErrMsg();
    printf("[probe] %-40s ret=%d recent=%s\n", tag, (int)ret, msg ? msg : "(null)");
}

static int try_ex_combo(const char *tag, int device_scalar, int lda, aclComputeType compute,
                        float *dA, float *dx, float *dy, const float *hostAlpha,
                        const float *hostBeta, const void *devAlpha, const void *devBeta) {
    const void *a = device_scalar ? devAlpha : (const void *)hostAlpha;
    const void *b = device_scalar ? devBeta : (const void *)hostBeta;
    aclError ret = aclblasGemvEx(ACL_TRANS_N, M, N, a, dA, lda, ACL_FLOAT, dx, 1, ACL_FLOAT, b, dy,
                                 1, ACL_FLOAT, compute, (aclrtStream)g_stream);
    report(tag, ret);
    return (int)ret;
}

int main(void) {
    /* 0) operator-model environment — the working GemmEx path always sets it. */
    const char *model_dir = std::getenv("CU2CCE_GEMM_MODEL_DIR");
    if (model_dir != nullptr && model_dir[0] != '\0') {
        aclError r = aclopSetModelDir(model_dir);
        printf("[probe] aclopSetModelDir(%s) ret=%d\n", model_dir, (int)r);
    } else {
        printf("[probe] CU2CCE_GEMM_MODEL_DIR unset; built-in opp only\n");
    }

    /* fp32 fixtures */
    float hA[M * N], hx[N];
    for (int c = 0; c < N; ++c)
        for (int r = 0; r < M; ++r) hA[c * M + r] = (r == c) ? 1.0f : 0.0f;
    for (int i = 0; i < N; ++i) hx[i] = (float)(i + 1);
    float one = 1.0f, zero = 0.0f;

    float *dA = nullptr, *dx = nullptr, *dy = nullptr;
    void *devAlpha = nullptr, *devBeta = nullptr;
    if (cudaStreamCreate(&g_stream) != cudaSuccess) {
        printf("gemv_param_probe FAILED: stream\n");
        return 1;
    }
    if (cudaMalloc((void **)&dA, sizeof(hA)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dx, sizeof(hx)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dy, sizeof(float) * M) != cudaSuccess) return 1;
    if (cudaMalloc(&devAlpha, sizeof(float)) != cudaSuccess) return 1;
    if (cudaMalloc(&devBeta, sizeof(float)) != cudaSuccess) return 1;
    if (cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dx, hx, sizeof(hx), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(devAlpha, &one, sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(devBeta, &zero, sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    printf("===== Ex combos (model dir configured above) =====\n");
    try_ex_combo("Ex: dev  scalar + lda=8  + N + HIGH", 1, 8, ACL_COMPUTE_HIGH_PRECISION, dA, dx,
                 dy, &one, &zero, devAlpha, devBeta);
    try_ex_combo("Ex: host scalar + lda=8  + N + HIGH", 0, 8, ACL_COMPUTE_HIGH_PRECISION, dA, dx,
                 dy, &one, &zero, devAlpha, devBeta);
    try_ex_combo("Ex: host scalar + lda=-1 + N + HIGH", 0, -1, ACL_COMPUTE_HIGH_PRECISION, dA, dx,
                 dy, &one, &zero, devAlpha, devBeta);
    try_ex_combo("Ex: dev  scalar + lda=-1 + N + HIGH", 1, -1, ACL_COMPUTE_HIGH_PRECISION, dA, dx,
                 dy, &one, &zero, devAlpha, devBeta);

    printf("===== native aclblasHgemv row (FP16 specialization) =====\n");
    aclFloat16 h16A[M * N], h16x[N];
    aclFloat16 one16, zero16;
    unsigned short bits;
    bits = probe_f32_to_f16(1.0f);
    std::memcpy(&one16, &bits, sizeof(one16));
    bits = probe_f32_to_f16(0.0f);
    std::memcpy(&zero16, &bits, sizeof(zero16));
    for (int c = 0; c < N; ++c)
        for (int r = 0; r < M; ++r) {
            unsigned short v = probe_f32_to_f16((r == c) ? 1.0f : 0.0f);
            std::memcpy(&h16A[c * M + r], &v, sizeof(v));
        }
    for (int i = 0; i < N; ++i) {
        unsigned short v = probe_f32_to_f16((float)(i + 1));
        std::memcpy(&h16x[i], &v, sizeof(v));
    }
    aclFloat16 *d16A = nullptr, *d16x = nullptr, *d16y = nullptr;
    if (cudaMalloc((void **)&d16A, sizeof(h16A)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&d16x, sizeof(h16x)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&d16y, sizeof(h16x)) != cudaSuccess) return 1;
    if (cudaMemcpy(d16A, h16A, sizeof(h16A), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(d16x, h16x, sizeof(h16x), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    aclError hret = aclblasHgemv(ACL_TRANS_N, M, N, &one16, d16A, M, d16x, 1, &zero16, d16y, 1,
                                 ACL_COMPUTE_HIGH_PRECISION, (aclrtStream)g_stream);
    report("Hgemv: host scalar + lda=8 + N + HIGH", hret);

    printf("===== matrix done (ret=0 rows are viable contracts) =====\n");
    cudaFree(dA);
    cudaFree(dx);
    cudaFree(dy);
    cudaFree(devAlpha);
    cudaFree(devBeta);
    cudaFree(d16A);
    cudaFree(d16x);
    cudaFree(d16y);
    printf("gemv_param_probe OK (see table above)\n");
    return 0;
}
