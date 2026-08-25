#include <cstdio>
extern "C" int run_kernel_a();
extern "C" int run_kernel_b();
int main() {
    if (run_kernel_a() != 0 || run_kernel_b() != 0) return 1;
    std::printf("make_cce build_compat OK (2 CUDA objects + host)\n");
    return 0;
}
