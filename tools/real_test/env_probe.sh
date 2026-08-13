#!/bin/bash
# ============================================================
# env_probe.sh —— 真机环境探测（分工①：只负责"找包的位置"，不构建、不跑测试）
#
# 职责：
#   1. 定位 CANN：ASCEND_TOOLKIT_HOME / ASCEND_HOME / setenv.bash
#   2. 定位工具链：bisheng 编译器、ACL 头/库、acl_cblas、simt_api SIMT 头、npu-smi
#   3. 只读核对 docs/03_Comparison/CURRENT_STATE.md 的"待真机确认项"
#      （grep 头文件签名，不调用任何昇腾运行时 API）
#   4. 结果写 OUT_DIR/probe.env（供 build_bridge.sh / run_tests.sh source）
#      + OUT_DIR/logs/probe.log（完整明细）
#
# 用法：
#   tools/real_test/env_probe.sh
#   OUT_DIR=/tmp/probe tools/real_test/env_probe.sh   # 覆盖输出目录
#
# 退出码：0 = 环境齐备；1 = 缺 CANN/编译器/核心库（签名未命中只 WARN 不阻断）
# ============================================================
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

# ---------------- 2. 编译器 / 工具链 ----------------
log "== 2. 编译器 / 工具链 =="
BISHENG=""
for cand in \
    "$ATH/compiler/ccec_compiler/bin/bisheng" \
    "$ATH/compiler/ccec_compiler/bin/ccec" \
    $(find "${ATH:-/nonexistent}" -maxdepth 6 -type f -name "bisheng" 2>/dev/null | head -3); do
    if [ -x "$cand" ]; then BISHENG="$cand"; break; fi
done
if [ -n "$BISHENG" ]; then ok "bisheng=$BISHENG"; else miss "未找到 bisheng 编译器"; MISS=$((MISS+1)); fi
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
if [ -f "$ATH/lib64/libascendcl.so" ]; then
    ACL_RT_LIB="-lascendcl"; ok "lib64/libascendcl.so（链接参数 ${ACL_RT_LIB}）"
elif [ -f "$ATH/lib64/libacl_rt.so" ]; then
    ACL_RT_LIB="-lacl_rt"; ok "lib64/libacl_rt.so（链接参数 ${ACL_RT_LIB}）"
else
    miss "lib64/libascendcl.so 与 libacl_rt.so 都不存在"; MISS=$((MISS+1))
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
    found="$(find "$ATH" -maxdepth 6 -type d -name "simt_api" 2>/dev/null | head -1)"
    if [ -n "$found" ]; then
        SIMT_API_INCLUDE="$found"; ok "simt_api 目录：$found"
    else
        warn "未找到 simt_api 头（asc_simt.h）。kernel 用到 __syncthreads/__shared__ 宏时需先装 asc-devkit"
    fi
fi
if [ -n "$SIMT_API_INCLUDE" ]; then
    for h in asc_simt.h device_sync_functions.h device_warp_functions.h device_atomic_functions.h; do
        [ -f "$SIMT_API_INCLUDE/$h" ] && ok "simt_api/$h" || warn "simt_api/$h 缺失"
    done
fi

# ---------------- 5. 硬件 / 驱动（只读查询） ----------------
log "== 5. 硬件 / 驱动 =="
NPU_SMI=""
command -v npu-smi >/dev/null 2>&1 && NPU_SMI="$(command -v npu-smi)"
[ -z "$NPU_SMI" ] && [ -n "$ASCEND_HOME" ] && [ -x "$ASCEND_HOME/driver/tools/npu-smi" ] && NPU_SMI="$ASCEND_HOME/driver/tools/npu-smi"
if [ -n "$NPU_SMI" ]; then
    ok "npu-smi=$NPU_SMI"
    log "  --- npu-smi info（前 20 行：芯片型号 / 健康状态 / CANN 版本） ---"
    "$NPU_SMI" info 2>&1 | head -20 | tee -a "$PROBE_LOG"
else
    warn "未找到 npu-smi（无昇腾驱动？纯编译验证可忽略本项）"
