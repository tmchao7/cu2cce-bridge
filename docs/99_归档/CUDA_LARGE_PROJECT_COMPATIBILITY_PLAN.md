# CUDA 大型项目兼容性计划（归档）

## Baseline

Branch: `exp/build-compat`.

Current real-device baseline:

```text
PURESIMT_REGRESSION_PASS
CMAKE_CCE_REGRESSION_PASS
BUILD_COMPATIBILITY_REGRESSION_PASS
CUDA_SAMPLES_CMAKE_REGRESSION_PASS
CUCC_RULES_REGRESSION_PASS
LARGE_PROJECT_REGRESSION_PASS
```

## Goals

- Validate an unmodified upstream CUDA CMake project.
- Make CMake, Make/Ninja, and cucc parameter policy share one backend driver.
- Keep CUDA libraries and device-link/RDC as separately declared capabilities.
- Preserve the puresimt direct path and never generate legacy `.cce` or wrapper source.
- Extend the validated build-graph surface: multi-directory projects, cross-target
  dependencies, install/export, and runtime-API diagnostics for samples that are not
  yet supported by CANN.

## Work order

1. Maintain the compatibility matrices and option policy.
2. Extend cucc query/preprocess/dependency tests and rule validation
   (`--proxy-mode`, mock_nvcc query/dependency short-circuit).
3. Extend CMake target graph, package, install/export, and unsupported-target diagnostics.
4. Add a pinned NVIDIA CUDA Samples CMake harness.
5. Fetch the upstream source into `build/real_test/external/` without modifying it.
6. Run the curated upstream samples on Ascend; promote only evidence-backed states.

## Large-project validation carrier

`examples/large_project` exercises the multi-directory patterns a real large CUDA
code base uses:

```text
top/    app/main.cpp -> large_util_lib; depends on util/gen_data_file
comp/   large_kernel_lib (single .cu kernel library)
util/   large_util_lib (store.cpp), generate_kernel_data, large_gen_data,
        custom-command generated input
```

Backend boundary exercised: exactly one `.cu` per linked library target (ccec emits
duplicate eager-registration helpers when multiple `.cu` objects are aggregated through
the same archive/RDC path). The carrier keeps the kernel library as the single CUDA
translation unit; everything else is host C++.

Expected run marker: `large_project OK (multi-dir, cross-target deps, install/export)`.
Server gate: `LARGE_PROJECT_REGRESSION_PASS` when the full `cmake_large_project` case
(configure/build/run) passes.

## External project pin

`NVIDIA/cuda-samples` tag `v12.8` is the first selected historical tag with the modern
CMake sample structure. It is closer to the CANN CUDA 11.6 compatibility surface than the
current v13.x tree, but remains an `EXPERIMENTAL` build-graph test rather than a claim of
full CUDA 12.8 semantic support. GPU architecture lists, separable compilation, and
unsupported library samples are disabled or diagnosed by the harness.

Resolved revision:

```text
db3eea23946bca2e90a75eca2b5b3e07158a9e11
```

## External profile

- Required (build + run): `vectorAdd`, `matrixMul`.
- Diagnostic-only (in configure/build graph; run failure is recorded and does not
  block required PASS): `simpleStreams`, `simpleMultiCopy`, `asyncAPI`.
- Excluded from graph: `Samples/2_Concepts_and_Techniques/reduction`
  (cross-warp `thread_block_tile` + device-side double, outside stable CANN 9.2 SIMT).
- Runtime APIs still `ACL_ERROR_API_NOT_SUPPORT`: `cudaHostRegister`,
  `cudaHostGetDevicePointer`, `cudaHostUnregister`, `cudaHostGetFlags` — declarations and
  diagnostics exist, no no-op fake success.

## Fetch security

External fetch URL must be http/https with a public remote host.
`tools/real_test/validate_fetch_url.py` rejects loopback, private, link-local,
multicast, reserved and special-use addresses (including hostnames that resolve to
them). Without the validator, only exact known-good GitHub hosts are allowed.

## Acceptance

```text
PURESIMT_REGRESSION_PASS
CMAKE_CCE_REGRESSION_PASS
BUILD_COMPATIBILITY_REGRESSION_PASS
CUDA_SAMPLES_CMAKE_REGRESSION_PASS
CUCC_RULES_REGRESSION_PASS
LARGE_PROJECT_REGRESSION_PASS
```

The upstream sample source tree remains outside version control under
`build/real_test/external/nvidia_cuda_samples/`; only the harness, profile, fetch script,
and revision are committed.