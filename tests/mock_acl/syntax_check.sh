#!/bin/bash
# ============================================================
# syntax_check.sh —— 本机（macOS）可运行的语法验证
#
# 用 mock ACL 头 + clang 语法检查验证 cu2cce-bridge 的头文件链、
# wrapper 实现、示例的宏替换自洽性。不需要真机/CANN。
#
# 验证内容：
#   1. examples/vectorAdd/vectorAdd.cu 的 host 侧（cudaMalloc→wcudaMalloc→aclrtMalloc 链路）
#   2. src/.../cuda_runtime_wrapper.cpp（wcuda* 实现）
#
# 用法：bash tests/mock_acl/syntax_check.sh
# ============================================================
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
MOCK="$ROOT/tests/mock_acl"
INCLUDE="$ROOT/include"
SRC="$ROOT/src/bridge/runtime/src"

PASS=0
FAIL=0

check() {
    local name="$1"; shift
    if clang++ -std=c++17 -fsyntax-only \
        -I "$INCLUDE" -I "$INCLUDE/bridge/runtime" -I "$MOCK" "$@" 2>&1; then
        echo "✓ $name"
        PASS=$((PASS+1))
    else
        echo "✗ $name"
        FAIL=$((FAIL+1))
    fi
}

check "vectorAdd.cu (host side)" -x c++ "$ROOT/examples/vectorAdd/vectorAdd.cu"
check "cuda_runtime_wrapper.cpp" -x c++ "$SRC/cuda_runtime_wrapper.cpp"

echo "---"
echo "PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ]
