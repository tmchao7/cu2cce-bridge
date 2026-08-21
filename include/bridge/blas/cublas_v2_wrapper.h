#ifndef __CUBLAS_V2_WRAPPER_H__
#define __CUBLAS_V2_WRAPPER_H__

/*
 * cublas_v2 兼容层（Phase A4 最小子集）。
 *
 * 仿照 cu-bridge 的 include/bridge/blas/cublas_v2_wrapper.h：
 *   宏把 cublas 符号映射到 cu2cce 的 wcublas* wrapper / aclblas 原生符号。
 *
 * 关键差异（2026-08-07 对 torch_npu acl_cblas.h 核实）：
 *   - 昇腾 aclblasGemmEx 无 handle，直接传 aclrtStream；无 aclblasCreate/Destroy/SetStream。
 *   - 因此 cublas 的 handle 由 cu2cce 自定义结构 _wcceBlasHandle 承载（内部只存 stream）。
 *   - aclTransType 枚举为 N/T/NZ/NZ_T（无 C），CUBLAS_OP_C 映射到 ACL_TRANS_NZ。
 */

#include <acl/ops/acl_cblas.h>
#include <acl/acl_base.h>

//------------ types ------------//
#define cublasStatus aclError
#define cublasStatus_t aclError
#define CUBLAS_STATUS_SUCCESS ACL_SUCCESS
#define CUBLAS_STATUS_NOT_INITIALIZED ACL_ERROR_UNINITIALIZE
#define CUBLAS_STATUS_ALLOC_FAILED ACL_ERROR_RT_MEMORY_ALLOCATION
#define CUBLAS_STATUS_INVALID_VALUE ACL_ERROR_INVALID_PARAM
#define CUBLAS_STATUS_ARCH_MISMATCH ACL_ERROR_FEATURE_UNSUPPORTED
#define CUBLAS_STATUS_MAPPING_ERROR ACL_ERROR_INVALID_PARAM
#define CUBLAS_STATUS_EXECUTION_FAILED ACL_ERROR_RT_INTERNAL_ERROR
#define CUBLAS_STATUS_INTERNAL_ERROR ACL_ERROR_RT_INTERNAL_ERROR

// trans 操作（cublasOperation_t：N=0, T=1, C=2；aclTransType：N=0, T=1, NZ=2）
#define cublasOperation_t aclTransType
#define CUBLAS_OP_N ACL_TRANS_N
#define CUBLAS_OP_T ACL_TRANS_T
#define CUBLAS_OP_C ACL_TRANS_NZ /* acl 无 C(共轭转置)，映射到 NZ */

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

/* cu2cce 内部 cublas handle：昇腾无原生 cublasHandle，用结构承载关联的 stream。
 * 前端类型为不透明指针（对应 CUDA 的 cublasHandle_t）。 */
typedef struct _wcceBlasHandle {
    aclrtStream stream; /* 关联的 stream（昇腾 aclblas 计算接口直接传 stream） */
} _wcceBlasHandle;
typedef _wcceBlasHandle *cublasHandle_t;

#ifdef __cplusplus
extern "C" {
#endif

/* wcublas* 声明（签名保持 cublas 形状） */
aclError wcublasCreate(cublasHandle_t *handle);
aclError wcublasDestroy(cublasHandle_t handle);
aclError wcublasSetStream(cublasHandle_t handle, aclrtStream stream);
aclError wcublasGetStream(cublasHandle_t handle, aclrtStream *stream);
aclError wcublasGetVersion(int *version);

/* CUBLAS 的 Sgemm 形状：
 *   cublasSgemm(handle, transa, transb, m, n, k, &alpha, A, lda, B, ldb, &beta, C, ldc)
 * wrapper 内部转 aclblasGemmEx（补 dataType/transC/computeType/stream）。
 * 注意：cublas 调用方传的是 cudaMalloc 出的 Device 指针，float * 可与 aclblasGemmEx
 * 的 void\* 形参隐式转换；示例见 examples/cublasMatmul/matmul.cu。 */
aclError wcublasSgemm(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const float *alpha, const float *A, int lda, const float *B, int ldb,
                      const float *beta, float *C, int ldc);
aclError wcublasDgemm(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const double *alpha, const double *A, int lda, const double *B,
                      int ldb, const double *beta, double *C, int ldc);

/* BLAS1/BLAS2 占位（昇腾 aclblas 的 FP32 gemv 用 aclblasGemvEx，axpy 等待核对；当前占位） */
aclError wcublasSgemv(cublasHandle_t handle, aclTransType trans, int m, int n, const float *alpha,
                      const float *A, int lda, const float *x, int incx, const float *beta, float *y,
                      int incy);
aclError wcublasSaxpy(cublasHandle_t handle, int n, const float *alpha, const float *x, int incx,
                      float *y, int incy);
aclError wcublasScopy(cublasHandle_t handle, int n, const float *x, int incx, float *y, int incy);
aclError wcublasSdot(cublasHandle_t handle, int n, const float *x, int incx, const float *y,
                     int incy, float *result);
aclError wcublasSscal(cublasHandle_t handle, int n, const float *alpha, float *x, int incx);
aclError wcublasSswap(cublasHandle_t handle, int n, float *x, int incx, float *y, int incy);
aclError wcublasSnrm2(cublasHandle_t handle, int n, const float *x, int incx, float *result);

#ifdef __cplusplus
}
#endif

#endif //__CUBLAS_V2_WRAPPER_H__
