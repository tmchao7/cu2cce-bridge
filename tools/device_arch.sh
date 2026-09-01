#!/usr/bin/env bash
# device_arch.sh —— resolve the Ascend NPU architecture for this machine.
#
# cu-bridge parity: tools/multi_target.sh + tools/get_device_arch.sh
# (env CUCC_TARGETS first, then device query via mx-smi, then error tips).
# NPU adaptation: the architecture is an aicore target such as
# dav-c310-vec, consumed as --cce-aicore-arch by ccec.
#
# Resolution order:
#   1. CU2CCE_NPU_ARCH / CUCC_NPU_ARCH from the environment
#   2. npu-smi device query mapped through the soc->arch table
#   3. cached value from a previous run ($CU2CCE_HOME/device_arch.cache)
#   4. dav-c310-vec default + usage tip (non-fatal; caller decides)
#
# Usage: CU2CCE_NPU_ARCH="$(bash device_arch.sh)"
set -u

print_tip() {
    echo "device_arch: tip: set CU2CCE_NPU_ARCH explicitly (e.g. dav-c310-vec) " \
         "or install npu-smi for device detection; using default" >&2
}

# 1) environment wins (mirrors CUCC_TARGETS-first semantics)
if [ -n "${CU2CCE_NPU_ARCH:-}" ]; then
    if [[ "$CU2CCE_NPU_ARCH" != dav-* ]]; then
        echo "device_arch: NPU architecture must be an Ascend dav-* target" >&2
        exit 2
    fi
    echo "$CU2CCE_NPU_ARCH"
    exit 0
fi
if [ -n "${CUCC_NPU_ARCH:-}" ]; then
    if [[ "$CUCC_NPU_ARCH" != dav-* ]]; then
        echo "device_arch: NPU architecture must be an Ascend dav-* target" >&2
        exit 2
    fi
    echo "$CUCC_NPU_ARCH"
    exit 0
fi
if [ -n "${CUCC_TARGETS_FROM_DEVICE:-}" ]; then
    targets="${CUCC_TARGETS_FROM_DEVICE//,/ }"
    read -r -a target_list <<< "$targets"
    if [ "${#target_list[@]}" -ne 1 ] || [[ "${target_list[0]}" != dav-* ]]; then
        echo "device_arch: CUCC_TARGETS_FROM_DEVICE must contain exactly one Ascend dav-* architecture" >&2
        exit 2
    fi
    echo "${target_list[0]}"
    exit 0
fi
if [ -n "${CUCC_TARGETS:-}" ]; then
    targets="${CUCC_TARGETS//,/ }"
    read -r -a target_list <<< "$targets"
    if [ "${#target_list[@]}" -ne 1 ] || [[ "${target_list[0]}" != dav-* ]]; then
        echo "device_arch: CUCC_TARGETS must contain exactly one Ascend dav-* architecture" >&2
        exit 2
    fi
    echo "${target_list[0]}"
    exit 0
fi

# 2) device query: npu-smi prints product names like "Ascend950DT_9582".
#    Map the observed Ascend SoC families onto their aicore targets; extend
#    the table as new SoCs appear (never guess an unlisted one).
SOC_NAME=""
if command -v npu-smi >/dev/null 2>&1; then
    SOC_NAME="$(npu-smi info 2>/dev/null | grep -oE 'Ascend[0-9A-Za-z_]+' | head -n 1)"
fi
case "${SOC_NAME}" in
    Ascend950*) echo "dav-c310-vec"; exit 0 ;;
    Ascend910B*|Ascend910_9*) echo "dav-c100-vec"; exit 0 ;;
esac

# 3) cached value from a previous explicit setting/probe
CACHE="${CU2CCE_HOME:-${CUBRIDGE_HOME:-$HOME}}/cu2cce-bridge/device_arch.cache"
if [ -f "$CACHE" ]; then
    cached="$(cat "$CACHE" 2>/dev/null || true)"
    if [ -n "$cached" ]; then
        echo "$cached"
        exit 0
    fi
fi

# 4) default + tip (keep the pipeline alive; callers may treat stderr as a hint)
print_tip
echo "dav-c310-vec"
exit 0
