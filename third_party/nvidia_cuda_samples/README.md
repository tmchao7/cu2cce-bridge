# NVIDIA CUDA Samples support files

This directory contains the unmodified helper headers required by the official
NVIDIA CUDA Samples used by the puresimt regression:

- `Common/helper_cuda.h`
- `Common/helper_functions.h`
- `Common/helper_image.h`
- `Common/helper_string.h`
- `Common/helper_timer.h`
- `Common/exception.h`

Source provenance: `NVIDIA/cuda-samples`, `cpp/0_Introduction`, checked out for
use with `examples/puresimt/vectorAdd/vectorAdd.cu` and
`examples/puresimt/matrixMul/matrixMul.cu`. Each copied source retains its
upstream NVIDIA BSD-3-Clause license notice.
