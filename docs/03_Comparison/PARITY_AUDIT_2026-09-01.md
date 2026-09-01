# cu-bridge parity audit

Reference: `6ab1076` (3.8.0)
Implementation: `190a93e`

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
| legacy_findcuda_surface | MISSING | 29 reference FindCUDA commands | 22 current FindCUDA commands | Missing reference commands: _cuda_get_important_host_flags, cuda_add_cuda_include_once, cuda_build_shared_library, cuda_compute_build_path, cuda_compute_separable_compilation_object_file_name, cuda_find_helper_file, cuda_find_library_local_first, cuda_find_library_local_first_with_path_ext, cuda_include_nvcc_dependencies, cuda_link_separable_compilation_objects, cuda_parse_nvcc_options, find_library_local_first. |
| low_frequency_wrappers | MISSING | conf_bazel.json, conf_gfortran.json, MPI/Fortran wrappers | No Bazel/gfortran/MPI wrapper | Audit-only gap; implement only safe detection/diagnostic or explicit NPU rejection. |
| shim_manifest | ALIGNED | link.json + stub_link.json | config/shim_layout.json + build_shim_tree.py | Reference CUDA alias entries: 48; current manifest aliases: 9; 37 reference names are not aliases in the NPU shim and require policy/validation rather than blind copying. |

## cucc

| ID | Status | Reference | Implementation | Note |
|---|---|---|---|---|
| options_file | ALIGNED | --options-file recursive expansion | safe shlex response/options expansion | No shell evaluation; nesting is depth-limited. |
| query_skipstring | ALIGNED | 13 reference skipstring queries | 12 configured queries plus driver banner handling | -E is a query passthrough rule; -V remains an honest cu2cce driver banner. |
| rule_pipeline | EQUIVALENT_NPU | 68 filters / 209 replacer pairs | 146 drops / 14 replacements plus NPU policy | Rules preserve semantics where ccec has an equivalent and reject/drop the rest honestly. |
| architecture_policy | EQUIVALENT_NPU | CUCC_TARGETS -> --offload-arch for MACA | CU2CCE_NPU_ARCH/device_arch.sh -> --cce-aicore-arch=dav-* | sm_* is metadata or compatibility macro input, never a ccec target. |
| library_policy | EQUIVALENT_NPU | CUDA libraries map to MACA library names | validated bridge/CANN mapping, experimental cublas, map_to diagnostics | No unvalidated CUDA library is presented as an Ascend equivalent. |
| device_artifacts | NOT_APPLICABLE_NPU | PTX/CUBIN/FATBIN and MACA device-link paths | explicit reject for NVIDIA artifacts and unvalidated RDC/device-link | puresimt produces Ascend code through ccec and has no NVIDIA artifact path. |
| cucc_low_frequency_modes | MISSING | CUCC_USE_GOPENMP, CUCC_USE_PYTHON, Bazel/gfortran condition paths | Python driver is native; no Bazel/gfortran/MPI dispatch | The safe NPU plan is explicit diagnostics/rejection, not MACA compiler forwarding. |
| observability | ALIGNED | debug echo and generated command trace | CU2CCE_DEBUG, CU2CCE_TRACE_FILE, --dryrun, --proxy-mode | Trace records argv decisions without source contents. |

## Summary

- `ALIGNED`: 9
- `EQUIVALENT_NPU`: 4
- `NOT_APPLICABLE_NPU`: 2
- `MISSING`: 3
- `DIAGNOSTIC_ONLY`: 0
