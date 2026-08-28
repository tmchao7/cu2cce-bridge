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

printf '\n===== [1/8] device + CANN gate =====\n'
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

printf '\n===== [2/8] bridge runtime rebuild =====\n'
if bash tools/real_test/build_bridge.sh; then
    echo '[PASS] bridge runtime rebuild'
else
    echo '[FAIL] bridge runtime rebuild'
    FAIL=$((FAIL + 1))
fi

REPO_INC="$PWD/include"
REPO_LIB="$PWD/build/real_test/install/lib"
export LD_LIBRARY_PATH="$REPO_LIB:$CANN_BASE/lib64:${LD_LIBRARY_PATH:-}"

printf '\n===== [3/8] official CANN GEMM model =====\n'
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

# GEMV-shaped single-op model (GEMM with n=1) for the aclblasGemvEx path.
# Non-blocking: gemv is experimental and the [7/8] probe records the outcome.
cat >"$OUT/gemv_fp16_fp32.json" <<'JSON'
[
  {
    "op": "GEMM",
    "input_desc": [
      {"format": "ND", "shape": [8, 8], "type": "float"},
      {"format": "ND", "shape": [8, 1], "type": "float"},
      {"format": "ND", "shape": [8, 1], "type": "float"},
      {"format": "ND", "shape": [], "type": "float"},
      {"format": "ND", "shape": [], "type": "float"}
    ],
    "output_desc": [{"format": "ND", "shape": [8, 1], "type": "float"}],
    "attr": [
      {"name": "transpose_a", "type": "bool", "value": false},
      {"name": "transpose_b", "type": "bool", "value": false}
    ]
  }
]
JSON
if [ "$DEVICE_HEALTHY" -eq 1 ] && [ -x "$ATC" ] && [ -n "$SOC_NAME" ] && \
   "$ATC" --singleop="$OUT/gemv_fp16_fp32.json" --soc_version="$SOC_NAME" \
       --output="$MODEL_DIR/gemv_fp32" >"$OUT/atc_gemv.log" 2>&1; then
    echo '[PASS] ATC GEMV-shaped model generated'
else
    echo '[PROBE-ONLY] ATC GEMV model generation failed (gemv stays experimental)'
    tail -n 5 "$OUT/atc_gemv.log" 2>/dev/null || true
fi

printf '\n===== [4/8] cublasSgemm -> aclblasGemmEx =====\n'
CCEC="$CANN_BASE/bin/ccec"

# 通用 BLAS 用例：编译 + 运行 + 标记校验（tc_name tc_src tc_ok_marker）。
# sgemm/gemmex 共享同一份 ATC GEMM 模型目录。
run_blas_case() {
    local tc_name="$1" tc_src="$2" tc_ok="$3"
    local bin="$OUT/$tc_name"
    local clog="$OUT/${tc_name}_compile.log"
    local cmd=("$CCEC" -x asc --enable-simt --cce-aicore-arch=dav-c310-vec -std=c++17 \
        "$tc_src" \
        -I"$REPO_INC" -I"$CANN_BASE/include" \
        -L"$REPO_LIB" -L"$CANN_BASE/lib64" \
        -Wl,-rpath,"$REPO_LIB" -Wl,-rpath,"$CANN_BASE/lib64" \
        -lcce_bridge_runtime -lacl_cblas -lascendcl -lstdc++ -lpthread -lm \
        -o "$bin")
    echo "[INFO] [$tc_name] COMPILE_CMD:"
    printf '  %q ' "${cmd[@]}"
    printf '\n'
    local rc=125
    if [ "$DEVICE_HEALTHY" -eq 1 ]; then
        "${cmd[@]}" >"$clog" 2>&1
        rc=$?
    else
        echo '[SKIP] device gate failed; compile not attempted' >"$clog"
    fi
    if [ "$rc" -ne 0 ]; then
        echo "[FAIL] [$tc_name] compile/link rc=$rc"
        cat "$clog" 2>/dev/null
        FAIL=$((FAIL + 1))
        return 0
    fi
    echo "[PASS] [$tc_name] compile/link"
    if CU2CCE_GEMM_MODEL_DIR="$MODEL_DIR" "$bin" >"$OUT/${tc_name}.run.log" 2>&1 && \
       grep -Fq "$tc_ok" "$OUT/${tc_name}.run.log"; then
        echo "[PASS] [$tc_name]：$(grep -F "$tc_ok" "$OUT/${tc_name}.run.log")"
    else
        echo "[FAIL] [$tc_name] runtime"
        cat "$OUT/${tc_name}.run.log" 2>/dev/null
        FAIL=$((FAIL + 1))
    fi
}

