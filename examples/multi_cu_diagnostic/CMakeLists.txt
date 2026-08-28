cmake_minimum_required(VERSION 3.18)
project(cu2cce_multi_cu_diagnostic LANGUAGES CXX CUDA)

# Diagnostic carrier for the multi-.cu static library boundary. ccec emits
# duplicate eager-registration helpers per translation unit; a static archive
# that aggregates two .cu objects may fail at link time. This project never
# blocks the required profile: it records the outcome so the backend
# limitation stays documented and testable.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(CUDAToolkit REQUIRED)

add_library(multi_cu_lib STATIC
  kernel_a.cu
  kernel_b.cu
)
target_include_directories(multi_cu_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
# ccec cannot aggregate duplicate registration helpers yet. Keep separable
# compilation off for this diagnostic build, exactly like the external
# cuda-samples harness does.
set_target_properties(multi_cu_lib PROPERTIES CUDA_SEPARABLE_COMPILATION OFF)

add_executable(multi_cu_diag_app main.cpp)
target_link_libraries(multi_cu_diag_app PRIVATE multi_cu_lib CUDA::cudart)