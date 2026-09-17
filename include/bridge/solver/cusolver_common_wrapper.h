#ifndef __CU2CCE_PARITY_BRIDGE_SOLVER_CUSOLVER_COMMON_WRAPPER_H__
#define __CU2CCE_PARITY_BRIDGE_SOLVER_CUSOLVER_COMMON_WRAPPER_H__

/*
 * cusolver common types/constants — compile-time stub.
 *
 * cusolverDn/Mg/Sp/Rf are UNSUPPORTED on Ascend (no 1:1 CANN shim).
 * This header provides the CUDA type/enum surface so upstream source
 * code compiles; the wrapper functions return CUSOLVER_STATUS_NOT_SUPPORTED
 * at runtime.  See CLAUDE.md: "cusolver: unsupported, no 1:1 shim yet".
 */

#include <acl/acl_base.h>

/* cusolverStatus_t — CUDA uses a distinct enum; map to aclError so the
 * bridge error-handling path (ACL_ERROR_API_NOT_SUPPORT etc.) is reused. */
#define cusolverStatus_t aclError
#define cusolverStatus int

#define CUSOLVER_STATUS_SUCCESS ACL_SUCCESS
#define CUSOLVER_STATUS_NOT_INITIALIZED ACL_ERROR_UNINITIALIZE
#define CUSOLVER_STATUS_ALLOC_FAILED ACL_ERROR_RT_MEMORY_ALLOCATION
#define CUSOLVER_STATUS_INVALID_VALUE ACL_ERROR_INVALID_PARAM
#define CUSOLVER_STATUS_ARCH_MISMATCH ACL_ERROR_FEATURE_UNSUPPORTED
#define CUSOLVER_STATUS_MAPPING_ERROR ACL_ERROR_INVALID_PARAM
#define CUSOLVER_STATUS_EXECUTION_FAILED ACL_ERROR_RT_INTERNAL_ERROR
#define CUSOLVER_STATUS_INTERNAL_ERROR ACL_ERROR_RT_INTERNAL_ERROR
#define CUSOLVER_STATUS_MATRIX_TYPE_NOT_SUPPORTED ACL_ERROR_FEATURE_UNSUPPORTED
#define CUSOLVER_STATUS_NOT_SUPPORTED ACL_ERROR_API_NOT_SUPPORT
#define CUSOLVER_STATUS_ZERO_PIVOT ACL_ERROR_FAILURE
#define CUSOLVER_STATUS_INVALID_LICENSE ACL_ERROR_FAILURE
#define CUSOLVER_STATUS_IRS_NOT_SUPPORTED ACL_ERROR_FEATURE_UNSUPPORTED

/* cusolverEigMode_t — jobz selector for Syevd/gesvd family. */
typedef enum {
    CUSOLVER_EIG_MODE_NOVECTOR = 0,
    CUSOLVER_EIG_MODE_VECTOR = 1
} cusolverEigMode_t;

/* cusolverEigType_t — which eigenvalues (all / positive / negative). */
typedef enum {
    CUSOLVER_EIG_TYPE_1 = 1,
    CUSOLVER_EIG_TYPE_2 = 2,
    CUSOLVER_EIG_TYPE_3 = 3
} cusolverEigType_t;

/* cusolverNorm_t — norm selector used by gesvd. */
typedef enum {
    CUSOLVER_INF_NORM = 1,
    CUSOLVER_MAX_NORM = 2,
    CUSOLVER_ONE_NORM = 3,
    CUSOLVER_FRO_NORM = 4
} cusolverNorm_t;

/* cusolverIRSRefinement_t — iterative refinement selector. */
typedef enum {
    CUSOLVER_IRS_REFINE_NONE = 0,
    CUSOLVER_IRS_REFINE_CLASSICAL = 1,
    CUSOLVER_IRS_REFINE_GMRES = 2,
    CUSOLVER_IRS_REFINE_CLASSICAL_GMRES = 3
} cusolverIRSRefinement_t;

#endif /* __CU2CCE_PARITY_BRIDGE_SOLVER_CUSOLVER_COMMON_WRAPPER_H__ */
