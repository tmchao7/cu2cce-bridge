# CMake CUDA demo

This is an ordinary CUDA CMake project. Its `CMakeLists.txt` and `.cu` source do
not include ACL or Ascend-specific syntax.

On an Ascend/CANN host after building cu2cce-bridge:

```bash
export CUCC_PATH=/path/to/cu2cce-bridge/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$PATH

cmake_cce -S examples/cmake_demo -B /tmp/cu2cce-cmake-build
cmake --build /tmp/cu2cce-cmake-build
/tmp/cu2cce-cmake-build/cmake_vector_add
```

Expected output:

```text
cmake vectorAdd OK (N=1024)
```
