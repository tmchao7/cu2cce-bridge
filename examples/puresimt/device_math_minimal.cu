/*
 * Diagnostic-only probe v5: PURE single-precision device-math verification
 * through the new execution-space keyword map.
 *
 * Round-5 server evidence: the math op-chain compiles fully when inlined;
 * this version moves the chain into a standalone helper spelled with
 * NVIDIA's "__device__", which now maps onto CANN's
 * __SIMT_DEVICE_FUNCTIONS_DECL__ selector via simt_adaptor.h. A DIAG-PASS
 * here therefore proves BOTH surfaces at once: single-precision math AND
 * the execution-space spelling bridge.
 *
 * Policy unchanged: a failing symbol names itself; nothing is faked, and
 * the harness records outcomes as DIAG-ONLY without blocking gates.
 */
#include <cstdio>
#include <cuda_runtime.h>

__device__ float probe_chain(float x) {
    /* one call site per probed symbol family */
    float v = expf(x);
    v += logf(v > 0.f ? v : 1.f);
    v += sqrtf(fabsf(v));
    v += sinf(v) + cosf(x);
    v += tanhf(v);
    v += powf(v * 0.25f + 1.f, 2.f);
    v += fminf(v, 10.f) + fmaxf(v, -10.f);
    return v;
}

__global__ void probe_kernel(float x, float *out) {
    *out = probe_chain(x);
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
