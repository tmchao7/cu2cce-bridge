/*
 * Diagnostic-only carrier for the proactive FP64 type policy.
 *
 * Mentor guidance (2026-08-27): types the Ascend backend cannot run must be
 * reported by cucc itself with actionable "downgrade to float" advice, and
 * the user edits their code manually. This file deliberately uses device
 * double next to an equivalent float kernel:
 *
 *   CU2CCE_DEVICE_FP64_POLICY=error  -> build blocked BEFORE ccec with
 *                                       [type-policy] file:line listing;
 *   CU2CCE_DEVICE_FP64_POLICY=warn   -> same report, then ccec runs (and on
 *                                       current CANN rejects the double
 *                                       kernel downstream);
 *   allow                            -> scan skipped entirely.
 *
 * Note: the standalone __device__ spelling is unrecognized by the bisheng
 * SIMT front-end, so the float twin here lives entirely inside the
 * __global__ kernel body (helper-function spellings are tracked separately
 * by the inventory evidence step).
 *
 * The harness records this as DIAG-ONLY; it never enters required gates.
 */
#include <cstdio>
#include <cuda_runtime.h>

__global__ void ok_kernel(const float *in, float *out) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    out[i] = in[i] * 2.f;
}

__global__ void bad_double_kernel(const double *in, double *out) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    out[i] = in[i] * 2.0; /* intentional: triggers the FP64 type policy */
}

int main() {
    std::printf("double_policy_diag reached runtime (policy diagnostics are above)\n");
    return 0;
}
