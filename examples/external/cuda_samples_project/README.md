# NVIDIA CUDA Samples CMake harness

This harness adds selected, unmodified sample subprojects from a pinned
`NVIDIA/cuda-samples` checkout. The checkout is fetched into
`build/real_test/external/`, not committed into this repository. No upstream
source file is rewritten; CUDA helper headers are injected by the rule-driven
`tools/cu2cce_driver.py` from `third_party/nvidia_cuda_samples/Common`.

The current profile uses the historical `v12.8` tag because it is the first
selected tree with the modern per-sample CMake structure. It is an experimental
CMake build-graph test, not a claim that all CUDA 12.8 APIs are supported by
CANN.

The harness intentionally disables CUDA separable compilation on the sample
targets. ccec currently emits duplicate registration helpers when multiple `.cu`
objects are aggregated through the same RDC/static-archive path.

The external profile uses `CU2CCE_LIBC_MODE=system` so libstdc++ can include the real
`stdlib.h`, `stdio.h`, and `math.h` declarations. The default puresimt profile keeps
`CU2CCE_LIBC_MODE=shadow` for the minimal-header glibc workaround used by the existing
regression set.

The `reduction` sample is intentionally excluded from the required profile. It combines
CUDA Cooperative Groups cross-warp tiles (`64..512`) and device-side `double`, both outside
the stable CANN 9.2 SIMT capability. It remains a documented diagnostic limitation rather
than a silently modified or falsely passing sample.

The required CMake profile builds and runs `vectorAdd` and `matrixMul`.
`simpleStreams`, `simpleMultiCopy`, and `asyncAPI` are part of the harness's
configure/build graph as a diagnostic-only profile: they never block the
required PASS. The runtime APIs they depend on (`cudaHostRegister`,
`cudaHostGetDevicePointer`, full `cudaDeviceProp` fields, event/stream flag
semantics) are either implemented as diagnostics
(`ACL_ERROR_API_NOT_SUPPORT`) or pending in the runtime wrapper.

Security: the external fetch only accepts http/https URLs whose host is a
public internet address. `tools/real_test/validate_fetch_url.py` rejects
localhost, loopback, private, link-local, multicast, and reserved addresses
(including hostnames that resolve to them).