cmake_minimum_required(VERSION 3.16)
project(cu2cce-bridge C CXX)

# ============================================================
# cu2cce-bridge —— 把 CUDA 程序迁移到昇腾 NPU 的桥接层
# 仿照 cu-bridge 顶层 CMakeLists.txt（Apache-2.0）。
#
# ⚠️ 仅在装有 CANN 的昇腾真机上构建（本机 macOS 无 CANN，勿在本机构建）。
# 需要环境：
#   ASCEND_TOOLKIT_HOME 指向 CANN 安装目录（默认 /usr/local/Ascend/ascend-toolkit/latest）
#   LD_LIBRARY_PATH 包含 ${ASCEND_TOOLKIT_HOME}/lib64
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED ENV{ASCEND_TOOLKIT_HOME})
    set(ASCEND_TOOLKIT_HOME "/usr/local/Ascend/ascend-toolkit/latest")
else()
    set(ASCEND_TOOLKIT_HOME $ENV{ASCEND_TOOLKIT_HOME})
endif()
message(STATUS "ASCEND_TOOLKIT_HOME=${ASCEND_TOOLKIT_HOME}")

set(ASCEND_INCLUDE "${ASCEND_TOOLKIT_HOME}/include")
set(ASCEND_LIB     "${ASCEND_TOOLKIT_HOME}/lib64")

# CANN 9.x publishes the public Runtime in libacl_rt.so. Keep the older
# libascendcl.so spelling only as a compatibility fallback for older images.
set(ASCEND_RUNTIME_LIB "")
set(_ASCEND_RUNTIME_CANDIDATES
    "${ASCEND_LIB}/libacl_rt.so"
    "${ASCEND_LIB}/libascendcl.so"
    "$ENV{CANN_BASE}/lib64/libacl_rt.so"
    "$ENV{CANN_BASE}/lib64/libascendcl.so"
    "$ENV{CANN_BASE}/runtime/lib64/libacl_rt.so"
    "$ENV{CANN_BASE}/runtime/lib64/libascendcl.so")
foreach(_runtime_candidate IN LISTS _ASCEND_RUNTIME_CANDIDATES)
    if(NOT ASCEND_RUNTIME_LIB AND EXISTS "${_runtime_candidate}")
        set(ASCEND_RUNTIME_LIB "${_runtime_candidate}")
    endif()
endforeach()
if(NOT ASCEND_RUNTIME_LIB)
    # Keep configure diagnostics useful on non-CANN hosts; the actual build
    # will fail clearly if no usable Runtime is supplied.
    set(ASCEND_RUNTIME_LIB "${ASCEND_LIB}/libacl_rt.so")
    message(WARNING "cu2cce: no CANN Runtime library found; assuming ${ASCEND_RUNTIME_LIB}")
elseif(ASCEND_RUNTIME_LIB MATCHES "libascendcl\\.so$")
    message(STATUS "cu2cce: using legacy CANN Runtime ${ASCEND_RUNTIME_LIB}; prefer libacl_rt.so")
else()
    message(STATUS "cu2cce: using canonical CANN Runtime ${ASCEND_RUNTIME_LIB}")
endif()

# 产物：libcce_bridge_runtime.so（对应 cu-bridge 的 bridge 运行库，装入 ${INSTALL_PREFIX}/lib）
set(LIB_DEST_DIR "lib")

