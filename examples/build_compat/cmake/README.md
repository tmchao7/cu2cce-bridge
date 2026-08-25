# Modern CMake build compatibility smoke

This project covers a static CUDA library, a shared CUDA library, two `.cu`
translation units, a host-only C++ source, target linking, and install/export
generation. Each linked library contains one `.cu` translation unit because
ccec currently emits duplicate eager registration helpers when multiple `.cu`
objects are placed in one static archive.

```bash
cmake_cce -S examples/build_compat/cmake -B /tmp/cu2cce-build-compat
cmake --build /tmp/cu2cce-build-compat
/tmp/cu2cce-build-compat/build_compat_app
```
