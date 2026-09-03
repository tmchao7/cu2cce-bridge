#!/bin/bash
# virtual_toolkit_env.sh —— shared sourced helper for cmake_cce/make_cce/pre_cce.
#
# cu-bridge parity: their make_maca / cmake_mock / pre_makefile all share the
# same virtual-CUDA-dir build logic driven by one manifest (link.json /
# stub_link.json). This library is the single place that
#   - derives the CUDA-shaped host triple,
#   - creates the make.log persistence sentinel,
#   - runs tools/mock/virtualDir.py over tools/link.json or tools/mock/stub_link.json,
#   - verifies the virtual toolkit nvcc and reports the CUDA dir.
#
# Sourcing contract: $CUCC_PATH must be set (validated by the callers); the
# helpers honour $CANN_BASE with the usual default.

# Derive a stable per-build virtual toolkit root. CMake caches the CUDA compiler path, so
# keying the default directory by the canonical build directory prevents two
# projects (or two bridge installs) from overwriting one another. Explicit
# CU2CCE_CMAKE_SHIM_DIR remains authoritative for callers that need a fixed path.
cu2cce_virtual_toolkit_root_for_build() {
    local base="$1" build_dir="$2" canonical parent digest
    if [[ "$build_dir" != /* ]]; then
        build_dir="$PWD/$build_dir"
    fi
    if [ -d "$build_dir" ]; then
        canonical="$(cd "$build_dir" && pwd -P)" || return 1
    else
        parent="$(cd "$(dirname "$build_dir")" && pwd -P)" || return 1
        canonical="$parent/$(basename "$build_dir")"
    fi
    digest="$(printf '%s' "$canonical" | cksum | awk '{print $1}')"
    [ -n "$digest" ] || return 1
    printf '%s/cmake-shim-%s\n' "${base%/}" "$digest"
}

# Recover a virtual toolkit root recorded by an existing CMake build tree. This keeps
# incremental builds compatible with trees configured before build-keyed default
# isolation was introduced. It only accepts CMake's own CUDA_DIR/bin/nvcc shape.
cu2cce_cmake_cache_virtual_toolkit_root() {
    local build_dir="$1" compiler root
    [ -f "$build_dir/CMakeCache.txt" ] || return 1
    compiler="$(sed -n 's#^CMAKE_CUDA_COMPILER:[^=]*=##p' \
        "$build_dir/CMakeCache.txt" | tail -n 1)"
    case "$compiler" in
        */CUDA_DIR/bin/nvcc)
            root="${compiler%/CUDA_DIR/bin/nvcc}"
            [ -d "$root" ] && printf '%s\n' "$root"
            ;;
        *) return 1 ;;
    esac
}

# Resolve the CUDA-shaped host triple for display purposes. virtualDir.py
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

# Resolve the actual ccec binary without assuming one CANN installation layout.
# Explicit CCEC_PATH wins; otherwise accept CANN_BASE/ASCEND_TOOLKIT_HOME and
# versioned /usr/local/Ascend/cann-* trees. The result is absolute because
# cmake_cce may change directory before the build phase.
cu2cce_resolve_ccec() {
    local requested="${CCEC_PATH:-}" candidate directory
    if [ -n "$requested" ]; then
        if [ ! -x "$requested" ]; then
            echo "cu2cce: requested CCEC_PATH is not executable: $requested" >&2
            return 1
        fi
        directory="$(cd "$(dirname "$requested")" && pwd -P)" || return 1
        printf '%s/%s\n' "$directory" "$(basename "$requested")"
        return 0
    fi
    local candidates=(
        "${CANN_BASE:-/usr/local/Ascend/latest}/bin/ccec"
        "${ASCEND_TOOLKIT_HOME:-/usr/local/Ascend/ascend-toolkit/latest}/bin/ccec"
        "/usr/local/Ascend/latest/bin/ccec"
    )
    shopt -s nullglob
    candidates+=(/usr/local/Ascend/cann-*/bin/ccec)
    shopt -u nullglob
    for candidate in "${candidates[@]}"; do
        if [ -x "$candidate" ]; then
            directory="$(cd "$(dirname "$candidate")" && pwd -P)" || return 1
            printf '%s/%s\n' "$directory" "$(basename "$candidate")"
            return 0
        fi
    done
    return 1
}

