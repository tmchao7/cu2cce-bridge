#!/bin/bash
# ============================================================
# env_probe.sh —— 真机环境探测（分工①：只负责"找包的位置"，不构建、不跑测试）
#
# 职责：
#   1. 定位 CANN：ASCEND_TOOLKIT_HOME / ASCEND_HOME / setenv.bash
#   2. 定位工具链：bisheng 编译器、ACL 头/库、acl_cblas、simt_api SIMT 头、npu-smi
#   3. 只读核对 docs/03_Comparison/CURRENT_STATE.md 的"待真机确认项"
#      （grep 头文件签名，不调用任何昇腾运行时 API）
#   4. 核对目标架构 --npu-arch=dav-c310（内部 950 系列，导师 2026-08-19 提供）：
#      dump __NPU_ARCH__ 宏值、比对 simt_api 门控、最小 SIMT kernel 实测编译
#      （产物写 /tmp，不触碰仓库构建目录）
#   5. 结果写 OUT_DIR/probe.env（供 build_bridge.sh / run_tests.sh source）
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
BISHENG=""
# 候选 1：常见路径（不同 CANN 版本布局不同；真机 9.2.0 实测在 cann-*/tools/bisheng_compiler）
for cand in \
    "$ATH/compiler/ccec_compiler/bin/bisheng" \
    "$ATH/compiler/ccec_compiler/bin/ccec" \
    "$ATH/compiler/bisheng/bin/bisheng" \
    "$ATH/compiler/tikcpp/bin/bisheng" \
    "$ASCEND_HOME"/cann-*/tools/bisheng_compiler/bin/bisheng \
    /usr/local/Ascend/cann-*/tools/bisheng_compiler/bin/bisheng; do
    if [ -x "$cand" ]; then BISHENG="$cand"; break; fi
done
# 候选 2：PATH 里的 bisheng
if [ -z "$BISHENG" ] && command -v bisheng >/dev/null 2>&1; then
    BISHENG="$(command -v bisheng)"
fi
# 候选 3：全盘搜索（ASCEND_HOME 下 maxdepth 8）
if [ -z "$BISHENG" ]; then
    log "  常规路径未命中，在 ${ASCEND_HOME:-/usr/local/Ascend} 下搜索 bisheng/ccec（maxdepth 8）..."
    found="$(find "${ASCEND_HOME:-/usr/local/Ascend}" -maxdepth 8 -type f -name "bisheng" 2>/dev/null | head -5)"
    for cand in $found; do
        if [ -x "$cand" ]; then BISHENG="$cand"; break; fi
    done
fi
if [ -n "$BISHENG" ]; then
    ok "bisheng=$BISHENG"
    # bisheng 内置架构线索：conf/lib 文本资源里的 dav-* 架构名（只读）
    BISHENG_ROOT="$(dirname "$(dirname "$BISHENG")")"
    log "  --- ${BISHENG_ROOT} 中的 dav-* 架构线索（conf/lib 文本文件） ---"
    for dir in "$BISHENG_ROOT"/conf "$BISHENG_ROOT"/lib; do
        [ -d "$dir" ] || continue
        hits="$(grep -rhoE 'dav-[a-z0-9]+' "$dir" \
                --include="*.h" --include="*.td" --include="*.def" --include="*.json" \
                --include="*.txt" --include="*.cfg" 2>/dev/null | sort -u | head -15)"
        if [ -n "$hits" ]; then
            log "  $dir:"
            echo "$hits" | tee -a "$PROBE_LOG"
        fi
    done
else
    miss "未找到 bisheng 编译器"
    log "  请手动排查并把结果发回，例如："
    log "    find /usr/local/Ascend -maxdepth 8 -type f \\( -name bisheng -o -name ccec \\) 2>/dev/null"
    log "    ls \$ASCEND_TOOLKIT_HOME/compiler/ 2>/dev/null"
    MISS=$((MISS+1))
fi
# ccec：CANN 自带编译器（用户实测脚本用 /usr/local/Ascend/latest/bin/ccec，
# 架构参数 --cce-aicore-arch=dav-c310，与 bisheng 的 --npu-arch 体系不同）
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
        if LD_LIBRARY_PATH="$ATH/lib64:$ASCEND_HOME/driver/lib64:${LD_LIBRARY_PATH:-}" \
           "$PROBE_BIN" >>"$PROBE_LOG" 2>&1; then
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

# ---------------- 7. 目标架构核对（dav-c310，内部 950 系列） ----------------
log "== 7. 目标架构核对（dav-c310，内部 950 系列） =="
CUCC_NPU_ARCH="${CUCC_NPU_ARCH:-dav-c310}"
NPU_ARCH_MACRO=""
SIMT_GATE_ARCHS=""
ARCH_COMPILE_OK="0"
ARCH_FALLBACK=""
COMPILER_KIND=""

arch_tmp="$(mktemp -d)"
printf '__NPU_ARCH__\n' > "$arch_tmp/macro_probe.cce"
cat > "$arch_tmp/arch_probe.asc" <<'EOF'
#include "asc_simt.h"
__global__ void arch_probe_kernel(float *gmOut, const float *gmIn, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    float v = gmIn[i];
    v = asc_shfl(v, 0);
    asc_syncthreads();
    gmOut[i] = v;
}
EOF

