# puresimt examples

This is the user-facing example set for the `main` branch puresimt path. The
CUDA source files intentionally keep NVIDIA's original names and source style:

- `vectorAdd/vectorAdd.cu` — official NVIDIA CUDA Samples vector addition.
- `matrixMul/matrixMul.cu` — official NVIDIA CUDA Samples shared-memory matrix multiplication.

These `.cu` files are not rewritten with ACL calls, Ascend C syntax, bridge
macros, or a `p` prefix. The bridge supplies CUDA-compatible headers and host
runtime symbols underneath the original CUDA source. NVIDIA helper headers live
in `third_party/nvidia_cuda_samples/Common` and are injected only by the build
or regression command.

The puresimt regression intentionally tests the original CUDA examples rather
than hand-written puresimt-specific copies. The bridge/compiler is responsible
for making the same source compile and run on Ascend.

For isolating CUDA-header compatibility from NVIDIA helper-header compatibility,
the same regression also includes two deliberately small self-contained files:

- `vectorAdd_minimal.cu`
- `matrixMul_minimal.cu`

They still use ordinary CUDA syntax and are not rewritten for Ascend.


The next operator-focused tests use the same minimal-source policy:

- `atomicAdd_minimal.cu` — global-memory `atomicAdd`/`atomicMin`/`atomicMax`, based on the NVIDIA `simpleAtomicIntrinsics` pattern.
- `reduction_minimal.cu` — shared-memory tree reduction plus cross-block `atomicAdd`, based on the NVIDIA `reduction` pattern.

- `warpShuffle_minimal.cu` — warp shuffle reduction, based on NVIDIA `shfl_scan`.
- `scan_minimal.cu` — inclusive block scan, based on NVIDIA `scan`.
- `transpose_minimal.cu` — shared-memory tiled transpose, based on NVIDIA `transpose`.

- `vote_minimal.cu` — `__all_sync`/`__any_sync`/`__ballot_sync`, based on NVIDIA `simpleVoteIntrinsics`.
- `dot_minimal.cu` — shared-memory dot product and atomic accumulation.
- `stencil_minimal.cu` — 1D shared-memory halo stencil.
