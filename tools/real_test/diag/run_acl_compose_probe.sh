#!/bin/bash
# ============================================================
# run_acl_compose_probe.sh —— 库组合探针（107002 第 5 轮）
#
# 同一段分配代码，4 种链接组合，锁定"多带哪个库导致 aclrtMalloc 翻 107002"：
#   mode1 baseline  （纯 -lascendcl）
#   mode2 +runtime   （+ -lruntime）
#   mode3 +wrapper   （+ 安装目录 libcce_bridge_runtime.so，需先 build_bridge）
#   mode4 +both      （+ -lruntime + wrapper，最接近 minimal）
#
# 另核查 CANN 多根（readlink/md5）：minimal 的 libruntime 解析到
# /usr/local/Ascend/cann-9.2.0/lib64，而探针 1 用的 rpath 是 latest/lib64——
# 若两套 libascendcl.so 不同，双加载也可能导致 107002。
#
# 用法：bash tools/real_test/run_acl_compose_probe.sh
# ============================================================

if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    echo "❌ 请勿用 source 执行，请直接运行：bash ${BASH_SOURCE[0]}" >&2
    return 2 2>/dev/null || exit 2
fi

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SRC="$ROOT/tools/real_test/acl_compose_probe.cpp"
INSTALL="$ROOT/build/real_test/install"
WRAPPER="$INSTALL/lib/libcce_bridge_runtime.so"

[ -f "$SRC" ] || { echo "❌ 找不到 $SRC（先 git pull）"; exit 1; }
[ -f "$WRAPPER" ] || { echo "❌ 找不到 $WRAPPER（先跑 bash tools/real_test/build_bridge.sh）"; exit 1; }

# CANN 根
CANN_LATEST=/usr/local/Ascend/latest
[ -d "$CANN_LATEST" ] || CANN_LATEST=/usr/local/Ascend/ascend-toolkit/latest
INC="$CANN_LATEST/include"

echo "================ 0. CANN 多根核查 ================"
for d in /usr/local/Ascend/latest /usr/local/Ascend/cann-9.2.0 /usr/local/Ascend/ascend-toolkit/latest; do
    [ -d "$d" ] || continue
    echo "--- $d ---"
    ls -ld "$d"
    for so in libascendcl.so libacl_rt.so libruntime.so; do
        for cand in "$d/lib64/$so" "$d/runtime/lib64/$so"; do
            if [ -e "$cand" ]; then
                printf "   %-8s %s -> %s\n" "$so" "$(readlink -f "$cand")" "$(md5sum "$cand" | cut -d' ' -f1)"
            fi
        done
    done
done

echo ""
echo "================ 1. 编译 4 个组合 ================"
compile() { # compile <mode> <libs...>
    local mode="$1"; shift
    g++ -std=c++17 "$SRC" -o "/tmp/compose_$mode" \
        -I"$INC" -L"$CANN_LATEST/lib64" "$@" \
        -Wl,-rpath,"$CANN_LATEST/lib64" || { echo "❌ 组合 $mode 编译失败"; return 1; }
    echo "  编译 OK: /tmp/compose_$mode  (libs: $*)"
}

compile base   -lascendcl
compile runtim -lascendcl -lruntime
compile wrap   -lascendcl -L"$INSTALL/lib" -lcce_bridge_runtime
compile both   -lascendcl -lruntime -L"$INSTALL/lib" -lcce_bridge_runtime
echo "(运行库 rpath 统一指向 latest；组合机如解析到其他根，见第 0 节输出)"

echo ""
echo "================ 2. 逐个运行（看 aclrtMalloc ret） ================"
for mode in base runtim wrap both; do
    echo "----- mode: $mode -----"
    LD_LIBRARY_PATH="$(dirname "$(readlink -f "$CANN_LATEST/lib64/libascendcl.so")")" "/tmp/compose_$mode"
done

echo ""
echo "================ 3. 各组合实际解析的库（关键！见解析到的路径） ================"
for mode in base runtim wrap both; do
    echo "--- $mode ---"
    LD_LIBRARY_PATH="$(dirname "$(readlink -f "$CANN_LATEST/lib64/libascendcl.so")")" \
        ldd "/tmp/compose_$mode" | grep -E "ascendcl|acl_rt|runtime|cce_bridge" | sort
done