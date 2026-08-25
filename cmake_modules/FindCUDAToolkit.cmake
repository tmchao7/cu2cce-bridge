# Minimal CUDA Toolkit compatibility module for cu2cce-bridge.
#
# This is deliberately conservative: it exposes only the CUDA runtime target
# that is implemented by the bridge. It does not fabricate CUDA math-library
# targets or claim that the complete NVIDIA Toolkit is present.

set(_CU2CCE_PREFIX "$ENV{CUCC_PATH}")
if(NOT _CU2CCE_PREFIX)
  get_filename_component(_CU2CCE_PREFIX "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

set(_CU2CCE_CANN_BASE "$ENV{CANN_BASE}")
if(NOT _CU2CCE_CANN_BASE)
  set(_CU2CCE_CANN_BASE "/usr/local/Ascend/latest")
endif()

set(CUDAToolkit_FOUND TRUE)
set(CUDA_FOUND TRUE)
set(CUDAToolkit_VERSION "11.6")
set(CUDA_VERSION "11.6")
set(CUDAToolkit_INCLUDE_DIRS "${_CU2CCE_PREFIX}/include")
set(CUDA_INCLUDE_DIRS "${CUDAToolkit_INCLUDE_DIRS}")
set(CUDAToolkit_LIBRARY_ROOT "${_CU2CCE_PREFIX}")
set(CUDAToolkit_LIBRARY_DIR "${_CU2CCE_PREFIX}/lib")
set(CUDA_TOOLKIT_ROOT_DIR "${_CU2CCE_PREFIX}")
set(CUDA_TOOLKIT_ROOT "${_CU2CCE_PREFIX}")
set(CUDAToolkit_BIN_DIR "${_CU2CCE_PREFIX}/bin")
set(CUDA_NVCC_EXECUTABLE "${_CU2CCE_PREFIX}/bin/cucc")
set(CUDA_COMPILER "${CUDA_NVCC_EXECUTABLE}")

set(_CU2CCE_RUNTIME "${_CU2CCE_PREFIX}/lib/libcce_bridge_runtime.so")
if(NOT EXISTS "${_CU2CCE_RUNTIME}")
  set(CUDAToolkit_FOUND FALSE)
  set(CUDA_FOUND FALSE)
  if(CUDAToolkit_FIND_REQUIRED OR CUDA_FIND_REQUIRED)
    message(FATAL_ERROR
      "cu2cce: CUDA runtime bridge not found at ${_CU2CCE_RUNTIME}. "
      "Build/install cu2cce-bridge and set CUCC_PATH.")
  endif()
  return()
endif()

if(NOT TARGET CUDA::cudart)
  add_library(CUDA::cudart UNKNOWN IMPORTED)
  set_target_properties(CUDA::cudart PROPERTIES
    IMPORTED_LOCATION "${_CU2CCE_RUNTIME}"
    INTERFACE_INCLUDE_DIRECTORIES "${_CU2CCE_PREFIX}/include"
  )

  # A C++ host target that links CUDA::cudart does not pass through cucc's
  # automatic link injection. Add only libraries that exist on this CANN
  # installation; the CUDA runtime bridge remains the primary target.
  set(_CU2CCE_EXTRA_LIBS)
  foreach(_lib
      "${_CU2CCE_CANN_BASE}/lib64/libascendcl.so"
      "${_CU2CCE_CANN_BASE}/lib64/libruntime.so"
      "${_CU2CCE_CANN_BASE}/runtime/lib64/libruntime.so"
      "${_CU2CCE_CANN_BASE}/lib64/libstdc++.so"
      "${_CU2CCE_CANN_BASE}/lib64/libpthread.so")
    if(EXISTS "${_lib}")
      list(APPEND _CU2CCE_EXTRA_LIBS "${_lib}")
    endif()
  endforeach()
  if(_CU2CCE_EXTRA_LIBS)
    set_property(TARGET CUDA::cudart APPEND PROPERTY
      INTERFACE_LINK_LIBRARIES "${_CU2CCE_EXTRA_LIBS}")
  endif()
endif()

set(CUDAToolkit_CUDART_LIBRARY "${_CU2CCE_RUNTIME}")
set(CUDA_CUDART_LIBRARY "${_CU2CCE_RUNTIME}")
set(CUDAToolkit_LIBRARIES CUDA::cudart)
set(CUDA_LIBRARIES CUDA::cudart)


if(CUDAToolkit_FIND_QUIETLY)
  return()
endif()
message(STATUS "Found cu2cce CUDA runtime bridge: ${_CU2CCE_RUNTIME}")
