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
