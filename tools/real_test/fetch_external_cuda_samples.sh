#!/bin/bash
# Fetch the pinned NVIDIA CUDA Samples source used by the build compatibility smoke.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT_DIR="${OUT_DIR:-$ROOT/build/real_test}"
SOURCE_DIR="${CUDA_SAMPLES_SOURCE_DIR:-$OUT_DIR/external/nvidia_cuda_samples}"
REPO_URL="${CUDA_SAMPLES_REPO:-https://github.com/NVIDIA/cuda-samples.git}"
REF="${CUDA_SAMPLES_REF:-db3eea23946bca2e90a75eca2b5b3e07158a9e11}"

mkdir -p "$(dirname "$SOURCE_DIR")"
if [ -d "$SOURCE_DIR/.git" ]; then
    git -C "$SOURCE_DIR" fetch --depth 1 origin "$REF"
else
    git clone --filter=blob:none --no-checkout "$REPO_URL" "$SOURCE_DIR"
    git -C "$SOURCE_DIR" fetch --depth 1 origin "$REF"
fi
git -C "$SOURCE_DIR" checkout --detach "$REF"

actual="$(git -C "$SOURCE_DIR" rev-parse HEAD)"
if [ "$actual" != "$REF" ]; then
    echo "external CUDA Samples revision mismatch: expected $REF, got $actual" >&2
    exit 1
fi
printf 'CUDA_SAMPLES_SOURCE_DIR=%s\nCUDA_SAMPLES_REF=%s\n' "$SOURCE_DIR" "$actual"
