cmake_minimum_required(VERSION 3.20)
project(cu2cce_external_cuda_samples LANGUAGES C CXX CUDA)

if(NOT DEFINED CUDA_SAMPLES_SOURCE_DIR)
  message(FATAL_ERROR "CUDA_SAMPLES_SOURCE_DIR must point to a pinned NVIDIA cuda-samples checkout")
endif()
if(NOT EXISTS "${CUDA_SAMPLES_SOURCE_DIR}/Samples")
  message(FATAL_ERROR "invalid CUDA samples checkout: ${CUDA_SAMPLES_SOURCE_DIR}")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)
# The upstream sample files set NVIDIA architectures themselves. cmake_cce/cucc
# diagnoses and drops those GPU-generation flags; this harness does not invent an
# Ascend architecture or enable CUDA separable compilation.
set(CMAKE_CUDA_ARCHITECTURES OFF CACHE STRING "" FORCE)
# Host-only upstream sources also include cuda_runtime.h and libstdc++ headers.
# Use the system libc/math declarations for this external profile; the ordinary
# puresimt regression keeps the shadow mode by default.
add_compile_definitions(__CU2CCE_SYSTEM_LIBC__)

set(_CU2CCE_SAMPLES
  "Samples/0_Introduction/vectorAdd"
  "Samples/0_Introduction/matrixMul"
  "Samples/2_Concepts_and_Techniques/reduction"
  "Samples/0_Introduction/simpleStreams"
  "Samples/0_Introduction/simpleMultiCopy"
)
foreach(_relative IN LISTS _CU2CCE_SAMPLES)
  if(NOT EXISTS "${CUDA_SAMPLES_SOURCE_DIR}/${_relative}/CMakeLists.txt")
    message(FATAL_ERROR "missing upstream sample CMakeLists.txt: ${_relative}")
  endif()
  get_filename_component(_name "${_relative}" NAME)
  add_subdirectory(
    "${CUDA_SAMPLES_SOURCE_DIR}/${_relative}"
    "${CMAKE_CURRENT_BINARY_DIR}/${_name}"
  )
endforeach()

# The current ccec puresimt backend does not aggregate identical device
# registration helpers from separate translation units through RDC. Disable the
# upstream sample request for separable compilation while keeping source files
# untouched; this is an explicit compatibility boundary, not a hidden rewrite.
foreach(_target vectorAdd matrixMul reduction simpleStreams simpleMultiCopy)
  if(TARGET "${_target}")
    set_target_properties("${_target}" PROPERTIES CUDA_SEPARABLE_COMPILATION OFF)
  endif()
endforeach()

# The upstream reduction translation unit uses CUDA spellings (__device__ and
# cg::reduce). Force the bridge facade before the upstream include so an
# installed CANN cooperative_groups.h cannot win by include-order accident.
if(TARGET reduction AND DEFINED ENV{CUCC_PATH})
  set(_CU2CCE_COOP_GROUPS "$ENV{CUCC_PATH}/include/cooperative_groups.h")
  if(EXISTS "${_CU2CCE_COOP_GROUPS}")
    target_compile_options(reduction PRIVATE
      "$<$<COMPILE_LANGUAGE:CUDA>:-include>"
      "$<$<COMPILE_LANGUAGE:CUDA>:${_CU2CCE_COOP_GROUPS}>")
  endif()
endif()
