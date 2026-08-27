#include <cstddef>
#include <cstdio>
#include <vector>

extern "C" int run_mul2(const float *in, float *out, int n);
extern "C" int run_add1(const float *in, float *out, int n);

int main() {
    constexpr int n = 4;
    std::vector<float> in(static_cast<std::size_t>(n), 2.0f);
    std::vector<float> out(static_cast<std::size_t>(n), 0.0f);
    if (run_mul2(in.data(), out.data(), n) != 0) {
        std::fprintf(stderr, "multi_cu: run_mul2 failed\n");
        return 1;
    }
    if (run_add1(in.data(), out.data(), n) != 0) {
        std::fprintf(stderr, "multi_cu: run_add1 failed\n");
        return 1;
    }
    for (int i = 0; i < n; ++i) {
        if (out[static_cast<std::size_t>(i)] != in[static_cast<std::size_t>(i)] * 2.0f + 1.0f) {
            std::fprintf(stderr, "multi_cu: element %d wrong\n", i);
            return 1;
        }
    }
    std::printf("multi_cu diagnostic OK (two .cu, one static library)\n");
    return 0;
}