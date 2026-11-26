# cu2cce-bridge Code Review 与 Simplifier 设计（归档）

**Date:** 2026-08-31
**Scope:** `cucc`, CMake/Make/Ninja wrappers, fake CUDA toolkit shim, CMake compatibility modules, and contract tests.

## 目标

Improve maintainability and remove avoidable complexity in the build compatibility layer while preserving the current puresimt command semantics, diagnostics, environment-variable contract, and documented backend limits.

Every review decision uses a two-axis test: first align the mechanism with the local `cu-bridge` reference implementation; then verify that the change remains correct for Ascend NPU rather than copying MACA-specific compiler, linker, runtime, or device assumptions.

## 待解决问题

1. `tools/pre_cce` reconstructs the wrapped command through `eval "$*"`, which loses argv boundaries and makes arguments containing spaces or shell metacharacters unsafe. The wrapper already receives a complete argv vector, so it should execute `"$@"` directly.
2. `tools/cu2cce_driver.py::RuleDriver.prepare` contains phase setup, argument classification, injection ordering, finalization, trace emission, and debug output in one 216-line method. The logic should be split into private helpers without changing rule precedence or output ordering.
3. `cmake_modules/FindCUDA.cmake` repeats CUDA-language enablement and target property attachment logic across legacy macros. Small private helpers can make the supported legacy surface easier to audit.
4. The current tests are strong on static contracts and rule behavior but do not exercise `pre_cce` argv preservation end-to-end. Add a focused regression test before changing that wrapper.

## 后端感知的 parity 规则

The parity target is the mechanism, not a literal MACA implementation. Keep the following Ascend-specific facts explicit during review:

- device compilation remains `ccec -x asc --enable-simt --cce-aicore-arch=${CU2CCE_NPU_ARCH}`;
- architecture selection is an Ascend `dav-*` target resolved by `device_arch.sh`, not an `sm_*` list;
- host linking remains the system `g++`/`gcc` path with explicit `libruntime.so`, `libascendcl.so`, and `libcce_bridge_runtime.so` requirements where needed;
- CUDA device-link/RDC and NVIDIA artifact generation stay rejected because they have no validated Ascend puresimt equivalent;
- fake CUDA library aliases are compatibility carriers only and must not claim complete CUDA/MACA library equivalence;
- multi-translation-unit registration limitations and other CANN/NPU backend constraints must remain visible in tests and docs.

## 非目标

- No change to the puresimt compiler route (`ccec -x asc --enable-simt`).
- No reintroduction of Python kernel conversion, `.cce` generation, or legacy lowering.
- No expansion of unsupported PTX/CUBIN/FATBIN, device-link/RDC, Bazel, MPI, or full CUDA library surfaces.
- No redesign of the CMake two-phase protocol or the shim manifest.
- No automatic changes to the real-machine regression script beyond tests needed to lock the refactor.

## 设计

### wrapper 安全

Change `pre_cce` from shell-string reconstruction to direct argv execution. Preserve the existing environment preparation, cleanup behavior, and exit status. The test command will record each argument as a separate line and pass an argument containing spaces plus shell metacharacters; the expected output must match the original argv exactly.

### driver 分解

Keep `RuleDriver.prepare` as the public orchestration entry point. Extract only cohesive, side-effect-aware helpers:

- `_transform_arguments(expanded, phase, context)` for the existing rule-precedence loop;
- `_order_injections(injections, source_is_cuda)` for `-x asc`, injected include ordering, and prefix construction;
- `_finalize_prepared_command(...)` for output normalization, conda flags, gcc-toolchain/regex cleanup, trace, and debug reporting.

The helpers will reuse existing state (`actions`, `injected`, warnings, trace) and return ordinary lists/tuples. No rule data or action ordering changes are allowed.

### FindCUDA 清理

Add private CMake helpers for enabling CUDA only when necessary and attaching include/link/options to legacy targets. Keep the public macro names, accepted options, variables, and explicit PTX/CUBIN/FATBIN errors unchanged.

## 验证

After each change:

- run the focused test and observe the intended RED/GREEN cycle;
- run `python3 -m unittest discover -s tests -p 'test_*.py' -v`;
- run `python3 tools/validate_cucc_rules.py config/cu2cce_rules.json`;
- run `python3 -m py_compile ...` for all Python sources/tests;
- run `bash -n` on all shell entrypoints;
- run `bash tests/mock_acl/syntax_check.sh`;
- run `git diff --check`.

CANN/NPU builds remain out of scope on macOS and must be validated through `tools/real_test/server_cmd.txt` on the Ascend server.
