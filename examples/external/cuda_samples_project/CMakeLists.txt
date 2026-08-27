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
# Ascend architecture or enable CUDA separable compilation. It also does not
# rewrite any upstream source, including the sample-local Common helper dir.
set(CMAKE_CUDA_ARCHITECTURES OFF CACHE STRING "" FORCE)
# Host-only upstream sources also include cuda_runtime.h and libstdc++ headers.
# Use the system libc/math declarations for this external profile; the ordinary
# puresimt regression keeps the shadow mode by default.
add_compile_definitions(__CU2CCE_SYSTEM_LIBC__)

# Upstream samples under this revision include their CUDA helpers through
# "../Common/". The harness keeps the unmodified source tree in
# CUDA_SAMPLES_SOURCE_DIR and relies on the rule-driven cucc include policy for
# library-style samples; per-sample Common dirs are injected by the rule driver
# only when the sample needs them (vectorAdd/matrixMul).
set(_CU2CCE_PROFILE_SAMPLES
  "Samples/0_Introduction/vectorAdd"
  "Samples/0_Introduction/matrixMul"
  "Samples/0_Introduction/simpleStreams"
  "Samples/0_Introduction/simpleMultiCopy"
  "Samples/0_Introduction/asyncAPI"
)
# Diagnostic-only samples: they exercise CUDA runtime surface that CANN 9.2
# may not fully provide (host-mapped/pinned memory, cudaDeviceProp fields,
# event flags). They never block the required profile.
set(_CU2CCE_DIAGNOSTIC_SAMPLES
  "Samples/0_Introduction/simpleStreams"
  "Samples/0_Introduction/simpleMultiCopy"
  "Samples/0_Introduction/asyncAPI"
)
foreach(_relative IN LISTS _CU2CCE_PROFILE_SAMPLES)
  if(EXISTS "${CUDA_SAMPLES_SOURCE_DIR}/${_relative}/CMakeLists.txt")
    get_filename_component(_name "${_relative}" NAME)
    set_property(GLOBAL APPEND PROPERTY cu2cce_sample_targets "${_name}")
    if(_relative IN_LIST _CU2CCE_DIAGNOSTIC_SAMPLES)
      set_property(GLOBAL APPEND PROPERTY cu2cce_diagnostic_samples "${_name}")
    endif()
    add_subdirectory(
      "${CUDA_SAMPLES_SOURCE_DIR}/${_relative}"
      "${CMAKE_CURRENT_BINARY_DIR}/${_name}"
    )
  endif()
endforeach()

# The current ccec puresimt backend does not aggregate identical device
# registration helpers from separate translation units through RDC. Disable the
# upstream request for separable compilation while keeping source files
# untouched; this is an explicit compatibility boundary, not a hidden rewrite.
#
# Upstream v12.x sample CMakeLists also hard-code per-target CUDA_ARCHITECTURES
# lists (asyncAPI requests sm_61..sm_120). Those GPU generations are meaningless
# on Ascend, yet they make CMake emit an NVIDIA binary-artifact pipeline:
# the server log showed ld.lld being asked to open
# "=asyncAPI/CMakeFiles/asyncAPI.dir/sm_120.cubin" and failing. Every sample
# target is therefore forced back onto the neutral configuration AFTER it has
# registered itself; upstream sources stay unmodified.
get_property(_CU2CCE_SAMPLE_TARGETS GLOBAL PROPERTY cu2cce_sample_targets)
foreach(_target IN LISTS _CU2CCE_SAMPLE_TARGETS)
  if(TARGET "${_target}")
    set_target_properties("${_target}" PROPERTIES
      CUDA_ARCHITECTURES OFF
      CUDA_SEPARABLE_COMPILATION OFF
      CUDA_CUBIN_COMPILATION OFF
      CUDA_FATBIN_COMPILATION OFF
    )
  endif()
endforeach()

# Samples known to exceed the current CANN backend are excluded from the
# external graph entirely (they never even reach configure). The harness does
# not claim to support reduction's cross-warp thread_block_tile instantiations
# or device-side double; keeping it out of the target list is the explicit
# boundary (no fatal error, no silent acceptance).
set(_CU2CCE_EXCLUDED_SAMPLES
  "Samples/2_Concepts_and_Techniques/reduction"
)
foreach(_relative IN LISTS _CU2CCE_EXCLUDED_SAMPLES)
  if(EXISTS "${CUDA_SAMPLES_SOURCE_DIR}/${_relative}/CMakeLists.txt")
    message(STATUS "cu2cce: excluding known-unsupported sample ${_relative}")
  endif()
endforeach()
