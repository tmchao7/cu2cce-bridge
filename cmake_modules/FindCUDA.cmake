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
  _cu2cce_enable_cuda()
  add_executable(${target} ${_cmake_options} ${_sources})
  _cu2cce_attach_target(${target} PRIVATE PRIVATE "${_nvcc_options}")
endfunction()

function(cuda_add_library target)
  _cu2cce_legacy_parse("${target}" _sources _cmake_options _nvcc_options ${ARGN})
  if(NOT _sources)
    message(FATAL_ERROR "cu2cce: cuda_add_library(${target}) has no CUDA sources")
  endif()
  _cu2cce_enable_cuda()

  set(_library_type)
  set(_library_options)
  foreach(_option IN LISTS _cmake_options)
    if(_option STREQUAL "STATIC" OR _option STREQUAL "SHARED" OR _option STREQUAL "MODULE")
      set(_library_type "${_option}")
    elseif(_option STREQUAL "EXCLUDE_FROM_ALL")
      list(APPEND _library_options EXCLUDE_FROM_ALL)
    endif()
  endforeach()
  if(_library_type)
    add_library(${target} ${_library_type} ${_library_options} ${_sources})
  else()
    add_library(${target} ${_library_options} ${_sources})
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
  # Legacy `cuda_compile(objs foo.cu ... OPTIONS ...)`: consume the same
  # source/options split as the reference implementation. Returning OPTIONS
  # tokens as source names makes CMake generate bogus build rules.
  _cu2cce_legacy_parse("legacy" _cu2cce_sources _cu2cce_cmake_options _cu2cce_nvcc_options ${ARGN})
  _cu2cce_enable_cuda()
  set(${out_var})
  foreach(_src IN LISTS _cu2cce_sources)
    if(_src MATCHES "\\.cu$")
      if(_cu2cce_nvcc_options)
        set_source_files_properties("${_src}" PROPERTIES
          LANGUAGE CUDA COMPILE_OPTIONS "${_cu2cce_nvcc_options}")
      else()
        set_source_files_properties("${_src}" PROPERTIES LANGUAGE CUDA)
      endif()
    endif()
    list(APPEND ${out_var} "${_src}")
  endforeach()
endmacro()

macro(cuda_wrap_srcs cuda_target format generated_files)
  # Legacy CUDA_WRAP_SRCS parity: split OPTIONS before filtering sources,
  # mark CUDA inputs for the real CMake CUDA rules, and return only sources.
  string(TOUPPER "${format}" _cu2cce_format)
  if(_cu2cce_format STREQUAL "PTX" OR
     _cu2cce_format STREQUAL "CUBIN" OR
     _cu2cce_format STREQUAL "FATBIN")
    message(FATAL_ERROR
      "cu2cce: CUDA ${_cu2cce_format} generation is unsupported on Ascend puresimt")
  endif()
  _cu2cce_legacy_parse("${cuda_target}" _cu2cce_sources _cu2cce_cmake_options _cu2cce_nvcc_options ${ARGN})
  # CUDA_COMPILE_BASE appends PHONY as a helper-only marker after OPTIONS;
  # it must not become a compiler option on the real CUDA source.
  list(REMOVE_ITEM _cu2cce_nvcc_options PHONY)
  _cu2cce_enable_cuda()
  set(${generated_files})
  foreach(_src IN LISTS _cu2cce_sources)
    if(_src MATCHES "\\.cu$")
      if(_cu2cce_nvcc_options)
        set_source_files_properties("${_src}" PROPERTIES
          LANGUAGE CUDA COMPILE_OPTIONS "${_cu2cce_nvcc_options}")
      else()
        set_source_files_properties("${_src}" PROPERTIES LANGUAGE CUDA)
      endif()
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

