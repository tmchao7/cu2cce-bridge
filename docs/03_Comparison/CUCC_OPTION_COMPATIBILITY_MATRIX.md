# cucc CUDA Option Compatibility Matrix

Rules are implemented by `config/cu2cce_rules.json` and shared by `cucc` and the CMake
`mock_nvcc` shim.

| Option family | Forms | Phase | Action | Status | Evidence / boundary |
|---|---|---|---|---|---|
| `-I` | separate/equal | all | FORWARD | `SUPPORTED` for tested builds | include paths preserved |
| `-D/-U` | separate/equal | all | FORWARD | `SUPPORTED` for tested builds | macros preserved |
| `-O*`, `-g`, `-fPIC` | exact/prefix | compile/link | FORWARD | `SUPPORTED` for tested shapes | shared CUDA output injects PIC when absent |
| `-std=` | equal | compile | FORWARD | `SUPPORTED` for tested C++17 | ccec receives gnu++17 baseline |
| `-W*` | prefix | all | FORWARD | `EXPERIMENTAL` | warning spelling may be backend-specific |
| `-L/-l` | separate/equal | link | FORWARD | `EXPERIMENTAL` | unvalidated CUDA libraries warn and remain unmodified |
| `-Xcompiler/--compiler-options` | separate/equal | compile/link | REPLACE | `EXPERIMENTAL` | `shlex`/comma-split host flags, per-flag NPU filtering |
| `-Xlinker` / `--linker-options` | separate/equal | link | FORWARD/REPLACE | `SUPPORTED` for command shape | `--linker-options <arg>` maps to `-Xlinker <arg>`; actual ccec link semantics require real proof |
| `-gencode/--generate-code` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | never mapped to fake Ascend architecture |
| `-arch/--gpu-architecture` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | `CU2CCE_NPU_ARCH` selects ccec target |
| `--ptxas-options/-Xptxas` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | ptxas is not used |
| `--extended-lambda` | exact | compile | DROP | `DIAGNOSTIC_ONLY` | ccec does not need nvcc spelling |
| `--lineinfo`/`-lineinfo` | exact | compile | DROP | `DIAGNOSTIC_ONLY` | not part of the current ccec policy |
| `--cuda-gpu-arch=sm_*` | equal | compile | DROP | `DIAGNOSTIC_ONLY` | Clang/CMake GPU architecture spelling is not used by ccec |
| `-M/-MM` | exact | dependency | FORWARD | `EXPERIMENTAL` | real ccec dependency output pending |
| `-MD/-MF/-MT` | exact/separate | compile/dependency | FORWARD | `EXPERIMENTAL` | dedicated Makefile smoke |
| `-E` | exact | query/preprocess | QUERY PASSTHROUGH | `SUPPORTED` for mechanism | ccec receives the original query without SIMT/runtime injection; output semantics remain diagnostic-only until real proof |
| `--version/-V/--help` | exact | query | DRIVER | `SUPPORTED` | compatibility identity output |
| `--dryrun` | exact | all | DRIVER | `SUPPORTED` | prints final ccec command |
| `-shared` | exact | compile/link | INJECT `-fPIC` | `SUPPORTED` for tested shape | required for ccec TLS/registration relocations |
| `--device-link/-dlink` | exact | device link | DROP + phase degrade | `DIAGNOSTIC_ONLY` | CMake layer forces `CUDA_SEPARABLE_COMPILATION OFF`; stray flag no longer hard-crashes configure |
| `-rdc` / `-rdc=true` / `--relocatable-device-code(=true)` | exact/equal | device link | REJECT | `UNSUPPORTED` | separable compilation not validated |
| `-rdc=off` / `--relocatable-device-code=false` | equal | all | DROP | `DIAGNOSTIC_ONLY` | benign separable-off spelling |
| `-maxrregcount/--maxrregcount` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | NVIDIA register-control not used |
| `--generate-line-info` | exact | compile | DROP | `DIAGNOSTIC_ONLY` | same policy as `-lineinfo` |
| `-cubin` | exact | compile | DROP | `DIAGNOSTIC_ONLY` | CUBIN artifact unsupported |
| `--Werror` | exact | all | DROP | `DIAGNOSTIC_ONLY` | ccec uses host `-Werror` spelling |
| `-m64/--machine` | separate/equal | all | DROP | `DIAGNOSTIC_ONLY` | host bitness fixed by ccec |
| `-Xcudafe/--cudafe` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | cudafe not part of toolchain |
| `-Xnvlink/-Xcyco` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | nvlink/cyco not used |
| `--use_fast_math/--fmad/--prec-div/--prec-sqrt/--ftz` | exact/equal | compile | DROP | `DIAGNOSTIC_ONLY` | NVIDIA float semantics not mapped; use ccec options |
| `-G` | exact | compile | DROP | `DIAGNOSTIC_ONLY` | NVIDIA device-debug mode not used |
| `-Wl,-Bstatic/-Bdynamic/--start-group/--end-group` | exact | link | DROP | `DIAGNOSTIC_ONLY` | cudart static grouping not needed |
| `--resource-usage` | exact | compile | DROP | `DIAGNOSTIC_ONLY` | not part of ccec toolchain |
| `--forward-unknown-to-host-compiler=*` | equal | all | DROP | `DIAGNOSTIC_ONLY` | nvcc forwarding control not needed |
| `--pre-include/--include-path/--library-path` | separate/equal | compile | REPLACE | `SUPPORTED` for command shape | preserve the value and map to ccec `-include`/`-I`/`-L` |
| `--ptx/--cubin/--fatbin` | exact | compile | REJECT | `UNSUPPORTED` | puresimt consumes CUDA source directly |
| `-lcudart` | exact | link | DROP + runtime inject | `SUPPORTED` for bridge path | uses `libcce_bridge_runtime.so` |
| `-lcublas/-lcufft/-lcusparse` | exact | link | WARN_FORWARD | `EXPERIMENTAL`/`UNSUPPORTED` | no blanket CANN replacement |

