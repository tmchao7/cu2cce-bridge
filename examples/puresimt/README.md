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

- `core_ops_minimal.cu` — arithmetic, bitwise logic, compare/select, conversion, and memory basis.
- `math_minimal.cu` — FMA and elementary math functions.

- `event_minimal.cu` — event record/query/synchronize/elapsed-time semantics.
- `pitchedMemcpy_minimal.cu` — `cudaMallocPitch` and `cudaMemcpy2D` with 2D indexing.
- `multiKernel_minimal.cu` — ordered async copies and two kernels in one stream.

- `atomicVariants_minimal.cu` — `atomicSub`/`atomicExch` plus shared-memory atomic add.
- `dynamicShared_minimal.cu` — `extern __shared__` and dynamic launch shared bytes.
- `index3D_minimal.cu` — 3D grid/block flattening.
- `threadFence_minimal.cu` — global stores, `__threadfence`, and atomic publication flag.

- `half_minimal.cu` — CUDA FP16/half2 aliases and conversion round-trip, backed by CANN `simt_api/asc_fp16.h`.
- `bf16x2_minimal.cu` — CUDA BF16/BF16x2 aliases and conversion round-trip, backed by CANN `simt_api/asc_bf16.h`.


## Coverage model

The suite is organized as a functional basis rather than a collection of repeated
examples: scalar arithmetic/logic, comparisons and conversions; global/shared/dynamic
memory; 1D/2D/3D indexing; barriers/fences; atomic families; warp collectives;
reduction/scan; stream/event ordering; pitched copies; and FP16/BF16x2 conversions.
The official NVIDIA vectorAdd/matrixMul sources provide the no-source-rewrite anchor,
while minimal files isolate one CUDA semantic at a time.
