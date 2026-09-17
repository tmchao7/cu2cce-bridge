#ifndef __CU2CCE_PARITY_BRIDGE_SOLVER_CUSOLVERDN_WRAPPER_H__
#define __CU2CCE_PARITY_BRIDGE_SOLVER_CUSOLVERDN_WRAPPER_H__

/*
 * cusolverDn compile-time stub — UNSUPPORTED on Ascend.
 *
 * Provides the CUDA cusolverDn API surface (handle + common dense-solver
 * function declarations) so upstream source code compiles.  All functions
 * return CUSOLVER_STATUS_NOT_SUPPORTED at runtime; there is no 1:1 CANN
 * shim for dense LU/Cholesky/QR/SVD/Syevd yet.
 *
 * See CLAUDE.md: "cusolver: unsupported -> CANN ops-math (aol) dense-solver
 * operators; no 1:1 shim yet".
 */

#include "cusolver_common_wrapper.h"
#include "../blas/cublas_v2_wrapper.h"  /* cublasFillMode_t, cublasOperation_t */

/* Opaque handle — mirrors CUDA's cusolverDnHandle_t. */
typedef struct _wcceSolverDnHandle *cusolverDnHandle_t;

/* cusolverDnCreate / Destroy are the only non-solver lifecycle functions. */
#define cusolverDnCreate wcusolverDnCreate
#define cusolverDnDestroy wcusolverDnDestroy

/* Syevd (symmetric eigensolver) — the PCA_on_GPU path. */
#define cusolverDnSsyevd_bufferSize wcusolverDnSsyevd_bufferSize
#define cusolverDnSsyevd wcusolverDnSsyevd
#define cusolverDnDsyevd_bufferSize wcusolverDnDsyevd_bufferSize
#define cusolverDnDsyevd wcusolverDnDsyevd

/* LU (getrf/getrs) — common dense linear solve. */
#define cusolverDnSgetrf wcusolverDnSgetrf
#define cusolverDnSgetrs wcusolverDnSgetrs
#define cusolverDnDgetrf wcusolverDnDgetrf
#define cusolverDnDgetrs wcusolverDnDgetrs

/* Cholesky (potrf/potrs). */
#define cusolverDnSpotrf wcusolverDnSpotrf
#define cusolverDnSpotrs wcusolverDnSpotrs
#define cusolverDnDpotrf wcusolverDnDpotrf
#define cusolverDnDpotrs wcusolverDnDpotrs

/* QR (geqrf). */
#define cusolverDnSgeqrf wcusolverDnSgeqrf
#define cusolverDnDgeqrf wcusolverDnDgeqrf

/* SVD (gesvd). */
#define cusolverDnSgesvd wcusolverDnSgesvd
#define cusolverDnDgesvd wcusolverDnDgesvd

#ifdef __cplusplus
extern "C" {
#endif

/* Lifecycle. */
cusolverStatus_t wcusolverDnCreate(cusolverDnHandle_t *handle);
cusolverStatus_t wcusolverDnDestroy(cusolverDnHandle_t handle);

/* Syevd — single precision. */
cusolverStatus_t wcusolverDnSsyevd_bufferSize(
    cusolverDnHandle_t handle, cusolverEigMode_t jobz, cublasFillMode_t uplo,
    int n, const float *A, int lda, const float *W, int *lwork);
cusolverStatus_t wcusolverDnSsyevd(
    cusolverDnHandle_t handle, cusolverEigMode_t jobz, cublasFillMode_t uplo,
    int n, float *A, int lda, float *W, float *work, int lwork, int *info);

/* Syevd — double precision (Ascend has no FP64; returns NOT_SUPPORTED). */
cusolverStatus_t wcusolverDnDsyevd_bufferSize(
    cusolverDnHandle_t handle, cusolverEigMode_t jobz, cublasFillMode_t uplo,
    int n, const double *A, int lda, const double *W, int *lwork);
cusolverStatus_t wcusolverDnDsyevd(
    cusolverDnHandle_t handle, cusolverEigMode_t jobz, cublasFillMode_t uplo,
    int n, double *A, int lda, double *W, double *work, int lwork, int *info);

/* LU — single precision. */
cusolverStatus_t wcusolverDnSgetrf(
    cusolverDnHandle_t handle, int m, int n, float *A, int lda,
    float *work, int lwork, int *pivot, int *info);
cusolverStatus_t wcusolverDnSgetrs(
    cusolverDnHandle_t handle, cublasOperation_t trans, int n, int nrhs,
    const float *A, int lda, const int *pivot, float *B, int ldb, int *info);

/* LU — double precision. */
cusolverStatus_t wcusolverDnDgetrf(
    cusolverDnHandle_t handle, int m, int n, double *A, int lda,
    double *work, int lwork, int *pivot, int *info);
cusolverStatus_t wcusolverDnDgetrs(
    cusolverDnHandle_t handle, cublasOperation_t trans, int n, int nrhs,
    const double *A, int lda, const int *pivot, double *B, int ldb, int *info);

/* Cholesky — single precision. */
cusolverStatus_t wcusolverDnSpotrf(
    cusolverDnHandle_t handle, cublasFillMode_t uplo, int n, float *A,
    int lda, int *info);
cusolverStatus_t wcusolverDnSpotrs(
    cusolverDnHandle_t handle, cublasFillMode_t uplo, int n, int nrhs,
    const float *A, int lda, float *B, int ldb, int *info);

/* Cholesky — double precision. */
cusolverStatus_t wcusolverDnDpotrf(
    cusolverDnHandle_t handle, cublasFillMode_t uplo, int n, double *A,
    int lda, int *info);
cusolverStatus_t wcusolverDnDpotrs(
    cusolverDnHandle_t handle, cublasFillMode_t uplo, int n, int nrhs,
    const double *A, int lda, double *B, int ldb, int *info);

/* QR. */
cusolverStatus_t wcusolverDnSgeqrf(
    cusolverDnHandle_t handle, int m, int n, float *A, int lda,
    float *tau, float *work, int lwork, int *info);
cusolverStatus_t wcusolverDnDgeqrf(
    cusolverDnHandle_t handle, int m, int n, double *A, int lda,
    double *tau, double *work, int lwork, int *info);

/* SVD. */
cusolverStatus_t wcusolverDnSgesvd(
    cusolverDnHandle_t handle, char jobu, char jobvt, int m, int n,
    float *A, int lda, float *S, float *U, int ldu, float *VT, int ldvt,
    float *work, int lwork, float *rwork, int *info);
cusolverStatus_t wcusolverDnDgesvd(
    cusolverDnHandle_t handle, char jobu, char jobvt, int m, int n,
    double *A, int lda, double *S, double *U, int ldu, double *VT, int ldvt,
    double *work, int lwork, double *rwork, int *info);

#ifdef __cplusplus
}
#endif

#endif /* __CU2CCE_PARITY_BRIDGE_SOLVER_CUSOLVERDN_WRAPPER_H__ */
