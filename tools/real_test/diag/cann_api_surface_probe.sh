#!/bin/bash
# Read-only CANN public API surface probe.
# It reports compact evidence for the headers used by the CUDA -> Ascend map.
set -u

CANN_ROOT="${CANN_BASE:-/usr/local/Ascend/latest}"
TOOLKIT_ROOT="${ASCEND_TOOLKIT_HOME:-/usr/local/Ascend/ascend-toolkit/latest}"
BRIDGE_INCLUDE=""
CCEC=""
NPU_ARCH="${CU2CCE_NPU_ARCH:-dav-c310-vec}"
SIMT_ROOT="${SIMT_API_ROOT:-}"
OUTPUT=""
while [ "$#" -gt 0 ]; do
    case "$1" in
        --cann-base)
            [ "$#" -ge 2 ] || { echo "cann_api_surface_probe: --cann-base requires a value" >&2; exit 2; }
            CANN_ROOT="$2"; shift 2 ;;
        --bridge-include)
            [ "$#" -ge 2 ] || { echo "cann_api_surface_probe: --bridge-include requires a value" >&2; exit 2; }
            BRIDGE_INCLUDE="$2"; shift 2 ;;
        --ccec)
            [ "$#" -ge 2 ] || { echo "cann_api_surface_probe: --ccec requires a value" >&2; exit 2; }
            CCEC="$2"; shift 2 ;;
        --npu-arch)
            [ "$#" -ge 2 ] || { echo "cann_api_surface_probe: --npu-arch requires a value" >&2; exit 2; }
            NPU_ARCH="$2"; shift 2 ;;
        --simt-root)
            [ "$#" -ge 2 ] || { echo "cann_api_surface_probe: --simt-root requires a value" >&2; exit 2; }
            SIMT_ROOT="$2"; shift 2 ;;
        --output)
            [ "$#" -ge 2 ] || { echo "cann_api_surface_probe: --output requires a value" >&2; exit 2; }
            OUTPUT="$2"; shift 2 ;;
        -h|--help)
            echo "usage: $0 [--cann-base ROOT] [--bridge-include DIR] [--ccec FILE] [--npu-arch dav-*] [--simt-root DIR] [--output FILE]"; exit 0 ;;
        *)
            echo "cann_api_surface_probe: unknown option: $1" >&2; exit 2 ;;
    esac
done

if [ -n "$OUTPUT" ]; then
    mkdir -p "$(dirname "$OUTPUT")"
    exec > >(tee "$OUTPUT")
fi

include_root="$CANN_ROOT/include"
# Older CANN packages keep Ascend C SIMT headers in a versioned
# x86_64/aarch64 asc include tree rather than directly under CANN/include.
if [ -z "$SIMT_ROOT" ] || [ ! -f "$SIMT_ROOT/asc_simt.h" ]; then
    for candidate in \
        "$CANN_ROOT"/x86_64-linux/asc/include/simt_api \
        "$CANN_ROOT"/aarch64-linux/asc/include/simt_api \
        "$TOOLKIT_ROOT"/x86_64-linux/asc/include/simt_api \
        "$TOOLKIT_ROOT"/aarch64-linux/asc/include/simt_api; do
        if [ -f "$candidate/asc_simt.h" ]; then
            SIMT_ROOT="$candidate"
            break
        fi
    done
fi
required_headers=(
    "acl/acl_rt.h"
    "acl/acl_rt_api.h"
)
optional_headers=(
    "acl/acl_prof.h"
    "acl/acl_dump.h"
    "acl/acl_tdt.h"
    "acl/acl_tdt_queue.h"
    "hccl/hccl.h"
    "hccl/hccl_mc2.h"
    "simt/asc_simt.h"
    "simt_api/asc_simt.h"
    "simt_api/device_warp_functions.h"
    "simt_api/device_atomic_functions.h"
)
status=0

