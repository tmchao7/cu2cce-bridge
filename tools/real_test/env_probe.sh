#!/bin/bash
# ============================================================
# env_probe.sh —— 真机环境探测（分工①：只负责"找包的位置"，不构建、不跑测试）
#
# 职责：
#   1. 定位 CANN：ASCEND_TOOLKIT_HOME / ASCEND_HOME / setenv.bash
#   2. 定位工具链：bisheng 编译器、ACL 头/库、acl_cblas、simt_api SIMT 头、npu-smi
#   3. 只读核对 docs/04_对比与验证/CURRENT_STATE.md 的"待真机确认项"
#      （grep 头文件签名，不调用任何昇腾运行时 API）
#   4. 核对目标架构 --cce-aicore-arch=dav-c310-vec（950 系列 puresimt）：
#      dump __NPU_ARCH__ 宏值、比对 simt_api 门控、最小 SIMT kernel 实测编译
#      （产物写 /tmp，不触碰仓库构建目录）
#   5. 结果写 OUT_DIR/probe.env（供 build_bridge.sh 与 server_cmd.txt 使用）
#      + OUT_DIR/logs/probe.log（完整明细）
#
# 用法：
#   tools/real_test/env_probe.sh
#   OUT_DIR=/tmp/probe tools/real_test/env_probe.sh   # 覆盖输出目录
#
# 退出码：0 = 环境齐备；1 = 缺 CANN/编译器/核心库（签名未命中只 WARN 不阻断）
# ============================================================

# 防误用：本脚本末尾会 exit，被 source 会直接退出交互 shell（真机教训）
if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    echo "❌ 请勿用 source 执行本脚本，请直接运行：bash ${BASH_SOURCE[0]}" >&2
    return 2 2>/dev/null || exit 2
fi

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT_DIR="${OUT_DIR:-$ROOT/build/real_test}"
mkdir -p "$OUT_DIR/logs"
PROBE_ENV="$OUT_DIR/probe.env"
PROBE_LOG="$OUT_DIR/logs/probe.log"
: > "$PROBE_ENV"
: > "$PROBE_LOG"

log()  { echo "$*" | tee -a "$PROBE_LOG"; }
ok()   { echo "  [OK]   $*" | tee -a "$PROBE_LOG"; }
miss() { echo "  [MISS] $*" | tee -a "$PROBE_LOG"; }
warn() { echo "  [WARN] $*" | tee -a "$PROBE_LOG"; }

MISS=0   # 硬缺失计数（决定退出码）
WARN=0   # 签名核对未命中计数（只提示，不阻断）