fi
if [ -n "$ASCEND_HOME" ] && [ -f "$ASCEND_HOME/driver/version.info" ]; then
    log "  --- driver/version.info ---"
    cat "$ASCEND_HOME/driver/version.info" | tee -a "$PROBE_LOG"
fi

# ---------------- 6. API 签名只读核对（对照 CURRENT_STATE.md） ----------------
log "== 6. API 签名只读核对（grep 头文件，对照 docs/03_Comparison/CURRENT_STATE.md §3/§5） =="
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

log "  [runtime 已核实基线复核]"
sig_dir aclInit              "$ACL_INC_DIR" "aclInit"
sig     aclrtLaunchSIMTKernelWithArgsArray "$ACL_RT_H" "aclrtLaunchSIMTKernelWithArgsArray"
sig     aclrtQueryEventStatus "$ACL_RT_H" "aclrtQueryEventStatus"
sig     aclrtStreamQuery      "$ACL_RT_H" "aclrtStreamQuery"
sig     aclrtGetSocName       "$ACL_RT_H" "aclrtGetSocName"
sig     aclrtResetDevice      "$ACL_RT_H" "aclrtResetDevice"
sig     aclrtCreateEventWithFlag "$ACL_RT_H" "aclrtCreateEventWithFlag"
sig     aclrtGetDeviceInfo    "$ACL_RT_H" "aclrtGetDeviceInfo"
sig_dir ACL_SUCCESS           "$ACL_INC_DIR" "ACL_SUCCESS"
sig_dir aclrtEventRecordedStatus "$ACL_INC_DIR" "aclrtEventRecordedStatus"
log "  [待真机确认项（CURRENT_STATE.md §5）]"
sig_absent aclrtGetDeviceProperties "$ACL_RT_H" "aclrtGetDeviceProperties"
sig_absent aclrtGetVersion          "$ACL_RT_H" "aclrtGetVersion"
sig_absent ACL_DT_DOUBLE            "$ACL_BASE_H" "ACL_DT_DOUBLE"

log "  [cublas→aclblas（待确认项 #4）]"
sig_dir aclblasGemmEx "$ACL_INC_DIR" "aclblasGemmEx"
sig_dir aclblasGemvEx "$ACL_INC_DIR" "aclblasGemvEx"
sig_dir aclblasAxpy   "$ACL_INC_DIR" "aclblasAxpy"
sig_dir aclblasCopy   "$ACL_INC_DIR" "aclblasCopy"
sig_dir aclblasDot    "$ACL_INC_DIR" "aclblasDot"
sig_dir aclblasScal   "$ACL_INC_DIR" "aclblasScal"
sig_dir aclblasSwap   "$ACL_INC_DIR" "aclblasSwap"
sig_dir aclblasNrm2   "$ACL_INC_DIR" "aclblasNrm2"
sig_absent aclblasCreate "$ACL_CBLAS_H" "aclblasCreate"

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

# ---------------- 7. 汇总 + 写 probe.env ----------------
log ""
log "== 7. 汇总 =="
{
    echo "# 由 env_probe.sh 生成（$(date '+%F %T')），供 build_bridge.sh / run_tests.sh source"
    echo "ASCEND_HOME=\"$ASCEND_HOME\""
    echo "ASCEND_TOOLKIT_HOME=\"$ATH\""
    echo "ASCEND_INCLUDE=\"$ATH/include\""
    echo "ASCEND_LIB=\"$ATH/lib64\""
    echo "BISHENG_PATH=\"$BISHENG\""
    echo "SIMT_API_INCLUDE=\"$SIMT_API_INCLUDE\""
    echo "ACL_RT_LIB=\"$ACL_RT_LIB\""
    echo "HAS_ACL_CBLAS=\"$HAS_ACL_CBLAS\""
    echo "NPU_SMI=\"$NPU_SMI\""
    echo "OUT_DIR=\"$OUT_DIR\""
    echo "CUCC_PATH=\"${CUCC_PATH:-$OUT_DIR/install}\""
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
