#!/bin/bash
# ============================================================
# run_tests.sh —— 真机运行测试（分工③：真实执行"编译 + 启动运行"命令）
#
# 用法：
#   tools/real_test/run_tests.sh host          # 纯 host API 冒烟（cudaMalloc/Memcpy/Memset/cublasCreate）
#   tools/real_test/run_tests.sh vectoradd     # 端到端 vectorAdd（kernel 直通 + 数值校验）
#   tools/real_test/run_tests.sh all           # 全部（默认）
#   tools/real_test/run_tests.sh all --direct  # 绕过 cucc 直接调 bisheng（排查驱动问题的对照路径）
#
# 前置：env_probe.sh + build_bridge.sh
# 说明：当前 bin/cucc 只在命令行出现 *.o 时才追加链接库，
#       因此统一用两步法：cucc -c 编译 → cucc *.o 链接（这也是对驱动本身的测试）。
#
# 退出码：0 = 全部通过；1 = 有失败（日志见 build/real_test/logs/）
# ============================================================

# 防误用：本脚本末尾会 exit，被 source 会直接退出交互 shell
if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    echo "❌ 请勿用 source 执行本脚本，请直接运行：bash ${BASH_SOURCE[0]}" >&2
    return 2 2>/dev/null || exit 2
fi

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT_DIR="${OUT_DIR:-$ROOT/build/real_test}"
PROBE_ENV="$OUT_DIR/probe.env"

[ -f "$PROBE_ENV" ] || { echo "❌ 未找到 ${PROBE_ENV}，先跑 tools/real_test/env_probe.sh"; exit 1; }
# shellcheck source=/dev/null
source "$PROBE_ENV"

MODE="all"; DIRECT=0
for a in "$@"; do
    case "$a" in
        host|vectoradd|all) MODE="$a";;
        --direct) DIRECT=1;;
        *) echo "未知参数：$a"; exit 2;;
    esac
done

# ---------------- 环境 ----------------
CUCC_PATH="${CUCC_PATH:-$OUT_DIR/install}"
export CUCC_PATH
export CUDA_PATH="$CUCC_PATH"
export ASCEND_TOOLKIT_HOME ASCEND_INCLUDE ASCEND_LIB BISHENG_PATH
export PATH="$CUCC_PATH/bin:$PATH"
export LD_LIBRARY_PATH="$CUCC_PATH/lib:$ASCEND_LIB:${LD_LIBRARY_PATH:-}"
# kernel 若用 __syncthreads/__shared__ 宏，simt_api 头需进 include 路径（clang 认 CPLUS_INCLUDE_PATH）
if [ -n "${SIMT_API_INCLUDE:-}" ]; then
    export CPLUS_INCLUDE_PATH="$SIMT_API_INCLUDE:${CPLUS_INCLUDE_PATH:-}"
fi
# CANN 环境（若 setenv.bash 存在，补齐运行时需要的变量）
[ -f "$ASCEND_TOOLKIT_HOME/bin/setenv.bash" ] && source "$ASCEND_TOOLKIT_HOME/bin/setenv.bash"

[ -x "$CUCC_PATH/bin/cucc" ] || { echo "❌ 未找到 $CUCC_PATH/bin/cucc（先跑 build_bridge.sh）"; exit 1; }
[ -f "$CUCC_PATH/lib/libcce_bridge_runtime.so" ] || { echo "❌ 未找到 $CUCC_PATH/lib/libcce_bridge_runtime.so（先跑 build_bridge.sh）"; exit 1; }

mkdir -p "$OUT_DIR/logs"

PASS=0; FAIL=0

run_step() {  # run_step <名称> <命令...>
    local name="$1"; shift
    local log="$OUT_DIR/logs/${name}.log"
    echo "----- $name -----"
    if "$@" > "$log" 2>&1; then
        echo "  [PASS] $name"
        PASS=$((PASS+1))
        return 0
    else
        echo "  [FAIL] ${name}（日志 ${log}）"
        tail -n 15 "$log" | sed 's/^/         /'
        FAIL=$((FAIL+1))
        return 1
    fi
}

