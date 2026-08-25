# Legacy CUDA CMake smoke

This project intentionally uses the older CUDA CMake API:

```cmake
find_package(CUDA REQUIRED)
cuda_add_executable(legacy_vector_add legacy_vector_add.cu)
```

It is supported by the minimal `FindCUDA.cmake` installed by `cmake_cce`.
The implementation still builds the original CUDA source directly through
`cucc → ccec`; it does not generate `.cce` or wrapper code.