# ---------------- 1. 定位 CANN（ASCEND_TOOLKIT_HOME） ----------------
log "== 1. CANN 安装目录 =="
ATH="${ASCEND_TOOLKIT_HOME:-}"
if [ -z "$ATH" ]; then
    for base in /usr/local/Ascend /opt/Ascend "$HOME/Ascend" "$HOME/.local/Ascend"; do
        for cand in "$base/ascend-toolkit/latest" "$base"/ascend-toolkit/*; do
            if [ -f "$cand/include/acl/acl_rt.h" ]; then ATH="$cand"; break 2; fi
        done
    done
fi
if [ -z "$ATH" ]; then
    log "  常见路径未命中，在 /usr/local /opt 下搜索 ascend-toolkit 目录（maxdepth 5）..."
    for cand in $(find /usr/local /opt -maxdepth 5 -type d -name "ascend-toolkit" 2>/dev/null | head -5); do
        for sub in "$cand/latest" "$cand"/*; do
            if [ -f "$sub/include/acl/acl_rt.h" ]; then ATH="$sub"; break 2; fi
        done
    done
fi
if [ -n "$ATH" ] && [ -f "$ATH/include/acl/acl_rt.h" ]; then
    ok "ASCEND_TOOLKIT_HOME=$ATH"
else
    miss "未找到 CANN（ascend-toolkit，需含 include/acl/acl_rt.h）"
    log "  请手动：export ASCEND_TOOLKIT_HOME=/path/to/ascend-toolkit/latest 后重跑"
    ATH=""
fi
ASCEND_HOME=""
if [ -n "$ATH" ]; then
    ASCEND_HOME="${ASCEND_HOME:-$(dirname "$(dirname "$ATH")")}"
    log "  推断 ASCEND_HOME=$ASCEND_HOME"
fi
# CANN_BASE：CANN 安装根（含 bin/ccec、runtime/、set_env.sh；用户实测脚本用 /usr/local/Ascend/latest）
CANN_BASE=""
for cand in "$ASCEND_HOME/latest" /usr/local/Ascend/latest; do
    if [ -x "$cand/bin/ccec" ]; then CANN_BASE="$cand"; break; fi
done
if [ -n "$CANN_BASE" ]; then
    ok "CANN_BASE=${CANN_BASE}（含 bin/ccec）"
else
    warn "未找到 CANN_BASE（含 bin/ccec 的目录，如 /usr/local/Ascend/latest）"
fi

# ---------------- 2. 编译器 / 工具链 ----------------
log "== 2. 编译器 / 工具链 =="
# main/puresimt 只检查 CANN 自带 ccec；旧 bisheng fallback 不属于 main。
# ccec：CANN 自带编译器（用户实测脚本用 /usr/local/Ascend/latest/bin/ccec，
# 架构参数 --cce-aicore-arch=dav-c310-vec；main 不回退旧 bisheng 路线）
CCEC_PATH=""
for cand in "$CANN_BASE/bin/ccec" "$ATH/bin/ccec" "$ASCEND_HOME"/latest/bin/ccec \
            "$ASCEND_HOME"/cann-*/bin/ccec; do
    if [ -x "$cand" ]; then CCEC_PATH="$cand"; break; fi
done
[ -z "$CCEC_PATH" ] && command -v ccec >/dev/null 2>&1 && CCEC_PATH="$(command -v ccec)"
if [ -n "$CCEC_PATH" ]; then
    ok "ccec=$CCEC_PATH"
else
    warn "未找到 ccec（CANN 自带编译器）；真机实测 /usr/local/Ascend/latest/bin/ccec"