expect_output() {  # expect_output <名称> <日志文件> <关键字>
    local name="$1" log="$2" key="$3"
    if grep -q "$key" "$log"; then
        echo "  [PASS] ${name}（输出含 \"${key}\"）"
        PASS=$((PASS+1))
        return 0
    else
        echo "  [FAIL] ${name}（输出缺少 \"${key}\"，日志 ${log}）"
        tail -n 15 "$log" | sed 's/^/         /'
        FAIL=$((FAIL+1))
        return 1
    fi
}

# ---------------- 编译/链接两条路径 ----------------
# 路径 1（默认）：走 bin/cucc 驱动，测的是驱动本身
cucc_compile() { cucc -c "$1" -o "$2"; }
cucc_link()    { cucc "$1" -o "$2"; }

# 路径 2（--direct）：绕过 cucc 直接调 bisheng，用于排查驱动问题时隔离变量
direct_compile() {
    "$BISHENG_PATH" -std=c++17 -xcce --npu-arch="${CUCC_NPU_ARCH:-dav-c310}" \
        -include "$CUCC_PATH/include/bridge/kernel/simt_adaptor.h" \
        -I "$CUCC_PATH/include" -I "$ASCEND_INCLUDE" \
        -D CUDA_VERSION=11060 -c "$1" -o "$2"
}
direct_link() {
    local link_args=( "$BISHENG_PATH" "$1" -o "$2" \
        -L "$CUCC_PATH/lib" -lcce_bridge_runtime \
        -L "$ASCEND_LIB" "${ACL_RT_LIB:-}" )
    [ "${HAS_ACL_CBLAS:-0}" = "1" ] && link_args+=( -lacl_cblas )
    "${link_args[@]}"
}

# ---------------- 测试 1：纯 host API 冒烟 ----------------
test_host() {
    local obj="$OUT_DIR/host_smoke.o" bin="$OUT_DIR/host_smoke"
    if [ "$DIRECT" = "1" ]; then
        run_step "host_compile_direct" direct_compile "$ROOT/tests/host_bridge_smoke.cu" "$obj" || return 1
        run_step "host_link_direct"    direct_link    "$obj" "$bin" || return 1
    else
        run_step "host_compile_cucc" cucc_compile "$ROOT/tests/host_bridge_smoke.cu" "$obj" || return 1
        run_step "host_link_cucc"    cucc_link    "$obj" "$bin" || return 1
    fi
    run_step "host_run" "$bin" || return 1
    expect_output "host_check_out" "$OUT_DIR/logs/host_run.log" "host bridge smoke OK" || return 1
    echo "✅ host API 冒烟通过"
}

# ---------------- 测试 2：vectorAdd 端到端（kernel 直通） ----------------
test_vectoradd() {
    local obj="$OUT_DIR/vectorAdd.o" bin="$OUT_DIR/vectorAdd"
    if [ "$DIRECT" = "1" ]; then
        run_step "vectoradd_compile_direct" direct_compile "$ROOT/examples/vectorAdd/vectorAdd.cu" "$obj" || return 1
        run_step "vectoradd_link_direct"    direct_link    "$obj" "$bin" || return 1
    else
        run_step "vectoradd_compile_cucc" cucc_compile "$ROOT/examples/vectorAdd/vectorAdd.cu" "$obj" || return 1
        run_step "vectoradd_link_cucc"    cucc_link    "$obj" "$bin" || return 1
    fi
    run_step "vectoradd_run" "$bin" || return 1
    expect_output "vectoradd_check_out" "$OUT_DIR/logs/vectoradd_run.log" "vectorAdd OK" || return 1
    echo "✅ vectorAdd 端到端通过（kernel 直通 + 数值校验）"
}

# ---------------- 主流程 ----------------
echo "== run_tests：CUCC_PATH=$CUCC_PATH 模式=$([ "$DIRECT" = "1" ] && echo direct-bisheng || echo cucc) MODE=$MODE =="
case "$MODE" in
    host)      test_host;;
    vectoradd) test_vectoradd;;
    all)       test_host; test_vectoradd;;
esac

echo "== 汇总：PASS=$PASS FAIL=${FAIL}（日志目录 $OUT_DIR/logs/） =="
[ "$FAIL" -eq 0 ]
