# Examples

`examples/puresimt/` is the maintained example set for the `main` branch. Every
source file uses ordinary CUDA syntax; no file contains ACL calls, Ascend C syntax,
`__simt_vf__`, `cce::async_invoke`, or hidden bridge parameters.

## Official NVIDIA sources

- `puresimt/vectorAdd/vectorAdd.cu`
- `puresimt/matrixMul/matrixMul.cu`

These files retain their upstream CUDA source and are the strongest no-source-rewrite
compatibility evidence.

## Minimal CUDA semantic/operator sources

- Memory and launch: `vectorAdd_minimal.cu`, `matrixMul_minimal.cu`,
  `pitchedMemcpy_minimal.cu`, `multiKernel_minimal.cu`, `event_minimal.cu`.
- Arithmetic and types: `core_ops_minimal.cu`, `math_minimal.cu`, `half_minimal.cu`,
  `bf16x2_minimal.cu`.
- SIMT and synchronization: `warpShuffle_minimal.cu`, `vote_minimal.cu`,
  `scan_minimal.cu`, `reduction_minimal.cu`, `dynamicShared_minimal.cu`,
  `threadFence_minimal.cu`, `index3D_minimal.cu`.
- Parallel operators: `atomicAdd_minimal.cu`, `atomicVariants_minimal.cu`,
  `dot_minimal.cu`, `stencil_minimal.cu`, `transpose_minimal.cu`.

The complete list and expected output strings are maintained in
`tools/real_test/server_cmd.txt`, the only main-branch real-device regression entrypoint.


完整测试分类、API、数据类型、内存模型和支持状态见
`docs/02_能力清单/PURESIMT_COMPATIBILITY_MATRIX.md`。


项目整体原理和测试设计哲学见
`docs/01_架构设计/01_global_overview.md`；发布前状态见
`docs/04_对比与验证/RELEASE_READINESS_2026-09-04.md`。

## CMake compatibility examples

- `cmake_demo/`: standard `project(... LANGUAGES CUDA)` smoke.
- `cmake_compat/`: multiple `.cu` files, static/shared CUDA libraries, a
  host-only executable and `CUDA::cudart`.
- `cmake_compat_legacy/`: minimal legacy `FindCUDA` and
  `cuda_add_executable` compatibility.

These projects are configured with `cmake_cce`; see the CMake section of
`QUICKSTART.md`.
