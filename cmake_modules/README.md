# CMake compatibility modules

These modules are installed for projects configured through `cmake_cce`.
They are intentionally minimal and conservative:

- `FindCUDAToolkit.cmake` exposes the bridge-backed `CUDA::cudart` target.
- `FindCUDA.cmake` supports the common legacy `find_package(CUDA)` plus
  lowercase and uppercase `cuda_add_executable`/`cuda_add_library`,
  `cuda_compile`, `cuda_wrap_srcs`, and common helper names.
  CUDA math-library adders remain explicit FATAL_ERROR paths when no
  validated Ascend equivalent exists.

They delegate to ordinary CMake targets and do not convert CUDA kernels or
produce `.cce` files. PTX/CUBIN/FATBIN generation and unvalidated CUDA
libraries remain unsupported or experimental.
