# Minimal legacy FindCUDA compatibility for cu2cce-bridge.
#
# Supported legacy surface:
#   find_package(CUDA)
#   cuda_add_executable() / cuda_add_library() / cuda_compile() /
#   cuda_wrap_srcs() / cuda_include_directories() /
#   cuda_select_nvcc_arch_flags() / CUDA_NVCC_FLAGS
#   uppercase legacy aliases CUDA_ADD_* / CUDA_COMPILE / CUDA_WRAP_SRCS
#   CUDA_INCLUDE_DIRS / CUDA_LIBRARIES / CUDA_NVCC_EXECUTABLE
#
# The implementation delegates to normal CMake targets. It never generates
# .cce files or performs kernel source conversion. PTX/CUBIN/FATBIN helpers
# fail with an explicit message because puresimt compiles CUDA directly with
# ccec on Ascend.

include(CMakeParseArguments)

set(_CU2CCE_PREFIX "$ENV{CUCC_PATH}")
if(NOT _CU2CCE_PREFIX)
  get_filename_component(_CU2CCE_PREFIX "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

set(CUDA_FOUND TRUE)
set(CUDA_VERSION "11.6")
set(CUDA_VERSION_MAJOR 11)
set(CUDA_VERSION_MINOR 6)
set(CUDA_VERSION_PATCH 0)
set(CUDA_TOOLKIT_ROOT_DIR "${_CU2CCE_PREFIX}")
set(CUDA_INCLUDE_DIRS "${_CU2CCE_PREFIX}/include")
set(CUDA_NVCC_EXECUTABLE "${_CU2CCE_PREFIX}/bin/cucc")
set(CUDA_LIBRARIES "${_CU2CCE_PREFIX}/lib/libcce_bridge_runtime.so")
set(CUDA_CUDART_LIBRARY "${CUDA_LIBRARIES}")
set(CUDA_TOOLKIT_INCLUDE "${CUDA_INCLUDE_DIRS}")
set(CUDA_TOOLKIT_LIBRARY_ROOT "${_CU2CCE_PREFIX}")

function(_cu2cce_legacy_parse target out_sources out_options out_compile_options)
  set(_sources)
  set(_cmake_options)
  set(_nvcc_options ${CUDA_NVCC_FLAGS})
  set(_in_options FALSE)
  foreach(_arg IN LISTS ARGN)
    if(_arg STREQUAL "OPTIONS")
      set(_in_options TRUE)
    elseif(_in_options)
      list(APPEND _nvcc_options "${_arg}")
    elseif(_arg STREQUAL "WIN32" OR
           _arg STREQUAL "MACOSX_BUNDLE" OR
           _arg STREQUAL "EXCLUDE_FROM_ALL" OR
           _arg STREQUAL "STATIC" OR
           _arg STREQUAL "SHARED" OR
           _arg STREQUAL "MODULE")
      list(APPEND _cmake_options "${_arg}")
    else()
      list(APPEND _sources "${_arg}")
    endif()
  endforeach()
  set(${out_sources} "${_sources}" PARENT_SCOPE)
  set(${out_options} "${_cmake_options}" PARENT_SCOPE)
  set(${out_compile_options} "${_nvcc_options}" PARENT_SCOPE)
endfunction()

function(_cu2cce_enable_cuda)
  get_property(_enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
  if(NOT "CUDA" IN_LIST _enabled_languages)
    enable_language(CUDA)
  endif()
endfunction()

function(_cu2cce_attach_target target include_scope link_scope compile_options)
  target_include_directories(${target} ${include_scope} "${CUDA_INCLUDE_DIRS}")
  target_link_libraries(${target} ${link_scope} "${CUDA_LIBRARIES}")
  if(compile_options)
    target_compile_options(${target} PRIVATE ${compile_options})
  endif()
endfunction()

function(cuda_add_executable target)
  _cu2cce_legacy_parse("${target}" _sources _cmake_options _nvcc_options ${ARGN})
  if(NOT _sources)
    message(FATAL_ERROR "cu2cce: cuda_add_executable(${target}) has no CUDA sources")
  endif()
  add_executable(${target} ${_cmake_options} ${_sources})
  _cu2cce_attach_target(${target} PRIVATE PRIVATE "${_nvcc_options}")
endfunction()

function(cuda_add_library target)
  _cu2cce_legacy_parse("${target}" _sources _cmake_options _nvcc_options ${ARGN})
  if(NOT _sources)
    message(FATAL_ERROR "cu2cce: cuda_add_library(${target}) has no CUDA sources")
  endif()

  set(_library_type)
  foreach(_option IN LISTS _cmake_options)
    if(_option STREQUAL "STATIC" OR _option STREQUAL "SHARED" OR _option STREQUAL "MODULE")
      set(_library_type "${_option}")
    endif()
  endforeach()
  if(_library_type)
    add_library(${target} ${_library_type} ${_sources})
  else()
    add_library(${target} ${_sources})
  endif()
  _cu2cce_attach_target(${target} PRIVATE PUBLIC "${_nvcc_options}")
endfunction()

function(cuda_select_nvcc_arch_flags out_var)
  # NVIDIA architecture selection has no direct meaning for the Ascend target.
  # Return an empty list instead of inventing an sm_XX value.
  set(${out_var} "" PARENT_SCOPE)
endfunction()

function(cuda_include_directories)
  include_directories(${ARGN})
endfunction()

macro(cuda_compile out_var)
  # Legacy `cuda_compile(objs foo.cu ...)`: generate CUDA objects for the
  # listed sources. Implemented as a LANGUAGE-CUDA passthrough — the sources
  # are compiled by the CUDA rules recorded from the shim (nvcc -> cucc), and
  # the returned list doubles as the source list for add_executable/
  # add_library, matching how legacy projects consume the variable.
  _cu2cce_enable_cuda()
  set(${out_var})
  foreach(_src ${ARGN})
    if(_src MATCHES "\\.cu$")
      set_source_files_properties("${_src}" PROPERTIES LANGUAGE CUDA)
    endif()
    list(APPEND ${out_var} "${_src}")
  endforeach()
endmacro()

macro(cuda_wrap_srcs cuda_target format generated_files)
  # Legacy CUDA_WRAP_SRCS parity: filter the CUDA sources out of ARGN, mark
  # them for the CUDA compiler, and return them via ${generated_files}.
  _cu2cce_enable_cuda()
  set(${generated_files})
  foreach(_src ${ARGN})
    if(_src MATCHES "\\.cu$")
      set_source_files_properties("${_src}" PROPERTIES LANGUAGE CUDA)
      list(APPEND ${generated_files} "${_src}")
    endif()
  endforeach()
endmacro()

macro(cuda_compile_ptx out_var)
  message(FATAL_ERROR "cu2cce: CUDA PTX generation is unsupported on Ascend puresimt")
endmacro()
macro(cuda_compile_fatbin out_var)
  message(FATAL_ERROR "cu2cce: CUDA FATBIN generation is unsupported on Ascend puresimt")
endmacro()
macro(cuda_compile_cubin out_var)
  message(FATAL_ERROR "cu2cce: CUDA CUBIN generation is unsupported on Ascend puresimt")
endmacro()

message(STATUS "Found cu2cce legacy CUDA compatibility: ${CUDA_NVCC_EXECUTABLE}")

# Reference FindCUDA exposes uppercase aliases. Keep the aliases as thin
# wrappers around the validated lowercase implementation so legacy projects
# configure without introducing a second code path.
macro(CUDA_ADD_EXECUTABLE target)
  cuda_add_executable(${target} ${ARGN})
endmacro()

macro(CUDA_ADD_LIBRARY target)
  cuda_add_library(${target} ${ARGN})
endmacro()

macro(CUDA_COMPILE out_var)
  cuda_compile(${out_var} ${ARGN})
endmacro()

macro(CUDA_WRAP_SRCS cuda_target format generated_files)
  cuda_wrap_srcs(${cuda_target} ${format} ${generated_files} ${ARGN})
endmacro()

macro(CUDA_INCLUDE_DIRECTORIES)
  cuda_include_directories(${ARGN})
endmacro()

function(_cu2cce_fail_unmapped_legacy_library library target)
  message(FATAL_ERROR
    "cu2cce: ${library} for ${target} has no validated Ascend equivalent; "
    "use an explicit CANN/bridge target instead of a CUDA library alias")
endfunction()

macro(CUDA_ADD_CUBLAS_TO_TARGET target)
  _cu2cce_fail_unmapped_legacy_library("CUDA::cublas" "${target}")
endmacro()
macro(CUDA_ADD_CUFFT_TO_TARGET target)
  _cu2cce_fail_unmapped_legacy_library("CUDA::cufft" "${target}")
endmacro()
macro(CUDA_ADD_CUSOLVER_TO_TARGET target)
  _cu2cce_fail_unmapped_legacy_library("CUDA::cusolver" "${target}")
endmacro()
macro(CUDA_ADD_CUSPARSE_TO_TARGET target)
  _cu2cce_fail_unmapped_legacy_library("CUDA::cusparse" "${target}")
endmacro()
macro(CUDA_ADD_CURAND_TO_TARGET target)
  _cu2cce_fail_unmapped_legacy_library("CUDA::curand" "${target}")
endmacro()

# Additional FindCUDA helper names commonly used by older projects. They are
# compatibility shims over the same normal-CMake implementation; none creates
# NVIDIA device artifacts on the Ascend puresimt backend.
macro(CUDA_GET_SOURCES_AND_OPTIONS sources cmake_options options)
  _cu2cce_legacy_parse("legacy" _cu2cce_sources _cu2cce_cmake_options _cu2cce_nvcc_options ${ARGN})
  set(${sources} "${_cu2cce_sources}")
  set(${cmake_options} "${_cu2cce_cmake_options}")
  set(${options} "${_cu2cce_nvcc_options}")
endmacro()

macro(cuda_compile_base cuda_target format generated_files)
  cuda_wrap_srcs(${cuda_target} ${format} ${generated_files} ${ARGN})
endmacro()

macro(cuda_unset_include_and_libraries)
  unset(CUDA_INCLUDE_DIRS)
  unset(CUDA_LIBRARIES)
endmacro()

macro(CUDA_BUILD_CLEAN_TARGET)
  # CMake's native clean target already covers generated objects in this
  # direct-language implementation; no extra CUDA artifact cleanup is needed.
endmacro()
