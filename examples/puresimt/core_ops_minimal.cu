/*
 * Minimal CUDA core-operator coverage for cu2cce puresimt.
 *
 * This test is intentionally small and self-contained. It covers the scalar
 * completeness basis in one kernel: arithmetic, bitwise logic, comparison /
 * select, int<->float conversion, global load/store, local temporaries, and
 * shared-memory load/store with a barrier.
 */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 256
#define THREADS N

__global__ void coreOperators(const int *input, int *arithmetic, int *logic,
                              int *compare, int *convert, int *memory) {
    __shared__ int shared[THREADS];
    int tid = threadIdx.x;
    int value = input[tid];

    /* Global load -> shared store -> barrier -> shared load -> global store. */
    shared[tid] = value;
    __syncthreads();
    int loaded = shared[tid];

    /* Integer arithmetic: add, subtract, multiply, divide. */
    int add = loaded + 3;
    int sub = add - 1;
    int mul = sub * 2;
    arithmetic[tid] = mul / 2;

    /* Bitwise logic: AND, OR, XOR, NOT, and shifts. */
    unsigned bits = (unsigned)loaded;
    unsigned logicValue = ((bits & 0x0fu) | 0x20u) ^ 0x03u;
    logicValue ^= (~bits) & 0x01u;
    logic[tid] = (int)((logicValue << 1) >> 1);

    /* Comparison and conditional select. */
    compare[tid] = (loaded == 0) ? 10 : ((loaded < 128) ? 20 : 30);

    /* int -> float -> int conversion. */
    float asFloat = (float)loaded + 0.5f;
    convert[tid] = (int)asFloat;

    /* Local arithmetic result plus shared-memory result. */
    memory[tid] = loaded + shared[(tid + 1) & (THREADS - 1)];
}

int main(void) {
    int hInput[N], hArithmetic[N], hLogic[N], hCompare[N], hConvert[N], hMemory[N];
    for (int i = 0; i < N; ++i) {
        hInput[i] = i;
        hArithmetic[i] = hLogic[i] = hCompare[i] = hConvert[i] = hMemory[i] = 0;
    }

    int *dInput = nullptr, *dArithmetic = nullptr, *dLogic = nullptr;
    int *dCompare = nullptr, *dConvert = nullptr, *dMemory = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dInput, sizeof(hInput)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dArithmetic, sizeof(hArithmetic)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dLogic, sizeof(hLogic)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dCompare, sizeof(hCompare)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dConvert, sizeof(hConvert)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dMemory, sizeof(hMemory)) != cudaSuccess) return 1;
    if (cudaMemcpy(dInput, hInput, sizeof(hInput), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    coreOperators<<<1, THREADS, 0, stream>>>(dInput, dArithmetic, dLogic, dCompare, dConvert, dMemory);
    if (cudaGetLastError() != cudaSuccess || cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(hArithmetic, dArithmetic, sizeof(hArithmetic), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (cudaMemcpy(hLogic, dLogic, sizeof(hLogic), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (cudaMemcpy(hCompare, dCompare, sizeof(hCompare), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (cudaMemcpy(hConvert, dConvert, sizeof(hConvert), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (cudaMemcpy(hMemory, dMemory, sizeof(hMemory), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int i = 0; i < N; ++i) {
        int expectedLogic = (int)((((((unsigned)i & 0x0fu) | 0x20u) ^ 0x03u) ^ ((~(unsigned)i) & 0x01u)) << 1 >> 1);
        int expectedCompare = (i == 0) ? 10 : ((i < 128) ? 20 : 30);
        int expectedMemory = i + ((i + 1) & (N - 1));
        if (hArithmetic[i] != i + 2 || hLogic[i] != expectedLogic ||
            hCompare[i] != expectedCompare || hConvert[i] != i || hMemory[i] != expectedMemory) {
            printf("core_ops_minimal FAILED at %d (arith=%d logic=%d compare=%d convert=%d memory=%d)\n",
                   i, hArithmetic[i], hLogic[i], hCompare[i], hConvert[i], hMemory[i]);
            return 1;
        }
    }

    cudaFree(dInput); cudaFree(dArithmetic); cudaFree(dLogic);
    cudaFree(dCompare); cudaFree(dConvert); cudaFree(dMemory);
    cudaStreamDestroy(stream); cudaDeviceReset();
    printf("core_ops_minimal OK (arithmetic logic compare convert memory)\n");
    return 0;
}
