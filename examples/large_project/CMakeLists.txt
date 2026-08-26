cmake_minimum_required(VERSION 3.18)
project(cu2cce_large_project LANGUAGES CXX CUDA)

# Multi-directory / multi-target / cross-directory dependency validation for
# the cu2cce-bridge CMake compatibility surface. This project intentionally
# exercises the patterns that a large real CUDA code base uses:
#   - subdirectories with their own targets and includes
#   - CUDA library targets consumed by host and by other CUDA targets
#   - install/export of the full target graph
# It adds no new backend requirement beyond one .cu per linked library.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(CUDAToolkit REQUIRED)

add_subdirectory(comp)
add_subdirectory(util)

add_executable(large_project_app app/main.cpp)
target_link_libraries(large_project_app PRIVATE large_util_lib)
add_dependencies(large_project_app generate_kernel_data)
target_include_directories(large_project_app PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/app")

install(TARGETS large_project_app
  RUNTIME DESTINATION bin
)
install(EXPORT largeProjectTargets
  DESTINATION lib/cmake/large_project
  NAMESPACE large_project::
)