#include <cstddef>

extern "C" int fill_sequence(float *data, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        data[i] = static_cast<float>(i) + 0.5f;
    }
    return 0;
}