file(GLOB BRIDGE_RUNTIME_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/bridge/runtime/src/*.cpp"
)
# Experimental secondary capability: cublas→aclblas. It is compiled into the
# bridge library for standalone validation, but is not part of puresimt acceptance.
file(GLOB BRIDGE_BLAS_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/bridge/blas/src/*.cpp"
)

add_library(cce_bridge_runtime SHARED ${BRIDGE_RUNTIME_SRC} ${BRIDGE_BLAS_SRC})
set_target_properties(cce_bridge_runtime PROPERTIES EXPORT_NAME runtime)

target_include_directories(cce_bridge_runtime
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${ASCEND_INCLUDE}
        # wrapper 源文件 #include "cuda_runtime_wrapper.h" / "cublas_v2_wrapper.h"
        # 等（相对 include/bridge/{runtime,blas}），需补子目录（与 mock syntax_check 一致）
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bridge/runtime
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bridge/blas
)

target_link_libraries(cce_bridge_runtime
    PRIVATE
        ${ASCEND_RUNTIME_LIB}
        ${ASCEND_LIB}/libacl_cblas.so
)

target_compile_definitions(cce_bridge_runtime PRIVATE CUBRIDGE_EXPORT_SYMBOL)

install(TARGETS cce_bridge_runtime
        EXPORT cu2cceTargets
        LIBRARY DESTINATION ${LIB_DEST_DIR}
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)
install(EXPORT cu2cceTargets
        NAMESPACE cu2cce::
        FILE cu2cceTargets.cmake
        DESTINATION lib/cmake/cu2cce-bridge)

# 安装 include 兼容头（含 bridge/kernel/simt_adaptor.h，供用户编译时 -I$CUDA_PATH/include 使用）
install(DIRECTORY include/ DESTINATION include)

# 安装 cucc 驱动（CUCC_PATH/bin）
install(PROGRAMS bin/cucc DESTINATION bin)

# Keep the installed nvcc compatibility entry as a regular wrapper file.
# The generated virtual toolkit still uses manifest-controlled links internally,
# but the install tree itself does not rely on symlink compatibility files.
install(PROGRAMS bin/nvcc DESTINATION bin)

# 安装用户可直接调用的构建入口到 bin；nvcc_entry/mock 是虚拟 CUDA 工具链内部入口。
install(PROGRAMS tools/make_cce tools/cmake_cce tools/ninja_cce
        DESTINATION bin)
install(PROGRAMS tools/cu2cce_driver.py tools/validate_cucc_rules.py
        tools/validate_cuda_api_parity.py tools/generate_cuda_api_parity_inventory.py
        tools/compare_cubridge_parity.py tools/virtual_toolkit_env.sh
        DESTINATION tools)
install(PROGRAMS tools/device_arch.sh tools/pre_cce tools/hostcc_cce
        tools/gfortran_cce tools/mpi_cce
        DESTINATION tools)
install(FILES bin/conf.json DESTINATION bin)
install(PROGRAMS bin/gnu bin/gfortran
        DESTINATION bin)
install(DIRECTORY tools/mock DESTINATION tools)
# Explicit PROGRAMS install keeps the mock entrypoints executable on CMake
# versions/filesystems that do not preserve mode bits for DIRECTORY installs.
install(PROGRAMS tools/mock/virtualDir.py tools/mock/mock_nvcc.py
        tools/mock/nvcc_entry.sh
        DESTINATION tools/mock)
install(PROGRAMS tools/real_test/fetch_external_cuda_samples.sh
        tools/real_test/verify_artifacts.py
        DESTINATION tools/real_test)

# Optional CMake compatibility modules used by cmake_cce. These modules only
# expose validated bridge-backed targets and legacy build macros; they do not
# perform CUDA kernel conversion.
install(DIRECTORY cmake_modules/ DESTINATION cmake_modules)
install(FILES
    config/artifact_contract.json
    config/cuda_api_parity_inventory.json
    DESTINATION config)
install(FILES tools/link.json DESTINATION tools)

# Native cu2cce package. This is intentionally separate from CUDA::cudart:
# it identifies the bridge itself and does not claim complete CUDA library
# compatibility.
install(FILES
    cmake/cu2cce-bridgeConfig.cmake
    cmake/cu2cce-bridgeConfigVersion.cmake
    DESTINATION lib/cmake/cu2cce-bridge)

# 安装示例（真机验证资产）
install(DIRECTORY examples/ DESTINATION examples)
# NVIDIA CUDA Samples helper headers are dependencies of the unmodified puresimt
# example sources; install them alongside the examples without changing those sources.
install(DIRECTORY third_party/nvidia_cuda_samples/
        DESTINATION third_party/nvidia_cuda_samples)
