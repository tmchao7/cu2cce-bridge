#!/bin/bash
# ============================================================
# run_minimal_echo.sh —— 逐步复现 minimal 的 107002（第 6 轮）
#
# 1) 用 ccec 直接编译 echo.cce（无 kernel，最小解析面）
# 2) 链接注入 wrapper + 记录所有中间产物（--keep）与编译诊断
# 3) 运行 echo：确认"wrapper 激活后以 aclrtMalloc 方式分配"是否也 107002
#
# 用法：bash tools/real_test/run_minimal_echo.sh
# ============================================================

if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    echo "❌ 请勿用 source 执行，请直接运行：bash ${BASH_SOURCE[0]}" >&2
    return 2 2>/dev/null || exit 2
fi

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SRC="$ROOT/tools/real_test/diag/echo.cce"
INSTALL="$ROOT/build/real_test/install"
BIN=/tmp/echo

CANN_LATEST=/usr/local/Ascend/latest
[ -d "$CANN_LATEST" ] || CANN_LATEST=/usr/local/Ascend/ascend-toolkit/latest
CCEC="$CANN_LATEST/bin/ccec"
[ -x "$CCEC" ] || { echo "❌ 无 $CCEC，先确认 CANN 编译器"; exit 1; }
INC="$CANN_LATEST/include"

echo "== 编译 echo.cce（ccec，-xcce，无 kernel 解析） =="
# -xcce 编译 .cce 源；v- 保留中间 .cce；error-limit 打开完整诊断
"$CCEC" -std=gnu++17 -xcce --cce-aicore-arch=dav-c310 \
    -I "$INSTALL/include/shadow" -I "$INSTALL/include" -I "$INC" \
    -D__CU2CCE_NO_LIBC__ -D CUDA_VERSION=11060 \
    -c "$SRC" -o /tmp/echo.o 2>&1 | tee /tmp/echo_compile.log
rc=${PIPESTATUS[0]}
if [ $rc -ne 0 ]; then
    echo "❌ 编译失败 rc=$rc，完整输出在上 /tmp/echo_compile.log，请贴回"
    exit $rc
fi
echo "== 编译 OK：/tmp/echo.o =="

echo "== 链接（注入 wrapper + 所有 CANN 运行库，与 cucc 对齐） =="
"$CCEC" /tmp/echo.o -o "$BIN" \
    -L "$INSTALL/lib" -Wl,-rpath,"$INSTALL/lib" -lcce_bridge_runtime \
    -L "$CANN_LATEST/lib64" -Wl,-rpath,"$CANN_LATEST/lib64" \
    -Wl,-rpath,"$CANN_LATEST/runtime/lib64" \
    -lascendcl -lruntime -lstdc++ -lpthread -lm 2>&1 | tee /tmp/echo_link.log
rc=${PIPESTATUS[0]}
if [ $rc -ne 0 ]; then
    echo "❌ 链接失败 rc=$rc，完整输出在上 /tmp/echo_link.log，请贴回"
    exit $rc
fi
echo "== 链接 OK：$BIN =="

echo "== ldd：实际加载的库 =="
ldd "$BIN" | grep -E "ascendcl|acl_rt|runtime|cce_bridge"

echo "== 运行 =="
WCUDA_DEBUG=1 "$BIN"