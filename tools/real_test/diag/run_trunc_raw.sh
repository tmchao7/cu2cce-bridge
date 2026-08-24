#!/bin/bash
# ============================================================
# run_trunc_raw.sh —— 不链接 wrapper 的裸流程对照（107002 终极隔离）
#
# 编译一个不依赖 libcce_bridge_runtime.so 的裸 ACL 程序（完整复刻
# minimal 的 host 流程 + 逐行打印），跑一次对照。若它 ret=0 → 排除
# 所有"顺序/环境"类解释，把差异锁定在 wrapper 本身。
# ============================================================

if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    echo "❌ 请勿用 source 执行，请直接运行：bash ${BASH_SOURCE[0]}" >&2
    return 2 2>/dev/null || exit 2
fi

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SRC="$ROOT/tools/real_test/trunc_raw.c"

CANN_LATEST=/usr/local/Ascend/latest
[ -d "$CANN_LATEST" ] || CANN_LATEST=/usr/local/Ascend/ascend-toolkit/latest
INC="$CANN_LATEST/include"
LIB="$CANN_LATEST/lib64"

[ -f "$SRC" ] || { echo "❌ 找不到 $SRC（先 git pull）"; exit 1; }

echo "== 编译（仅 -lascendcl，完全不碰 wrapper） =="
gcc -o /tmp/trunc_raw "$SRC" \
    -I"$INC" -L"$LIB" -lascendcl -Wl,-rpath,"$LIB" || exit 1
echo "== ldd（确认无 cce_bridge_runtime） =="
ldd /tmp/trunc_raw | grep -E "ascendcl|acl_rt|runtime|cce_bridge" || true

echo "== 运行 =="
/tmp/trunc_raw