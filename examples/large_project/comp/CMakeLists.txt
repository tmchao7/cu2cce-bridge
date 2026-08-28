cmake_minimum_required(VERSION 3.18)

# CUDA kernel library living in its own subdirectories. The backend supports
# one .cu per linked library target, so each library keeps a single CUDA source.
add_library(large_kernel_lib STATIC kernel.cu)

# The public include directory must be expressed with a generator-expressed
# install location so CMake can export this target. A bare source-directory
# path in INTERFACE_INCLUDE_DIRECTORIES makes install(EXPORT) fail with
# "prefixed in the source directory".
target_include_directories(large_kernel_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
install(TARGETS large_kernel_lib EXPORT largeProjectTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)