fi
# 打印 compiler 目录结构帮助排查（只读）
if [ -d "$ATH/compiler" ]; then
    log "  --- $ATH/compiler/ 目录内容 ---"
    ls "$ATH/compiler" 2>/dev/null | tee -a "$PROBE_LOG"
    for sub in "$ATH"/compiler/*/bin; do
        [ -d "$sub" ] || continue
        log "  --- $sub 内容（前 20 项） ---"
        ls "$sub" 2>/dev/null | head -20 | tee -a "$PROBE_LOG"
    done
fi
[ -f "$ATH/bin/setenv.bash" ] && ok "setenv.bash=$ATH/bin/setenv.bash" || warn "无 $ATH/bin/setenv.bash（依赖 shell 已配置 PATH/LD_LIBRARY_PATH）"

for tool in gcc g++ make cmake; do
    if command -v "$tool" >/dev/null 2>&1; then
        ok "$tool=$(command -v "$tool")"
        [ "$tool" = "gcc" ] || [ "$tool" = "g++" ] && { log "    版本：$("$tool" --version 2>/dev/null | head -1)"; }
        [ "$tool" = "cmake" ] && { log "    版本：$(cmake --version 2>/dev/null | head -1)"; }
    else
        miss "缺 $tool"; MISS=$((MISS+1))
    fi
done

# ---------------- 3. ACL 头文件 / 库 ----------------
log "== 3. ACL 头文件 / 库 =="
for h in acl/acl_rt.h acl/acl_base.h acl/ops/acl_cblas.h; do
    if [ -f "$ATH/include/$h" ]; then ok "include/$h"; else miss "include/$h"; MISS=$((MISS+1)); fi
done

ACL_RT_LIB=""
if [ -f "$ATH/lib64/libacl_rt.so" ]; then
    ACL_RT_LIB="-lacl_rt"; ok "lib64/libacl_rt.so（链接参数 ${ACL_RT_LIB}）"
elif [ -f "$ATH/lib64/libascendcl.so" ]; then
    ACL_RT_LIB="-lascendcl"; ok "lib64/libascendcl.so（旧版 fallback，链接参数 ${ACL_RT_LIB}）"
else
    miss "lib64/libacl_rt.so 与 libascendcl.so 都不存在"; MISS=$((MISS+1))
fi

HAS_ACL_CBLAS=0
if [ -f "$ATH/lib64/libacl_cblas.so" ]; then
    HAS_ACL_CBLAS=1; ok "lib64/libacl_cblas.so"
else
    warn "lib64/libacl_cblas.so 不存在（cublas 桥链接需要，请确认 CANN 是否安装 blas 组件）"
fi

# ---------------- 4. SIMT 头（simt_api，kernel 直通依赖） ----------------
log "== 4. SIMT 头（simt_api） =="
SIMT_API_INCLUDE=""
if [ -f "$ATH/include/simt_api/asc_simt.h" ]; then
    SIMT_API_INCLUDE="$ATH/include/simt_api"
    ok "simt_api 位于 CANN include 内：$SIMT_API_INCLUDE"
else
    found=""
    # 候选 A：版本化目录布局（真机 9.2.0 实测：cann-*/x86_64-linux/asc/include/simt_api）
    for found_cand in \
        "$ASCEND_HOME"/cann-*/x86_64-linux/asc/include/simt_api \
        "$ASCEND_HOME"/cann-*/aarch64-linux/asc/include/simt_api \
        /usr/local/Ascend/cann-*/x86_64-linux/asc/include/simt_api \
        /usr/local/Ascend/cann-*/aarch64-linux/asc/include/simt_api; do
        if [ -f "$found_cand/asc_simt.h" ]; then found="$found_cand"; break; fi
    done
    # 候选 B：在 ASCEND_HOME 下找 simt_api 目录或 asc_simt.h（内部工具链布局可能与公开版不同）
    [ -z "$found" ] && found="$(find "${ASCEND_HOME:-$ATH}" -maxdepth 10 -type d -name "simt_api" 2>/dev/null | head -1)"
    [ -z "$found" ] && found="$(dirname "$(find "${ASCEND_HOME:-$ATH}" -maxdepth 10 -name "asc_simt.h" 2>/dev/null | head -1)" 2>/dev/null)"
    if [ -n "$found" ] && [ -f "$found/asc_simt.h" ]; then
        SIMT_API_INCLUDE="$found"; ok "simt_api 目录：$found"
    else
        warn "未找到 simt_api 头（asc_simt.h）。kernel 用到 __syncthreads/__shared__ 宏时需先装 asc-devkit"
        log "  请手动排查并把结果发回，例如："
        log "    find /usr/local/Ascend -maxdepth 10 \\( -type d -name simt_api -o -name asc_simt.h \\) 2>/dev/null"
    fi
fi
if [ -n "$SIMT_API_INCLUDE" ]; then
    # asc_simt.h 并非自包含：内部 #include "simt_api/common_functions.h"，
    # 编译时 -I 需同时加 simt_api 的父目录（真机 2026-08-19 实测）
    SIMT_API_ROOT="$(dirname "$SIMT_API_INCLUDE")"
    log "  simt_api 父目录（-I 需要）：$SIMT_API_ROOT"
    for h in asc_simt.h common_functions.h device_types.h device_functions.h \
             device_sync_functions.h device_warp_functions.h device_atomic_functions.h; do
        [ -f "$SIMT_API_INCLUDE/$h" ] && ok "simt_api/$h" || warn "simt_api/$h 缺失"
    done
