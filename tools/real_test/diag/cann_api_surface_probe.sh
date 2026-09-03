#!/bin/bash
# Read-only CANN public API surface probe.
# It reports compact evidence for the headers used by the CUDA -> Ascend map.
set -u

CANN_ROOT="${CANN_BASE:-/usr/local/Ascend/latest}"
OUTPUT=""
while [ "$#" -gt 0 ]; do
    case "$1" in
        --cann-base)
            [ "$#" -ge 2 ] || { echo "cann_api_surface_probe: --cann-base requires a value" >&2; exit 2; }
            CANN_ROOT="$2"; shift 2 ;;
        --output)
            [ "$#" -ge 2 ] || { echo "cann_api_surface_probe: --output requires a value" >&2; exit 2; }
            OUTPUT="$2"; shift 2 ;;
        -h|--help)
            echo "usage: $0 [--cann-base ROOT] [--output FILE]"; exit 0 ;;
        *)
            echo "cann_api_surface_probe: unknown option: $1" >&2; exit 2 ;;
    esac
done

if [ -n "$OUTPUT" ]; then
    mkdir -p "$(dirname "$OUTPUT")"
    exec > >(tee "$OUTPUT")
fi

include_root="$CANN_ROOT/include"
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
)
status=0

check_header() {
    local header="$1" required="$2" path="$include_root/$header"
    if [ -f "$path" ]; then
        echo "[PASS] header $header exists"
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
int main() { return 0; }
CPP
    if command -v g++ >/dev/null 2>&1 &&
        g++ -fsyntax-only -I"$include_root" "$compile_root/include_probe.cpp" \
            >"$compile_root/compile.log" 2>&1; then
        echo "[PASS] include compile acl/acl_rt.h"
    else
        echo "[FAIL] include compile acl/acl_rt.h"
        tail -n 10 "$compile_root/compile.log" 2>/dev/null || true
        status=1
    fi
fi

exit "$status"
