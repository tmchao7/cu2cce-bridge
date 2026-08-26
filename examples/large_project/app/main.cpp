#include <cstddef>
#include <cstdio>
#include <vector>

#include <cuda_runtime.h>

#include "comp/kernel.h"
#include "util/store.h"

int main() {
    constexpr int n = 8;
    std::vector<float> in(static_cast<std::size_t>(n), 1.0f);
    std::vector<float> out(static_cast<std::size_t>(n), 0.0f);
    if (fill_sequence(in.data(), static_cast<std::size_t>(n)) != 0) {
        std::fprintf(stderr, "large_project: fill_sequence failed\n");
        return 1;
    }
    if (run_triple(in.data(), out.data(), n) != 0) {
        std::fprintf(stderr, "large_project: run_triple failed\n");
        return 1;
    }
    for (int i = 0; i < n; ++i) {
        float expected = (static_cast<float>(i) + 0.5f) * 3.0f;
        if (out[static_cast<std::size_t>(i)] != expected) {
            std::fprintf(stderr, "large_project: element %d got %f want %f\n",
                         i, out[static_cast<std::size_t>(i)], expected);
            return 1;
        }
    }
    std::printf("large_project OK (multi-dir, cross-target deps, install/export)\n");
    return 0;
}