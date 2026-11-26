# cu2cce-bridge Code Review 与 Simplifier 实施计划（归档）

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Simplify the build compatibility implementation and fix the unsafe `pre_cce` argv reconstruction without changing puresimt or documented compatibility behavior.

**Architecture:** Preserve the existing three-layer design: shell wrappers prepare the environment, the Python driver owns cucc argument semantics, and CMake modules expose a deliberately small legacy surface. Extract cohesive helpers only where the current implementation already has a stable behavior contract. Compare each layer to `../cu-bridge`, but retain Ascend-specific compiler, architecture, runtime, linker, and unsupported-feature boundaries.

**Tech Stack:** Bash, Python 3, CMake language modules, JSON rule data, Python `unittest`.

**Spec:** `docs/99_归档/CODE_REVIEW_SIMPLIFIER_SPEC_2026-08-31.md`

## 全局约束

- Treat `cu-bridge` parity as mechanism parity, not literal MACA backend copying.
- Keep the puresimt route as `ccec -x asc --enable-simt`.
- Do not generate `.cce`, `__simt_vf__`, `cce::async_invoke`, or converter output.
- Preserve rule precedence, injected argument ordering, diagnostics, and environment-variable names.
- Preserve Ascend `dav-*` architecture resolution, host `g++` linking, explicit `libruntime`/`libascendcl` requirements, and puresimt unsupported-feature boundaries.
- Do not claim CANN/NPU validation from macOS; use `tools/real_test/server_cmd.txt` on the real server.
- Do not push `origin`; the project policy uses `gitcode`.

### Task 1: Add an end-to-end `pre_cce` argv regression test

**Files:**
- Modify: `tests/test_cmake_compat.py`
- Test: `tests/test_cmake_compat.py`

**Interfaces:**
- Consumes: `tools/pre_cce` and the repository virtual toolkit directory builder.
- Produces: a regression test proving wrapped command arguments remain separate and exact.

- [ ] **Step 1: Write the failing test**

Add a test that creates a temporary executable recorder, invokes `pre_cce` with `CUCC_PATH` set to the repository, `CU2CCE_HOME` set to a temporary directory, and arguments `alpha beta value;$(printf unsafe)`. The recorder writes one argv item per line. Assert the recorder receives the command path followed by the two exact values, with no shell expansion.

- [ ] **Step 2: Run the focused test to verify it fails**

Run:

```bash
python3 -m unittest tests.test_cmake_compat.CMakeCompatibilityContractTest.test_pre_cce_preserves_argument_boundaries -v
```

Expected: FAIL because current `pre_cce` uses `eval "$*"` and reconstructs the argv as shell text.

### Task 2: Fix `pre_cce` direct execution and simplify its final dispatch

**Files:**
- Modify: `tools/pre_cce:77-78`
- Test: `tests/test_cmake_compat.py`

**Interfaces:**
- Consumes: Task 1 recorder contract.
- Produces: `pre_cce` preserving the exact wrapped argv and exit status.

- [ ] **Step 1: Replace shell reconstruction with direct argv execution**

Replace the final command reconstruction with:

```bash
exec "$@"
```

Keep the existing environment setup and clean-goal handling unchanged.

- [ ] **Step 2: Run the focused test to verify it passes**

Run:

```bash
python3 -m unittest tests.test_cmake_compat.CMakeCompatibilityContractTest.test_pre_cce_preserves_argument_boundaries -v
```

Expected: PASS and the recorder output contains the exact command path and two original arguments.

- [ ] **Step 3: Run the CMake compatibility tests**

Run:

```bash
python3 -m unittest tests.test_cmake_compat -v
```

Expected: all CMake compatibility contract tests pass.

### Task 3: Extract stable stages from `RuleDriver.prepare`

**Files:**
- Modify: `tools/cu2cce_driver.py:942-1157`
- Test: `tests/test_cucc_rules.py`

**Interfaces:**
- Consumes: existing `RuleDriver.prepare(argv) -> PreparedCommand` contract.
- Produces: private helpers `_transform_arguments`, `_order_injections`, and `_finalize_prepared_command`; `prepare` remains behaviorally identical.

- [ ] **Step 1: Add a focused ordering contract test**

Add a test that prepares a CUDA compile containing user include paths, `-Xcompiler`, and an output ending in `.o`, then asserts:

```python
assert prepared.args[:2] == ["-x", "asc"]
assert prepared.args.index("-I") < prepared.args.index("-c")
assert prepared.args.count("-include") >= 2
assert prepared.args[0:2] == ["-x", "asc"]
```

Also assert the existing CUDA flag, runtime header, and SIMT adaptor behavior remains present.

- [ ] **Step 2: Run the focused test before refactoring**

Run:

```bash
python3 -m unittest tests.test_cucc_rules.CuccRuleDriverTest.test_compile_injection_order_contract -v
```

Expected: PASS against the current implementation; this locks the behavior being preserved during extraction.

- [ ] **Step 3: Extract `_transform_arguments`**

Move only the current `prepare` argument loop into a private method with this signature:

```python
def _transform_arguments(
    self, expanded: Sequence[str], phase: str
) -> List[str]:
```

Preserve the existing local `i`, `consumed`, special cases, rule order (`reject -> replace -> drop -> forward`), and updates to `self.actions`/`self.injected`.

- [ ] **Step 4: Extract `_order_injections`**

Add:

```python
def _order_injections(
    self,
    injections: List[str],
    source_is_cuda: bool,
) -> Tuple[List[str], List[str]]:
```

