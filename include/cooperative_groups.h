#ifndef __CU2CCE_COOPERATIVE_GROUPS_H__
#define __CU2CCE_COOPERATIVE_GROUPS_H__

/*
 * Minimal CUDA cooperative_groups compatibility for the puresimt path.
 *
 * This is intentionally a small source-level facade for the group operations
 * used by the external reduction sample. It does not emulate CUDA's complete
 * cooperative_groups library or introduce a new kernel lowering path.
 */

#include <cuda_runtime.h>

namespace cooperative_groups {

template <unsigned int BlockSize>
struct block_tile_memory {
    unsigned char storage[1];
};

struct thread_block {
    __device__ __forceinline__ unsigned int size() const { return blockDim.x; }
    __device__ __forceinline__ unsigned int thread_rank() const { return threadIdx.x; }
};

template <unsigned int Size>
struct thread_block_tile {
    __device__ __forceinline__ unsigned int size() const { return Size; }
    __device__ __forceinline__ unsigned int thread_rank() const { return threadIdx.x % Size; }
    __device__ __forceinline__ unsigned int meta_group_rank() const { return threadIdx.x / Size; }
    __device__ __forceinline__ unsigned int meta_group_size() const {
        return (blockDim.x + Size - 1) / Size;
    }

    template <typename T>
    __device__ __forceinline__ T shfl_down(T value, unsigned int delta) const {
        return __shfl_down_sync(0xffffffffu, value, delta);
    }

    __device__ __forceinline__ void sync() const { __syncthreads(); }
};

__device__ __forceinline__ thread_block this_thread_block() { return thread_block{}; }

template <unsigned int BlockSize>
__device__ __forceinline__ thread_block this_thread_block(block_tile_memory<BlockSize> &) {
    return thread_block{};
}

__device__ __forceinline__ void sync(const thread_block &) { __syncthreads(); }

template <unsigned int Size>
__device__ __forceinline__ thread_block_tile<Size> tiled_partition(const thread_block &) {
    return thread_block_tile<Size>{};
}

template <typename T>
struct plus {
    __device__ __forceinline__ T operator()(const T &lhs, const T &rhs) const {
        return lhs + rhs;
    }
};

template <unsigned int Size, typename T, typename BinaryOp>
__device__ __forceinline__ T reduce(const thread_block_tile<Size> &group,
                                     T value, BinaryOp op) {
    for (unsigned int offset = Size / 2; offset > 0; offset /= 2) {
        value = op(value, group.shfl_down(value, offset));
    }
    return value;
}

} // namespace cooperative_groups

#endif // __CU2CCE_COOPERATIVE_GROUPS_H__
