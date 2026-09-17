#ifndef __CUBLAS_V2_WRAPPER_H__
#define __CUBLAS_V2_WRAPPER_H__

/*
 * EXPERIMENTAL secondary capability: cublas_v2 → acl_cblas.
 * It is independent from the main/puresimt kernel path.
 *
 * cublas_v2 兼容层（最小子集）。
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

// fill mode（cublasFillMode_t：NVIDIA 值 0=L, 1=U, 2=F）
typedef enum {
    CUBLAS_FILL_MODE_LOWER = 0,
    CUBLAS_FILL_MODE_UPPER = 1,
    CUBLAS_FILL_MODE_FULL = 2
} cublasFillMode_t;

//------------ GemmEx dtype/compute 常量（值与 NVIDIA library_types.h 对齐） ------------//
#if !defined(CUDA_R_16F)
typedef enum {
    CUDA_R_16F = 150,
    CUDA_R_32F = 151,
    CUDA_R_64F = 152,
    CUDA_R_8I = 160,
    CUDA_R_32I = 170
} cudaDataType;
#endif

/* cublasComputeType_t 的 NVIDIA 值（_16F=64, _32F=68）。桥只接受 32F 族并
 * 映射到 ACL_COMPUTE_HIGH_PRECISION；16F/其它诚实返回 not-support。 */
#if !defined(CUBLAS_COMPUTE_32F)
typedef unsigned long long cublasComputeType_t;
#define CUBLAS_COMPUTE_16F 64ULL
#define CUBLAS_COMPUTE_32F 68ULL
#endif

//------------ 模式类 API（无 ACL 对应；wcublas 层存取 + 按语义落实） ------------//
typedef enum {
    CUBLAS_POINTER_MODE_HOST = 0,
    CUBLAS_POINTER_MODE_DEVICE = 1
} cublasPointerMode_t;

typedef enum {
    CUBLAS_DEFAULT_MATH = 0,
    CUBLAS_TENSOR_OP_MATH = 1,
    CUBLAS_PEDANTIC_MATH = 2
} cublasMath_t;

typedef enum {
    CUBLAS_ATOMICS_NOT_ALLOWED = 0,
    CUBLAS_ATOMICS_ALLOWED = 1
} cublasAtomicsMode_t;

//------------ function replace ------------//
#define cublasCreate wcublasCreate
#define cublasDestroy wcublasDestroy
#define cublasSetStream wcublasSetStream
#define cublasGetStream wcublasGetStream
#define cublasGetVersion wcublasGetVersion
#define cublasSetMathMode wcublasSetMathMode
#define cublasGetMathMode wcublasGetMathMode
#define cublasSetPointerMode wcublasSetPointerMode
#define cublasGetPointerMode wcublasGetPointerMode
#define cublasSetAtomicsMode wcublasSetAtomicsMode
#define cublasGetAtomicsMode wcublasGetAtomicsMode
#define cublasSgemm wcublasSgemm
#define cublasDgemm wcublasDgemm
#define cublasGemmEx wcublasGemmEx
#define cublasSgemmEx wcublasSgemmEx
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
    cublasPointerMode_t pointerMode; /* HOST(默认)：GemmEx 把 host scalar staging 到 device */
    cublasMath_t mathMode;           /* 存取透传：性能提示语义，桥不据此改变数值行为 */
    cublasAtomicsMode_t atomicsMode; /* 存取透传：昇腾无对应调度开关 */
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

/* 模式类：PointerMode 会被 GemmEx 真实落实（HOST→staging device scalar；
 * DEVICE→直接透传）；MathMode/AtomicsMode 存取透传（性能提示语义），
 * 越界枚举值一律拒绝。 */
aclError wcublasSetMathMode(cublasHandle_t handle, cublasMath_t mode);
aclError wcublasGetMathMode(cublasHandle_t handle, cublasMath_t *mode);
aclError wcublasSetPointerMode(cublasHandle_t handle, cublasPointerMode_t mode);
aclError wcublasGetPointerMode(cublasHandle_t handle, cublasPointerMode_t *mode);
aclError wcublasSetAtomicsMode(cublasHandle_t handle, cublasAtomicsMode_t mode);
aclError wcublasGetAtomicsMode(cublasHandle_t handle, cublasAtomicsMode_t *mode);

/* CUBLAS 的 Sgemm 形状：
 *   cublasSgemm(handle, transa, transb, m, n, k, &alpha, A, lda, B, ldb, &beta, C, ldc)
 * wrapper 内部转 aclblasGemmEx（补 dataType/transC/computeType/stream）。
 * 注意：cublas 调用方传的是 cudaMalloc 出的 Device 指针，float * 可与 aclblasGemmEx
 * 的 void\* 形参隐式转换；puresimt 主线不依赖此 experimental wrapper。
 * ⚠️ 昇腾计算接口要求非空 aclrtStream：wcublasSgemm 在 handle->stream 为空时，
 *    内部自动先 aclrtCreateStream 再调用 aclblasGemmEx（见 wrapper 实现 2026-08-21）。 */
aclError wcublasSgemm(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const float *alpha, const float *A, int lda, const float *B, int ldb,
                      const float *beta, float *C, int ldc);
aclError wcublasDgemm(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m, int n,
                      int k, const double *alpha, const double *A, int lda, const double *B,
                      int ldb, const double *beta, double *C, int ldc);

/* cublasGemmEx：dtype 显式入口。支持组合（真机 acl_cblas.h 界定）：
 *   (16F,16F,16F) / (16F,16F,32F) / (8I,8I,32I)；其它组合诚实 NOT_SUPPORT。
 * compute 只接受 CUBLAS_COMPUTE_32F / CUDA_R_32F（→ACL_COMPUTE_HIGH_PRECISION）。
 * alpha/beta 语义按 PointerMode：HOST（默认）staging 到 device；DEVICE 直接透传。 */
aclError wcublasGemmEx(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m,
                       int n, int k, const void *alpha, const void *A, cudaDataType Atype, int lda,
                       const void *B, cudaDataType Btype, int ldb, const void *beta, void *C,
                       cudaDataType Ctype, int ldc, cublasComputeType_t computeType);
/* cublasSgemmEx：GemmEx 的 32F compute 固定形态。 */
aclError wcublasSgemmEx(cublasHandle_t handle, aclTransType transa, aclTransType transb, int m,
                        int n, int k, const void *alpha, const void *A, cudaDataType Atype, int lda,
                        const void *B, cudaDataType Btype, int ldb, const void *beta, void *C,
                        cudaDataType Ctype, int ldc);

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