fi

# ---------------- 5. 硬件 / 驱动 / 运行时设备探测 ----------------
log "== 5. 硬件 / 驱动 / 运行时设备探测 =="

# 5.1 昇腾相关环境变量（一次看清 PATH/设备 ID/工具链配置）
log "  --- 昇腾相关环境变量 ---"
env | grep -iE "ascend|cann|npu" | sort | tee -a "$PROBE_LOG" || log "  （无）"

# 5.2 设备节点（判断是否有直连 NPU）
DEV_NODES="$(ls /dev/davinci* 2>/dev/null | head -10)"
if [ -n "$DEV_NODES" ]; then
    ok "设备节点存在：$(echo "$DEV_NODES" | tr '\n' ' ')"
else
    warn "无 /dev/davinci* 设备节点（编译机 / 容器无设备映射；能否运行见 5.5）"
fi

# 5.3 查询工具（npu-smi / ascend-dmi）
NPU_SMI=""
command -v npu-smi >/dev/null 2>&1 && NPU_SMI="$(command -v npu-smi)"
[ -z "$NPU_SMI" ] && command -v ascend-dmi >/dev/null 2>&1 && NPU_SMI="$(command -v ascend-dmi)"
[ -z "$NPU_SMI" ] && [ -n "$ASCEND_HOME" ] && [ -x "$ASCEND_HOME/driver/tools/npu-smi" ] && NPU_SMI="$ASCEND_HOME/driver/tools/npu-smi"
if [ -n "$NPU_SMI" ]; then
    ok "查询工具=$NPU_SMI"
    log "  --- $NPU_SMI info（前 20 行：芯片型号 / 健康状态） ---"
    "$NPU_SMI" info 2>&1 | head -20 | tee -a "$PROBE_LOG"
else
    warn "未找到 npu-smi / ascend-dmi（无昇腾驱动工具；能否运行见 5.5）"
fi

# 5.4 版本信息文件（driver / cann / toolkit）
for vf in \
    "$ASCEND_HOME/driver/version.info" \
    "$ASCEND_HOME/cann-"*/version.cfg \
    "$ATH/version.cfg" \
    "$ATH/compiler/version.info"; do
    for f in $vf; do
        if [ -f "$f" ]; then
            log "  --- $f ---"
            cat "$f" | tee -a "$PROBE_LOG"
        fi
    done
done

# 5.5 运行时设备探测：编译运行最小 ACL 查询程序（只读：GetDeviceCount + GetSocName）
log "  --- 运行时设备探测（probe_device） ---"
PROBE_SRC="$ROOT/tools/real_test/probe_device.cpp"
PROBE_BIN="$OUT_DIR/probe_device"
DEVICE_QUERY="0"
if [ -f "$PROBE_SRC" ] && command -v g++ >/dev/null 2>&1 && [ -n "$ATH" ]; then
    if g++ "$PROBE_SRC" -o "$PROBE_BIN" -I "$ATH/include" -L "$ATH/lib64" \
           -lascendcl -lpthread -ldl >>"$PROBE_LOG" 2>&1; then
        if [ -n "${LD_LIBRARY_PATH:-}" ]; then
            LD_LIBRARY_PATH="$ATH/lib64:$ASCEND_HOME/driver/lib64:$LD_LIBRARY_PATH" \
                "$PROBE_BIN" >>"$PROBE_LOG" 2>&1
        else
            LD_LIBRARY_PATH="$ATH/lib64:$ASCEND_HOME/driver/lib64" \
                "$PROBE_BIN" >>"$PROBE_LOG" 2>&1
        fi
        if [ "$?" -eq 0 ]; then
            DEVICE_QUERY="1"
            ok "运行时设备查询成功（有可用 NPU，输出见 probe.log）"
        else
            warn "运行时设备查询失败（无设备或依赖缺失），输出见 probe.log"
            WARN=$((WARN+1))
        fi
    else
        warn "probe_device 编译失败（g++ 链接 libascendcl 出错），不影响其余探测"
    fi
