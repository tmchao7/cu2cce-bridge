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

add_library(cce_bridge_runtime SHARED ${BRIDGE_RUNTIME_SRC})

target_include_directories(cce_bridge_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${ASCEND_INCLUDE}
)

target_link_libraries(cce_bridge_runtime
    PRIVATE
        ${ASCEND_LIB}/libascendcl.so
        # CANN 9.x 推荐 libacl_rt.so；若链接失败改用 -lascendcl
        # ${ASCEND_LIB}/libacl_rt.so
)

target_compile_definitions(cce_bridge_runtime PRIVATE CUBRIDGE_EXPORT_SYMBOL)

install(TARGETS cce_bridge_runtime LIBRARY DESTINATION ${LIB_DEST_DIR})

# 安装 include 兼容头（供用户编译时 -I$CUDA_PATH/include 使用，CUDA_PATH 指向安装目录）
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
