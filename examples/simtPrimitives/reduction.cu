/*
 * reduction.cu —— SIMT 原语真机验证（方向 1：__syncthreads / __shared__ / atomicAdd）
 *
 * 目标：验证 simt_adaptor.h 的映射（CURRENT_STATE 待确认 #6）：
 *   __syncthreads() → asc_syncthreads()
 *   __shared__      → __ubuf__（共享内存/Unified Buffer）
 *   atomicAdd()     → asc_atomic_add()
 *
 * CUDA 侧保持纯 CUDA 写法（零改写）：standard 归约 kernel + 一个 smem 原子计数
 * 探针。由 tools/kernel_convert.py 自动转换：
 *   - kernel 体 → SIMT VF 计算体（warp 原语上下文，simt_callee 限制）；
 *   - 外层 → 全局 wrapper + cce::async_invoke 启动（block 形状以隐藏参数传入）；
 *   - __shared__ 数组 → __ubuf__；__syncthreads → asc_syncthreads；atomicAdd → asc_atomic_add。
 *
 * 验证点：
 *   1. __ubuf__ 数组在 SIMT VF 计算体 / kernel 里的声明与访问能编译（待确认 #6 的一部分）；
 *   2. __syncthreads 能藏在 SIMT VF 计算体内（ccec 只允许 simt_vf/simt_callee 调用）；
 *   3. atomicAdd(&shared_counter, 1) 能隐式解析到 asc_atomic_add 的 __ubuf__ 重载；
 *   4. 数值：sum = 0+1+...+(N-1)，blocks 个 partial，counter == blocks。
 *
 * 真机编译/运行：由 tools/real_test/server_cmd.txt 用 cucc 走 kernel_convert 链路。
 */
#include <cuda_runtime.h>
#include <stdio.h>

#define N 1024
#define THREADS 256

/* ==================== kernel：两阶段块内归约（__shared__ + __syncthreads） ==================== */
__global__ void blockReduceKernel(const float *in, float *partial, int n) {
    __shared__ float sdata[THREADS];

    int tid = threadIdx.x;
    int i = blockIdx.x * blockDim.x + tid;
    float v = (i < n) ? in[i] : 0.0f;
    sdata[tid] = v;

    __syncthreads();

    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            sdata[tid] += sdata[tid + stride];
        }
        __syncthreads();
    }

    if (tid == 0) {
        partial[blockIdx.x] = sdata[0];
    }
}

/* ==================== kernel：smem 原子计数探针（atomicAdd → asc_atomic_add） ==================== */
__global__ void smemAtomicCounterKernel(unsigned *counter, unsigned n) {
    __shared__ unsigned sc;

    if (threadIdx.x == 0) {
        sc = 0;
    }
    __syncthreads();

    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        atomicAdd(&sc, 1u);
    }

    __syncthreads();

    if (threadIdx.x == 0) {
        counter[blockIdx.x] = sc;
    }
}

int main(void) {
    float *in = nullptr;
    float *partial = nullptr;
    unsigned *counter = nullptr;
    cudaError_t err;

    err = cudaMalloc((void **)&in, N * sizeof(float));
    if (err != cudaSuccess) return 1;
    err = cudaMalloc((void **)&partial, THREADS * sizeof(float));
    if (err != cudaSuccess) return 1;
    err = cudaMalloc((void **)&counter, THREADS * sizeof(unsigned));
    if (err != cudaSuccess) return 1;

    float *host_in = new float[N];
    float *host_partial = new float[THREADS];
    unsigned *host_counter = new unsigned[THREADS];
    for (int i = 0; i < N; ++i) host_in[i] = (float)i;

    err = cudaMemcpy(in, host_in, N * sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return 1;

    cudaStream_t stream = nullptr;
    err = cudaStreamCreate(&stream);
    if (err != cudaSuccess) return 1;

    int blocks = (N + THREADS - 1) / THREADS;
    blockReduceKernel<<<blocks, THREADS, 0, stream>>>(in, partial, N);
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) {
        printf("reduction sync failed: %d (%s)\n", (int)err, cudaGetErrorString(err));
        return 1;
    }

    smemAtomicCounterKernel<<<blocks, THREADS, 0, stream>>>(counter, N);
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) {
        printf("atomic counter sync failed: %d (%s)\n", (int)err, cudaGetErrorString(err));
        return 1;
    }

    err = cudaMemcpy(host_partial, partial, blocks * sizeof(float), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return 1;
    err = cudaMemcpy(host_counter, counter, blocks * sizeof(unsigned), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return 1;

    float sum = 0.0f;
    int bad = 0;
    for (int b = 0; b < blocks; ++b) {
        sum += host_partial[b];
        if (host_counter[b] != THREADS) {
            if (b == 0) printf("counter[%d]=%u expected=%d\n", b, host_counter[b], THREADS);
            bad = 1;
        }
    }

    float expect = (float)(N * (N - 1) / 2);
    if (sum != expect) {
        printf("reduction FAILED: sum=%.0f expected=%.0f (counter_bad=%d)\n",
               sum, expect, bad);
        return 1;
    }
    if (bad != 0) {
        printf("atomic counter FAILED\n");
        return 1;
    }

    printf("simt primitives OK (reduction sum=%.0f, atomic counter=%u)\n",
           sum, (unsigned)THREADS);

    cudaStreamDestroy(stream);
    cudaFree(in);
    cudaFree(partial);
    cudaFree(counter);
    delete[] host_in;
    delete[] host_partial;
    delete[] host_counter;
    return 0;
}