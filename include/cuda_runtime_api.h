/*
 * CUDA Runtime API 兼容入口（C 风格）。
 *
 * 仿照 cu-bridge 的 include/cuda_runtime_api.h：真正的头文件体在 __cuda_runtime.h，
 * 本文件仅做转发，供 `#include <cuda_runtime_api.h>` 的用户代码使用。
 */
#include <__cuda_runtime.h>
