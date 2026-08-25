#include <cstdio>
#include "compat_api.h"

int main() {
    if (run_static_kernel_test() != 0) {
        std::fprintf(stderr, "static CUDA library test failed\n");
        return 1;
    }
    if (run_shared_kernel_test() != 0) {
        std::fprintf(stderr, "shared CUDA library test failed\n");
        return 1;
    }
    std::printf("cmake_compat OK (multi-cu, static, shared, CUDA::cudart)\n");
    return 0;
}
