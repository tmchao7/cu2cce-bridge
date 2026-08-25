cmake_minimum_required(VERSION 3.18)
project(cu2cce_legacy_cmake LANGUAGES CXX CUDA)

find_package(CUDA REQUIRED)
cuda_add_executable(legacy_vector_add legacy_vector_add.cu)
