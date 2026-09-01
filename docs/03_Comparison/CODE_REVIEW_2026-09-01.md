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
