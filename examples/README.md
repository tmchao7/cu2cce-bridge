# Examples

`examples/puresimt/` is the maintained example set for the `main` branch.
It contains the original CUDA source files from NVIDIA CUDA Samples used to
validate the no-source-rewrite puresimt path:

- `puresimt/vectorAdd/vectorAdd.cu`
- `puresimt/matrixMul/matrixMul.cu`
- `puresimt/vectorAdd_minimal.cu`
- `puresimt/matrixMul_minimal.cu`
- `puresimt/atomicAdd_minimal.cu`
- `puresimt/reduction_minimal.cu`

Experimental probes and hand-written bridge-specific copies are not part of the
maintained examples set. Historical versions remain available in Git history.

- `puresimt/warpShuffle_minimal.cu`
- `puresimt/scan_minimal.cu`
- `puresimt/transpose_minimal.cu`

- `puresimt/vote_minimal.cu`
- `puresimt/dot_minimal.cu`
- `puresimt/stencil_minimal.cu`
