#!/bin/bash
# syntax_check.sh —— 本机（macOS）可运行的语法验证（mock ACL + clang）
# 注意：examples/vectorAdd/vectorAdd.cu 含 kernel（<<<>>>/__global__），需 bisheng 编译，
#       本脚本只验证纯 host 链路（tests/host_bridge_smoke.cu）。
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
check "host_bridge_smoke.cu" -x c++ "$ROOT/tests/host_bridge_smoke.cu"
check "cuda_runtime_wrapper.cpp" -x c++ "$SRC/cuda_runtime_wrapper.cpp"
check "cublas_v2_wrapper.cpp" -x c++ "$BLAS_SRC/cublas_v2_wrapper.cpp"
echo "---"; echo "PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ]
