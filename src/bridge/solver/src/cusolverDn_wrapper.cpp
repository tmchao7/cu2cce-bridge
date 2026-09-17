/*
 * cusolverDn stub implementations — UNSUPPORTED on Ascend.
 *
 * All functions return CUSOLVER_STATUS_NOT_SUPPORTED.  This file lets
 * upstream code that uses cusolverDn (LU, Cholesky, QR, SVD, Syevd) link
 * against the bridge runtime; the actual solver operations have no 1:1
 * CANN shim yet and must be reimplemented using CANN ops-math (aol)
 * dense-solver operators or a user-side alternative.
 *
 * See CLAUDE.md: "cusolver: unsupported -> CANN ops-math (aol)
 * dense-solver operators; no 1:1 shim yet".
 */

#include "cusolverDn_wrapper.h"

#include <acl/acl_base.h>

cusolverStatus_t wcusolverDnCreate(cusolverDnHandle_t *handle) {
    if (handle == nullptr) {
        return ACL_ERROR_INVALID_PARAM;
    }
    *handle = nullptr;
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnDestroy(cusolverDnHandle_t handle) {
    (void)handle;
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnSsyevd_bufferSize(
    cusolverDnHandle_t handle, cusolverEigMode_t jobz, cublasFillMode_t uplo,
    int n, const float *A, int lda, int *lwork) {
    (void)handle; (void)jobz; (void)uplo; (void)n; (void)A; (void)lda;
    if (lwork != nullptr) { *lwork = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnSsyevd(
    cusolverDnHandle_t handle, cusolverEigMode_t jobz, cublasFillMode_t uplo,
    int n, float *A, int lda, float *W, float *work, int lwork, int *info) {
    (void)handle; (void)jobz; (void)uplo; (void)n; (void)A; (void)lda;
    (void)W; (void)work; (void)lwork;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnDsyevd_bufferSize(
    cusolverDnHandle_t handle, cusolverEigMode_t jobz, cublasFillMode_t uplo,
    int n, const double *A, int lda, int *lwork) {
    (void)handle; (void)jobz; (void)uplo; (void)n; (void)A; (void)lda;
    if (lwork != nullptr) { *lwork = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnDsyevd(
    cusolverDnHandle_t handle, cusolverEigMode_t jobz, cublasFillMode_t uplo,
    int n, double *A, int lda, double *W, double *work, int lwork, int *info) {
    (void)handle; (void)jobz; (void)uplo; (void)n; (void)A; (void)lda;
    (void)W; (void)work; (void)lwork;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnSgetrf(
    cusolverDnHandle_t handle, int m, int n, float *A, int lda,
    float *work, int lwork, int *pivot, int *info) {
    (void)handle; (void)m; (void)n; (void)A; (void)lda;
    (void)work; (void)lwork; (void)pivot;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnSgetrs(
    cusolverDnHandle_t handle, cublasOperation_t trans, int n, int nrhs,
    const float *A, int lda, const int *pivot, float *B, int ldb, int *info) {
    (void)handle; (void)trans; (void)n; (void)nrhs; (void)A; (void)lda;
    (void)pivot; (void)B; (void)ldb;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnDgetrf(
    cusolverDnHandle_t handle, int m, int n, double *A, int lda,
    double *work, int lwork, int *pivot, int *info) {
    (void)handle; (void)m; (void)n; (void)A; (void)lda;
    (void)work; (void)lwork; (void)pivot;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnDgetrs(
    cusolverDnHandle_t handle, cublasOperation_t trans, int n, int nrhs,
    const double *A, int lda, const int *pivot, double *B, int ldb, int *info) {
    (void)handle; (void)trans; (void)n; (void)nrhs; (void)A; (void)lda;
    (void)pivot; (void)B; (void)ldb;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnSpotrf(
    cusolverDnHandle_t handle, cublasFillMode_t uplo, int n, float *A,
    int lda, int *info) {
    (void)handle; (void)uplo; (void)n; (void)A; (void)lda;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnSpotrs(
    cusolverDnHandle_t handle, cublasFillMode_t uplo, int n, int nrhs,
    const float *A, int lda, float *B, int ldb, int *info) {
    (void)handle; (void)uplo; (void)n; (void)nrhs; (void)A; (void)lda;
    (void)B; (void)ldb;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnDpotrf(
    cusolverDnHandle_t handle, cublasFillMode_t uplo, int n, double *A,
    int lda, int *info) {
    (void)handle; (void)uplo; (void)n; (void)A; (void)lda;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnDpotrs(
    cusolverDnHandle_t handle, cublasFillMode_t uplo, int n, int nrhs,
    const double *A, int lda, double *B, int ldb, int *info) {
    (void)handle; (void)uplo; (void)n; (void)nrhs; (void)A; (void)lda;
    (void)B; (void)ldb;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnSgeqrf(
    cusolverDnHandle_t handle, int m, int n, float *A, int lda,
    float *tau, float *work, int lwork, int *info) {
    (void)handle; (void)m; (void)n; (void)A; (void)lda;
    (void)tau; (void)work; (void)lwork;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnDgeqrf(
    cusolverDnHandle_t handle, int m, int n, double *A, int lda,
    double *tau, double *work, int lwork, int *info) {
    (void)handle; (void)m; (void)n; (void)A; (void)lda;
    (void)tau; (void)work; (void)lwork;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnSgesvd(
    cusolverDnHandle_t handle, char jobu, char jobvt, int m, int n,
    float *A, int lda, float *S, float *U, int ldu, float *VT, int ldvt,
    float *work, int lwork, float *rwork, int *info) {
    (void)handle; (void)jobu; (void)jobvt; (void)m; (void)n; (void)A; (void)lda;
    (void)S; (void)U; (void)ldu; (void)VT; (void)ldvt; (void)work; (void)lwork;
    (void)rwork;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}

cusolverStatus_t wcusolverDnDgesvd(
    cusolverDnHandle_t handle, char jobu, char jobvt, int m, int n,
    double *A, int lda, double *S, double *U, int ldu, double *VT, int ldvt,
    double *work, int lwork, double *rwork, int *info) {
    (void)handle; (void)jobu; (void)jobvt; (void)m; (void)n; (void)A; (void)lda;
    (void)S; (void)U; (void)ldu; (void)VT; (void)ldvt; (void)work; (void)lwork;
    (void)rwork;
    if (info != nullptr) { *info = 0; }
    return CUSOLVER_STATUS_NOT_SUPPORTED;
}