# CMake command names are case-insensitive, so the lowercase public
# implementations above are also callable as CUDA_ADD_EXECUTABLE,
# CUDA_ADD_LIBRARY, CUDA_COMPILE, CUDA_WRAP_SRCS, and
# CUDA_INCLUDE_DIRECTORIES. Do not define duplicate uppercase wrappers: doing
# so would replace the same command and recursively call itself.

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

# Remaining low-level FindCUDA helper names from cu-bridge. These helpers keep
# configuration-time compatibility only; they never create CUDA artifacts or
# enable Ascend separable compilation.
macro(CUDA_FIND_HELPER_FILE _name _extension)
  set(_cu2cce_helper_path "${CMAKE_CURRENT_LIST_DIR}/FindCUDA/${_name}.${_extension}")
  set(CUDA_${_name} "${_cu2cce_helper_path}" CACHE INTERNAL
      "Location of ${_name}.${_extension}" FORCE)
  if(NOT EXISTS "${_cu2cce_helper_path}")
    if(CUDA_FIND_REQUIRED)
      message(FATAL_ERROR "${_name}.${_extension} is not provided by cu2cce-bridge")
    elseif(NOT CUDA_FIND_QUIETLY)
      message(STATUS "${_name}.${_extension} is not provided by cu2cce-bridge")
    endif()
  endif()
endmacro()

macro(CUDA_INCLUDE_NVCC_DEPENDENCIES dependency_file)
  set(CUDA_NVCC_DEPEND)
  set(CUDA_NVCC_DEPEND_REGENERATE FALSE)
  if(NOT EXISTS "${dependency_file}")
    file(WRITE "${dependency_file}" "# cu2cce FindCUDA generated file. Do not edit.\n")
  endif()
  include("${dependency_file}")
  if(CUDA_NVCC_DEPEND)
    foreach(_dependency IN LISTS CUDA_NVCC_DEPEND)
      if(NOT EXISTS "${_dependency}")
        set(CUDA_NVCC_DEPEND_REGENERATE TRUE)
      endif()
    endforeach()
  else()
    set(CUDA_NVCC_DEPEND_REGENERATE TRUE)
  endif()
  if(CUDA_NVCC_DEPEND_REGENERATE)
    set(CUDA_NVCC_DEPEND "${dependency_file}")
    file(WRITE "${dependency_file}" "# cu2cce FindCUDA generated file. Do not edit.\n")
  endif()
endmacro()

macro(cuda_find_library_local_first_with_path_ext _var _names _doc _path_ext)
  set(_cu2cce_library_suffixes)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    list(APPEND _cu2cce_library_suffixes
      "${_path_ext}lib/x64" "${_path_ext}lib64" "${_path_ext}libx64")
  endif()
  find_library(${_var}
    NAMES ${_names}
    PATHS "${CUDA_TOOLKIT_TARGET_DIR}" ENV CUDA_PATH ENV CUDA_LIB_PATH
    PATH_SUFFIXES ${_cu2cce_library_suffixes}
    DOC "${_doc}" NO_DEFAULT_PATH)
  if(NOT ${_var} AND NOT CMAKE_CROSSCOMPILING)
    find_library(${_var} NAMES ${_names} DOC "${_doc}")
  endif()
endmacro()

macro(cuda_find_library_local_first _var _names _doc)
  cuda_find_library_local_first_with_path_ext("${_var}" "${_names}" "${_doc}" "")
endmacro()

macro(find_library_local_first _var _names _doc)
  cuda_find_library_local_first("${_var}" "${_names}" "${_doc}")
endmacro()

macro(CUDA_PARSE_NVCC_OPTIONS _option_prefix)
  set(_cu2cce_found_config)
  foreach(_arg ${ARGN})
    foreach(_config ${CUDA_configuration_types})
      string(TOUPPER "${_config}" _config_upper)
      if(_arg STREQUAL "${_config_upper}")
        set(_cu2cce_found_config "_${_arg}")
        set(_arg)
      endif()
    endforeach()
    if(_arg)
      list(APPEND ${_option_prefix}${_cu2cce_found_config} "${_arg}")
    endif()
  endforeach()
