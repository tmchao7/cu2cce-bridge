#!/bin/bash
# clean.sh —— remove generated real-test binaries/logs without touching source.
#
# Usage:
#   bash tools/real_test/clean.sh          # logs, sample binaries, probes
#   bash tools/real_test/clean.sh --all    # also remove bridge install/build dirs

set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD="$ROOT/build/real_test"

case "${1:-}" in
    ""|--samples)
        rm -rf \
            "$BUILD/puresimt" \
            "$BUILD/logs" \
            "$BUILD/probe_device" \
            "$BUILD"/*.o \
            "$BUILD"/*.out \
            "$BUILD"/*.log
        echo "Cleaned real-test sample binaries/logs under $BUILD"
        ;;
    --all)
        rm -rf \
            "$BUILD/puresimt" \
            "$BUILD/logs" \
            "$BUILD/probe_device" \
            "$BUILD"/*.o \
            "$BUILD"/*.out \
            "$BUILD"/*.log \
            "$BUILD/install" \
            "$BUILD/cmake-build" \
            "$BUILD/probe.env"
        echo "Cleaned all generated real-test outputs under $BUILD"
        ;;
    -h|--help)
        sed -n '1,12p' "$0"
        ;;
    *)
        echo "Usage: bash tools/real_test/clean.sh [--samples|--all]" >&2
        exit 2
        ;;
esac
