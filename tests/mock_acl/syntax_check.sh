#!/bin/bash
# syntax_check.sh —— 本机（macOS）可运行的语法验证（mock ACL + clang）
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
MOCK="$ROOT/tests/mock_acl"
INCLUDE="$ROOT/include"
SRC="$ROOT/src/bridge/runtime/src"
BLAS_SRC="$ROOT/src/bridge/blas/src"

PASS=0; FAIL=0
check() {
    local name="$1"; shift
    if clang++ -std=c++17 -fsyntax-only \
        -I "$INCLUDE" -I "$INCLUDE/bridge/runtime" -I "$INCLUDE/bridge/blas" -I "$MOCK" "$@" 2>&1; then
        echo "✓ $name"; PASS=$((PASS+1))
    else
        echo "✗ $name"; FAIL=$((FAIL+1))
    fi
}
check "vectorAdd.cu (host side)" -x c++ "$ROOT/examples/vectorAdd/vectorAdd.cu"
check "cuda_runtime_wrapper.cpp" -x c++ "$SRC/cuda_runtime_wrapper.cpp"
check "cublas_v2_wrapper.cpp" -x c++ "$BLAS_SRC/cublas_v2_wrapper.cpp"
echo "---"; echo "PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ]
