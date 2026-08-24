#!/bin/bash
# ============================================================
# run_acl_probe.sh —— 107002 排查：裸 ACL 分配探针（历史 runtime context 排障）
#
# 本脚本需要在安装 CANN 的 Ascend Linux 环境中执行：
#   1. 编译 acl_alloc_probe.cpp（g++ + -lascendcl，失败自动换 -lacl_rt）
#   2. 运行 /tmp/acl_probe，逐项打印每步返回码
#
# 判读（哪个组合 OK 就是哪条路径可用）：
#   A OK           → 裸 ACL 能分配，问题在 wrapper（查 wcudaMalloc 行为）
#   A FAIL / B OK  → 大页 policy 问题（§5.3，桥改用 NORMAL_ONLY）
#   A FAIL / C OK  → SetDevice 后需等待设备就绪（时序）
#   A FAIL / D OK  → 需显式 aclrtCreateContext（§4 候选根因 3）
#   E FAIL / HBM free 极小 → 环境问题：HBM 被占满
# ============================================================

if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    echo "❌ 请勿用 source 执行本脚本，请直接运行：bash ${BASH_SOURCE[0]}" >&2
    return 2 2>/dev/null || exit 2
fi

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SRC="$ROOT/tools/real_test/acl_alloc_probe.cpp"
BIN=/tmp/acl_probe

# CANN 头与库：优先 latest 软链，回退 ascend-toolkit/latest
CANN_LATEST=/usr/local/Ascend/latest
[ -d "$CANN_LATEST" ] || CANN_LATEST=/usr/local/Ascend/ascend-toolkit/latest
INC="$CANN_LATEST/include"
LIB=""
for c in "$CANN_LATEST/lib64" "$CANN_LATEST/runtime/lib64" "$CANN_LATEST/../..//lib64"; do
    if [ -f "$c/libascendcl.so" ]; then LIB="$c"; break; fi
done
[ -n "$LIB" ] || { echo "❌ 未找到 libascendcl.so（查 $CANN_LATEST 下各 lib64）"; exit 1; }
echo "== CANN=$CANN_LATEST  include=$INC  lib=$LIB"

# ACL 库名：9.x 通常 -lascendcl；个别环境只有 libacl_rt.so → 换 -lacl_rt
ACL_LIB="-lascendcl"
if ! echo '#include "acl/acl_rt.h"
int main(){return 0;}' | g++ -x c++ -std=c++17 -I"$INC" -L"$LIB" -lascendcl -o /tmp/__acl_ld_test - 2>/dev/null; then
    echo "== -lascendcl 链接测试失败，改用 -lacl_rt =="
    ACL_LIB="-lacl_rt"
fi
rm -f /tmp/__acl_ld_test

echo "== 编译探针 =="
g++ -std=c++17 "$SRC" -o "$BIN" \
    -I"$INC" -L"$LIB" "$ACL_LIB" \
    -Wl,-rpath,"$LIB" || exit 1

echo "== 运行探针 =="
"$BIN"