cmake_minimum_required(VERSION 3.18)

# Host-side utility library plus a CustomCommand generating an input data file.
# This exercises cross-directory CMake target dependencies exercised by large CUDA
# projects: a custom command in this directory drives a generator that the app
# target in the top directory consumes.
add_library(large_host_lib STATIC store.cpp)

add_executable(generate_kernel_data gen_data.cpp)
target_include_directories(generate_kernel_data PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/input_data.bin"
  COMMAND generate_kernel_data "$<TARGET_FILE:generate_kernel_data>" --
  DEPENDS generate_kernel_data
  COMMENT "generating large_project input data"
)
add_custom_target(gen_data_file DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/input_data.bin")

add_library(large_data_header STATIC data.h)
# Reference the generated file in an interface property so the cross-target
# ordering is expressed in the CMake graph rather than in source code.
target_compile_definitions(large_data_header INTERFACE
  CU2CCE_LARGE_INPUT_FILE="${CMAKE_CURRENT_BINARY_DIR}/input_data.bin")