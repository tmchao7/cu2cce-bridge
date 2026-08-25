# Modern CMake build compatibility smoke

This project covers a nested CUDA library target, two `.cu` translation units,
a host-only C++ source, target linking, and install/export generation.

```bash
cmake_cce -S examples/build_compat/cmake -B /tmp/cu2cce-build-compat
cmake --build /tmp/cu2cce-build-compat
/tmp/cu2cce-build-compat/build_compat_app
```
