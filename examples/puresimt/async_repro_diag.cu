/*
 * Diagnostic-only minimal repro for the asyncAPI single-element increment
 * loss observed on bisheng SIMT (see 00_project_journey/PENDING_ISSUES_ASCEND.md).
 *
 * NVIDIA asyncAPI evidence: 26 back-to-back kernel launches, each +1 on
 * every element; final check finds EXACTLY ONE element one short, at a
 * drifting index, while every bridge-side call returns success.
 *
 * Two isolated variants pin down the condition:
 *   A) pure back-to-back launches, one final stream sync;
 *   B) asyncAPI-style: per-round event record + event-synchronize.
 * Each variant runs ROUNDS rounds over ZERO-initialized memory and reports
 * the mismatch count plus first mismatch details. Mismatch count == 0 is
 * the healthy outcome; a count of exactly 1 per run reproduces the field
 * issue in minimal form. DIAGNOSTIC-ONLY.
 */
#include <cstdio>
#include <cuda_runtime.h>

__global__ void inc_kernel(float *data, long long n) {
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) data[i] += 1.0f;
}

static float *g_buf = nullptr;
static long long g_n = 0;

// returns mismatch count; -1 on API failure
static int verify_all(int expect) {
    if (cudaStreamSynchronize(nullptr) != cudaSuccess) return -1;
    float *h = (float *)malloc(sizeof(float) * (size_t)g_n);
    if (h == nullptr) return -1;
    if (cudaMemcpy(h, g_buf, sizeof(float) * (size_t)g_n, cudaMemcpyDeviceToHost) != cudaSuccess) {
        free(h);
        return -1;
    }
    int bad = 0;
    long long first_bad = -1;
    float got = 0.f;
    for (long long i = 0; i < g_n; ++i) {
        if (h[i] != (float)expect) {
            if (bad == 0) {
                first_bad = i;
                got = h[i];
            }
            ++bad;
        }
    }
    if (bad > 0) {
        printf("    MISMATCH count=%d first_idx=%lld got=%f expect=%d\n", bad, first_bad, (double)got,
               expect);
    }
    free(h);
    return bad;
}

static int run_variant(const char *tag, int use_events, cudaStream_t stream, int rounds) {
    if (cudaMemset(g_buf, 0, sizeof(float) * (size_t)g_n) != cudaSuccess) return 1;
    cudaEvent_t ev = nullptr;
    if (use_events && cudaEventCreate(&ev) != cudaSuccess) return 1;
    const int THREADS = 256;
    unsigned blocks = (unsigned)((g_n + THREADS - 1) / THREADS);
    for (int r = 0; r < rounds; ++r) {
        inc_kernel<<<blocks, THREADS, 0, stream>>>(g_buf, g_n);
        if (use_events) {
            cudaEventRecord(ev, stream);
            cudaEventSynchronize(ev);
        }
    }
    int bad = verify_all(rounds);
    if (use_events && ev != nullptr) cudaEventDestroy(ev);
    printf("[async_repro] %s: mismatch=%d (expect 0)\n", tag, bad);
    return bad;
}

int main(void) {
    g_n = 1LL << 22; /* 4M elements, 16MB */
    if (cudaMalloc((void **)&g_buf, sizeof(float) * (size_t)g_n) != cudaSuccess) {
        printf("async_repro_diag FAILED: cudaMalloc\n");
        return 1;
    }
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) {
        printf("async_repro_diag FAILED: stream\n");
        return 1;
    }

    int badA = run_variant("variantA back-to-back", 0, stream, 26);
    int badB = run_variant("variantB event-per-round", 1, stream, 26);
    /* repeat the asyncAPI-style variant to expose drift across runs */
    int badB2 = run_variant("variantB event-per-round#2", 1, stream, 26);

    cudaStreamDestroy(stream);
    cudaFree(g_buf);
    cudaDeviceReset();
    if (badA == 0 && badB == 0 && badB2 == 0) {
        printf("async_repro_diag OK (no element loss in either variant)\n");
        return 0;
    }
    printf("async_repro_diag MISMATCH (A=%d B=%d B#2=%d) — backend-semantics evidence\n", badA, badB,
           badB2);
    return 1;
}
