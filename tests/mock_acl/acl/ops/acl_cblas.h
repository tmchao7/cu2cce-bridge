#ifndef ACL_CBLAS_MOCK
#define ACL_CBLAS_MOCK
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef void *aclblasHandle;
typedef int aclError;
typedef enum { ACL_TRANS_N=0, ACL_TRANS_T=1, ACL_TRANS_C=2 } aclTransType;
typedef enum { ACL_DT_FLOAT16=1, ACL_DT_FLOAT=0, ACL_DT_INT8=2, ACL_DT_INT32=3 } aclDataType;
typedef enum { ACL_COMPUTE_HIGH_PRECISION=0, ACL_COMPUTE_LOW_PRECISION=1 } aclComputeType;
#define ACL_ERROR_NONE 0
/* 与 acl_rt.h 的 aclrtStream 一致 */
typedef void *aclrtStream;

aclError aclblasCreate(aclblasHandle *handle);
aclError aclblasDestroy(aclblasHandle handle);
aclError aclblasSetStream(aclblasHandle handle, aclrtStream stream);
aclError aclblasGetStream(aclblasHandle handle, aclrtStream *stream);
aclError aclblasGemmEx(aclTransType transA, aclTransType transB, aclTransType transC,
                       int m, int n, int k, const void *alpha,
                       const void *matrixA, int lda, aclDataType dataTypeA,
                       const void *matrixB, int ldb, aclDataType dataTypeB,
                       const void *beta, void *matrixC, int ldc, aclDataType dataTypeC,
                       aclComputeType type, aclrtStream stream);
#ifdef __cplusplus
}
#endif
#endif