else
    warn "跳过运行时探测（缺 probe_device.cpp / g++ / CANN）"
fi

# ---------------- 6. API 签名只读核对（对照 CURRENT_STATE.md） ----------------
log "== 6. API 签名只读核对（grep 头文件，对照 docs/04_对比与验证/CURRENT_STATE.md §3/§5） =="
ACL_RT_H="$ATH/include/acl/acl_rt.h"
ACL_BASE_H="$ATH/include/acl/acl_base.h"
ACL_CBLAS_H="$ATH/include/acl/ops/acl_cblas.h"
ACL_INC_DIR="$ATH/include/acl"

# 指定单个头文件 grep
sig() {
    local name="$1" file="$2" pat="$3"
    if [ ! -f "$file" ]; then miss "${name}（头不存在：${file}）"; MISS=$((MISS+1)); return; fi
    if grep -qE "$pat" "$file"; then ok "$name"; else warn "${name}（未匹配 ${pat}）"; WARN=$((WARN+1)); fi
}
# 目录内递归 grep（不限定具体头文件）
sig_dir() {
    local name="$1" dir="$2" pat="$3"
    if [ ! -d "$dir" ]; then miss "${name}（目录不存在：${dir}）"; MISS=$((MISS+1)); return; fi
    if grep -rqE "$pat" "$dir" 2>/dev/null; then ok "$name"; else warn "${name}（在 ${dir} 未匹配 ${pat}）"; WARN=$((WARN+1)); fi
}
# 预期不存在
sig_absent() {
    local name="$1" file="$2" pat="$3"
    if [ ! -f "$file" ]; then miss "${name}（头不存在：${file}）"; MISS=$((MISS+1)); return; fi
    if grep -qE "$pat" "$file"; then warn "${name}（存在，预期不存在，注意）"; WARN=$((WARN+1)); else ok "$name 不存在（符合预期）"; fi
}
# 打印函数声明上下文（用于确认签名）
show_sig() {
    local name="$1" dir="$2"
    local hit
    hit="$(grep -rn -A 2 "$name" "$dir" 2>/dev/null | grep -v '^\s*//' | head -8)"
    if [ -n "$hit" ]; then
        log "  --- ${name} 声明上下文（前 8 行，去注释） ---"
        echo "$hit" | tee -a "$PROBE_LOG"
    else
        warn "${name}：整个 ${dir} 目录都未匹配"
        WARN=$((WARN+1))
    fi
}

log "  [runtime 已核实基线复核]"
sig_dir aclInit              "$ACL_INC_DIR" "aclInit"
sig     aclrtLaunchSIMTKernelWithArgsArray "$ACL_RT_H" "aclrtLaunchSIMTKernelWithArgsArray"
sig     aclrtQueryEventStatus "$ACL_RT_H" "aclrtQueryEventStatus"
sig     aclrtStreamQuery      "$ACL_RT_H" "aclrtStreamQuery"
sig_dir aclrtGetSocName       "$ACL_INC_DIR" "aclrtGetSocName"
sig     aclrtResetDevice      "$ACL_RT_H" "aclrtResetDevice"
sig     aclrtCreateEventWithFlag "$ACL_RT_H" "aclrtCreateEventWithFlag"
sig     aclrtGetDeviceInfo    "$ACL_RT_H" "aclrtGetDeviceInfo"
sig_dir ACL_SUCCESS           "$ACL_INC_DIR" "ACL_SUCCESS"
sig_dir aclrtEventRecordedStatus "$ACL_INC_DIR" "aclrtEventRecordedStatus"
log "  [待真机确认项（CURRENT_STATE.md §5）]"
sig_absent aclrtGetDeviceProperties "$ACL_RT_H" "aclrtGetDeviceProperties"
sig_absent ACL_DT_DOUBLE            "$ACL_BASE_H" "ACL_DT_DOUBLE"

