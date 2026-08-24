/* Minimal CUDA atomicSub/atomicExch coverage for cu2cce puresimt. */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 1024

__global__ void atomicVariants(int *subValue, int *exchangeValue, int *sharedResult) {
    __shared__ int shared[N];
    int tid = threadIdx.x;
    shared[tid] = 1;
    __syncthreads();
    atomicSub(subValue, 1);
    if (tid == 0) atomicExch(exchangeValue, 1234);
    atomicAdd(&shared[tid], tid);
    __syncthreads();
    if (tid == 0) {
        int sum = 0;
        for (int i = 0; i < N; ++i) sum += shared[i];
        *sharedResult = sum;
    }
}

int main(void) {
    int hSub = N, hExchange = 0, hShared = 0;
    int *dSub = nullptr, *dExchange = nullptr, *dShared = nullptr;
    cudaStream_t stream = nullptr;
    if (cudaStreamCreate(&stream) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dSub, sizeof(int)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dExchange, sizeof(int)) != cudaSuccess) return 1;
    if (cudaMalloc((void **)&dShared, sizeof(int)) != cudaSuccess) return 1;
    if (cudaMemcpy(dSub, &hSub, sizeof(int), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dExchange, &hExchange, sizeof(int), cudaMemcpyHostToDevice) != cudaSuccess) return 1;
    if (cudaMemcpy(dShared, &hShared, sizeof(int), cudaMemcpyHostToDevice) != cudaSuccess) return 1;

    atomicVariants<<<1, N, 0, stream>>>(dSub, dExchange, dShared);
    if (cudaGetLastError() != cudaSuccess || cudaStreamSynchronize(stream) != cudaSuccess) return 1;
    if (cudaMemcpy(&hSub, dSub, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (cudaMemcpy(&hExchange, dExchange, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;
    if (cudaMemcpy(&hShared, dShared, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    int expectedShared = N + N * (N - 1) / 2;
    if (hSub != 0 || hExchange != 1234 || hShared != expectedShared) {
        printf("atomicVariants_minimal FAILED (sub=%d exchange=%d shared=%d expected_shared=%d)\n",
               hSub, hExchange, hShared, expectedShared);
        return 1;
    }
    cudaFree(dSub); cudaFree(dExchange); cudaFree(dShared);
    cudaStreamDestroy(stream); cudaDeviceReset();
    printf("atomicVariants_minimal OK (sub=0 exchange=1234 shared=%d)\n", hShared);
    return 0;
}
