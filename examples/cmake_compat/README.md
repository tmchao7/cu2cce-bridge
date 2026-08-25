# CMake compatibility smoke

This is an ordinary CUDA CMake project used by the Ascend regression suite.
It covers:

- multiple `.cu` translation units;
- a static CUDA library;
- a shared CUDA library;
- a host-only C++ executable;
- `find_package(CUDAToolkit)` and `CUDA::cudart`.

Use it through `cmake_cce`:

```bash
cmake_cce -S examples/cmake_compat -B /tmp/cu2cce-cmake-compat
cmake --build /tmp/cu2cce-cmake-compat
/tmp/cu2cce-cmake-compat/cmake_compat_app
```

Expected output:

```text
cmake_compat OK (multi-cu, static, shared, CUDA::cudart)
```
