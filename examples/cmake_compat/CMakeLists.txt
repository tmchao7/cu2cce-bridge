cmake_minimum_required(VERSION 3.18)
project(cu2cce_cmake_compat LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

find_package(CUDAToolkit REQUIRED)

add_library(cmake_static_kernel STATIC static_kernel.cu)
add_library(cmake_shared_kernel SHARED shared_kernel.cu)

add_executable(cmake_compat_app main.cpp)
target_link_libraries(cmake_compat_app
    PRIVATE
        cmake_static_kernel
        cmake_shared_kernel
        CUDA::cudart
)
target_compile_definitions(cmake_compat_app PRIVATE CU2CCE_CMAKE_COMPAT=1)
target_include_directories(cmake_compat_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
