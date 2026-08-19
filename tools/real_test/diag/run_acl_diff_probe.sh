#!/bin/bash
# ============================================================
# run_acl_diff_probe.sh —— 差异定位探针编译+运行（107002 排查第 4 轮）
#
# 与 run_acl_probe.sh 同套路：自动找 CANN 头/库、-lascendcl 链接失败
# 自动切 -lacl_rt、编译失败打印完整 g++ 报错。
# 关键：内部全部用绝对路径，不依赖当前 shell 目录。
#
# 用法（仓库内任何目录均可）：
#   bash tools/real_test/run_acl_diff_probe.sh
# ============================================================

if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    echo "❌ 请勿用 source 执行，请直接运行：bash ${BASH_SOURCE[0]}" >&2
    return 2 2>/dev/null || exit 2
fi

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SRC="$ROOT/tools/real_test/acl_diff_probe.cpp"
BIN=/tmp/acl_diff

[ -f "$SRC" ] || { echo "❌ 找不到 $SRC（先 git pull 拿最新代码）"; exit 1; }

# CANN 头与库：优先 latest 软链，回退 ascend-toolkit/latest
CANN_LATEST=/usr/local/Ascend/latest
[ -d "$CANN_LATEST" ] || CANN_LATEST=/usr/local/Ascend/ascend-toolkit/latest
INC="$CANN_LATEST/include"
LIB=""
for c in "$CANN_LATEST/lib64" "$CANN_LATEST/runtime/lib64" "$CANN_LATEST"/../*/lib64; do
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

echo "== 编译 $SRC -> $BIN =="
g++ -std=c++17 "$SRC" -o "$BIN" \
    -I"$INC" -L"$LIB" "$ACL_LIB" \
    -Wl,-rpath,"$LIB"
rc=$?
if [ $rc -ne 0 ]; then
    echo "❌ 编译失败（rc=$rc），上面的 g++ 报错就是原因，请贴回聊天"
    exit $rc
fi

echo "== 运行 $BIN =="
"$BIN"