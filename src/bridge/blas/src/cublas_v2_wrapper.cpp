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
 * 差异（2026-08-07 核实 torch_npu acl_cblas.h）：
 *   - aclblasGemmEx 无 handle、直接传 aclrtStream；
 *   - 无 aclblasCreate/Destroy/SetStream/GetStream；
 *   - 因此 cu2cce 自定义 cublasHandle_t（_wcceBlasHandle 结构）承载 stream。
 */

#include "cublas_v2_wrapper.h"

#include <cstdlib>

/* CUBLAS 的成功码与 ACL 相同（0）。handle 是 cu2cce 内部结构，存关联 stream。 */

aclError wcublasCreate(cublasHandle_t *handle) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    _wcceBlasHandle *h = (_wcceBlasHandle *)std::calloc(1, sizeof(_wcceBlasHandle));
    if (h == nullptr) {
        return ACL_ERROR_BAD_ALLOC;
    }
    h->stream = nullptr; /* 默认 stream（nullptr 由调用方后续 SetStream） */
    *handle = h;
    return ACL_SUCCESS;
}

aclError wcublasDestroy(cublasHandle_t handle) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    std::free(handle);
    return ACL_SUCCESS;
}

aclError wcublasSetStream(cublasHandle_t handle, aclrtStream stream) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    handle->stream = stream;
    return ACL_SUCCESS;
}

aclError wcublasGetStream(cublasHandle_t handle, aclrtStream *stream) {
    if (handle == nullptr || stream == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    *stream = handle->stream;
    return ACL_SUCCESS;
}

aclError wcublasGetVersion(int *version) {
    /* 昇腾无 cublasGetVersion 对应，返回 0（版本由 aclrtGetVersion 查） */
    if (version != nullptr) {
        *version = 0;
    }
    return ACL_SUCCESS;
}

/* 通用 GEMM 内部实现：s/d 共用，用 aclblasGemmEx */
static aclError gemm_impl(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m,
                          int n, int k, const void *alpha, const void *A, int lda,
                          aclDataType dtA, const void *B, int ldb, aclDataType dtB,
                          const void *beta, void *C, int ldc, aclDataType dtC) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    /* aclblasGemmEx 无 handle，直接传 stream（从 cu2cce handle 结构取）。
     * transC 昇腾仅支持 ACL_TRANS_N；computeType 用高精度。 */
    return aclblasGemmEx(transa, transb, ACL_TRANS_N, m, n, k, alpha, A, lda, dtA, B, ldb, dtB,
                         beta, C, ldc, dtC, ACL_COMPUTE_HIGH_PRECISION, handle->stream);
}

aclError wcublasSgemm(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const float *alpha, const float *A, int lda, const float *B, int ldb,
                      const float *beta, float *C, int ldc) {
    return gemm_impl(handle, transa, transb, m, n, k, alpha, A, lda, ACL_DT_FLOAT, B, ldb,
                     ACL_DT_FLOAT, beta, C, ldc, ACL_DT_FLOAT);
}

aclError wcublasDgemm(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const double *alpha, const double *A, int lda, const double *B,
                      int ldb, const double *beta, double *C, int ldc) {
    (void)handle; (void)transa; (void)transb; (void)m; (void)n; (void)k;
    (void)alpha; (void)A; (void)lda; (void)B; (void)ldb; (void)beta; (void)C; (void)ldc;
    /* ⚠️ 昇腾 aclblasGemmEx 不支持 double（aclDataType 无 ACL_DT_DOUBLE，原生多 FP16/FP32/int8）。
     * 真机上若需 double，可转 FP32 或确认 CANN 是否有 double 变体；当前返回"不支持"。 */
    return ACL_ERROR_API_NOT_SUPPORT;
}

/* ---------------- BLAS1/BLAS2 ----------------
 * aclblasGemvEx 存在（FP32 gemv，torch_cblas.h 确认）。Sgemv 真实映射；
 * axpy/copy/dot/scal/swap/nrm2 昇腾无对应，保持 not-support 占位。 */

static aclError not_supported_placeholder() { return ACL_ERROR_API_NOT_SUPPORT; }

aclError wcublasSgemv(cublasHandle_t handle, aclTransType trans, int m, int n, const float *alpha,
                      const float *A, int lda, const float *x, int incx, const float *beta,
                      float *y, int incy) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    /* aclblasGemvEx(transA, m, n, alpha, a, lda, dtA, x, incx, dtX, beta, y, incy, dtY, type, stream) */
    return aclblasGemvEx(trans, m, n, alpha, A, lda, ACL_DT_FLOAT, x, incx, ACL_DT_FLOAT, beta, y,
                         incy, ACL_DT_FLOAT, ACL_COMPUTE_HIGH_PRECISION, handle->stream);
}

aclError wcublasSaxpy(cublasHandle_t handle, int n, const float *alpha, const float *x, int incx,
                      float *y, int incy) {
    (void)handle; (void)n; (void)alpha; (void)x; (void)incx; (void)y; (void)incy;
    return not_supported_placeholder();
}

aclError wcublasScopy(cublasHandle_t handle, int n, const float *x, int incx, float *y, int incy) {
    (void)handle; (void)n; (void)x; (void)incx; (void)y; (void)incy;
    return not_supported_placeholder();
}

aclError wcublasSdot(cublasHandle_t handle, int n, const float *x, int incx, const float *y,
                     int incy, float *result) {
    (void)handle; (void)n; (void)x; (void)incx; (void)y; (void)incy; (void)result;
    return not_supported_placeholder();
}

aclError wcublasSscal(cublasHandle_t handle, int n, const float *alpha, float *x, int incx) {
    (void)handle; (void)n; (void)alpha; (void)x; (void)incx;
    return not_supported_placeholder();
}

aclError wcublasSswap(cublasHandle_t handle, int n, float *x, int incx, float *y, int incy) {
    (void)handle; (void)n; (void)x; (void)incx; (void)y; (void)incy;
    return not_supported_placeholder();
}

aclError wcublasSnrm2(cublasHandle_t handle, int n, const float *x, int incx, float *result) {
    (void)handle; (void)n; (void)x; (void)incx; (void)result;
    return not_supported_placeholder();
}
