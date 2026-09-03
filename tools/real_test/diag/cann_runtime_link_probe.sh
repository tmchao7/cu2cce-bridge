#!/bin/bash
# Compile/link a minimal public CANN Runtime call without changing the repo.
set -u

CANN_ROOT="${CANN_BASE:-/usr/local/Ascend/latest}"
TOOLKIT_ROOT="${ASCEND_TOOLKIT_HOME:-/usr/local/Ascend/ascend-toolkit/latest}"
BRIDGE_LIB=""
OUTPUT=""
while [ "$#" -gt 0 ]; do
    case "$1" in
        --cann-base)
            [ "$#" -ge 2 ] || { echo "cann_runtime_link_probe: --cann-base requires a value" >&2; exit 2; }
            CANN_ROOT="$2"; shift 2 ;;
        --bridge-lib)
            [ "$#" -ge 2 ] || { echo "cann_runtime_link_probe: --bridge-lib requires a value" >&2; exit 2; }
            BRIDGE_LIB="$2"; shift 2 ;;
        --output)
            [ "$#" -ge 2 ] || { echo "cann_runtime_link_probe: --output requires a value" >&2; exit 2; }
            OUTPUT="$2"; shift 2 ;;
        -h|--help)
            echo "usage: $0 [--cann-base ROOT] [--bridge-lib FILE] [--output FILE]"; exit 0 ;;
        *)
            echo "cann_runtime_link_probe: unknown option: $1" >&2; exit 2 ;;
    esac
done

if [ -n "$OUTPUT" ]; then
    mkdir -p "$(dirname "$OUTPUT")"
    exec > >(tee "$OUTPUT")
fi

include_root="$CANN_ROOT/include"
if [ ! -f "$include_root/acl/acl_rt.h" ] && [ -f "$TOOLKIT_ROOT/include/acl/acl_rt.h" ]; then
    include_root="$TOOLKIT_ROOT/include"
fi
lib_dirs=(
    "$CANN_ROOT/lib64"
    "$CANN_ROOT/runtime/lib64"
    "$TOOLKIT_ROOT/lib64"
    "$TOOLKIT_ROOT/runtime/lib64"
)
runtime_lib=""
runtime_flag=""
for dir in "${lib_dirs[@]}"; do
    if [ -f "$dir/libacl_rt.so" ]; then
        runtime_lib="$dir/libacl_rt.so"
        runtime_flag="-lacl_rt"
        break
    fi
done
if [ -z "$runtime_lib" ]; then
    for dir in "${lib_dirs[@]}"; do
        if [ -f "$dir/libascendcl.so" ]; then
            runtime_lib="$dir/libascendcl.so"
            runtime_flag="-lascendcl"
            break
        fi
    done
fi

if [ ! -f "$include_root/acl/acl_rt.h" ]; then
    echo "[FAIL] runtime header acl/acl_rt.h missing: $include_root/acl/acl_rt.h"
    exit 1
fi
if [ -z "$runtime_lib" ]; then
    echo "[FAIL] CANN Runtime library missing: expected libacl_rt.so or legacy libascendcl.so"
    exit 1
fi

work="${TMPDIR:-/tmp}/cu2cce-runtime-link.$$"
mkdir -p "$work"
trap 'rm -rf "$work"' EXIT
cat >"$work/runtime_probe.cpp" <<'CPP'
#include <acl/acl_rt.h>
int main() {
    uint32_t count = 0;
    return static_cast<int>(aclrtGetDeviceCount(&count));
}
CPP

if ! command -v g++ >/dev/null 2>&1; then
    echo "[FAIL] g++ is unavailable"
    exit 1
fi

if g++ "$work/runtime_probe.cpp" -I"$include_root" \
    -L"$(dirname "$runtime_lib")" "$runtime_flag" \
    -Wl,-rpath,"$(dirname "$runtime_lib")" -o "$work/runtime_probe" \
    >"$work/link.log" 2>&1; then
    echo "[PASS] CANN Runtime link $runtime_flag -> $(basename "$runtime_lib")"
else
    echo "[FAIL] CANN Runtime link $runtime_flag"
    tail -n 20 "$work/link.log" 2>/dev/null || true
    exit 1
fi

if command -v readelf >/dev/null 2>&1; then
    readelf -d "$work/runtime_probe" >"$work/readelf.log" 2>&1 || true
    if grep -Eq 'NEEDED.*(libacl_rt|libascendcl)' "$work/readelf.log"; then
        echo "[PASS] runtime probe NEEDED records CANN Runtime"
    else
        echo "[FAIL] runtime probe NEEDED does not record CANN Runtime"
        tail -n 20 "$work/readelf.log" 2>/dev/null || true
        exit 1
    fi
    if grep -Eq 'RPATH|RUNPATH' "$work/readelf.log"; then
        echo "[PASS] runtime probe RPATH/RUNPATH recorded"
    else
        echo "[FAIL] runtime probe RPATH/RUNPATH missing"
        exit 1
    fi
fi

if [ -n "$BRIDGE_LIB" ]; then
    if [ -f "$BRIDGE_LIB" ]; then
        echo "[PASS] bridge runtime present: $BRIDGE_LIB"
    else
        echo "[FAIL] bridge runtime missing: $BRIDGE_LIB"
        exit 1
    fi
fi
