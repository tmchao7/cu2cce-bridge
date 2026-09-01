# cu-bridge parity audit

Reference: `6ab1076` (3.8.0)
Reference path: `/Users/tmchao7/repos/09_expLearn/cu-bridge-muxi`
Implementation: `654f50d`

## cmake_make

| ID | Status | Reference | Implementation | Note |
|---|---|---|---|---|
| cmake_entry | ALIGNED | cmake_maca -> cmake_mock/cmake_assist | cmake_cce -> persistent CUDA-shaped shim | Explicit cmake_cce is the NPU entry; system cmake is not replaced. |
| cmake_two_phase | ALIGNED | configure mock, build real make_maca/ninja_maca | configure mock_nvcc, build cucc through generator wrapper | The backend compiler remains ccec, not mxcc. |
| make_wrapper | ALIGNED | make_maca builds link.json toolkit tree | make_cce builds manifest-driven toolkit tree | Manifest-based implementation is the NPU equivalent. |
| ninja_wrapper | ALIGNED | ninja_maca rebuilds shim and exports environment | ninja_cce repoints persistent shim and runs ninja | Direct ninja lifecycle is audited separately from CMake dispatch. |
| pre_wrapper | ALIGNED | pre_makefile prepares CUDA environment | pre_cce prepares CUDA-shaped Ascend environment | Direct argv execution is safer than reference eval string reconstruction. |
| host_link_carrier | EQUIVALENT_NPU | bin/gnu links through mxcc and MACA libraries | hostcc_cce keeps g++ and injects bridge/CANN libraries | Literal mxcc copying is invalid on Ascend. |
| system_cmake_replacement | NOT_APPLICABLE_NPU | USE_CMAKE_MACA=1 replaces system cmake | No system replacement; cmake_cce remains explicit | Avoids root requirement, global mutation, and restore failure risk. |
| legacy_findcuda_surface | ALIGNED | 29 reference FindCUDA commands | 34 current FindCUDA commands | Missing reference commands: none. |
| low_frequency_wrappers | EQUIVALENT_NPU | conf_bazel.json, conf_gfortran.json, MPI/Fortran wrappers | gfortran_cce/mpi_cce host-only carriers; no Bazel wrapper | Fortran/MPI host work is delegated safely; CUDA sources are rejected; Bazel remains a separate gap. |
| shim_manifest | ALIGNED | link.json + stub_link.json | config/shim_layout.json + build_shim_tree.py | Reference CUDA alias entries: 48; current manifest aliases: 9; 37 reference names are not aliases in the NPU shim and require policy/validation rather than blind copying. |

## cucc

| ID | Status | Reference | Implementation | Note |
|---|---|---|---|---|
| options_file | ALIGNED | --options-file recursive expansion | safe shlex response/options expansion | No shell evaluation; nesting is depth-limited. |
| query_skipstring | ALIGNED | 13 reference skipstring queries | 12 configured queries plus driver banner handling | -E is a query passthrough rule; -V remains an honest cu2cce driver banner. |
| rule_pipeline | EQUIVALENT_NPU | 68 filters / 209 replacer pairs | 145 drops / 22 replacements plus NPU policy | Rules preserve semantics where ccec has an equivalent and reject/drop the rest honestly. |
| architecture_policy | EQUIVALENT_NPU | CUCC_TARGETS -> --offload-arch for MACA | CU2CCE_NPU_ARCH/device_arch.sh -> --cce-aicore-arch=dav-* | sm_* is metadata or compatibility macro input, never a ccec target. |
| library_policy | EQUIVALENT_NPU | CUDA libraries map to MACA library names | validated bridge/CANN mapping, experimental cublas, map_to diagnostics | No unvalidated CUDA library is presented as an Ascend equivalent. |
| device_artifacts | NOT_APPLICABLE_NPU | PTX/CUBIN/FATBIN and MACA device-link paths | explicit reject for NVIDIA artifacts and unvalidated RDC/device-link | puresimt produces Ascend code through ccec and has no NVIDIA artifact path. |
| cucc_openmp_mode | EQUIVALENT_NPU | CUCC_USE_GOPENMP -> -fopenmp/-fopenmp=libgomp | CUCC_USE_GOPENMP -> ccec/clang -fopenmp/-fopenmp=libomp | The runtime name changes for the Ascend clang backend; real CANN link acceptance remains a server probe. |
| cucc_low_frequency_modes | DIAGNOSTIC_ONLY | CUCC_USE_PYTHON, Bazel/TF condition paths | Python driver is native; WCUDA_BAZEL_TF_BUILD is explicit reject | The safe NPU plan is explicit rejection, not MACA compiler forwarding. |
| observability | ALIGNED | debug echo and generated command trace | CU2CCE_DEBUG, CU2CCE_TRACE_FILE, --dryrun, --proxy-mode | Trace records argv decisions without source contents. |

## Summary

- `ALIGNED`: 10
- `EQUIVALENT_NPU`: 6
- `NOT_APPLICABLE_NPU`: 2
- `DIAGNOSTIC_ONLY`: 1
- `MISSING`: 0

## Artifact alignment

2026-09-01 起，parity 不再只记录入口和参数机制；`server_cmd.txt` 的 full profile
还对 cucc/CMake/Make/Ninja 的实际输出运行 artifact contract。契约检查产物类别、
相对路径、ELF/archive 类型、`.d`、CMake/Ninja metadata、CMake Makefiles 的
`link.txt`、`NEEDED`、RPATH/RUNPATH 以及禁止的 NVIDIA/MACA 中间产物。Ascend
与 MACA 的 device binary、runtime 库名和 host linker 不要求字节级相同，差异必须
落在 NPU-aware 说明中。

Full profile 对 `examples/build_compat/cmake` 同时运行 Unix Makefiles 与 Ninja，
使用同构的 `.o/.a/.so/ELF` artifact contract，并检查 Ninja 的
`CMakeFiles/rules.ninja` compiler metadata；direct `ninja_cce` 会再次验证该产物。
CMake 的每个 build case 先通过 `cmake_cce --build` 完成第二阶段切换，再检查
`CUDA_DIR/bin/nvcc -> cucc`，避免 mock-only build 被误报为真实产物 PASS。
