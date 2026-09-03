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
set(_CU2CCE_TOOLKIT "$ENV{ASCEND_TOOLKIT_HOME}")
if(NOT _CU2CCE_TOOLKIT)
  set(_CU2CCE_TOOLKIT "/usr/local/Ascend/ascend-toolkit/latest")
endif()

set(_CU2CCE_CANN_INCLUDE_DIRS "${_CU2CCE_PREFIX}/include")
foreach(_include_dir
    "${_CU2CCE_CANN_BASE}/include"
    "${_CU2CCE_TOOLKIT}/include")
  if(EXISTS "${_include_dir}")
    list(APPEND _CU2CCE_CANN_INCLUDE_DIRS "${_include_dir}")
  endif()
endforeach()
list(REMOVE_DUPLICATES _CU2CCE_CANN_INCLUDE_DIRS)

set(CUDAToolkit_FOUND TRUE)
set(CUDA_FOUND TRUE)
set(CUDAToolkit_VERSION "11.6")
set(CUDA_VERSION "11.6")
set(CUDAToolkit_INCLUDE_DIRS "${_CU2CCE_CANN_INCLUDE_DIRS}")
set(CUDA_INCLUDE_DIRS "${CUDAToolkit_INCLUDE_DIRS}")
set(CUDAToolkit_LIBRARY_ROOT "${_CU2CCE_PREFIX}")
set(CUDAToolkit_LIBRARY_DIR "${_CU2CCE_PREFIX}/lib")
set(CUDA_TOOLKIT_ROOT_DIR "${_CU2CCE_PREFIX}")
set(CUDA_TOOLKIT_ROOT "${_CU2CCE_PREFIX}")
set(CUDAToolkit_BIN_DIR "${_CU2CCE_PREFIX}/bin")
set(CUDA_NVCC_EXECUTABLE "${_CU2CCE_PREFIX}/bin/cucc")
set(CUDA_COMPILER "${CUDA_NVCC_EXECUTABLE}")

set(_CU2CCE_RUNTIME "${_CU2CCE_PREFIX}/lib/libcce_bridge_runtime.so")
set(_CU2CCE_ACL_RUNTIME "")
foreach(_candidate
    "${_CU2CCE_CANN_BASE}/lib64/libacl_rt.so"
    "${_CU2CCE_TOOLKIT}/lib64/libacl_rt.so"
    "${_CU2CCE_CANN_BASE}/lib64/libascendcl.so"
    "${_CU2CCE_TOOLKIT}/lib64/libascendcl.so"
    "${_CU2CCE_CANN_BASE}/runtime/lib64/libacl_rt.so"
    "${_CU2CCE_CANN_BASE}/runtime/lib64/libascendcl.so")
  if(NOT _CU2CCE_ACL_RUNTIME AND EXISTS "${_candidate}")
    set(_CU2CCE_ACL_RUNTIME "${_candidate}")
  endif()
endforeach()
if(NOT _CU2CCE_ACL_RUNTIME)
  # Preserve the canonical spelling when CANN is not visible during a local
  # configure; real linking remains responsible for reporting the missing ABI.
  set(_CU2CCE_ACL_RUNTIME "${_CU2CCE_CANN_BASE}/lib64/libacl_rt.so")
endif()
set(CUDAToolkit_ACL_RUNTIME_LIBRARY "${_CU2CCE_ACL_RUNTIME}")

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
    INTERFACE_INCLUDE_DIRECTORIES "${CUDAToolkit_INCLUDE_DIRS}"
  )

  # A C++ host target that links CUDA::cudart does not pass through cucc's
  # automatic link injection. Add only libraries that exist on this CANN
  # installation; the CUDA runtime bridge remains the primary target.
  set(_CU2CCE_EXTRA_LIBS)
  foreach(_lib
      "${_CU2CCE_ACL_RUNTIME}"
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

# Alias-family targets so large projects that reference the common CUDA
# imported-target spellings keep configuring. These are all backed by the
# bridge runtime (puresimt consumes CUDA source directly; there is no separate
# device-runtime library on Ascend). CUDA::toolkit carries the include dirs;
# cudart_static/cudadevrt are aliases of the same bridge runtime so link lines
# with those spellings resolve instead of failing with missing targets.
if(NOT TARGET CUDA::toolkit)
  add_library(CUDA::toolkit INTERFACE IMPORTED)
  set_target_properties(CUDA::toolkit PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CUDAToolkit_INCLUDE_DIRS}")
endif()
foreach(_alias cudart_static cudadevrt cudart)
  if(NOT TARGET CUDA::${_alias})
    add_library(CUDA::${_alias} INTERFACE IMPORTED)
    set_target_properties(CUDA::${_alias} PROPERTIES
      INTERFACE_LINK_LIBRARIES CUDA::cudart)
  endif()
endforeach()

set(CUDAToolkit_LIBRARIES CUDA::cudart)
set(CUDA_LIBRARIES CUDA::cudart)
set(CUDAToolkit_CUDART_STATIC_LIBRARY "${_CU2CCE_RUNTIME}")
set(CUDAToolkit_CUDA_DRIVER_LIBRARY "${_CU2CCE_RUNTIME}")


if(CUDAToolkit_FIND_QUIETLY)
  return()
endif()
message(STATUS "Found cu2cce CUDA runtime bridge: ${_CU2CCE_RUNTIME}")
