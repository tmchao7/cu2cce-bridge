# cu2cce-bridge 全量 Code Review / Code Simplifier（2026-09-01）

## Review 范围

本轮以当前 `main` 为整体 review 对象，覆盖：

- `cucc` / `tools/cu2cce_driver.py`；
- `cmake_cce`、`make_cce`、`ninja_cce`、`pre_cce`；
- `device_arch.sh`、fake toolkit shim；
- legacy `FindCUDA.cmake`；
- artifact contract/verifier 和真机回归 harness；
- 与本地 `cu-bridge-muxi` `6ab1076` 的机制 parity。

Review 采用 defect-first 原则，同时保留 Ascend NPU 约束：不引入 `mxcc`、
`-fgpu-rdc`、`--maca-link`，不把 MACA 库名或 device artifact 伪装为 Ascend 能力。

## Findings

### [P1] query 模式被 CANN 环境检查提前拦截 — 已修复

`cmake_cce --version`、`cmake_cce -E ...`、`cmake_cce --help` 的声明行为是直接透传
系统 CMake，但原实现先要求 `CUCC_PATH`、CANN `ccec` 和 helper 存在。现在 query
判断移到所有 Ascend 环境检查之前，开发机可安全查询 wrapper 信息，真机配置路径不变。

### [P1] legacy FindCUDA public adder 未启用 CUDA language — 已修复

只写 `project(... CXX)`、随后 `find_package(CUDA)` / `cuda_add_executable()` 的 legacy
工程会在生成阶段无法确定 `.cu` target 的 linker language。现在 public executable/library
adder 在解析并确认 source 非空后调用 `_cu2cce_enable_cuda()`，同时保留 Ascend
puresimt 的直接 `.cu -> ccec` 路线。

### [P2] 架构 cache 可注入非法 CUDA/命令内容 — 已修复

`device_arch.cache` 原先只要非空就返回，`sm_80` 或包含换行/额外参数的值可能被传入
ccec 的 `--cce-aicore-arch`。现在缓存、环境变量和 target alias 都使用统一的严格
`dav-[A-Za-z0-9_.-]+` 校验；非法 cache 会被忽略并回退到设备探测/default，不会默认为
合法 NPU target。

### [P2] wrapper 对非法 `CUCC_TARGETS_FROM_DEVICE` 静默回退 — 已修复

共享 fake toolkit helper 只将 `CUCC_TARGETS` 视为显式 target，导致
`CUCC_TARGETS_FROM_DEVICE=sm_80` 时吞掉 `device_arch.sh` 的失败并回退
`dav-c310-vec`。现在两种 target alias 都走显式校验，错误会阻止 wrapper 继续构建。

## 第二轮 review findings（commit `4de3898`，已修复）

### [P1] CMake build wrapper 丢失 `--verbose` 与无值 `--parallel` 语义 — 已修复

`cmake_cce --build` 的 generator dispatch 原先丢弃 `--verbose`，并把无值
`--parallel` 原样变成 Ninja 的裸 `-j`；Ninja 会直接报
`option requires an argument -- j`。现在 Makefiles 映射为 `VERBOSE=1`/裸 `-j`，
Ninja 映射为 `-v`/`-j 0`，并增加 argv 级测试。

### [P1] CMake/Make/Ninja 自定义 shim 与相对安装路径可能漂移 — 已修复

`cmake_cce` 在进入 build directory 后执行 generator wrapper；相对
`CUCC_PATH` 会因此失效，`CU2CCE_CMAKE_SHIM_DIR` 也可能导致 Make/Ninja
重新生成另一棵 shim。现在各入口在跨目录前解析安装路径，并把已解析的
shim root 传递给 Make/Ninja/pre wrapper，确保 configure/build 使用同一份
CUDA-shaped toolkit。

### [P1] legacy FindCUDA `cuda_compile` 会把 `OPTIONS` 当作源文件 — 已修复

参考 `FindCUDA` 的 `cuda_compile(... OPTIONS ...)` 调用在当前实现中会将
`OPTIONS` 和其参数返还给 CMake，生成错误的源文件/构建规则。现在统一复用
legacy source/options parser；`cuda_wrap_srcs` 同时去除 helper-only 的
`PHONY` marker，并对 PTX/CUBIN/FATBIN format 明确 FATAL_ERROR。

### [P2] CUDA shim library path 过滤不支持非 x86 与 `--library-path` 形式 — 已修复

原逻辑硬编码 `targets/x86_64-linux/lib`，且只在输入已经是 `-L` 时过滤；
ARM/aarch64 的 CMake/Make 产物可能将临时 CUDA alias 路径带入最终链接。
现在按 `targets/<triple>/lib` 结构识别，并对 `--library-path` 的替换结果再次
应用路径策略。

### [P2] response file 缺失、规则 value 缺失与 dry-run quoting 不安全/不诚实 — 已修复

`@missing.rsp` 原先会静默透传；值参数在末尾会静默吞掉；`--dryrun` 用空格
拼接导致带空格路径不可重放。现在缺失值明确报错，缺失 response file 明确
报错，dry-run 使用 `shlex.join`，且 trace 仍不包含 options/source 文件内容。

### [P2] 版本与 device policy 在不同调用形态下不一致 — 已修复

`CUCC_CUDA_VERSION` 原先只影响宏注入，不影响 `cucc --version` banner；
FP64 preflight 也只覆盖 `-c` 而遗漏直接 `.cu -> executable`。现在版本 banner
与宏使用同一环境值，`compile_and_link` 也执行相同的 Ascend device-side FP64
策略。