Return `(prefix, ordered_injections)` where the returned injection list has `-x asc` removed when it is consumed into the prefix, and all `-I`/`-include` pairs are moved into the include prefix exactly as before.

- [ ] **Step 5: Extract final command construction**

Add a private finalizer that accepts the transformed result, phase, mode, inputs, output, and CUDA-source boolean; it must preserve:

- implicit `-c` for `.o` outputs;
- conda `LDFLAGS` link behavior;
- gcc-toolchain injection;
- regex cleanup;
- `PreparedCommand` fields;
- trace file contents;
- debug output.

- [ ] **Step 6: Reduce `prepare` to orchestration**

Leave `prepare` responsible only for response expansion, query bypass, phase/context setup, preflight checks, stage calls, and returning the final `PreparedCommand`.

- [ ] **Step 7: Run the focused and full driver tests**

Run:

```bash
python3 -m unittest tests.test_cucc_rules.CuccRuleDriverTest.test_compile_injection_order_contract -v
python3 -m unittest tests.test_cucc_rules -v
```

Expected: both commands pass with no changes to the prepared command contract.

### Task 4: Simplify repeated legacy FindCUDA support helpers

**Files:**
- Modify: `cmake_modules/FindCUDA.cmake:35-153`
- Test: `tests/test_cmake_compat.py`

**Interfaces:**
- Consumes: existing `cuda_add_executable`, `cuda_add_library`, `cuda_compile`, and `cuda_wrap_srcs` behavior.
- Produces: private helper functions for CUDA enablement and target attachment; public legacy macro names and variables remain unchanged.

- [ ] **Step 1: Add a static helper contract test**

Assert `FindCUDA.cmake` contains private helpers for enabling CUDA and applying the common include/link/compile options, while retaining the public macro names and explicit unsupported artifact errors.

- [ ] **Step 2: Run the focused test to establish the expected failure**

Run:

```bash
python3 -m unittest tests.test_cmake_compat.CMakeCompatibilityContractTest.test_findcuda_uses_shared_legacy_helpers -v
```

Expected: FAIL because the helpers do not exist yet.

- [ ] **Step 3: Add `_cu2cce_enable_cuda` and `_cu2cce_attach_target`**

Use the existing logic without changing accepted arguments:

```cmake
function(_cu2cce_enable_cuda)
  get_property(_enabled GLOBAL PROPERTY ENABLED_LANGUAGES)
  if(NOT "CUDA" IN_LIST _enabled)
    enable_language(CUDA)
  endif()
endfunction()

function(_cu2cce_attach_target target scope options)
  target_include_directories(${target} ${scope} "${CUDA_INCLUDE_DIRS}")
  target_link_libraries(${target} ${scope} "${CUDA_LIBRARIES}")
  if(options)
    target_compile_options(${target} PRIVATE ${options})
  endif()
endfunction()
```

Adapt the existing public functions/macros to call the helpers while preserving `PUBLIC` vs `PRIVATE` linkage semantics for libraries and executables.

- [ ] **Step 4: Replace duplicated language-enable checks**

Use `_cu2cce_enable_cuda()` in `cuda_compile` and `cuda_wrap_srcs`. Keep PTX/FATBIN/CUBIN macros as explicit `FATAL_ERROR` paths.

- [ ] **Step 5: Run the focused and full CMake tests**

Run:

```bash
python3 -m unittest tests.test_cmake_compat.CMakeCompatibilityContractTest.test_findcuda_uses_shared_legacy_helpers -v
python3 -m unittest tests.test_cmake_compat -v
```

Expected: all tests pass and the public legacy surface remains unchanged.

### Task 5: Full verification and review report

**Files:**
- Create: `docs/04_对比与验证/CODE_REVIEW_2026-08-31.md`
- Modify: only if verification exposes a directly related documentation mismatch.

**Interfaces:**
- Consumes: all prior refactors and test results.
- Produces: a review record with findings, changed files, intentional non-fixes, and verification evidence.

- [ ] **Step 1: Run rules and Python validation**

```bash
python3 tools/validate_cucc_rules.py config/cu2cce_rules.json
python3 -m py_compile tools/cu2cce_driver.py tools/mock/virtualDir.py tools/mock/mock_nvcc.py tests/test_cucc_rules.py tests/test_cmake_compat.py tests/test_blas_surface.py
python3 -m unittest discover -s tests -p 'test_*.py' -v
```

- [ ] **Step 2: Run shell and mock validation**

```bash
set -e
while IFS= read -r -d '' f; do bash -n "$f"; done < <(find bin tools -type f \( -name '*.sh' -o -name 'cucc' -o -name 'cmake_cce' -o -name 'make_cce' -o -name 'ninja_cce' -o -name 'pre_cce' -o -name 'hostcc_cce' -o -name 'virtual_toolkit_env.sh' \) -print0)
bash tests/mock_acl/syntax_check.sh
```

- [ ] **Step 3: Run static hygiene checks**

```bash
git diff --check
git status --short --branch
```

- [ ] **Step 4: Write the review report**

Record concrete findings with severity, file/line references, changes made, deliberate non-changes, the cu-bridge mechanism being matched, the Ascend-specific reason for any divergence, and the fact that CANN/NPU runtime validation was not performed locally.

- [ ] **Step 5: Re-read the diff before reporting**

Inspect:

```bash
git diff --stat
git diff -- tools/pre_cce tools/cu2cce_driver.py cmake_modules/FindCUDA.cmake tests/test_cmake_compat.py tests/test_cucc_rules.py docs/04_对比与验证/CODE_REVIEW_2026-08-31.md
```

Confirm no unrelated files changed and no legacy lowering path was reintroduced.
