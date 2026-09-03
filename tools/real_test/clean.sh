#!/bin/bash
# clean.sh —— clean all generated real-test/build outputs; never touch source files.
#
# Usage:
#   bash tools/real_test/clean.sh
#
# This removes build/real_test, the temporary CMake virtual toolkit directories created by
# the regression suite, and then recreates an empty output directory.

set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD="$ROOT/build/real_test"

rm -rf "$BUILD"
mkdir -p "$BUILD"

# The default cmake_cce virtual toolkit is process-scoped, but stale directories can be
# left behind after an interrupted configure. Remove only our known prefix.
for toolkit in "${TMPDIR:-/tmp}"/cu2cce-cmake-virtual-toolkit-* "${TMPDIR:-/tmp}"/cu2cce-cmake-shim-*; do
    [ -e "$toolkit" ] || [ -L "$toolkit" ] || continue
    rm -rf "$toolkit"
done

# A caller-selected virtual toolkit under this repository's generated build tree is safe
# to remove together with build/real_test; never delete arbitrary user paths.
if [ -n "${CU2CCE_CMAKE_SHIM_DIR:-}" ] &&
   [[ "$CU2CCE_CMAKE_SHIM_DIR" == "$ROOT/build/"* ]]; then
    rm -rf "$CU2CCE_CMAKE_SHIM_DIR"
fi

echo "Cleaned all generated real-test outputs: $BUILD"