echo "[INFO] ASCEND_HOME_PATH=$ASCEND_HOME_PATH"
echo "[INFO] CANN_BASE=$CANN_BASE"
echo "[INFO] CCEC=$CCEC"
echo "[INFO] CCEC_VERSION:"
"$CCEC" --version 2>&1 | head -n 3
run_blas_case cublas_sgemm examples/blas/cublas_sgemm.cu 'cublas_sgemm_experimental OK'

printf '\n===== [5/8] cublasGemmEx dtype-explicit (FP16xFP16->FP32) =====\n'
run_blas_case cublas_gemmex examples/blas/cublas_gemmex.cu 'cublas_gemmex_experimental OK'

printf '\n===== [6/8] cublasSgemv -> aclblasGemvEx =====\n'
run_blas_case cublas_gemv examples/blas/cublas_gemv.cu 'cublas_gemv_experimental OK'

printf '\n===== [7/8] aclblasGemvEx parameter-matrix probe (diagnostic-only) =====\n'
# GemvEx's contract differs from GemmEx's and the public doc site renders no
# content; this probe enumerates scalar-location x lda x transpose x compute
# in one run so the wrapper mapping can be fixed from evidence, not guesses.
# Its outcome never blocks the summary.
PROBE_BIN="$OUT/gemv_param_probe"
if [ "$DEVICE_HEALTHY" -eq 1 ] && \
   "$CCEC" -x asc --enable-simt --cce-aicore-arch=dav-c310-vec -std=c++17 \
       examples/blas/gemv_param_probe.cu \
       -I"$REPO_INC" -I"$CANN_BASE/include" \
       -L"$REPO_LIB" -L"$CANN_BASE/lib64" \
       -Wl,-rpath,"$REPO_LIB" -Wl,-rpath,"$CANN_BASE/lib64" \
       -lcce_bridge_runtime -lacl_cblas -lascendcl -lstdc++ -lpthread -lm \
       -o "$PROBE_BIN" >"$OUT/gemv_param_probe_compile.log" 2>&1; then
    if "$PROBE_BIN" >"$OUT/gemv_param_probe.run.log" 2>&1; then
        echo '[INFO] probe matrix:'
        cat "$OUT/gemv_param_probe.run.log"
    else
        echo '[INFO] probe exited nonzero; matrix:'
        cat "$OUT/gemv_param_probe.run.log" 2>/dev/null
        tail -n 10 "$OUT/gemv_param_probe_compile.log" 2>/dev/null
    fi
else
    echo '[PROBE-ONLY] gemv_param_probe compile failed (non-blocking)'
    tail -n 15 "$OUT/gemv_param_probe_compile.log" 2>/dev/null || true
fi

printf '\n===== [8/8] summary =====\n'
echo "FAIL=$FAIL DEVICE_HEALTHY=$DEVICE_HEALTHY"
echo "Logs: $OUT"
if [ "$FAIL" -eq 0 ] && [ "$DEVICE_HEALTHY" -eq 1 ]; then
    echo 'BLAS_EXPERIMENTAL_REGRESSION_PASS'
else
    echo 'BLAS_EXPERIMENTAL_REGRESSION_FAIL'
fi
