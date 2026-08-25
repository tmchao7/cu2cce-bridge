# cu2cce-bridge CMake package configuration.
# Installed relative to <prefix>/lib/cmake/cu2cce-bridge/.

get_filename_component(_CU2CCE_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(cu2cce-bridge_VERSION "0.1.0")

if(NOT TARGET cu2cce::runtime)
  if(NOT EXISTS "${_CU2CCE_PREFIX}/lib/libcce_bridge_runtime.so")
    set(cu2cce-bridge_FOUND FALSE)
    if(cu2cce-bridge_FIND_REQUIRED)
      message(FATAL_ERROR
        "cu2cce-bridge runtime not found at "
        "${_CU2CCE_PREFIX}/lib/libcce_bridge_runtime.so")
    endif()
    return()
  endif()

  add_library(cu2cce::runtime SHARED IMPORTED)
  set_target_properties(cu2cce::runtime PROPERTIES
    IMPORTED_LOCATION "${_CU2CCE_PREFIX}/lib/libcce_bridge_runtime.so"
    INTERFACE_INCLUDE_DIRECTORIES "${_CU2CCE_PREFIX}/include"
  )
endif()

set(cu2cce-bridge_FOUND TRUE)
