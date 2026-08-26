cmake_minimum_required(VERSION 3.18)

# Host-side utility library plus a CustomCommand generating an input data file.
# This exercises cross-directory CMake target dependencies exercised by large CUDA
# projects: a custom command in this directory drives a generator that the app
# target in the top directory consumes.
add_library(large_host_lib STATIC store.cpp)
install(TARGETS large_host_lib EXPORT largeProjectTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

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

# Publish the generated-input location through an interface include directory
# (instead of an empty library target, which CMake cannot determine a linker
# language for and which must not be part of the install export).
add_library(large_gen_data INTERFACE)
target_include_directories(large_gen_data INTERFACE
  "${CMAKE_CURRENT_BINARY_DIR}")