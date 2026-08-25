# tools/real_test

## main 主线

`server_cmd.txt` 是 main / puresimt 唯一回归入口。它在 Ascend 真机上执行：

1. 设备健康门禁；
2. CANN 环境与 bridge 重建；
3. 官方 NVIDIA CUDA Samples 与最小 CUDA operator 编译、链接、运行；
4. `PURESIMT_REGRESSION_PASS` 汇总。

```bash
bash tools/real_test/server_cmd.txt
```

## 支持脚本

- `env_probe.sh`：探测 CANN/ccec/设备/头库，生成真机环境信息。
- `build_bridge.sh`：构建并安装 `libcce_bridge_runtime.so`。
- `server_cmd.txt`：正式 puresimt 验收入口；设置 `CU2CCE_BLAS_TEST=1` 时附加运行 experimental BLAS。
- `blas_regression.sh`：独立执行 cublasSgemm → aclblasGemmEx 实验回归。
- `clean.sh`：清空并重建整个 `build/real_test`，包括旧实验残留、install、cmake-build、日志和 probe。
- `diag/`：107002、ACL 分配和 runtime 组合排障资产，不属于默认回归。

main 只维护 `server_cmd.txt` 主线入口；converter-only 测试已经删除。


用例的完整分类和 `SUPPORTED/EXPERIMENTAL/UNSUPPORTED` 状态见
`docs/03_Comparison/PURESIMT_COMPATIBILITY_MATRIX.md`。矩阵是手工维护的评审文档，
`server_cmd.txt` 是执行事实来源。


## Experimental BLAS

默认不会运行 BLAS 实验，也不会影响 `PURESIMT_REGRESSION_PASS`。需要运行时：

```bash
CU2CCE_BLAS_TEST=1 bash tools/real_test/server_cmd.txt
```

BLAS 日志位于 `build/real_test/blas/`，成功标志为
`BLAS_EXPERIMENTAL_REGRESSION_PASS`。


## CMake integration (experimental)

The CMake path is tested on `exp/cmake-cce` with the standard CUDA project in
`examples/cmake_demo/`:

```bash
cmake_cce -S examples/cmake_demo -B /tmp/cu2cce-cmake-build
cmake --build /tmp/cu2cce-cmake-build
/tmp/cu2cce-cmake-build/cmake_vector_add
```

The integration is currently experimental until the Ascend real-device smoke
test reports `CMAKE_CCE_REGRESSION_PASS`.
