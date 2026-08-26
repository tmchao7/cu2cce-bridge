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


The external profile uses `CU2CCE_LIBC_MODE=system` so libstdc++ can include the real
`stdlib.h`, `stdio.h`, and `math.h` declarations. The default puresimt profile keeps
`CU2CCE_LIBC_MODE=shadow` for the minimal-header glibc workaround used by the existing
regression set.


The `reduction` sample is intentionally excluded from the required profile. It combines
CUDA Cooperative Groups cross-warp tiles (`64..512`) and device-side `double`, both outside
the stable CANN 9.2 SIMT capability. It remains a documented diagnostic limitation rather
than a silently modified or falsely passing sample.


The required CMake profile currently builds and runs `vectorAdd` and `matrixMul`.
`simpleStreams` and `simpleMultiCopy` are retained as diagnostic-only candidates while
CUDA device flags, device-property fields, pinned host memory, and host/device math
overload behavior are completed in the runtime wrapper.