log "  [真机新发现核对：aclrtGetVersion 签名（已废弃，官方建议 aclSysGetVersion）；"
log "   aclblasCreate 实为 aclblasCreateHandleFor* 系列（无独立 aclblasCreate）]"
show_sig aclrtGetVersion "$ACL_INC_DIR"
show_sig aclSysGetVersion "$ACL_INC_DIR"
show_sig aclblasCreateHandleFor "$ACL_INC_DIR"

log "  [cublas→aclblas（待确认项 #4）]"
sig_dir aclblasGemmEx "$ACL_INC_DIR" "aclblasGemmEx"
sig_dir aclblasGemvEx "$ACL_INC_DIR" "aclblasGemvEx"
sig_dir aclblasAxpy   "$ACL_INC_DIR" "aclblasAxpy"
sig_dir aclblasCopy   "$ACL_INC_DIR" "aclblasCopy"
sig_dir aclblasDot    "$ACL_INC_DIR" "aclblasDot"
sig_dir aclblasScal   "$ACL_INC_DIR" "aclblasScal"
sig_dir aclblasSwap   "$ACL_INC_DIR" "aclblasSwap"
sig_dir aclblasNrm2   "$ACL_INC_DIR" "aclblasNrm2"

log "  [SIMT 适配宏表对应物（待确认项 #6）]"
if [ -n "$SIMT_API_INCLUDE" ]; then
    for fn in asc_syncthreads asc_threadfence asc_threadfence_block \
              asc_shfl asc_shfl_up asc_shfl_down asc_shfl_xor \
              asc_all asc_any asc_ballot \
              asc_atomic_add asc_atomic_sub asc_atomic_exch asc_atomic_max asc_atomic_min \
              asc_reduce_add; do
        sig_dir "$fn" "$SIMT_API_INCLUDE" "$fn"
    done
else
    warn "跳过 SIMT 签名核对（未找到 simt_api 目录）"
fi

# ---------------- 7. puresimt 目标架构核对 ----------------
log "== 7. puresimt 目标架构核对（ccec + dav-c310-vec） =="
CUCC_NPU_ARCH="${CUCC_NPU_ARCH:-dav-c310-vec}"
SIMT_ARCH="$CUCC_NPU_ARCH"
case "$SIMT_ARCH" in
    *-vec) ;;
    *) SIMT_ARCH="${SIMT_ARCH}-vec" ;;
esac
NPU_ARCH_MACRO=""
SIMT_GATE_ARCHS=""
ARCH_COMPILE_OK="0"
ARCH_FALLBACK=""
COMPILER_KIND="ccec"

arch_tmp="$(mktemp -d)"
cat > "$arch_tmp/puresimt_probe.cu" <<'EOF'
#include "cuda_runtime.h"

__global__ void puresimt_probe_kernel(int *out) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i == 0) {
        out[0] = i;
    }
    __syncthreads();
}
EOF

if [ -n "$SIMT_API_INCLUDE" ]; then
    SIMT_GATE_ARCHS="$(grep -rhoE '__NPU_ARCH__ *== *[0-9]+' "$SIMT_API_INCLUDE" 2>/dev/null | grep -oE '[0-9]+' | sort -u | tr '\n' ' ')"
    log "  simt_api 门控值：${SIMT_GATE_ARCHS:-（无）}"
fi