### [P3] shim builder 损坏目录无法自愈 — 已修复

manifest-owned symlink path 若被旧版本/中断构建留下真实目录，
`Path.unlink()` 会失败。builder 现在在生成 fake toolkit 内移除 stale directory
后重建 symlink，并有真实复现回归测试。

### [P2] diagnostic-only 日志与历史输出目录会污染服务器终端/证据 — 已修复（`ade77c3`、`8ec6f8f`）

`server_cmd.txt` 的 diagnostic-only 失败分支原先会输出 25～60 行完整 ACL/wcuda
尾部，且 `rm -f "$OUT"/*` 不会删除上一轮留下的 artifact 子目录。现在诊断
路径统一只打印 12 行 tail 并保留完整日志路径；回归开始时用 `find` 清除 OUT
下的文件和目录，外部 source/build cache 仍按设计保留。

### [P1] CMake 回归原先可能只走 mock_nvcc，未证明 build 阶段执行真实 cucc — 已修复（`73d08ca`）

`server_cmd.txt` 的 configure 阶段本来使用 mock 是正确的，但后续普通
`cmake --build` 会继续调用 configure shim；这会让 CMake PASS 只证明
`mock_nvcc.py` 可用，而不能证明真实 `cucc -> ccec` 产物链路。现在所有
CMake、multi-cu diagnostic 和 external samples build 都通过 `cmake_cce --build`，
并在 artifact contract 前验证 build 日志包含第二阶段切换、且
`CUDA_DIR/bin/nvcc` 的最终 symlink 指向真实 `cucc`。

### [P1] response/options file 中的控制选项未进入 CLI 控制面 — 已修复（`6561a99`）

此前 `@file` 或 `--options-file` 中的 `--dryrun`、`-V` 会在主入口检测之后
才展开，可能被当作普通编译参数或丢给 ccec。现在统一安全展开一次，
控制 flag、普通规则管线和 trace provenance 共享同一份 argv；仍然不执行
文件内容。

### [P2] artifact contract schema 损坏时会泄漏内部异常 — 已修复（`0da8272`）

`verify_artifacts.py` 原先只检查顶层 `required` 类型；坏的 `metadata` entry
会在 server 进程中触发 `AttributeError`，而不是稳定的 contract failure。现在
required artifact、动态依赖、metadata、forbidden suffix 均在加载阶段校验，
错误统一以 `ContractError` 写入 artifact log 并返回 compact FAIL。

### [P2] capability library heuristic 以短前缀抢先匹配 — 已修复（`09145fc`）

`-lcublasLtCore` 原先会命中 `cublas` 而不是更具体的 `cublasLt`，
导致诊断信息错误。现在按 capability key 长度降序匹配；仍保持未验证 CUDA
库不自动映射、不伪造 Ascend 语义。

### [P2] Make/Ninja 在 `-C` 下的相对 shim 路径会漂移 — 已修复（`92d6b38`）

当 wrapper 先生成相对 `CUDA_PATH`、再由 Make/Ninja 切换工作目录时，
构建命令会解析到另一位置。现在 Make、Ninja、pre wrapper 在目录切换前
将 shim root 锚定为绝对路径，并有 Make/Ninja wrapper 级测试。

### [P2] `large_project` 的 install/export 没有进入 blocking artifact contract — 已修复（`aa3da05`、`783a04f`）

工程本身声明了 `install(EXPORT largeProjectTargets)`，但 harness 原先只运行
build binary，未执行 install，也未检查导出的 target graph。现在 full profile
会安装 `large_project`，并验证 `largeProjectTargets.cmake` 存在且包含
`large_project_app`、`large_kernel_lib`、`large_util_lib`。

## 本轮验证

```text
169 tests OK
cucc rules validator PASS
Python py_compile PASS
bash -n 所有入口脚本 PASS
mock ACL syntax PASS=3 FAIL=0
parity audit MISSING=0
git diff --check PASS
```

## Simplifier 结果

- `device_arch.sh` 的 dav 架构格式校验收敛到 `is_valid_dav_arch()`；
- artifact verifier 删除不可达的 `all_matches && not matches` 分支及无用 inspection state；
- `FindCUDA.cmake` 继续复用 target attachment helper，避免为 uppercase CMake 命令创建
 大小写冲突 wrapper；CMake 命令大小写不敏感本身提供 legacy 调用兼容。

## 有意保留的差异

- `cu-bridge` 的 `USE_CMAKE_MACA=1` 系统 CMake 替换不复制；cu2cce 使用显式
  `cmake_cce` + persistent shim，避免修改系统命令。
- MACA `mxcc` host/device link 机制不复制；Ascend 使用 ccec puresimt 和 host g++，
  必要时显式链接 `libruntime.so`、`libascendcl.so`、`libcce_bridge_runtime.so`。
- MACA 的 CUDA library 全量映射、PTX/CUBIN/FATBIN、RDC/device-link、symbol library
  不伪造；未验证的 Ascend 等价能力继续以 warning/diagnostic/reject 分层。
- 多 `.cu` 静态归档的 duplicate eager-registration 是已知 Ascend backend limitation，
  继续由 `multi_cu_diagnostic` 非阻塞取证。

## 验证

本机 macOS 无 CANN/NPU，因此只执行静态、mock 和契约验证；真实产物与设备语义仍需服务器
回归确认。当前验证包括：

```text
Python unittest 全量
cucc rules validator
Python py_compile
bash -n 所有入口脚本
mock ACL syntax check
FindCUDA module load
artifact contract schema tests
git diff --check
```
