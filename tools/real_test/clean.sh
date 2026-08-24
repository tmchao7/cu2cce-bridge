#!/bin/bash
# clean.sh —— clean all generated real-test/build outputs; never touch source files.
#
# Usage:
#   bash tools/real_test/clean.sh
#
# This intentionally removes the entire build/real_test directory, including
# stale outputs from earlier experimental/new-homogeneous runs, then recreates
# the empty directory for the next server_cmd/build_bridge run.

set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD="$ROOT/build/real_test"

rm -rf "$BUILD"
mkdir -p "$BUILD"
echo "Cleaned all generated real-test outputs: $BUILD"
