cmake_minimum_required(VERSION 3.18)
project(cu2cce_cmake_demo LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

add_executable(cmake_vector_add vector_add.cu)
