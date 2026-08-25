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

# 产物：libcce_bridge_runtime.so（对应 cu-bridge 的 bridge 运行库，装入 ${INSTALL_PREFIX}/lib）
set(LIB_DEST_DIR "${CMAKE_INSTALL_PREFIX}/lib")

file(GLOB BRIDGE_RUNTIME_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/bridge/runtime/src/*.cpp"
)
# Experimental secondary capability: cublas→aclblas. It is compiled into the
# bridge library for standalone validation, but is not part of puresimt acceptance.
file(GLOB BRIDGE_BLAS_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/bridge/blas/src/*.cpp"
)

add_library(cce_bridge_runtime SHARED ${BRIDGE_RUNTIME_SRC} ${BRIDGE_BLAS_SRC})

target_include_directories(cce_bridge_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${ASCEND_INCLUDE}
        # wrapper 源文件 #include "cuda_runtime_wrapper.h" / "cublas_v2_wrapper.h"
        # 等（相对 include/bridge/{runtime,blas}），需补子目录（与 mock syntax_check 一致）
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bridge/runtime
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bridge/blas
)

target_link_libraries(cce_bridge_runtime
    PRIVATE
        ${ASCEND_LIB}/libascendcl.so
        ${ASCEND_LIB}/libacl_cblas.so
        # CANN 9.x 推荐 libacl_rt.so；若链接失败改用 -lascendcl
        # ${ASCEND_LIB}/libacl_rt.so
)

target_compile_definitions(cce_bridge_runtime PRIVATE CUBRIDGE_EXPORT_SYMBOL)

install(TARGETS cce_bridge_runtime LIBRARY DESTINATION ${LIB_DEST_DIR})

# 安装 include 兼容头（含 bridge/kernel/simt_adaptor.h，供用户编译时 -I$CUDA_PATH/include 使用）
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

# 安装 cucc 驱动（CUCC_PATH/bin）
install(PROGRAMS bin/cucc DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# 安装用户可直接调用的构建入口到 bin；nvcc_entry/mock 是内部 shim。
install(PROGRAMS tools/make_cce tools/cmake_cce
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS tools/nvcc_entry.sh tools/cu2cce_driver.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/tools)
install(DIRECTORY tools/mock DESTINATION ${CMAKE_INSTALL_PREFIX}/tools)

# Optional CMake compatibility modules used by cmake_cce. These modules only
# expose validated bridge-backed targets and legacy build macros; they do not
# perform CUDA kernel conversion.
install(DIRECTORY cmake_modules/ DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake_modules)
install(FILES config/cu2cce_rules.json
        DESTINATION ${CMAKE_INSTALL_PREFIX}/config)

# Native cu2cce package. This is intentionally separate from CUDA::cudart:
# it identifies the bridge itself and does not claim complete CUDA library
# compatibility.
install(FILES
    cmake/cu2cce-bridgeConfig.cmake
    cmake/cu2cce-bridgeConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/cu2cce-bridge)

# 安装示例（真机验证资产）
install(DIRECTORY examples/ DESTINATION ${CMAKE_INSTALL_PREFIX}/examples)
# NVIDIA CUDA Samples helper headers are dependencies of the unmodified puresimt
# example sources; install them alongside the examples without changing those sources.
install(DIRECTORY third_party/nvidia_cuda_samples/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/third_party/nvidia_cuda_samples)
