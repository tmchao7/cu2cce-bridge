#!/usr/bin/env bash
# Read-only survey of the installed CANN "simt_api" header directory.
#
# Purpose: enumerate which CUDA-compatible device-side declarations THIS
# installed CANN actually ships, so bridge compat headers forward real
# symbols instead of guessing. This replaces scraping the Huawei doc site:
# those public pages are a JS-rendered SPA (a plain HTTP fetch returns only
# navigation chrome), and their ascendcopapi family documents AscendC
# operator APIs, not the SIMT path our bridge consumes.
#
# Output layout:
#   SIMT_API_ROOT=<path>
#   == HEADER <basename> (<path>) ==
#   <lineno>:<declaration excerpt>
#   ...
#   == SYMBOL SUMMARY (heuristic call-name count) ==
set -u

candidates=(
    "${SIMT_API_ROOT:-}"
    "${ASCEND_TOOLKIT_HOME:-}/../asc/include/simt_api"
    "${ASCEND_HOME_PATH:-}/../asc/include/simt_api"
    "${CANN_BASE:-}/../asc/include/simt_api"
    "/usr/local/Ascend/latest/../asc/include/simt_api"
)
ROOT=""
for cand in ${candidates[@]+"${candidates[@]}"}; do
    if [ -n "$cand" ] && [ -d "$cand" ]; then
        ROOT="$(cd "$cand" && pwd)"
        break
    fi
done
if [ -z "$ROOT" ] || [ ! -d "$ROOT" ]; then
    found="$(find /usr/local/Ascend -maxdepth 5 -type d -name simt_api 2>/dev/null | head -n 1)"
    if [ -n "$found" ]; then ROOT="$found"; fi
fi
if [ -z "$ROOT" ] || [ ! -d "$ROOT" ]; then
    echo "NO_SIMT_API_DIR_FOUND"
    exit 0  # 非阻断：如实记录“本机未找到 simt_api”
fi

echo "SIMT_API_ROOT=$ROOT"
echo
for hdr in "$ROOT"/*.h; do
    echo "== HEADER $(basename "$hdr") ($hdr) =="
    # Declaration-ish lines: contain '(' but are not preprocessor lines.
    grep -En '^[[:space:]]*[A-Za-z_].*\(' "$hdr" 2>/dev/null \
        | grep -vE ':.*#' \
        | head -n 400 || true
    echo
done

echo "== SYMBOL SUMMARY (heuristic call-name count, most frequent first) =="
grep -hoE '\b(__)?[a-z][a-z0-9_]*\(' "$ROOT"/*.h 2>/dev/null \
    | sed 's/(//' \
    | grep -vE '^(if|for|while|switch|return|sizeof|defined|case|do)$' \
    | sort | uniq -c | sort -rn | head -n 500

# ---- Bisheng internal vector surface: the definitive type/factory list ----
# These two greps answer, in one shot:
#   a) which ext_vector_type typedefs the compiler injects (char1..float4..)?
#   b) does the double family exist at all? (Ascend device has NO double —
#      expected: zero doubleN entries)
#   c) which make_* factories exist and their execution-space annotation?
DPP="$(find /usr/local/Ascend -path '*bisheng_compiler/lib/clang/*/include/__clang_dpp_types.h' 2>/dev/null | head -n 1)"
echo
echo "== COMPILER-NATIVE VECTOR TYPES ($DPP) =="
if [ -n "$DPP" ] && [ -f "$DPP" ]; then
    grep -nE 'typedef' "$DPP" | grep -E '__attribute__\(\(ext_vector_type' || echo '(no matches)'
    echo "-- double mentions --"
    grep -icE 'double' "$DPP" | sed 's/^/double token count: /'
else
    echo '(bisheng internal header not found under /usr/local/Ascend)'
fi

VF_IMPL="$(find "$ROOT/.." -name vector_functions_impl.h 2>/dev/null | head -n 1)"
echo
echo "== make_* FACTORY SURFACE ($VF_IMPL) =="
if [ -n "$VF_IMPL" ] && [ -f "$VF_IMPL" ]; then
    grep -nE 'make_(float|int|uint|char|uchar|short|ushort|longlong)[0-9]*' "$VF_IMPL" | head -n 200
    echo "-- double factory mentions --"
    grep -cE 'make_double' "$VF_IMPL" | sed 's/^/make_double count: /'
else
    echo '(vector_functions_impl.h not found next to simt_api)'
fi
exit 0