# 7.1 simt_api 头里 warp/atomic 的条件编译门控值
if [ -n "$SIMT_API_INCLUDE" ]; then
    SIMT_GATE_ARCHS="$(grep -rhoE '__NPU_ARCH__ *== *[0-9]+' "$SIMT_API_INCLUDE" 2>/dev/null | grep -oE '[0-9]+' | sort -u | tr '\n' ' ')"
    log "  simt_api 头中出现的 __NPU_ARCH__ 门控值：${SIMT_GATE_ARCHS:-（无）}"
else
    warn "无 simt_api 目录，跳过门控比对"
fi

# 7.2 编译实测函数（含 include 自动修复：缺头时 ASCEND_HOME 下 find 并补 -I，最多 3 轮）
# 用法：try_compile <名称> <编译器> <架构参数(含=)> <日志文件>；成功=0 失败=1
try_compile() {
    local name="$1" cc="$2" flag="$3" lg="$4"
    local args=( -std=c++17 -xcce "${flag}" )
    if [ -n "$SIMT_API_INCLUDE" ]; then
        # asc_simt.h 内部 include "simt_api/xxx.h"，必须加父目录
        args+=( -I "$(dirname "$SIMT_API_INCLUDE")" -I "$SIMT_API_INCLUDE" )
    fi
    args+=( -c "$arch_tmp/arch_probe.asc" -o "$arch_tmp/arch_probe.o" )
    rm -f "$arch_tmp/arch_probe.o"
    local try=0 miss_hdr miss_file hdr_dir
    while [ "$try" -lt 3 ]; do
        if "$cc" "${args[@]}" >"$lg" 2>&1 && [ -f "$arch_tmp/arch_probe.o" ]; then
            return 0
        fi
        miss_hdr="$(grep -oE "fatal error: '[^']+'" "$lg" | head -1 | sed "s/^fatal error: '//;s/'$//")"
        [ -z "$miss_hdr" ] && return 1
        miss_file="$(find "${ASCEND_HOME:-/usr/local/Ascend}" -maxdepth 12 -type f -path "*/${miss_hdr}" 2>/dev/null | head -1)"
        if [ -z "$miss_file" ]; then
            log "    （${name}：缺失头 ${miss_hdr} 在 ASCEND_HOME 下未找到，停止重试）"
            return 1
        fi
        # 关键：补的是能解析 include 相对路径的根目录。
        # 例：include "impl/utils/sys_macros.h"，文件在 .../asc/impl/utils/sys_macros.h
        #     → 需要的 -I 是 .../asc（从文件路径去掉 miss_hdr 相对路径），
        #       而不是 dirname（.../asc/impl/utils，那样永远解析不到）——真机实测教训
        hdr_dir="${miss_file%/${miss_hdr}}"
        [ -z "$hdr_dir" ] && hdr_dir="$(dirname "$miss_file")"
        args+=( -I "$hdr_dir" )
        try=$((try+1))
        log "    （${name}：自动补 include：${hdr_dir}）"
    done
    return 1
}

# 门控比对（编译成功后才调用）
check_gate() {
    local macro="$1"
    if echo "$macro" | grep -qE '^[0-9]+$' && [ -n "$SIMT_API_INCLUDE" ]; then
        if echo " ${SIMT_GATE_ARCHS} " | grep -q " ${macro} "; then
            ok "门控覆盖 ${macro}：warp/atomic 原语可用"
        else
            warn "门控不含 ${macro}！asc_shfl/asc_atomic_* 可能被条件编译裁掉"
            WARN=$((WARN+1))
        fi
    fi
}

# 7.3 编译器 1：ccec（CANN 自带，--cce-aicore-arch=dav-c310，导师指定架构实测可用）
if [ -n "$CCEC_PATH" ]; then
    log "  --- ccec --version ---"
    "$CCEC_PATH" --version 2>&1 | head -3 | tee -a "$PROBE_LOG"
    log "  --- 编译器 1/2：ccec + --cce-aicore-arch=${CUCC_NPU_ARCH} ---"
    if try_compile "ccec" "$CCEC_PATH" "--cce-aicore-arch=${CUCC_NPU_ARCH}" "$arch_tmp/ccec.log"; then
        ok "✅ ccec：最小 SIMT kernel（asc_shfl + asc_syncthreads）编译通过（--cce-aicore-arch=${CUCC_NPU_ARCH}）"
        COMPILER_KIND="ccec"
        ARCH_FALLBACK="${CUCC_NPU_ARCH}"
        ARCH_COMPILE_OK="1"
        # 宏值：-dM -E 精确提取（ccec -E 会展开大段内置头，不能整段 grep；真机实测
        # ccec+dav-c310 → __NPU_ARCH__=3510，与 simt_api 门控 3510 匹配）
        NPU_ARCH_MACRO="$("$CCEC_PATH" -dM -E -xcce "--cce-aicore-arch=${CUCC_NPU_ARCH}" -x cce - 2>/dev/null | grep -oE '__NPU_ARCH__[[:space:]]+[0-9]+' | awk '{print $2}' | head -1)"
        [ -z "$NPU_ARCH_MACRO" ] && NPU_ARCH_MACRO="$("$CCEC_PATH" -E -xcce "--cce-aicore-arch=${CUCC_NPU_ARCH}" "$arch_tmp/macro_probe.cce" 2>/dev/null | grep -oE '[0-9]+' | tail -1)"
        [ -n "$NPU_ARCH_MACRO" ] && log "    __NPU_ARCH__=${NPU_ARCH_MACRO}"
        check_gate "$NPU_ARCH_MACRO"
    else
        warn "ccec + ${CUCC_NPU_ARCH} 编译失败，末 10 行错误如下："
        tail -10 "$arch_tmp/ccec.log" | tee -a "$PROBE_LOG"
    fi