endmacro()

function(CUDA_ADD_CUDA_INCLUDE_ONCE)
  get_directory_property(_cu2cce_include_directories INCLUDE_DIRECTORIES)
  list(FIND _cu2cce_include_directories "${CUDA_INCLUDE_DIRS}" _cu2cce_found)
  if(_cu2cce_found LESS 0)
    include_directories("${CUDA_INCLUDE_DIRS}")
  endif()
endfunction()

function(CUDA_BUILD_SHARED_LIBRARY shared_flag)
  set(_cu2cce_args ${ARGN})
  list(FIND _cu2cce_args SHARED _cu2cce_shared)
  list(FIND _cu2cce_args MODULE _cu2cce_module)
  list(FIND _cu2cce_args STATIC _cu2cce_static)
  if(_cu2cce_shared GREATER -1 OR _cu2cce_module GREATER -1 OR _cu2cce_static GREATER -1)
    set(_cu2cce_type)
  elseif(BUILD_SHARED_LIBS)
    set(_cu2cce_type SHARED)
  else()
    set(_cu2cce_type STATIC)
  endif()
  set(${shared_flag} "${_cu2cce_type}" PARENT_SCOPE)
endfunction()

function(CUDA_COMPUTE_BUILD_PATH path build_path)
  file(TO_CMAKE_PATH "${path}" _cu2cce_path)
  if(IS_ABSOLUTE "${_cu2cce_path}")
    string(FIND "${_cu2cce_path}" "${CMAKE_CURRENT_BINARY_DIR}" _cu2cce_binary_pos)
    if(_cu2cce_binary_pos EQUAL 0)
      file(RELATIVE_PATH _cu2cce_path "${CMAKE_CURRENT_BINARY_DIR}" "${_cu2cce_path}")
    else()
      file(RELATIVE_PATH _cu2cce_path "${CMAKE_CURRENT_SOURCE_DIR}" "${_cu2cce_path}")
    endif()
  endif()
  string(REGEX REPLACE "^[/]+" "" _cu2cce_path "${_cu2cce_path}")
  string(REPLACE ":" "_" _cu2cce_path "${_cu2cce_path}")
  string(REPLACE "../" "__/" _cu2cce_path "${_cu2cce_path}")
  string(REPLACE " " "_" _cu2cce_path "${_cu2cce_path}")
  get_filename_component(_cu2cce_path "${_cu2cce_path}" PATH)
  set(${build_path} "${_cu2cce_path}" PARENT_SCOPE)
endfunction()

function(_cuda_get_important_host_flags important_flags flag_string)
  if(CMAKE_GENERATOR MATCHES "Visual Studio")
    string(REGEX MATCHALL "/M[DT][d]?" _cu2cce_flags "${flag_string}")
  else()
    string(REGEX MATCHALL "-fPIC" _cu2cce_flags "${flag_string}")
  endif()
  set(${important_flags} "${_cu2cce_flags}" PARENT_SCOPE)
endfunction()

function(CUDA_COMPUTE_SEPARABLE_COMPILATION_OBJECT_FILE_NAME output_file_var cuda_target object_files)
  if(object_files)
    set(${output_file_var}
      "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${cuda_target}.dir/${cuda_target}_intermediate_link.o"
      PARENT_SCOPE)
  else()
    set(${output_file_var} "" PARENT_SCOPE)
  endif()
endfunction()

function(CUDA_LINK_SEPARABLE_COMPILATION_OBJECTS output_file cuda_target options object_files)
  if(object_files)
    message(FATAL_ERROR
      "cu2cce: CUDA separable compilation is unsupported on Ascend puresimt; "
      "set CUDA_SEPARABLE_COMPILATION OFF for ${cuda_target}")
  endif()
endfunction()