if [ -n "$CCEC_PATH" ] && [ -x "$CCEC_PATH" ]; then
    args=( -std=gnu++17 -xcce --enable-simt "--cce-aicore-arch=${SIMT_ARCH}"
           -D__CU2CCE_NO_LIBC__ -D CUDA_VERSION=11060
           -I "$ROOT/include/shadow" -I "$ROOT/include" -I "$ACL_INC_DIR" )
    if [ -n "$SIMT_API_INCLUDE" ]; then
        args+=( -I "$(dirname "$SIMT_API_INCLUDE")" -I "$SIMT_API_INCLUDE"
                -I "$(dirname "$(dirname "$SIMT_API_INCLUDE")")" )
    fi
    args+=( -include "$ROOT/include/bridge/kernel/simt_adaptor.h"
            -c "$arch_tmp/puresimt_probe.cu" -o "$arch_tmp/puresimt_probe.o" )
    if "$CCEC_PATH" "${args[@]}" >"$arch_tmp/ccec.log" 2>&1; then
        ARCH_COMPILE_OK="1"
        ARCH_FALLBACK="$SIMT_ARCH"
        NPU_ARCH_MACRO="$($CCEC_PATH -dM -E -xcce "--cce-aicore-arch=${SIMT_ARCH}" \
            "$arch_tmp/puresimt_probe.cu" 2>/dev/null | grep -oE '__NPU_ARCH__[[:space:]]+[0-9]+' | awk '{print $2}' | head -1)"
        log "  ccec puresimt probe：PASS"
        [ -n "$NPU_ARCH_MACRO" ] && log "  __NPU_ARCH__=${NPU_ARCH_MACRO}"
    else
        warn "ccec puresimt probe 失败："
        tail -20 "$arch_tmp/ccec.log" | tee -a "$PROBE_LOG"
    fi
else
    warn "未找到 CANN ccec；main/puresimt 需要 ccec"
fi
rm -rf "$arch_tmp"

# ---------------- 8. 汇总 + 写 probe.env ----------------
log ""
log "== 8. 汇总 =="
{
    echo "# 由 env_probe.sh 生成（$(date '+%F %T')），供 build_bridge.sh / server_cmd.txt 使用"
    echo "ASCEND_HOME=\"$ASCEND_HOME\""
    echo "ASCEND_TOOLKIT_HOME=\"$ATH\""
    echo "ASCEND_INCLUDE=\"$ATH/include\""
    echo "ASCEND_LIB=\"$ATH/lib64\""
    echo "CCEC_PATH=\"$CCEC_PATH\""
    echo "CANN_BASE=\"$CANN_BASE\""
    echo "COMPILER_KIND=\"$COMPILER_KIND\""
    echo "SIMT_API_INCLUDE=\"$SIMT_API_INCLUDE\""
    echo "SIMT_API_ROOT=\"${SIMT_API_ROOT:-}\""
    echo "ACL_RT_LIB=\"$ACL_RT_LIB\""
    echo "HAS_ACL_CBLAS=\"$HAS_ACL_CBLAS\""
    echo "NPU_SMI=\"$NPU_SMI\""
    echo "DEVICE_QUERY=\"$DEVICE_QUERY\""
    echo "OUT_DIR=\"$OUT_DIR\""
    echo "CUCC_PATH=\"${CUCC_PATH:-$OUT_DIR/install}\""
    echo "CUCC_NPU_ARCH=\"$SIMT_ARCH\""
    echo "NPU_ARCH_MACRO=\"$NPU_ARCH_MACRO\""
    echo "SIMT_GATE_ARCHS=\"$SIMT_GATE_ARCHS\""
    echo "ARCH_COMPILE_OK=\"$ARCH_COMPILE_OK\""
    echo "ARCH_FALLBACK=\"$ARCH_FALLBACK\""
} > "$PROBE_ENV"
ok "probe.env 已写入：$PROBE_ENV"
ok "probe.log 已写入：$PROBE_LOG"
log "硬缺失=$MISS 签名未命中=$WARN"
if [ "$MISS" -eq 0 ]; then
    log "✅ 环境齐备，可执行 build_bridge.sh"
    exit 0
else
    log "❌ 有 $MISS 项硬缺失，先解决后重跑"
    exit 1
fi
