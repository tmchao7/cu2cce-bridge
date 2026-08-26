cmake_minimum_required(VERSION 3.18)

# CUDA kernel library living in its own subdirectories. The backend supports
# one .cu per linked library target, so each library keeps a single CUDA source.
add_library(large_kernel_lib STATIC kernel.cu)
target_include_directories(large_kernel_lib PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}")