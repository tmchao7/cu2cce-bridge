#!/bin/bash
# ============================================================
# run_all.sh —— 真机一键验证：探测环境 → 构建 bridge → 运行测试
#
# 用法：
#   tools/real_test/run_all.sh             # 全程走 cucc 驱动
#   tools/real_test/run_all.sh --direct    # 测试阶段改用直接 bisheng 对照路径
#
# 各步可单独执行（分工见同目录 README.md），本脚本只做串联。
# 退出码：任一步失败即停（set -e）。
# ============================================================

# 防误用：本脚本末尾会 exit，被 source 会直接退出交互 shell
if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    echo "❌ 请勿用 source 执行本脚本，请直接运行：bash ${BASH_SOURCE[0]}" >&2
    return 2 2>/dev/null || exit 2
fi

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RT="$ROOT/tools/real_test"

echo "########## [1/3] 环境探测（找包位置 + API 签名只读核对） ##########"
"$RT/env_probe.sh"

echo "########## [2/3] 构建 bridge 库（真实 cmake/make/install） ##########"
"$RT/build_bridge.sh"

echo "########## [3/3] 运行测试（真实编译 + 启动运行） ##########"
"$RT/run_tests.sh" all "$@"

echo "########## 全部完成 ✅（日志见 $ROOT/build/real_test/logs/） ##########"
