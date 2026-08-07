#ifndef __CUBLAS_V2_WRAPPER_H__
#define __CUBLAS_V2_WRAPPER_H__

/*
 * cublas_v2 兼容层（Phase A4 最小子集）。
 *
 * 仿照 cu-bridge 的 include/bridge/blas/cublas_v2_wrapper.h：
 *   宏把 cublas 符号映射到 cu2cce 的 wcublas* wrapper / aclblas 原生符号。
 *
 * 关键差异：cu-bridge 直接 `#define cublasSgemm mcblasSgemm`（mxmaca 有同名 BLAS）。
 *   昇腾没有 cublasSgemm 同名接口，而是统一的 aclblasGemmEx（数据类型入参）——
 *   因此 cublasSgemm → wcublasSgemm（wrapper），由 wrapper 内部补类型/stream 调 aclblasGemmEx。
 */

#include <acl/ops/acl_cblas.h>
#include <acl/acl_base.h>

//------------ types ------------//
#define cublasStatus aclError
#define cublasStatus_t aclError
#define cublasHandle_t aclblasHandle
#define CUBLAS_STATUS_SUCCESS ACL_ERROR_NONE
#define CUBLAS_STATUS_NOT_INITIALIZED ACL_ERROR_UNINITIALIZE
#define CUBLAS_STATUS_ALLOC_FAILED ACL_ERROR_RT_MEMORY_ALLOCATION
#define CUBLAS_STATUS_INVALID_VALUE ACL_ERROR_INVALID_PARAM
#define CUBLAS_STATUS_ARCH_MISMATCH ACL_ERROR_FEATURE_UNSUPPORTED
#define CUBLAS_STATUS_MAPPING_ERROR ACL_ERROR_INVALID_PARAM
#define CUBLAS_STATUS_EXECUTION_FAILED ACL_ERROR_RT_INTERNAL_ERROR
#define CUBLAS_STATUS_INTERNAL_ERROR ACL_ERROR_RT_INTERNAL_ERROR

// trans 操作（cublasOperation_t 枚举值：N=0, T=1, C=2，与 aclTransType 一致）
#define cublasOperation_t aclTransType
#define CUBLAS_OP_N ACL_TRANS_N
#define CUBLAS_OP_T ACL_TRANS_T
#define CUBLAS_OP_C ACL_TRANS_C

//------------ function replace ------------//
#define cublasCreate wcublasCreate
#define cublasDestroy wcublasDestroy
#define cublasSetStream wcublasSetStream
#define cublasGetStream wcublasGetStream
#define cublasGetVersion wcublasGetVersion
#define cublasSgemm wcublasSgemm
#define cublasDgemm wcublasDgemm
/* BLAS1/BLAS2（昇腾 aclblas 是否有 FP32 变体待真机核对；当前声明占位返回 not-support） */
#define cublasSgemv wcublasSgemv
#define cublasSaxpy wcublasSaxpy
#define cublasScopy wcublasScopy
#define cublasSdot wcublasSdot
#define cublasSscal wcublasSscal
#define cublasSswap wcublasSswap
#define cublasSnrm2 wcublasSnrm2

#ifdef __cplusplus
extern "C" {
#endif

/* wcublas* 声明（签名保持 cublas 形状） */
aclError wcublasCreate(aclblasHandle *handle);
aclError wcublasDestroy(aclblasHandle handle);
aclError wcublasSetStream(aclblasHandle handle, aclrtStream stream);
aclError wcublasGetStream(aclblasHandle handle, aclrtStream *stream);
aclError wcublasGetVersion(int *version);

/* CUBLAS 的 Sgemm 形状：
 *   cublasSgemm(handle, transa, transb, m, n, k, &alpha, A, lda, B, ldb, &beta, C, ldc)
 * wrapper 内部转 aclblasGemmEx（补 dataType/transC/computeType/stream）。 */
aclError wcublasSgemm(aclblasHandle handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const float *alpha, const float *A, int lda, const float *B, int ldb,
                      const float *beta, float *C, int ldc);
aclError wcublasDgemm(aclblasHandle handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const double *alpha, const double *A, int lda, const double *B,
                      int ldb, const double *beta, double *C, int ldc);

/* BLAS1/BLAS2 占位（昇腾 aclblas FP32 变体待真机核对；若存在则替换为真实映射） */
aclError wcublasSgemv(aclblasHandle handle, aclTransType trans, int m, int n, const float *alpha,
                      const float *A, int lda, const float *x, int incx, const float *beta, float *y,
                      int incy);
aclError wcublasSaxpy(aclblasHandle handle, int n, const float *alpha, const float *x, int incx,
                      float *y, int incy);
aclError wcublasScopy(aclblasHandle handle, int n, const float *x, int incx, float *y, int incy);
aclError wcublasSdot(aclblasHandle handle, int n, const float *x, int incx, const float *y,
                     int incy, float *result);
aclError wcublasSscal(aclblasHandle handle, int n, const float *alpha, float *x, int incx);
aclError wcublasSswap(aclblasHandle handle, int n, float *x, int incx, float *y, int incy);
aclError wcublasSnrm2(aclblasHandle handle, int n, const float *x, int incx, float *result);

#ifdef __cplusplus
}
#endif

#endif //__CUBLAS_V2_WRAPPER_H__
