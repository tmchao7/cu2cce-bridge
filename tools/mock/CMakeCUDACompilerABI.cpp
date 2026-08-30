// CUDA ABI probe stand-in (cu-bridge tools/mock/CMakeCUDACompilerABI.cpp
// parity, no backend-specific artifacts).
//
// When CMake runs its CMakeCUDACompilerABI.cu probe through the nvcc shim,
// mock_nvcc.py compiles THIS source with the host g++ instead of reaching
// ccec. The compiled binary embeds the INFO:... ABI strings from
// CMakeCompilerABI.h so CMake's binary scan succeeds, and the run path
// deliberately reports "no CUDA devices" so architecture auto-detection
// fails cleanly (the cucc CMake layer forces CUDA_ARCHITECTURES=OFF;
// the Ascend target is selected through CU2CCE_NPU_ARCH).

#include "CMakeCompilerABI.h"

#include <cstdio>

static bool cmakeCompilerCUDAArch()
{
    // Honest answer without a CUDA device: the NPU backend has no NVIDIA
    // device enumeration, so arch detection reports none and exits cleanly.
    std::fprintf(stderr, "No CUDA devices found.\n");
    return -1;
}

int main(int argc, char* argv[])
{
  int require = 0;
  require += info_sizeof_dptr[argc];
  require += info_byte_order_big_endian[argc];
  require += info_byte_order_little_endian[argc];
#if defined(ABI_ID)
  require += info_abi[argc];
#endif
  static_cast<void>(argv);

  if (!cmakeCompilerCUDAArch()) {
    // Convince the compiler that the non-zero return value depends
    // on the info strings so they are not optimized out.
    return require ? -1 : 1;
  }

  return 0;
}