else
    warn "ccec 未找到（CANN 自带编译器，--cce-aicore-arch=dav-c310）"
fi

# 7.4 编译器 2：bisheng（独立编译器，--npu-arch 候选：lib 发现 + 门控值）——仅当 ccec 未通过
if [ "$ARCH_COMPILE_OK" = "0" ] && [ -n "$BISHENG" ]; then
    log "  --- bisheng --version ---"
    "$BISHENG" --version 2>&1 | head -3 | tee -a "$PROBE_LOG"
    log "  --- bisheng --help 中架构相关行 ---"
    "$BISHENG" --help 2>&1 | grep -iE "npu-arch|cce-soc-version|offload-arch" | head -10 | tee -a "$PROBE_LOG"
    BISHENG_ARCHS=""
    if [ -n "${BISHENG_ROOT:-}" ]; then
        BISHENG_ARCHS="$(grep -rhoE 'dav-[a-z0-9]+' "$BISHENG_ROOT"/lib \
                --include="*.h" --include="*.td" --include="*.def" --include="*.json" \
                --include="*.txt" --include="*.cfg" 2>/dev/null | sort -u | tr '\n' ' ')"
        [ -n "$BISHENG_ARCHS" ] && log "  bisheng lib 中的真实架构：${BISHENG_ARCHS}"
    fi
    ARCH_CANDIDATES="${BISHENG_ARCHS}"
    for g in ${SIMT_GATE_ARCHS}; do ARCH_CANDIDATES="${ARCH_CANDIDATES} dav-${g}"; done
    log "  bisheng 候选架构顺序：${ARCH_CANDIDATES}"
    for ARCH in ${ARCH_CANDIDATES}; do
        log "  --- bisheng + --npu-arch=${ARCH} ---"
        MACRO="$("$BISHENG" -E -xcce --npu-arch="${ARCH}" "$arch_tmp/macro_probe.cce" 2>/dev/null | grep -vE '^#' | tr -d ' \n')"
        [ -n "$MACRO" ] && log "    __NPU_ARCH__=${MACRO}"
        if try_compile "bisheng/${ARCH}" "$BISHENG" "--npu-arch=${ARCH}" "$arch_tmp/bisheng_${ARCH#dav-}.log"; then
            ok "✅ bisheng：最小 SIMT kernel（asc_shfl + asc_syncthreads）编译通过（--npu-arch=${ARCH}）"
            COMPILER_KIND="bisheng"
            ARCH_FALLBACK="$ARCH"
            ARCH_COMPILE_OK="1"
            NPU_ARCH_MACRO="$MACRO"
            check_gate "$NPU_ARCH_MACRO"
            break
        else
            warn "bisheng + ${ARCH} 编译失败，末 5 行错误如下："
            tail -5 "$arch_tmp/bisheng_${ARCH#dav-}.log" | tee -a "$PROBE_LOG"
        fi
    done
fi
if [ "$ARCH_COMPILE_OK" = "0" ]; then
    warn "ccec 与 bisheng 均未通过编译，需与导师确认 950 系列的编译命令"
fi
rm -rf "$arch_tmp"

# ---------------- 8. 汇总 + 写 probe.env ----------------
log ""
log "== 8. 汇总 =="
{
    echo "# 由 env_probe.sh 生成（$(date '+%F %T')），供 build_bridge.sh / run_tests.sh source"
    echo "ASCEND_HOME=\"$ASCEND_HOME\""
    echo "ASCEND_TOOLKIT_HOME=\"$ATH\""
    echo "ASCEND_INCLUDE=\"$ATH/include\""
    echo "ASCEND_LIB=\"$ATH/lib64\""
    echo "BISHENG_PATH=\"$BISHENG\""
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
    echo "CUCC_NPU_ARCH=\"$CUCC_NPU_ARCH\""
    echo "NPU_ARCH_MACRO=\"$NPU_ARCH_MACRO\""
    echo "SIMT_GATE_ARCHS=\"$SIMT_GATE_ARCHS\""
    echo "ARCH_COMPILE_OK=\"$ARCH_COMPILE_OK\""
    echo "ARCH_FALLBACK=\"$ARCH_FALLBACK\""
    echo "BISHENG_ARCHS=\"${BISHENG_ARCHS:-}\""
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
