#!/bin/bash
# ============================================================
# build_bridge.sh —— 真机构建（分工②：真实执行 cmake/make/install）
#
# 前置：tools/real_test/env_probe.sh（缺 probe.env 时自动先探测）
#
# 用法：
#   tools/real_test/build_bridge.sh                    # 装到 build/real_test/install
#   tools/real_test/build_bridge.sh --prefix /opt/cu2cce  # 自定义安装目录（即 CUCC_PATH）
#   OUT_DIR=<dir> tools/real_test/build_bridge.sh      # 覆盖工作目录
#
# 退出码：0 = 构建安装成功；非 0 = 失败（CMake 报错见终端输出）
# ============================================================

# 防误用：本脚本末尾会 exit，被 source 会直接退出交互 shell
if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    echo "❌ 请勿用 source 执行本脚本，请直接运行：bash ${BASH_SOURCE[0]}" >&2
    return 2 2>/dev/null || exit 2
fi

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT_DIR="${OUT_DIR:-$ROOT/build/real_test}"
PROBE_ENV="$OUT_DIR/probe.env"
PROBE="$ROOT/tools/real_test/env_probe.sh"

if [ ! -f "$PROBE_ENV" ]; then
    echo "[build_bridge] 未找到 ${PROBE_ENV}，先执行环境探测..."
    "$PROBE"
fi

# shellcheck source=/dev/null
source "$PROBE_ENV"

CUCC_PATH="${CUCC_PATH:-$OUT_DIR/install}"
while [ $# -gt 0 ]; do
    case "$1" in
        --prefix) CUCC_PATH="$2"; shift 2;;
        *) echo "未知参数：$1"; exit 2;;
    esac
done

echo "== build_bridge：ASCEND_TOOLKIT_HOME=$ASCEND_TOOLKIT_HOME"
echo "== 安装目录 CUCC_PATH=$CUCC_PATH"
[ -n "$ASCEND_TOOLKIT_HOME" ] && [ -d "$ASCEND_TOOLKIT_HOME" ] || {
    echo "❌ probe 未找到 CANN，先解决 env_probe.sh 的 MISS 项"; exit 1; }

# CANN 环境（setenv.bash 若存在）
if [ -f "$ASCEND_TOOLKIT_HOME/bin/setenv.bash" ]; then
    # shellcheck source=/dev/null
    source "$ASCEND_TOOLKIT_HOME/bin/setenv.bash"
else
    echo "⚠️  无 $ASCEND_TOOLKIT_HOME/bin/setenv.bash，依赖当前 shell 已有 PATH/LD_LIBRARY_PATH"
fi
export LD_LIBRARY_PATH="$ASCEND_LIB:$CUCC_PATH/lib:${LD_LIBRARY_PATH:-}"

# 预检：CMakeLists.txt 直接引用这两个 .so 文件名，缺则构建必失败
echo "== 链接库预检（CMakeLists.txt 第 48-49 行直接引用） =="
for lib in libascendcl.so libacl_cblas.so; do
    if [ -f "$ASCEND_LIB/$lib" ]; then
        echo "  [OK]   $lib"
    else
        echo "  [MISS] $ASCEND_LIB/$lib"
        echo "         → 按真机实际库名修改 CMakeLists.txt（如改用 -lacl_rt），或建软链后重跑"
    fi
done

BUILD_DIR="$OUT_DIR/cmake-build"
NPROC="$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"

echo "== cmake 配置（构建目录 ${BUILD_DIR}） =="
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$CUCC_PATH"

echo "== 编译（-j${NPROC}） =="
cmake --build "$BUILD_DIR" -j"$NPROC"

echo "== 安装 =="
cmake --install "$BUILD_DIR"

echo "== 产物校验（${CUCC_PATH}） =="
fail=0
for f in lib/libcce_bridge_runtime.so \
         include/cuda_runtime.h include/bridge/kernel/simt_adaptor.h \
         bin/cucc bin/cmake_cce bin/make_cce bin/ninja_cce tools/nvcc_entry.sh \
         tools/cu2cce_driver.py tools/validate_cucc_rules.py config/cu2cce_rules.json \
         lib/cmake/cu2cce-bridge/cu2cceTargets.cmake \
         cmake_modules/FindCUDA.cmake cmake_modules/FindCUDAToolkit.cmake \
         lib/cmake/cu2cce-bridge/cu2cce-bridgeConfig.cmake; do
    if [ -e "$CUCC_PATH/$f" ]; then
        echo "  [OK]   $f"
    else
        echo "  [MISS] $f"
        fail=1
    fi
done
if [ "$fail" -eq 0 ]; then
    echo "✅ 构建安装成功"
    echo "   后续使用：export CUCC_PATH=$CUCC_PATH; export CUDA_PATH=\$CUCC_PATH"
else
    echo "❌ 有产物缺失，见上"
fi
exit "$fail"
