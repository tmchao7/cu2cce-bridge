#!/bin/bash
# CMake's nvcc entrypoint. All behavior belongs to mock_nvcc.py.
set -euo pipefail
exec python3 "${CUCC_PATH:?CUCC_PATH is not set}/tools/mock/mock_nvcc.py" "$@"
