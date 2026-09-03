# cmake_modules/override —— CMake CUDA 语言模块 override（机制层）

`tools/cmake_cce` 通过 `-DCMAKE_USER_MAKE_RULES_OVERRIDE_CUDA=...` 让 CMake 在
其内置 `CMakeDetermineCUDACompiler.cmake` 开始时先 include 本目录
`CUDAOverrides.cmake`，从而在**不替换/不改动系统 CMake 模块文件**的前提下，
把 CUDA 语言探测固定到 cu2cce 的虚拟 CUDA 工具链目录：

- `CMAKE_CUDA_COMPILER` = `$CUDA_PATH/bin/nvcc`（虚拟工具链入口）；
- `CMAKE_CUDA_DEVICE_LINKER` / `CMAKE_CUDA_FATBINARY` = 同一虚拟工具链入口（避免 CMake
  去探测缺失的 nvlink/fatbinary）；
- `CMAKE_CUDA_COMPILER_TOOLKIT_*` 指向 虚拟工具链目录的 CUDA 形状目录；
- vendor 正则接受虚拟工具链入口 `--version` 的身份（NVIDIA-compat）；
- `CMAKE_CUDA_RUNTIME_LIBRARY=None`、`CMAKE_CUDA_ARCHITECTURES=OFF`（架构由
  `CU2CCE_NPU_ARCH` 决定，零硬编码）。

## 回退开关

```bash
CU2CCE_CMAKE_NO_OVERRIDE=1 cmake_cce -S . -B build
```

不设置该 env 时，`cmake_cce` 会注入 override（默认开）。这是机制移植，不含
任何 MACA/架构映射；不改写任何 CUDA 源码，不改系统 CMake 安装。