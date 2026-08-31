# cu-bridge parity audit

Reference: `6ab1076` (3.8.0)
Implementation: `fe1560e`

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
| shim_manifest | ALIGNED | link.json + stub_link.json | config/shim_layout.json + build_shim_tree.py | Reference CUDA alias entries: 48; current manifest aliases: 9. |

## cucc

| ID | Status | Reference | Implementation | Note |
|---|---|---|---|---|
| options_file | ALIGNED | --options-file recursive expansion | safe shlex response/options expansion | No shell evaluation; nesting is depth-limited. |
| query_skipstring | ALIGNED | 13 reference skipstring queries | 12 configured queries plus driver banner handling | -E is a query passthrough rule; -V remains an honest cu2cce driver banner. |
| rule_pipeline | EQUIVALENT_NPU | 68 filters / 209 replacer pairs | 146 drops / 14 replacements plus NPU policy | Rules preserve semantics where ccec has an equivalent and reject/drop the rest honestly. |
| architecture_policy | EQUIVALENT_NPU | CUCC_TARGETS -> --offload-arch for MACA | CU2CCE_NPU_ARCH/device_arch.sh -> --cce-aicore-arch=dav-* | sm_* is metadata or compatibility macro input, never a ccec target. |
| library_policy | EQUIVALENT_NPU | CUDA libraries map to MACA library names | validated bridge/CANN mapping, experimental cublas, map_to diagnostics | No unvalidated CUDA library is presented as an Ascend equivalent. |
| device_artifacts | NOT_APPLICABLE_NPU | PTX/CUBIN/FATBIN and MACA device-link paths | explicit reject for NVIDIA artifacts and unvalidated RDC/device-link | puresimt produces Ascend code through ccec and has no NVIDIA artifact path. |
| observability | ALIGNED | debug echo and generated command trace | CU2CCE_DEBUG, CU2CCE_TRACE_FILE, --dryrun, --proxy-mode | Trace records argv decisions without source contents. |

## Summary

- `ALIGNED`: 9
- `EQUIVALENT_NPU`: 4
- `NOT_APPLICABLE_NPU`: 2
- `MISSING`: 0
- `DIAGNOSTIC_ONLY`: 0
