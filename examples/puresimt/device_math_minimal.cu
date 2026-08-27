/*
 * Diagnostic-only probe v4: PURE single-precision device-math verification.
 *
 * Policy: a missing symbol fails THIS carrier's compile with the symbol
 * name in the error text — that error IS the inventory. Nothing here is
 * faked; the harness records the outcome as DIAG-ONLY without blocking.
 *
 * Correctness model: the device kernel computes a fixed op-chain inline and
 * the host recomputes the same chain with libm; values must agree closely.
 *
 * Platform facts recorded by earlier probe rounds (2026-08-27):
 * - bisheng device side has NO double support at all; even doubleN vector
 *   types do not exist. Everything below stays float32 on both sides.
 * - The standalone spelling "__device__" is NOT recognized by the bisheng
 *   SIMT front-end ("unknown type name"), unlike __global__. Until the
 *   canonical device-helper spelling is proven via the extended inventory,
 *   this probe inlines all logic into the __global__ kernel body and calls
 *   no qualified helper functions.
 */
#include <cstdio>
#include <cuda_runtime.h>

__global__ void probe_kernel(float x, float *out) {
    /* one call site per probed symbol family */
    float v = expf(x);
    v += logf(v > 0.f ? v : 1.f);
    v += sqrtf(fabsf(v));
    v += sinf(v) + cosf(x);
    v += tanhf(v);
    v += powf(v * 0.25f + 1.f, 2.f);
    v += fminf(v, 10.f) + fmaxf(v, -10.f);
    *out = v;
}

static int close_enough(float got, float ref) {
    float diff = got - ref;
    if (diff < 0.f) diff = -diff;
    float tol = (ref < 0.f ? -ref : ref) * 2e-3f;
    if (tol < 1e-3f) tol = 1e-3f;
    return diff <= tol;
}

int main() {
    const float x = 0.75f;

    /* host reference recomputes the identical chain with libm */
    float ref = expf(x);
    ref += logf(ref > 0.f ? ref : 1.f);
    ref += sqrtf(fabsf(ref));
    ref += sinf(ref) + cosf(x);
    ref += tanhf(ref);
    ref += powf(ref * 0.25f + 1.f, 2.f);
    ref += fminf(ref, 10.f) + fmaxf(ref, -10.f);

    float got_h = 0.f;
    float *got_d = nullptr;
    if (cudaMalloc((void **)&got_d, sizeof(float)) != cudaSuccess || got_d == nullptr) {
        std::printf("cudaMalloc failed\n");
        return 1;
    }
    if (cudaMemset(got_d, 0, sizeof(float)) != cudaSuccess) {
        std::printf("cudaMemset failed\n");
        cudaFree(got_d);
        return 1;
    }
    probe_kernel<<<1, 32>>>(x, got_d);
    if (cudaDeviceSynchronize() != cudaSuccess) {
        std::printf("kernel launch failed\n");
        cudaFree(got_d);
        return 1;
    }
    if (cudaMemcpy(&got_h, got_d, sizeof(float), cudaMemcpyDeviceToHost) != cudaSuccess) {
        std::printf("cudaMemcpy failed\n");
        cudaFree(got_d);
        return 1;
    }
    cudaFree(got_d);

    if (!close_enough(got_h, ref)) {
        std::printf("MISMATCH device=%f host_ref=%f\n", (double)got_h, (double)ref);
        return 1;
    }

    std::printf("device_math_minimal OK (single-precision device math chain verified)\n");
    return 0;
}
