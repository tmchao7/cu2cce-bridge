/*
 * wcublas* wrapper 实现（Phase A4 最小子集）。
 *
 * 仿照 cu-bridge 的 src/bridge/blas/src/cublas_v2_wrapper.cpp，目标从 mcblas 换成 aclblas。
 *
 * ⚠️ 本文件在装有 CANN 的真机上编译（链接 libacl_cblas.so），本机仅作语法验证。
 *
 * 关键转换：cublasSgemm → aclblasGemmEx
 *   cublasSgemm(handle, transa, transb, m, n, k, &alpha, A, lda, B, ldb, &beta, C, ldc)
 *   → aclblasGemmEx(transa, transb, ACL_TRANS_N, m, n, k,
 *                   &alpha, A, lda, ACL_DT_FLOAT, B, ldb, ACL_DT_FLOAT,
 *                   &beta, C, ldc, ACL_DT_FLOAT, computeType, stream)
 * 差异：昇腾的 stream 从 handle 关联（aclblasGetStream 取），CUDA 的 stream 是显式参数。
 */

#include "cublas_v2_wrapper.h"

/* CUBLAS 的成功码与 ACL 相同（0）。handle 保存 stream 由 aclblas 内部维护。 */

aclError wcublasCreate(aclblasHandle *handle) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    return aclblasCreate(handle);
}

aclError wcublasDestroy(aclblasHandle handle) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    return aclblasDestroy(handle);
}

aclError wcublasSetStream(aclblasHandle handle, aclrtStream stream) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    return aclblasSetStream(handle, stream);
}

aclError wcublasGetStream(aclblasHandle handle, aclrtStream *stream) {
    if (handle == nullptr || stream == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    return aclblasGetStream(handle, stream);
}

aclError wcublasGetVersion(int *version) {
    /* 昇腾无 cublasGetVersion 对应，返回 0（版本由 aclrtGetVersion 查） */
    if (version != nullptr) {
        *version = 0;
    }
    return ACL_ERROR_NONE;
}

/* 通用 GEMM 内部实现：s/d 共用，用 aclblasGemmEx */
static aclError gemm_impl(aclblasHandle handle, aclTransType transa, aclTransType transb, int m,
                          int n, int k, const void *alpha, const void *A, int lda,
                          aclDataType dtA, const void *B, int ldb, aclDataType dtB,
                          const void *beta, void *C, int ldc, aclDataType dtC) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    aclrtStream stream = nullptr;
    aclError ret = aclblasGetStream(handle, &stream);
    if (ret != ACL_ERROR_NONE) {
        return ret;
    }
    /* transC 昇腾仅支持 ACL_TRANS_N；computeType 用高精度 */
    return aclblasGemmEx(transa, transb, ACL_TRANS_N, m, n, k, alpha, A, lda, dtA, B, ldb, dtB,
                         beta, C, ldc, dtC, ACL_COMPUTE_HIGH_PRECISION, stream);
}

aclError wcublasSgemm(aclblasHandle handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const float *alpha, const float *A, int lda, const float *B, int ldb,
                      const float *beta, float *C, int ldc) {
    return gemm_impl(handle, transa, transb, m, n, k, alpha, A, lda, ACL_DT_FLOAT, B, ldb,
                     ACL_DT_FLOAT, beta, C, ldc, ACL_DT_FLOAT);
}

aclError wcublasDgemm(aclblasHandle handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const double *alpha, const double *A, int lda, const double *B,
                      int ldb, const double *beta, double *C, int ldc) {
    (void)handle; (void)transa; (void)transb; (void)m; (void)n; (void)k;
    (void)alpha; (void)A; (void)lda; (void)B; (void)ldb; (void)beta; (void)C; (void)ldc;
    /* ⚠️ 昇腾 aclblasGemmEx 不支持 double（aclDataType 无 ACL_DT_DOUBLE，原生多 FP16/FP32/int8）。
     * 真机上若需 double，可转 FP32 或确认 CANN 是否有 double 变体；当前返回"不支持"。 */
    return ACL_ERROR_API_NOT_SUPPORT;
}
