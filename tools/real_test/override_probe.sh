#!/bin/bash
# Probe for the cmake_cce override-injection decision logic (used by the
# local contract test; never runs a real configure).
set -euo pipefail
CUCC_PATH="/Users/tmchao7/repos/09_expLearn/cu2cce-bridge"
args=()
have_override=0
for arg in "$@"; do
  case "$arg" in
    -DCMAKE_USER_MAKE_RULES_OVERRIDE*|-DCMAKE_USER_MAKE_RULES_OVERRIDE_CUDA*) have_override=1 ;;
  esac
done
CU2CCE_CMAKE_NO_OVERRIDE="${CU2CCE_CMAKE_NO_OVERRIDE:-0}"
if [ "$CU2CCE_CMAKE_NO_OVERRIDE" != "1" ] && [ "$have_override" -eq 0 ]; then
  if [ -f "$CUCC_PATH/cmake_modules/override/CUDAOverrides.cmake" ]; then
    args+=("-DCMAKE_USER_MAKE_RULES_OVERRIDE=$CUCC_PATH/cmake_modules/override/CUDAOverrides.cmake")
  fi
fi
printf 'injected_override_file=%s\n' "${#args[@]}"