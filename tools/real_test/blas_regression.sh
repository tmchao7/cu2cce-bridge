# blas_regression.sh —— experimental CUDA cuBLAS -> CANN ACL CBLAS regression
#
# This helper is invoked by server_cmd.txt only when CU2CCE_BLAS_TEST=1.
# Its result never changes the default puresimt acceptance path.

set +e
set +u
set -o pipefail
ulimit -c 0 2>/dev/null || true

cd /home/t50063365/workspace/repos/cu2cce-bridge
printf 'BLAS_HEAD=' && git log -1 --oneline

OUT="$PWD/build/real_test/blas"
mkdir -p "$OUT"
rm -f "$OUT"/*
FAIL=0
DEVICE_HEALTHY=0
DEVICE_ACCESS_DENIED=0

printf '\n===== [1/5] device + CANN gate =====\n'
for node in /dev/davinci0 /dev/davinci_manager /dev/hisi_hdc; do
    if [ ! -e "$node" ] || ! test -r "$node" || ! test -w "$node"; then
        echo "[BLOCKED] DEVICE_ACCESS_DENIED_OR_MISSING: $node"
        DEVICE_ACCESS_DENIED=1
    else
        echo "[PASS] device access: $node"
    fi
done

CANN_BASE="/usr/local/Ascend/latest"
[ -f "$CANN_BASE/set_env.sh" ] && source "$CANN_BASE/set_env.sh"
export ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-$CANN_BASE}"
export ASCEND_OPP_PATH="${ASCEND_OPP_PATH:-/usr/local/Ascend/cann-9.2.0/opp}"
PROBE="$PWD/build/real_test/probe_device"
if [ ! -x "$PROBE" ]; then
    g++ tools/real_test/probe_device.cpp -o "$PROBE" \
        -I"$CANN_BASE/include" -L"$CANN_BASE/lib64" \
        -Wl,-rpath,"$CANN_BASE/lib64" -lascendcl -lpthread -ldl \
        >"$OUT/probe_compile.log" 2>&1
fi
if [ -x "$PROBE" ] && "$PROBE" >"$OUT/probe.log" 2>&1; then
    echo '[PASS] device probe'
    DEVICE_PROBE_FAIL=0
else
    echo '[FAIL] device probe'
    cat "$OUT/probe.log" 2>/dev/null
    DEVICE_PROBE_FAIL=1
fi
if [ "$DEVICE_ACCESS_DENIED" -eq 0 ] && [ "$DEVICE_PROBE_FAIL" -eq 0 ]; then
    DEVICE_HEALTHY=1
    echo '[PASS] DEVICE_HEALTHY=1'
else
    echo '[BLOCKED] DEVICE_HEALTHY=0'
fi

printf '\n===== [2/5] bridge runtime rebuild =====\n'
if bash tools/real_test/build_bridge.sh; then
    echo '[PASS] bridge runtime rebuild'
else
    echo '[FAIL] bridge runtime rebuild'
    FAIL=$((FAIL + 1))
fi

REPO_INC="$PWD/include"
REPO_LIB="$PWD/build/real_test/install/lib"
export LD_LIBRARY_PATH="$REPO_LIB:$CANN_BASE/lib64:${LD_LIBRARY_PATH:-}"

printf '\n===== [3/5] official CANN GEMM model =====\n'
SOC_NAME="$(grep -hF 'probe_device: soc name = ' "$OUT/probe.log" 2>/dev/null | sed 's/.*soc name = //' | head -1)"
ATC="$CANN_BASE/bin/atc"
MODEL_DIR="$OUT/gemm_models"
MODEL_JSON="$OUT/gemm_fp16_fp32.json"
mkdir -p "$MODEL_DIR"
cat >"$MODEL_JSON" <<'JSON'
[
  {
    "op": "GEMM",
    "input_desc": [
      {"format": "ND", "shape": [32, 32], "type": "float16"},
      {"format": "ND", "shape": [32, 32], "type": "float16"},
      {"format": "ND", "shape": [32, 32], "type": "float"},
      {"format": "ND", "shape": [], "type": "float"},
      {"format": "ND", "shape": [], "type": "float"}
    ],
    "output_desc": [{"format": "ND", "shape": [32, 32], "type": "float"}],
    "attr": [
      {"name": "transpose_a", "type": "bool", "value": false},
      {"name": "transpose_b", "type": "bool", "value": false}
    ]
  }
]
JSON
if [ "$DEVICE_HEALTHY" -eq 1 ] && [ -x "$ATC" ] && [ -n "$SOC_NAME" ] && \
   "$ATC" --singleop="$MODEL_JSON" --soc_version="$SOC_NAME" \
       --output="$MODEL_DIR/gemm_fp16_fp32" >"$OUT/atc.log" 2>&1; then
    echo "[PASS] ATC model generated for $SOC_NAME"
else
    echo '[FAIL] ATC model generation'
    cat "$OUT/atc.log" 2>/dev/null
    FAIL=$((FAIL + 1))
fi

printf '\n===== [4/5] cublasSgemm -> aclblasGemmEx =====\n'
CCEC="$CANN_BASE/bin/ccec"
BIN="$OUT/cublas_sgemm"
COMPILE_LOG="$OUT/cublas_sgemm_compile.log"
COMPILE_CMD=("$CCEC" -x asc --enable-simt --cce-aicore-arch=dav-c310-vec -std=c++17 \
    examples/blas/cublas_sgemm.cu \
    -I"$REPO_INC" -I"$CANN_BASE/include" \
    -L"$REPO_LIB" -L"$CANN_BASE/lib64" \
    -Wl,-rpath,"$REPO_LIB" -Wl,-rpath,"$CANN_BASE/lib64" \
    -lcce_bridge_runtime -lacl_cblas -lascendcl -lstdc++ -lpthread -lm \
    -o "$BIN")
echo "[INFO] ASCEND_HOME_PATH=$ASCEND_HOME_PATH"
echo "[INFO] CANN_BASE=$CANN_BASE"
echo "[INFO] CCEC=$CCEC"
echo "[INFO] CCEC_VERSION:"
"$CCEC" --version 2>&1 | head -n 3
echo "[INFO] COMPILE_CMD:"
printf '  %q ' "${COMPILE_CMD[@]}"
printf '\n'
if [ "$DEVICE_HEALTHY" -eq 1 ]; then
    "${COMPILE_CMD[@]}" >"$COMPILE_LOG" 2>&1
    COMPILE_RC=$?
else
    COMPILE_RC=125
    echo '[SKIP] device gate failed; compile not attempted' >"$COMPILE_LOG"
fi
if [ "$COMPILE_RC" -eq 0 ]; then
    echo '[PASS] cublas_sgemm compile/link'
    ldd "$BIN" >"$OUT/cublas_sgemm.ldd.log" 2>&1 || true
    echo '[INFO] linked libraries:'
    grep -E 'cce_bridge|acl_cblas|ascendcl|runtime' "$OUT/cublas_sgemm.ldd.log" 2>/dev/null || cat "$OUT/cublas_sgemm.ldd.log"
    echo '[INFO] compile log:'
    cat "$COMPILE_LOG" 2>/dev/null
    if CU2CCE_GEMM_MODEL_DIR="$MODEL_DIR" "$BIN" >"$OUT/cublas_sgemm.run.log" 2>&1 && \
       grep -Fq 'cublas_sgemm_experimental OK' "$OUT/cublas_sgemm.run.log"; then
        echo "[PASS] cublas_sgemm：$(grep -F 'cublas_sgemm_experimental OK' "$OUT/cublas_sgemm.run.log")"
    else
        echo '[FAIL] cublas_sgemm runtime'
        cat "$OUT/cublas_sgemm.run.log" 2>/dev/null
        FAIL=$((FAIL + 1))
    fi
else
    echo "[FAIL] cublas_sgemm compile/link rc=$COMPILE_RC"
    echo "[INFO] failed compile command:"
    printf '  %q ' "${COMPILE_CMD[@]}"
    printf '\n[INFO] compiler output ($COMPILE_LOG):\n'
    cat "$COMPILE_LOG" 2>/dev/null
    FAIL=$((FAIL + 1))
fi

printf '\n===== [5/5] summary =====\n'
echo "FAIL=$FAIL DEVICE_HEALTHY=$DEVICE_HEALTHY"
echo "Logs: $OUT"
if [ "$FAIL" -eq 0 ] && [ "$DEVICE_HEALTHY" -eq 1 ]; then
    echo 'BLAS_EXPERIMENTAL_REGRESSION_PASS'
else
    echo 'BLAS_EXPERIMENTAL_REGRESSION_FAIL'
fi
