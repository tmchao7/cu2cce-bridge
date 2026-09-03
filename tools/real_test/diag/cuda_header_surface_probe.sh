#!/bin/bash
# Layered CUDA public-header probe. Unsupported libraries are include-only and
# diagnostic; core Runtime/SIMT include failures are blocking.
set -u

CANN_ROOT="${CANN_BASE:-/usr/local/Ascend/latest}"
BRIDGE_INCLUDE="${CU2CCE_BRIDGE_INCLUDE:-}"
CCEC="${CCEC_PATH:-}"
NPU_ARCH="${CU2CCE_NPU_ARCH:-dav-c310-vec}"
OUTPUT=""
while [ "$#" -gt 0 ]; do
    case "$1" in
        --cann-base) [ "$#" -ge 2 ] || exit 2; CANN_ROOT="$2"; shift 2 ;;
        --bridge-include) [ "$#" -ge 2 ] || exit 2; BRIDGE_INCLUDE="$2"; shift 2 ;;
        --ccec) [ "$#" -ge 2 ] || exit 2; CCEC="$2"; shift 2 ;;
        --npu-arch) [ "$#" -ge 2 ] || exit 2; NPU_ARCH="$2"; shift 2 ;;
        --output) [ "$#" -ge 2 ] || exit 2; OUTPUT="$2"; shift 2 ;;
        -h|--help) echo "usage: $0 --cann-base ROOT --bridge-include DIR --ccec FILE [--npu-arch dav-*] [--output FILE]"; exit 0 ;;
        *) echo "cuda_header_surface_probe: unknown option: $1" >&2; exit 2 ;;
    esac
done

if [ -n "$OUTPUT" ]; then
    mkdir -p "$(dirname "$OUTPUT")"
    exec > >(tee "$OUTPUT")
fi

status=0
include_root="$CANN_ROOT/include"
if [ ! -d "$include_root" ] && [ -n "${ASCEND_TOOLKIT_HOME:-}" ]; then
    include_root="$ASCEND_TOOLKIT_HOME/include"
fi
repo_root="$(cd "$(dirname "$0")/../../.." && pwd)"

reference_root="${CU2CCE_REFERENCE_ROOT:-$repo_root/../cu-bridge-muxi}"
reference_args=()
if [ -d "$reference_root" ]; then
    reference_args=(--reference "$reference_root")
fi
if python3 "$repo_root/tools/validate_cuda_header_wrapper_parity.py" \
    "${reference_args[@]}" \
    --implementation "$repo_root" \
    --inventory "$repo_root/config/cuda_api_parity_inventory.json" \
    --format text >"$work/parity.log" 2>&1; then
    echo "[PASS] public CUDA header inventory"
else
    echo "[FAIL] public CUDA header inventory"
    status=1
fi

work="${TMPDIR:-/tmp}/cu2cce-header-probe.$$"
mkdir -p "$work"
trap 'rm -rf "$work"' EXIT
cat >"$work/core.cu" <<'CPP'
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <cuda.h>
#include <cudaTypedefs.h>
#include <driver_functions.h>
#include <driver_types.h>
#include <cuda_surface_types.h>
#include <cuda_texture_types.h>
int main() { return 0; }
CPP
cat >"$work/library.cpp" <<'CPP'
#include <cublas.h>
#include <cublasLt.h>
#include <cublasXt.h>
#include <cublas_api.h>
#include <cufft.h>
#include <cufftXt.h>
#include <cufftw.h>
#include <curand.h>
#include <cusolverDn.h>
#include <cusolverMg.h>
#include <cusolverRf.h>
#include <cusolverSp.h>
#include <cusparse.h>
#include <cusparse_v2.h>
#include <cudnn.h>
#include <nccl.h>
#include <npp.h>
#include <nppcore.h>
#include <nppdefs.h>
#include <nppi.h>
#include <npps.h>
#include <nvjpeg.h>
#include <nvml.h>
#include <nvrtc.h>
int main() { return 0; }
CPP
cat >"$work/interop.cpp" <<'CPP'
#include <cudaEGL.h>
#include <cudaEGLTypedefs.h>
#include <cuda_egl_interop.h>
#include <cudaGL.h>
#include <cudaGLTypedefs.h>
#include <cuda_gl_interop.h>
#include <cudaVDPAU.h>
#include <cudaVDPAUTypedefs.h>
#include <cuda_vdpau_interop.h>
#include <nvToolsExtCuda.h>
#include <nvToolsExtCudaRt.h>
#include <nvToolsExtOpenCL.h>
#include <nvToolsExtSync.h>
int main() { return 0; }
CPP

if command -v g++ >/dev/null 2>&1 && g++ -fsyntax-only -I"$repo_root/include" -I"$include_root" "$work/library.cpp" >"$work/library.log" 2>&1; then
    echo "[DIAG-PASS] BLAS/FFT/RAND/SOLVER/SPARSE/DNN/NVRTC header include"
else
    echo "[DIAG-ONLY] BLAS/FFT/RAND/SOLVER/SPARSE/DNN/NVRTC header include unavailable"
    tail -n 8 "$work/library.log" 2>/dev/null || true
fi

if command -v g++ >/dev/null 2>&1 && g++ -fsyntax-only -I"$repo_root/include" -I"$include_root" "$work/interop.cpp" >"$work/interop.log" 2>&1; then
    echo "[DIAG-PASS] Interop/NVTX header include"
else
    echo "[DIAG-ONLY] Interop/NVTX header include unavailable"
    tail -n 8 "$work/interop.log" 2>/dev/null || true
fi

if [ -n "$CCEC" ] && [ -x "$CCEC" ] && [ -d "$BRIDGE_INCLUDE" ]; then
    if "$CCEC" -x asc --enable-simt --cce-aicore-arch="$NPU_ARCH" \
        -I"$BRIDGE_INCLUDE" -I"$include_root" -c "$work/core.cu" -o "$work/core.o" >"$work/core.log" 2>&1; then
        echo "[PASS] core runtime header include"
    else
        echo "[FAIL] core runtime header include"
        tail -n 20 "$work/core.log" 2>/dev/null || true
        status=1
    fi
else
    echo "[DIAG-ONLY] core runtime header include compiler unavailable"
fi

if [ -n "$CCEC" ] && [ -x "$CCEC" ] && [ -d "$BRIDGE_INCLUDE" ]; then
    if "$CCEC" -x asc --enable-simt --cce-aicore-arch="$NPU_ARCH" \
        -I"$BRIDGE_INCLUDE" -I"$include_root" -c "$repo_root/examples/puresimt/half_minimal.cu" \
        -o "$work/half.o" >"$work/device.log" 2>&1; then
        echo "[DIAG-PASS] CUDA device/FP16 header include"
    else
        echo "[DIAG-ONLY] CUDA device/FP16 header include unavailable"
        tail -n 10 "$work/device.log" 2>/dev/null || true
    fi
fi

exit "$status"
