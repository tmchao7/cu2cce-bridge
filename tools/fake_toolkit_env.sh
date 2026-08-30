#!/bin/bash
# fake_toolkit_env.sh —— shared sourced helper for cmake_cce/make_cce/pre_cce.
#
# cu-bridge parity: their make_maca / cmake_mock / pre_makefile all share the
# same virtual-CUDA-dir build logic driven by one manifest (link.json /
# stub_link.json). This library is the single place that
#   - derives the CUDA-shaped host triple,
#   - creates the make.log persistence sentinel,
#   - runs tools/build_shim_tree.py over config/shim_layout.json,
#   - verifies the shim nvcc and reports the CUDA dir.
#
# Sourcing contract: $CUCC_PATH must be set (validated by the callers); the
# helpers honour $CANN_BASE with the usual default.

# Resolve the CUDA-shaped host triple for display purposes. build_shim_tree.py
# computes the authoritative triple for the targets/ layout — keep the mapping
# identical (amd64/arm64 normalization).
cu2cce_host_triple() {
    local arch os
    arch="$(uname -m 2>/dev/null || echo x86_64)"
    case "$arch" in amd64) arch=x86_64 ;; arm64) arch=aarch64 ;; esac
    os="$(uname -s 2>/dev/null | tr '[:upper:]' '[:lower:]' || echo linux)"
    case "$os" in darwin) os=darwin ;; linux) os=linux ;; esac
    printf '%s-%s\n' "$arch" "$os"
}

# cu2cce_prepare_fake_toolkit DST_WCUDA NVCC_TARGET HOSTCC_BIN_DIR HOSTCC_MODE
#
# Builds the toolkit-shaped tree under $DST_WCUDA/CUDA_DIR:
#   DST_WCUDA     persistence root (sentinel: $DST_WCUDA/make.log)
#   NVCC_TARGET   what bin/nvcc points at (nvcc_entry.sh for configure,
#                 bin/cucc for build phases)
#   HOSTCC_DIR    where the {c++,g++,gcc,cc} entries are created
#   HOSTCC_MODE   "wrapper" (hostcc_cce) or "identity" (/usr/bin)
# Prints the CUDA dir on stdout; returns non-zero when the shim is broken.
cu2cce_prepare_fake_toolkit() {
    local dst="$1" nvcc_target="$2" hostcc_dir="$3" hostcc_mode="$4"
    if [ ! -f "$dst/make.log" ]; then
        rm -rf "$dst"
    fi
    mkdir -p "$dst"
    echo 1 > "$dst/make.log"
    local cuda_dir="$dst/CUDA_DIR"
    mkdir -p "$hostcc_dir"
    python3 "$CUCC_PATH/tools/build_shim_tree.py" \
        --cuda-dir "$cuda_dir" \
        --cucc-path "$CUCC_PATH" \
        --nvcc-target "$nvcc_target" \
        --hostcc-bin-dir "$hostcc_dir" \
        --hostcc-mode "$hostcc_mode" \
        --cann-base "${CANN_BASE:-/usr/local/Ascend/latest}" \
        --quiet || return 1
    if [ ! -e "$cuda_dir/bin/nvcc" ]; then
        echo "cu2cce: fake toolkit build failed under $dst (no shim nvcc)" >&2
        return 1
    fi
    printf '%s\n' "$cuda_dir"
}
