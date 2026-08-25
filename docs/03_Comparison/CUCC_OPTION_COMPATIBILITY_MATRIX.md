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
| `-Xcompiler/--compiler-options` | separate/equal | compile/link | REPLACE | `EXPERIMENTAL` | shell-split host flags |
| `-Xlinker` | separate/equal | link | DROP | `DIAGNOSTIC_ONLY` | nvcc linker forwarding is not needed |
| `-gencode/--generate-code` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | never mapped to fake Ascend architecture |
| `-arch/--gpu-architecture` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | `CU2CCE_NPU_ARCH` selects ccec target |
| `--ptxas-options/-Xptxas` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | ptxas is not used |
| `--extended-lambda` | exact | compile | DROP | `DIAGNOSTIC_ONLY` | ccec does not need nvcc spelling |
| `--lineinfo`/`-lineinfo` | exact | compile | DROP | `DIAGNOSTIC_ONLY` | not part of the current ccec policy |
| `--cuda-gpu-arch=sm_*` | equal | compile | DROP | `DIAGNOSTIC_ONLY` | Clang/CMake GPU architecture spelling is not used by ccec |
| `-M/-MM` | exact | dependency | FORWARD | `EXPERIMENTAL` | real ccec dependency output pending |
| `-MD/-MF/-MT` | exact/separate | compile/dependency | FORWARD | `EXPERIMENTAL` | dedicated Makefile smoke |
| `-E` | exact | preprocess | FORWARD | `EXPERIMENTAL` | phase is not linked |
| `--version/-V/--help` | exact | query | DRIVER | `SUPPORTED` | compatibility identity output |
| `--dryrun` | exact | all | DRIVER | `SUPPORTED` | prints final ccec command |
| `-shared` | exact | compile/link | INJECT `-fPIC` | `SUPPORTED` for tested shape | required for ccec TLS/registration relocations |
| `--device-link/-dlink` | exact | device link | REJECT | `UNSUPPORTED` | no current ccec aggregation path |
| `-rdc=true` | exact | device link | REJECT | `UNSUPPORTED` | separable compilation not validated |
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
