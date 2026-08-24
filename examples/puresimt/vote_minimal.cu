/* Minimal CUDA vote-intrinsics regression, based on NVIDIA simpleVoteIntrinsics. */
#include <cuda_runtime.h>
#include <stdio.h>

#define THREADS 256

__global__ void voteKernel(const int *input, int *allOut, int *anyOut, int *ballotOut) {
    int tid = threadIdx.x;
    int predicate = input[tid] != 0;
    /* CUDA vote intrinsics are warp collectives: every lane covered by the
     * mask must execute them. Only the stores are lane-0 work. */
    int all = __all_sync(0xffffffffu, predicate);
    int any = __any_sync(0xffffffffu, predicate);
    int ballot = (int)__ballot_sync(0xffffffffu, predicate);
    if (tid == 0) {
        allOut[0] = all;
        anyOut[0] = any;
        ballotOut[0] = ballot;
    }
}

int main(void) {
    int hInput[THREADS] = {0};
    int hAll = -1, hAny = -1, hBallot = 0;
    hInput[0] = 1;

    int *dInput = nullptr, *dAll = nullptr, *dAny = nullptr, *dBallot = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dInput, sizeof(hInput)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dAll, sizeof(int)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dAny, sizeof(int)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dBallot, sizeof(int)) != cudaSuccess) return 1;
    if (cudaMemcpy(dInput, hInput, sizeof(hInput), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    voteKernel<<<1, THREADS, 0, stream>>>(dInput, dAll, dAny, dBallot);
    if (cudaGetLastError() != cudaSuccess || cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(&hAll, dAll, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (cudaMemcpy(&hAny, dAny, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (cudaMemcpy(&hBallot, dBallot, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    if (hAll != 0 || hAny != 1 || hBallot != 1) {
        printf("vote_minimal FAILED (all=%d any=%d ballot=%d)\n", hAll, hAny, hBallot);
        return 1;
    }
    cudaFree(dInput); cudaFree(dAll); cudaFree(dAny); cudaFree(dBallot);
    cudaStreamDestroy(stream); cudaDeviceReset();
    printf("vote_minimal OK (all=%d, any=%d, ballot=%d)\n", hAll, hAny, hBallot);
    return 0;
}