# Prepend library directories without creating a trailing ':' when the caller
# did not inherit LD_LIBRARY_PATH. Empty path entries make the dynamic loader
# search the current directory and are unsafe for build wrappers.
cu2cce_prepend_ld_library_path() {
    local prefix="$1" existing="${LD_LIBRARY_PATH:-}"
    if [ -n "$existing" ]; then
        printf '%s:%s\n' "$prefix" "$existing"
    else
        printf '%s\n' "$prefix"
    fi
}

# Resolve one Ascend architecture for all build wrappers. CUCC_TARGETS is the
# cu-bridge-compatible alias, but unlike MACA's multi-target list the current
# ccec puresimt path accepts exactly one dav-* target. Explicit invalid input
# fails instead of silently falling back to dav-c310-vec.
cu2cce_resolve_npu_arch() {
    if [ -n "${CU2CCE_NPU_ARCH:-}" ]; then
        if [[ ! "$CU2CCE_NPU_ARCH" =~ ^dav-[[:alnum:]_.-]+$ ]]; then
            echo "cu2cce: NPU architecture must be an Ascend dav-* target" >&2
            return 2
        fi
        printf '%s\n' "$CU2CCE_NPU_ARCH"
        return 0
    fi
    if [ -n "${CUCC_NPU_ARCH:-}" ]; then
        if [[ ! "$CUCC_NPU_ARCH" =~ ^dav-[[:alnum:]_.-]+$ ]]; then
            echo "cu2cce: NPU architecture must be an Ascend dav-* target" >&2
            return 2
        fi
        printf '%s\n' "$CUCC_NPU_ARCH"
        return 0
    fi
    if [ -n "${CUCC_TARGETS:-}" ] || [ -n "${CUCC_TARGETS_FROM_DEVICE:-}" ]; then
        bash "$CUCC_PATH/tools/device_arch.sh"
        return $?
    fi
    bash "$CUCC_PATH/tools/device_arch.sh" 2>/dev/null || printf 'dav-c310-vec\n'
}

# cu2cce_prepare_virtual_toolkit DST_WCUDA NVCC_TARGET HOSTCC_BIN_DIR HOSTCC_MODE [MANIFEST]
#
# Builds the toolkit-shaped tree under $DST_WCUDA/CUDA_DIR:
#   DST_WCUDA     persistence root (sentinel: $DST_WCUDA/make.log)
#   NVCC_TARGET   what bin/nvcc points at (nvcc_entry.sh for configure,
#                 bin/cucc for build phases)
#   HOSTCC_DIR    where the {c++,g++,gcc,cc} entries are created
#   HOSTCC_MODE   "wrapper" (hostcc_cce) or "identity" (/usr/bin)
#   MANIFEST      optional tools/link.json or tools/mock/stub_link.json
# Prints the CUDA dir on stdout; returns non-zero when the virtual toolkit is broken.
cu2cce_prepare_virtual_toolkit() {
    local dst="$1" nvcc_target="$2" hostcc_dir="$3" hostcc_mode="$4"
    local manifest="${5:-$CUCC_PATH/tools/link.json}"
    if [ ! -f "$dst/make.log" ]; then
        rm -rf "$dst"
    fi
    mkdir -p "$dst"
    echo 1 > "$dst/make.log"
    local cuda_dir="$dst/CUDA_DIR"
    mkdir -p "$hostcc_dir"
    python3 "$CUCC_PATH/tools/mock/virtualDir.py" \
        --cuda-dir "$cuda_dir" \
        --cucc-path "$CUCC_PATH" \
        --nvcc-target "$nvcc_target" \
        --hostcc-bin-dir "$hostcc_dir" \
        --hostcc-mode "$hostcc_mode" \
        --manifest "$manifest" \
        --cann-base "${CANN_BASE:-/usr/local/Ascend/latest}" \
        --quiet || return 1
    if [ ! -e "$cuda_dir/bin/nvcc" ]; then
        echo "cu2cce: virtual CUDA toolkit build failed under $dst (no nvcc entry)" >&2
        return 1
    fi
    printf '%s\n' "$cuda_dir"
}

# Compatibility function name retained for existing integrations.
cu2cce_prepare_fake_toolkit() {
    cu2cce_prepare_virtual_toolkit "$@"
}

# Legacy helper names remain callable for external scripts; no duplicate implementation.
cu2cce_shim_root_for_build() {
    cu2cce_virtual_toolkit_root_for_build "$@"
}
cu2cce_cmake_cache_shim_root() {
    cu2cce_cmake_cache_virtual_toolkit_root "$@"
}
