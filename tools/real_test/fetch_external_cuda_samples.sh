#!/bin/bash
# Fetch the pinned NVIDIA CUDA Samples source used by the build compatibility smoke.
# Prefer GitHub's commit archive: it avoids a full git checkout of a large
# repository and keeps the exact source revision reproducible.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT_DIR="${OUT_DIR:-$ROOT/build/real_test}"
SOURCE_DIR="${CUDA_SAMPLES_SOURCE_DIR:-$OUT_DIR/external/nvidia_cuda_samples}"
REPO_URL="${CUDA_SAMPLES_REPO:-https://github.com/NVIDIA/cuda-samples.git}"
REF="${CUDA_SAMPLES_REF:-db3eea23946bca2e90a75eca2b5b3e07158a9e11}"
TAG="${CUDA_SAMPLES_TAG:-v12.8}"
FETCH_MODE="${CUDA_SAMPLES_FETCH_MODE:-git}"

mkdir -p "$(dirname "$SOURCE_DIR")"

# A previous successful archive fetch is reusable and avoids network work.
if [ -f "$SOURCE_DIR/.cu2cce_revision" ] &&
   [ "$(cat "$SOURCE_DIR/.cu2cce_revision")" = "$REF" ] &&
   [ -d "$SOURCE_DIR/Samples" ]; then
    printf 'CUDA_SAMPLES_SOURCE_DIR=%s\nCUDA_SAMPLES_REF=%s\n' "$SOURCE_DIR" "$REF"
    exit 0
fi

rm -rf "$SOURCE_DIR"
mkdir -p "$SOURCE_DIR"

archive_ok=0
if [ "$FETCH_MODE" = "archive" ] && command -v curl >/dev/null 2>&1 && command -v tar >/dev/null 2>&1; then
    ARCHIVE_DIR="$(mktemp -d "$(dirname "$SOURCE_DIR")/.cuda-samples-fetch.XXXXXX")"
    ARCHIVE="$ARCHIVE_DIR/cuda-samples.tar.gz"
    trap 'rm -rf "$ARCHIVE_DIR"' EXIT
    ARCHIVE_URL="${CUDA_SAMPLES_ARCHIVE_URL:-https://github.com/NVIDIA/cuda-samples/archive/${REF}.tar.gz}"
    echo "[INFO] downloading pinned CUDA Samples archive: $ARCHIVE_URL" >&2
    if curl --fail --location --retry 3 --http1.1 --connect-timeout 30 \
         --max-time "${CUDA_SAMPLES_CURL_TIMEOUT:-240}" \
         "$ARCHIVE_URL" --output "$ARCHIVE"; then
        tar -xzf "$ARCHIVE" -C "$ARCHIVE_DIR"
        extracted="$(find "$ARCHIVE_DIR" -mindepth 1 -maxdepth 1 -type d -name 'cuda-samples-*' -print -quit)"
        if [ -n "$extracted" ] && [ -d "$extracted/Samples" ]; then
            cp -a "$extracted/." "$SOURCE_DIR/"
            printf '%s\n' "$REF" > "$SOURCE_DIR/.cu2cce_revision"
            archive_ok=1
        fi
    fi
    if [ "$archive_ok" -eq 0 ]; then
        echo '[WARN] CUDA Samples archive fetch failed; falling back to shallow git fetch' >&2
        rm -rf "$ARCHIVE_DIR"
    fi
fi

if [ "$archive_ok" -eq 0 ]; then
    # Fallback for environments without curl/tar or when archive download is
    # blocked. This path is slower but preserves exact commit verification.
    if ! command -v git >/dev/null 2>&1; then
        echo "neither archive tools nor git are available for external CUDA Samples" >&2
        exit 1
    fi
    rm -rf "$SOURCE_DIR"
    # The pinned revision is the v12.8 tag. Cloning the tag directly avoids a
    # default-branch checkout followed by a second network fetch, which can
    # exceed the server smoke timeout on GitHub connections.
    if [ "$TAG" = "v12.8" ]; then
        git clone --depth 1 --filter=blob:none --branch "$TAG" "$REPO_URL" "$SOURCE_DIR"
    else
        git clone --filter=blob:none --no-checkout "$REPO_URL" "$SOURCE_DIR"
        git -C "$SOURCE_DIR" fetch --depth 1 origin "$REF"
        git -C "$SOURCE_DIR" checkout --detach "$REF"
    fi
    actual="$(git -C "$SOURCE_DIR" rev-parse HEAD)"
    if [ "$actual" != "$REF" ]; then
        echo "external CUDA Samples revision mismatch: expected $REF, got $actual" >&2
        exit 1
    fi
    printf '%s\n' "$actual" > "$SOURCE_DIR/.cu2cce_revision"
fi

if [ ! -d "$SOURCE_DIR/Samples" ]; then
    echo "external CUDA Samples source tree is incomplete: $SOURCE_DIR" >&2
    exit 1
fi
printf 'CUDA_SAMPLES_SOURCE_DIR=%s\nCUDA_SAMPLES_REF=%s\n' "$SOURCE_DIR" "$REF"
