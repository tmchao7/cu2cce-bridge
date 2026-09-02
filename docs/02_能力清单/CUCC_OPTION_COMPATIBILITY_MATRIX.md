# cucc CUDA 参数兼容性矩阵

规则由 `config/cu2cce_rules.json` 实现，并由 `cucc` 与 CMake 的 `mock_nvcc` shim 共同使用。
参数处理遵循 `reject → replace → drop → forward` 的优先级；`sm_*` 只用于兼容宏、trace 或诊断，不能直接作为 Ascend `ccec` 架构参数。

| 参数族 | 形式 | 阶段 | 动作 | 状态 | 证据 / 边界 |
|---|---|---|---|---|---|
| `-I` | separate/equal | all | FORWARD | `SUPPORTED` for tested builds | include paths preserved |
| `-D/-U` | separate/equal | all | FORWARD | `SUPPORTED` for tested builds | macros preserved |
| `-O*`, `-g`, `-fPIC` | exact/prefix | compile/link | FORWARD | `SUPPORTED` for tested shapes | shared CUDA output injects PIC when absent |
| `-std=` | equal | compile | FORWARD | `SUPPORTED` for tested C++17 | ccec receives gnu++17 baseline |
| `-W*` | prefix | all | FORWARD | `EXPERIMENTAL` | warning spelling may be backend-specific |
| `-L/-l` | separate/equal | link | FORWARD | `EXPERIMENTAL` | unvalidated CUDA libraries warn and remain unmodified |
| `-Xcompiler/--compiler-options` | separate/equal | compile/link | REPLACE | `EXPERIMENTAL` | `shlex`/comma-split host flags, per-flag NPU filtering |
| `-Xlinker` / `--linker-options` | separate/equal | link | FORWARD/REPLACE | `SUPPORTED` for command shape | `--linker-options <arg>` maps to `-Xlinker <arg>`; actual ccec link semantics require real proof |
| `CUCC_TARGETS/CUCC_TARGETS_FROM_DEVICE` | environment | all | RESOLVE | `SUPPORTED` for one `dav-*` target | cu-bridge-compatible source, validated for Ascend single-target ccec |
| `-gencode/--generate-code` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | never mapped to fake Ascend architecture |
| `-arch/--gpu-architecture/-code` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | `CU2CCE_NPU_ARCH` selects ccec target; `-code=sm_*` is metadata only and never reaches ccec |
| `--ptxas-options/-Xptxas` | separate/equal | compile | DROP | `DIAGNOSTIC_ONLY` | ptxas is not used |
| `--extended-lambda` | exact | compile | DROP | `DIAGNOSTIC_ONLY` | ccec does not need nvcc spelling |
| `--lineinfo`/`-lineinfo` | exact | compile | DROP | `DIAGNOSTIC_ONLY` | not part of the current ccec policy |
| `--cuda-gpu-arch=sm_*` | equal | compile | DROP | `DIAGNOSTIC_ONLY` | Clang/CMake GPU architecture spelling is not used by ccec |
| `-M/-MM` | exact | dependency | FORWARD | `EXPERIMENTAL` | real ccec dependency output pending |
| `-MD/-MF/-MT` | exact/separate | compile/dependency | FORWARD | `EXPERIMENTAL` | dedicated Makefile smoke |
| `-E` | exact | query/preprocess | QUERY PASSTHROUGH | `SUPPORTED` for mechanism | ccec receives the original query without SIMT/runtime injection; output semantics remain diagnostic-only until real proof |
| `--version/-V/--help` | exact | query | DRIVER | `SUPPORTED` | compatibility identity output |
| direct output contract | `.o/.d/executable/.so` | compile/link | ARTIFACT CHECK | `SUPPORTED` for validated shape | full server profile verifies file type, NEEDED/RPATH and forbidden intermediate suffixes |
| `WCUDA_BAZEL_TF_BUILD=1` | environment | all | REJECT | `UNSUPPORTED` | no validated Ascend equivalent; use explicit CMake/Make integration |
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

状态只对表中已经验证的参数形式和后端负责，不代表完整 NVIDIA CUDA 编译器或库等价。

## 头文件模式

| 模式 | 行为 | 状态 |
|---|---|---|
| `CU2CCE_LIBC_MODE=shadow` | injects minimal shadow libc headers and `__CU2CCE_NO_LIBC__` | `SUPPORTED` for existing puresimt regression |
| `CU2CCE_LIBC_MODE=system` | keeps system C/C++ headers and omits shadow libc injection | `EXPERIMENTAL` for external C++/CUDA projects |

外部 CUDA Samples profile 使用 `system` 模式，因为 libstdc++ 会包含 glibc 头文件，
其异常规格可能与最小 shadow 声明冲突。


`cuda_runtime.h` 还提供 CUDA 传递包含所需的常用数学声明：shadow 模式使用 bridge 数学声明，
`system` 模式使用系统 `<math.h>`，因此直接调用 `fabs` 的上游样例仍可保持源码兼容。


对于包含 `cuda_runtime.h` 的纯 host `.cpp` 编译单元，由于没有 device 编译标志，头文件选择系统
`<math.h>`；ccec CUDA 编译单元带有 `__CU2CCE_DEVICE_COMPILATION__`，在默认 shadow 模式下
继续使用 shadow 数学声明。


`cooperative_groups.h` 和 `cooperative_groups/reduce.h` 当前只为选定的外部 reduction 样例
提供最小源码外观，不代表完整 CUDA Cooperative Groups API 兼容。


runtime 头文件有意不自动包含 `shadow/math.h`：已有最小数学回归会显式请求它，而上游 C++/CUDA
项目在 `CU2CCE_LIBC_MODE=system` 下使用系统数学头，从而避免 shadow/system 异常规格冲突。


真机脚本中的官方 helper-heavy puresimt 样例显式选择 `__CU2CCE_SYSTEM_LIBC__` 并预包含系统
`math.h`；这是针对上游 helper 头的 driver 策略，不修改官方 CUDA 源码。


## device FP64 策略

`CU2CCE_DEVICE_FP64_POLICY=error` 是默认值。当 ccec 报告 device-side FP64 不支持时，cucc 保留
原始诊断并补充可执行建议：host 侧可以继续使用 `double`，但 device 侧运算应改为 `float`，
或提供明确的 host fallback/混合精度路径。cucc 不会静默重写所有 `double` token。
