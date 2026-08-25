cmake_minimum_required(VERSION 3.18)
project(cu2cce_build_compat LANGUAGES CXX CUDA)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)
find_package(CUDAToolkit REQUIRED)
# ccec emits one eager SIMT registration helper per CUDA translation unit.
# Keep one .cu per linked library in this smoke; a single static archive with
# multiple .cu files is diagnosed separately as an unsupported backend shape.
add_library(build_compat_static STATIC kernel_a.cu)
add_library(build_compat_shared SHARED kernel_b.cu)
add_executable(build_compat_app main.cpp)
target_link_libraries(build_compat_app PRIVATE build_compat_static build_compat_shared CUDA::cudart)
install(TARGETS build_compat_app build_compat_static build_compat_shared EXPORT buildCompatTargets)
install(EXPORT buildCompatTargets DESTINATION lib/cmake/build_compat)