A status applies only to the tested form and backend. It is not a promise of complete NVIDIA
CUDA compiler or library equivalence.

## Header mode

| Mode | Behavior | Status |
|---|---|---|
| `CU2CCE_LIBC_MODE=shadow` | injects minimal shadow libc headers and `__CU2CCE_NO_LIBC__` | `SUPPORTED` for existing puresimt regression |
| `CU2CCE_LIBC_MODE=system` | keeps system C/C++ headers and omits shadow libc injection | `EXPERIMENTAL` for external C++/CUDA projects |

The external CUDA Samples profile uses `system` mode because libstdc++ includes glibc
headers whose exception specifications conflict with the minimal shadow declarations.


`cuda_runtime.h` also provides the common CUDA-transitive math declaration behavior:
shadow mode includes the bridge math declarations, while system mode includes the real
system `<math.h>`. This keeps upstream samples that call `fabs` without a separate math
include source-compatible.


For host-only `.cpp` translation units that include `cuda_runtime.h`, the header selects
system `<math.h>` because no device-compilation marker is present. ccec CUDA translation
units receive `__CU2CCE_DEVICE_COMPILATION__` and keep the shadow math declarations in the
default shadow mode.


`cooperative_groups.h` and `cooperative_groups/reduce.h` currently provide a minimal
source-level facade for the selected external reduction sample. This is not a claim of
complete CUDA Cooperative Groups API compatibility.


The runtime header intentionally does not preinclude `shadow/math.h`. Existing minimal math
regressions explicitly request it, while upstream C++/CUDA projects use system math in
`CU2CCE_LIBC_MODE=system`; this prevents shadow/system exception-specification conflicts.


The official helper-heavy puresimt samples in the real-device script explicitly select
`__CU2CCE_SYSTEM_LIBC__` and preinclude system `math.h`; this is a compiler-driver policy
for upstream helper headers and does not modify the official CUDA sources.


## Device FP64 policy

`CU2CCE_DEVICE_FP64_POLICY=error` is the default. When ccec reports device-side FP64
unsupported, cucc preserves the original diagnostics and adds an actionable message: keep
host-side `double`, but change device-side arithmetic to `float` or provide an explicit
host fallback/mixed-precision path. cucc does not silently rewrite every `double` token.
