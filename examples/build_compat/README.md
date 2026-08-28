# Build compatibility examples

- `cmake/`: modern CMake, static CUDA library, two `.cu` files, host C++ and install/export.
- `make/`: Makefile, `nvcc` interception, response/dependency-oriented flags, multiple objects.

These are synthetic compatibility tests. They do not claim support for CUDA
libraries, device-link, PTX/CUBIN/FATBIN, or CUDA source rewriting.
