/*
 * Minimal CUDA atomicAdd-with-array-index regression for cu2cce puresimt.
 *
 * Mirrors the common "many threads, few elements" atomic-counter pattern
 * found in NVIDIA's simpleAtomicIntrinsics tutorial: a large thread grid
 * atomically increments a small fixed-size array indexed by tid % SIZE.
 * Unlike atomicAdd_minimal.cu (scalar pointer) and atomicVariants_minimal.cu
 * (shared-memory array), this variant exercises global-memory array-indexed
 * atomicAdd with a non-power-of-two block width and the 2-argument <<<>>>
 * launch form, keeping only CUDA runtime + stdio headers so bridge/header
 * failures stay isolated from C++ standard-library header issues.
 */
#include <cuda_runtime.h>
#include <stdio.h>

#define NUM_THREADS 10000
#define SIZE 10
#define BLOCK_WIDTH 100

__global__ void gpu_increment_atomic(int *d_a) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    tid = tid % SIZE;
    atomicAdd(&d_a[tid], 1);
}

int main(void) {
    printf("%d total threads in %d blocks writing into %d array elements\n",
           NUM_THREADS, NUM_THREADS / BLOCK_WIDTH, SIZE);

    int h_a[SIZE];
    const int ARRAY_BYTES = SIZE * sizeof(int);

    int *d_a = nullptr;
    if (cudaMalloc((void **)&d_a, ARRAY_BYTES) != cudaSuccess) {
        printf("cudaMalloc failed\n");
        return 1;
    }
    if (cudaMemset((void *)d_a, 0, ARRAY_BYTES) != cudaSuccess) {
        printf("cudaMemset failed\n");
        cudaFree(d_a);
        return 1;
    }

    gpu_increment_atomic<<<NUM_THREADS / BLOCK_WIDTH, BLOCK_WIDTH>>>(d_a);
    if (cudaGetLastError() != cudaSuccess) {
        printf("kernel launch failed\n");
        cudaFree(d_a);
        return 1;
    }
    if (cudaDeviceSynchronize() != cudaSuccess) {
        printf("cudaDeviceSynchronize failed\n");
        cudaFree(d_a);
        return 1;
    }

    if (cudaMemcpy(h_a, d_a, ARRAY_BYTES, cudaMemcpyDeviceToHost) != cudaSuccess) {
        printf("cudaMemcpy D2H failed\n");
        cudaFree(d_a);
        return 1;
    }

    printf("Number of times a particular Array index has been incremented is:\n");
    int ok = 1;
    for (int i = 0; i < SIZE; i++) {
        printf("index:%d --> %d times\n", i, h_a[i]);
        if (h_a[i] != NUM_THREADS / SIZE) ok = 0;
    }

    cudaFree(d_a);
    if (!ok) {
        printf("atomicIndex_minimal FAILED (expected %d per index)\n", NUM_THREADS / SIZE);
        return 1;
    }
    printf("atomicIndex_minimal OK (%d threads, %d elements, %d per index)\n",
           NUM_THREADS, SIZE, NUM_THREADS / SIZE);
    return 0;
}
