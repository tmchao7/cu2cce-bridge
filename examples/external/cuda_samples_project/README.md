# NVIDIA CUDA Samples CMake harness

This harness adds selected, unmodified sample subprojects from a pinned
`NVIDIA/cuda-samples` checkout. The checkout is fetched into
`build/real_test/external/`, not committed into this repository.

The current profile uses the historical `v12.8` tag because it is the first
selected tree with the modern per-sample CMake structure. It is an experimental
CMake build-graph test, not a claim that all CUDA 12.8 APIs are supported by
CANN.

The harness intentionally disables CUDA separable compilation on the sample
targets. ccec currently emits duplicate registration helpers when multiple `.cu`
objects are aggregated through the same RDC/static-archive path.
