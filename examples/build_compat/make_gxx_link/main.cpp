/*
 * Carrier for the lib64 symlink family: this project links the FINAL
 * executable with a PLAIN host compiler (g++) and -lcudart — the link only
 * succeeds because make_cce's CUDA_DIR/lib64 provides libcudart.so ->
 * libcce_bridge_runtime.so (cu-bridge link.json pre_make parity).
 *
 * main.cpp is compiled by g++ WITH the bridge headers on the include path,
 * so cuda* calls are macro-mapped to wcuda* at compile time and the bridge
 * .so satisfies them at link time. The .cu is compiled through cucc.
 */
#include <cstdio>
#include <cuda_runtime.h>

extern "C" int run_scale(const float *in, float *out, int n);

int main(void) {
    const int N = 8;
    const float h_in[N] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f};
    float h_out[N] = {0.f};

    float *d_in = nullptr;
    float *d_out = nullptr;
    if (cudaMalloc((void **)&d_in, sizeof(h_in)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&d_out, sizeof(h_out)) != cudaSuccess) return 1;
    if (cudaMemcpy(d_in, h_in, sizeof(h_in), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (run_scale(d_in, d_out, N) != 0) return 1;
    if (cudaMemcpy(h_out, d_out, sizeof(h_out), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int i = 0; i < N; ++i) {
        if (h_out[i] != h_in[i] * 2.f) {
            printf("make_gxx_link FAILED at %d: got %f expected %f\n",
                   i, (double)h_out[i], (double)(h_in[i] * 2.f));
            return 1;
        }
    }
    cudaFree(d_in);
    cudaFree(d_out);
    printf("make_gxx_link OK (plain g++ link with -lcudart -> bridge runtime)\n");
    return 0;
}
