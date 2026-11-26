# tools/real_test

## main 主线

`server_cmd.txt` 是 main / puresimt 唯一回归入口。它在 Ascend 真机上执行：

1. 设备健康门禁；
2. CANN 环境与 bridge 重建；
3. cucc 直通、CMake/Make/Ninja 构建和 artifact contract；
4. 官方 NVIDIA CUDA Samples 与最小 CUDA operator 编译、链接、运行；
5. 按 profile 输出主线/诊断汇总。

```bash
cd $HOME/workspace/repos/cu2cce-bridge
REMOTE="${CU2CCE_REGRESSION_REMOTE:-origin}"
git fetch "$REMOTE" main
git pull --ff-only "$REMOTE" main
bash tools/real_test/clean.sh
CU2CCE_REGRESSION_REMOTE="$REMOTE" CU2CCE_REGRESSION_PROFILE=full \
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
`docs/02_能力清单/PURESIMT_COMPATIBILITY_MATRIX.md`。矩阵是手工维护的评审文档，
`server_cmd.txt` 是执行事实来源。


## Experimental BLAS

默认不会运行 BLAS 实验，也不会影响 `PURESIMT_REGRESSION_PASS`。需要运行时：

```bash
CU2CCE_BLAS_TEST=1 bash tools/real_test/server_cmd.txt
```

BLAS 日志位于 `build/real_test/blas/`，成功标志为
`BLAS_EXPERIMENTAL_REGRESSION_PASS`。


## CMake integration (core scope validated; advanced features experimental)

The CMake path is tested on the CMake compatibility branch with these projects:

- `examples/cmake_demo/` — standard CUDA CMake;
- `examples/cmake_compat/` — multiple `.cu`, static/shared libraries and `CUDA::cudart`;
- `examples/cmake_compat_legacy/` — legacy `FindCUDA` macros.

```bash
cmake_cce -S examples/cmake_demo -B /tmp/cu2cce-cmake-build
cmake_cce --build /tmp/cu2cce-cmake-build
/tmp/cu2cce-cmake-build/cmake_vector_add
```

The core integration is validated on Ascend. The smoke test passed the standard,
multi-CUDA/static/shared, `CUDA::cudart`, legacy `FindCUDA`, and Ninja projects on
2026-09-02. Advanced CUDA device-link, PTX/CUBIN/FATBIN, and unvalidated CUDA library
packages remain experimental or unsupported. Set `CU2CCE_CMAKE_SHIM_DIR` to keep the compiler virtual toolkit directory in a persistent location; use a
different directory per concurrent project. The artifact contract additionally checks
`.o/.a/.so/ELF`, `.d`, `NEEDED`, RPATH/RUNPATH and forbidden NVIDIA intermediates.

## cucc rule diagnostics

The compiler driver is rule-driven. The installed policy is
`bin/conf.json`, shared by normal `cucc` and the CMake virtual toolkit nvcc entry.
For diagnostics:

```bash
CU2CCE_DEBUG=1 cucc -c examples/puresimt/vectorAdd_minimal.cu -o /tmp/vector.o
CU2CCE_TRACE_FILE=/tmp/cu2cce_trace.json \
cucc -c examples/puresimt/vectorAdd_minimal.cu -o /tmp/vector.o
```

The default policy warns and drops NVIDIA-only flags, preserves ordinary compiler
flags, and rejects PTX/CUBIN/FATBIN/device-link features.

## Build compatibility smoke

The current compatibility branch also exercises:

- `examples/build_compat/cmake/` — multi-CUDA static library plus host C++ and install/export;
- `examples/build_compat/make/` — Makefile `nvcc` interception, multiple objects and dependency flags;
- `ninja_cce` — wrapper for a CMake-generated Ninja build directory.

The final server summary reports `BUILD_COMPATIBILITY_REGRESSION_PASS` only when the
required Make/Ninja checks pass in addition to puresimt and CMake checks.


## External NVIDIA CUDA Samples

The build compatibility branch fetches a pinned upstream sample tree into
`build/real_test/external/nvidia_cuda_samples/` and configures the unmodified
selected sample CMake projects through `cmake_cce`. The pinned profile is
`v12.8` revision `db3eea23946bca2e90a75eca2b5b3e07158a9e11`; it is an experimental
build-graph test, not a full CUDA 12.8 compatibility claim.


## libc header modes

The ordinary puresimt regression keeps the default `CU2CCE_LIBC_MODE=shadow` mode.
External C++/CUDA projects that include libstdc++ headers should use
`CU2CCE_LIBC_MODE=system`; the external CUDA Samples profile sets this automatically.
