#ifndef ACL_CBLAS_MOCK
#define ACL_CBLAS_MOCK
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef int aclError;
typedef void *aclopHandle;
/* 与真实 acl_cblas.h 一致：N=0,T=1,NZ=2,NZ_T=3（无 C） */
typedef enum { ACL_TRANS_N=0, ACL_TRANS_T=1, ACL_TRANS_NZ=2, ACL_TRANS_NZ_T=3 } aclTransType;
/* aclDataType 部分（真机 9.2.0 实测：ACL_FLOAT 等命名，无 ACL_DT_* 前缀；无 double） */
typedef enum { ACL_FLOAT16=1, ACL_FLOAT=0, ACL_INT8=2, ACL_INT32=3 } aclDataType;
typedef enum { ACL_COMPUTE_HIGH_PRECISION=0, ACL_COMPUTE_LOW_PRECISION=1 } aclComputeType;
#define ACL_ERROR_NONE 0
#define ACL_SUCCESS 0
#define ACL_ERROR_INVALID_PARAM 100000
#define ACL_ERROR_UNINITIALIZE 100001
#define ACL_ERROR_RT_MEMORY_ALLOCATION 207001
#define ACL_ERROR_RT_INTERNAL_ERROR 507000
#define ACL_ERROR_BAD_ALLOC 200000
/* 与 acl_rt.h 的 aclrtStream 一致 */
typedef void *aclrtStream;

/* 真实 acl_cblas.h 无 aclblasCreate/Destroy/SetStream/GetStream；
 * 只有 aclblasGemmEx 等计算接口 + aclblasCreateHandleFor*（handle 版）。 */
aclError aclblasGemmEx(aclTransType transA, aclTransType transB, aclTransType transC,
                       int m, int n, int k, const void *alpha,
                       const void *matrixA, int lda, aclDataType dataTypeA,
                       const void *matrixB, int ldb, aclDataType dataTypeB,
                       const void *beta, void *matrixC, int ldc, aclDataType dataTypeC,
                       aclComputeType type, aclrtStream stream);
aclError aclblasGemvEx(aclTransType transA, int m, int n, const void *alpha,
                       const void *a, int lda, aclDataType dataTypeA,
                       const void *x, int incx, aclDataType dataTypeX,
                       const void *beta, void *y, int incy, aclDataType dataTypeY,
                       aclComputeType type, aclrtStream stream);
aclError aclblasCreateHandleForGemmEx(aclTransType transA, aclTransType transB,
                                      aclTransType transC, int m, int n, int k,
                                      aclComputeType type, aclopHandle **handle);
#ifdef __cplusplus
}
#endif
#endif
