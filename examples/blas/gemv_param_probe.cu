/*
 * Diagnostic-only aclblasGemvEx parameter-matrix probe, round 4 (DECISIVE).
 *
 * Round-4 evidence: ACL finally surfaced the real reason via
 * aclGetRecentErrMsg():
 *   Invalid_Argument(EH0001): Value lda = 8, ldb = 1, ldc = 1 for
 *   lda, ldb, ldc is invalid. Reason: only support -1.
 * The triple (lda, ldb, ldc) in that message maps to the GemvEx call's
 * (lda, incx, incy): ALL THREE reserved parameters must be -1 — the
 * backend only supports contiguous vectors. Round-2/3 failed because
 * incx=incy=1 were never -1; Hgemv hits the same validator.
 *
 * This probe runs the corrected contract (lda=-1, incx=-1, incy=-1) for
 * the Ex and Hgemv paths AND verifies numerics end-to-end (identity A:
 * y must equal x after stream sync). DIAGNOSTIC-ONLY.
 */
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <acl/ops/acl_cblas.h>
#include <acl/acl.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#define M 8
#define N 8

static cudaStream_t g_stream = nullptr;
static float *g_y = nullptr;

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

static unsigned short probe_f16_to_f16_bits(aclFloat16 v) {
    unsigned short bits;
    std::memcpy(&bits, &v, sizeof(bits));
    return bits;
}

static void report(const char *tag, aclError ret) {
    const char *msg = aclGetRecentErrMsg();
    printf("[probe] %-40s ret=%d recent=%s\n", tag, (int)ret, msg ? msg : "(null)");
}

static int verify_y(const char *tag) {
    float hy[M];
    if (cudaStreamSynchronize((cudaStream_t)g_stream) != cudaSuccess) {
        printf("[probe] %-40s VERIFY sync failed\n", tag);
        return 1;
    }
    if (cudaMemcpy(hy, g_y, sizeof(hy), cudaMemcpyDeviceToHost) != cudaSuccess) {
        printf("[probe] %-40s VERIFY d2h failed\n", tag);
        return 1;
    }
    for (int i = 0; i < M; ++i) {
        if (hy[i] != (float)(i + 1)) {
            printf("[probe] %-40s VERIFY FAILED at %d: got %f expected %d\n", tag, i,
                   (double)hy[i], i + 1);
            return 1;
        }
    }
    printf("[probe] %-40s VERIFY OK\n", tag);
    return 0;
}

int main(void) {
    const char *model_dir = std::getenv("CU2CCE_GEMM_MODEL_DIR");
    if (model_dir != nullptr && model_dir[0] != '\0') {
        aclError r = aclopSetModelDir(model_dir);
        printf("[probe] aclopSetModelDir(%s) ret=%d\n", model_dir, (int)r);
    } else {
        printf("[probe] CU2CCE_GEMM_MODEL_DIR unset; built-in opp only\n");
    }

    float hA[M * N], hx[N], one = 1.0f, zero = 0.0f;
    for (int c = 0; c < N; ++c)
        for (int r = 0; r < M; ++r) hA[c * M + r] = (r == c) ? 1.0f : 0.0f;
    for (int i = 0; i < N; ++i) hx[i] = (float)(i + 1);

    float *dA = nullptr, *dx = nullptr;
    if (cudaStreamCreate(&g_stream) != cudaSuccess) {
        printf("gemv_param_probe FAILED: stream\n");
        return 1;
    }
    if (cudaMalloc((void **)&dA, sizeof(hA)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dx, sizeof(hx)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&g_y, sizeof(float) * M) != cudaSuccess) return 1;
    if (cudaMemset(g_y, 0, sizeof(float) * M) != cudaSuccess) return 1;
    if (cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dx, hx, sizeof(hx), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    printf("===== corrected contract: lda=-1 incx=-1 incy=-1 =====\n");
    aclError ret = aclblasGemvEx(ACL_TRANS_N, M, N, &one, dA, -1, ACL_FLOAT, dx, -1, ACL_FLOAT,
                                 &zero, g_y, -1, ACL_FLOAT, ACL_COMPUTE_HIGH_PRECISION,
                                 (aclrtStream)g_stream);
    report("Ex: host scalar + (-1,-1,-1) + N", ret);
    if (ret == ACL_SUCCESS) {
        if (verify_y("Ex host-scalar numeric") != 0) {
            printf("gemv_param_probe FAILED: numeric mismatch\n");
            return 1;
        }
    }

    printf("===== native Hgemv with corrected contract =====\n");
    aclFloat16 h16A[M * N], h16x[N], one16, zero16;
    unsigned short bits = probe_f32_to_f16(1.0f);
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
    aclError hret = aclblasHgemv(ACL_TRANS_N, M, N, &one16, d16A, -1, d16x, -1, &zero16, d16y, -1,
                                 ACL_COMPUTE_HIGH_PRECISION, (aclrtStream)g_stream);
    report("Hgemv: host scalar + (-1,-1,-1) + N", hret);
    if (hret == ACL_SUCCESS) {
        /* fp16 identity A times fp16 x = fp16 x exactly for small integers */
        aclFloat16 hy16[M];
        if (cudaStreamSynchronize((cudaStream_t)g_stream) != cudaSuccess ||
            cudaMemcpy(hy16, d16y, sizeof(hy16), cudaMemcpyDeviceToHost) != cudaSuccess) {
            printf("gemv_param_probe FAILED: hgemv readback\n");
            return 1;
        }
        for (int i = 0; i < M; ++i) {
            if (probe_f16_to_f16_bits(hy16[i]) != probe_f32_to_f16((float)(i + 1))) {
                printf("gemv_param_probe FAILED: hgemv numeric mismatch at %d\n", i);
                return 1;
            }
        }
        printf("[probe] Hgemv numeric VERIFY OK\n");
    }

    printf("===== matrix done (ret=0 + VERIFY OK rows are the contract) =====\n");
    cudaFree(dA);
    cudaFree(dx);
    cudaFree(g_y);
    cudaFree(d16A);
    cudaFree(d16x);
    cudaFree(d16y);
    printf("gemv_param_probe OK (see table above)\n");
    return 0;
}
