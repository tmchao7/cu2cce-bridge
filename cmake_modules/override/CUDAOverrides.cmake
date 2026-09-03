# CUDA language-module override for cu2cce-bridge.
#
# Included by CMake at the start of its stock CMakeDetermineCUDACompiler.cmake
# via the CMAKE_USER_MAKE_RULES_OVERRIDE_CUDA hook (see tools/cmake_cce). It
# pins the deterministic compiler identity before the stock detection runs:
#
#   - the CUDA compiler is the cu2cce virtual-toolkit nvcc (CUDA_PATH/bin/nvcc),
#   - the device linker and fatbinary tool are the same virtual-toolkit entry, so modern CMake
#     (>=3.18) never probes for a missing nvlink/fatbinary binary,
#   - the CUDA toolkit root is the virtual toolkit's CUDA-shaped directory,
#   - the compiler vendor regex accepts the mock's --version identity.
#
# This is mechanism-only: no MACA/arch mapping, no hardcoded sm_* list, no
# pretend version claims other than the CUDA-compat 11.6 identity the virtual toolkit entry
# already reports. To bypass the override entirely (old pure -DCMAKE_CUDA_*+
# module-path behavior), run cmake_cce with CU2CCE_CMAKE_NO_OVERRIDE=1.

if(NOT DEFINED ENV{CUDA_PATH})
  message(FATAL_ERROR "cu2cce override: CUDA_PATH must point at the cu2cce virtual toolkit directory (set by cmake_cce)")
endif()

set(CMAKE_CUDA_COMPILER "$ENV{CUDA_PATH}/bin/nvcc" CACHE FILEPATH "cu2cce virtual-toolkit nvcc" FORCE)
set(CMAKE_CUDA_DEVICE_LINKER "$ENV{CUDA_PATH}/bin/nvcc")
set(CMAKE_CUDA_FATBINARY   "$ENV{CUDA_PATH}/bin/nvcc")
set(CMAKE_CUDA_HOST_LINK_LAUNCHER "$ENV{CUDA_PATH}/bin/nvcc")

set(CMAKE_CUDA_COMPILER_TOOLKIT_ROOT    "$ENV{CUDA_PATH}")
set(CMAKE_CUDA_COMPILER_TOOLKIT_PREFIX  "")
set(CMAKE_CUDA_COMPILER_TOOLKIT_LIBRARY_ROOT "$ENV{CUDA_PATH}/lib64")
set(CMAKE_CUDA_COMPILER_TOOLKIT_INCLUDE_DIRECTORIES "$ENV{CUDA_PATH}/include")

# The virtual toolkit entry reports an NVIDIA-compatible driver identity so stock detection and
# projects that branch on CMAKE_CUDA_COMPILER_ID keep working.
set(CMAKE_CUDA_COMPILER_ID_VENDOR_REGEX_NVIDIA "nvcc: NVIDIA \\(R\\) Cuda compiler driver")
set(CMAKE_CUDA_COMPILER_ID_FLAGS_ALWAYS "-v")
set(CMAKE_CUDA_COMPILER_ID_REQUIRE_SUCCESS ON)

# The runtime library is the bridge (libcce_bridge_runtime.so); CMake must not
# inject cudart_static/cudadevrt link flags.
set(CMAKE_CUDA_RUNTIME_LIBRARY "None" CACHE STRING "cu2cce bridge runtime" FORCE)

# No NVIDIA architecture list: the Ascend arch is selected by CU2CCE_NPU_ARCH.
set(CMAKE_CUDA_ARCHITECTURES OFF CACHE STRING "cu2cce: Ascend target arch selected by CU2CCE_NPU_ARCH" FORCE)