check_header() {
    local header="$1" required="$2" path="$include_root/$header"
    if [[ "$header" == simt_api/* ]] && [ -n "$SIMT_ROOT" ]; then
        path="$SIMT_ROOT/${header#simt_api/}"
    elif [ "$header" = "simt/asc_simt.h" ] && [ -n "$SIMT_ROOT" ]; then
        path="$SIMT_ROOT/asc_simt.h"
    fi
    if [ -f "$path" ]; then
        echo "[PASS] header $header exists: $path"
        return 0
    fi
    if [ "$required" = 1 ]; then
        echo "[FAIL] header $header missing: $path"
        status=1
    else
        echo "[INFO] header $header unavailable (optional/not verified)"
    fi
}

for header in "${required_headers[@]}"; do check_header "$header" 1; done
for header in "${optional_headers[@]}"; do check_header "$header" 0; done

runtime_header="$include_root/acl/acl_rt.h"
if [ -f "$runtime_header" ]; then
    symbol_count=0
    for symbol in aclInit aclrtGetDeviceCount aclrtSetDevice aclrtMalloc aclrtMemcpyAsync; do
        if grep -Eq "(^|[^[:alnum:]_])${symbol}[[:space:]]*[(]" "$runtime_header"; then
            symbol_count=$((symbol_count + 1))
        fi
    done
    echo "[INFO] acl_rt.h declared-symbol probes: $symbol_count/5"

    compile_root="${TMPDIR:-/tmp}/cu2cce-api-probe.$$"
    mkdir -p "$compile_root"
    trap 'rm -rf "$compile_root"' EXIT
    cat >"$compile_root/include_probe.cpp" <<'CPP'
#include <acl/acl_rt.h>
#include <acl/acl_rt_api.h>
int main() { return 0; }
CPP
    if command -v g++ >/dev/null 2>&1 &&
        g++ -fsyntax-only -I"$include_root" "$compile_root/include_probe.cpp" \
            >"$compile_root/compile.log" 2>&1; then
        echo "[PASS] include compile acl/acl_rt.h + acl/acl_rt_api.h"
    else
        echo "[FAIL] include compile acl/acl_rt.h + acl/acl_rt_api.h"
        tail -n 10 "$compile_root/compile.log" 2>/dev/null || true
        status=1
    fi
fi

# When a compiler and bridge include root are supplied, compile a representative
# CUDA device-language surface end-to-end. This is intentionally compile-only:
# execution remains part of the puresimt real-test cases.
if [ -n "$CCEC" ] || [ -n "$BRIDGE_INCLUDE" ]; then
    if [ -z "$CCEC" ] || [ ! -x "$CCEC" ]; then
        echo "[FAIL] device API compile compiler missing: ${CCEC:-<unset>}"
        status=1
    elif [ -z "$BRIDGE_INCLUDE" ] || [ ! -d "$BRIDGE_INCLUDE" ]; then
        echo "[FAIL] device API compile bridge include missing: ${BRIDGE_INCLUDE:-<unset>}"
        status=1
    else
        device_root="${TMPDIR:-/tmp}/cu2cce-device-api-probe.$$"
        mkdir -p "$device_root"
        trap 'rm -rf "$device_root"' EXIT
        cat >"$device_root/device_api_probe.cu" <<'CU'
#include <cuda_runtime.h>

__global__ void cu2cce_device_api_probe(unsigned int *out) {
    int lane = static_cast<int>(threadIdx.x);
    int shuffled = __shfl_down_sync(0xffffffffu, lane, 1, 32);
    int all = __all_sync(0xffffffffu, lane >= 0);
    int reduced = __reduce_add_sync(0xffffffffu, lane);
    int any = __any_sync(0xffffffffu, lane >= 0);
    unsigned int ballot = __ballot_sync(0xffffffffu, lane >= 0);
    atomicAdd(out, static_cast<unsigned int>(shuffled));
    atomicSub(out, static_cast<unsigned int>(all));
    atomicExch(out, static_cast<unsigned int>(any));
    atomicMax(out, static_cast<unsigned int>(reduced));
    atomicMin(out, ballot);
}
CU
        cat >"$device_root/device_api_extended_probe.cu" <<'CU'
#include <cuda_runtime.h>

__global__ void cu2cce_extended_device_api_probe(unsigned int *out) {
    int lane = static_cast<int>(threadIdx.x);
    unsigned int active = __activemask();
    int reduced = __reduce_add_sync(0xffffffffu, lane);
    int reduced_max = __reduce_max_sync(0xffffffffu, lane);
    int reduced_min = __reduce_min_sync(0xffffffffu, lane);
    unsigned int old = atomicCAS(out, 0u, static_cast<unsigned int>(lane));
    atomicInc(out, 0xffffffffu);
    atomicDec(out, 0xffffffffu);
    atomicAnd(out, active);
    atomicOr(out, static_cast<unsigned int>(reduced_max));
    atomicXor(out, static_cast<unsigned int>(reduced_min));
    atomicExch(out, old);
}
CU
        simt_flags=()
        if [ -n "$SIMT_ROOT" ] && [ -d "$SIMT_ROOT" ]; then
            simt_flags+=( -I"$(dirname "$SIMT_ROOT")" -I"$SIMT_ROOT" )
        fi
        if "$CCEC" -c "$device_root/device_api_probe.cu" \
            -o "$device_root/device_api_probe.o" \
            -x asc --enable-simt --cce-aicore-arch="$NPU_ARCH" \
            -I"$BRIDGE_INCLUDE" -I"$include_root" "${simt_flags[@]}" \
            >"$device_root/compile.log" 2>&1; then
            echo "[PASS] device API compile: core SIMT/warp/vote/reduce ($NPU_ARCH)"
        else
            echo "[FAIL] device API compile: core SIMT/warp/vote/reduce ($NPU_ARCH)"
            tail -n 30 "$device_root/compile.log" 2>/dev/null || true
            status=1
        fi
        if "$CCEC" -c "$device_root/device_api_extended_probe.cu" \
            -o "$device_root/device_api_extended_probe.o" \
            -x asc --enable-simt --cce-aicore-arch="$NPU_ARCH" \
            -DCU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS=1 \
            -I"$BRIDGE_INCLUDE" -I"$include_root" "${simt_flags[@]}" \
            >"$device_root/extended_compile.log" 2>&1; then
            echo "[DIAG-PASS] device API compile: extended atomics/active-mask ($NPU_ARCH)"
        else
            echo "[DIAG-ONLY] device API compile: extended atomics/active-mask unavailable for this CANN target"
            tail -n 12 "$device_root/extended_compile.log" 2>/dev/null || true
        fi
    fi
fi

exit "$status"
