/* Minimal CUDA math/FMA regression for cu2cce puresimt. */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 8

__global__ void mathOperators(float *output) {
    int i = threadIdx.x;
    float value = 0.0f;
    if (i == 0) value = fmaf(2.0f, 3.0f, 4.0f);
    if (i == 1) value = sqrtf(4.0f);
    if (i == 2) value = sinf(0.0f);
    if (i == 3) value = cosf(0.0f);
    if (i == 4) value = expf(0.0f);
    if (i == 5) value = logf(1.0f);
    if (i == 6) value = floorf(3.75f);
    if (i == 7) value = ceilf(3.25f);
    output[i] = value;
}

int main(void) {
    float hOutput[N] = {0};
    float *dOutput = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dOutput, sizeof(hOutput)) != cudaSuccess) return 1;

    mathOperators<<<1, N, 0, stream>>>(dOutput);
    if (cudaGetLastError() != cudaSuccess || cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(hOutput, dOutput, sizeof(hOutput), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    const float expected[N] = {10.0f, 2.0f, 0.0f, 1.0f, 1.0f, 0.0f, 3.0f, 4.0f};
    for (int i = 0; i < N; ++i) {
        if (fabsf(hOutput[i] - expected[i]) > 1e-5f) {
            printf("math_minimal FAILED at %d: got %.6f expected %.6f\n", i, hOutput[i], expected[i]);
            return 1;
        }
    }

    cudaFree(dOutput); cudaStreamDestroy(stream); cudaDeviceReset();
    printf("math_minimal OK (fma sqrt sin cos exp log floor ceil)\n");
    return 0;
}
