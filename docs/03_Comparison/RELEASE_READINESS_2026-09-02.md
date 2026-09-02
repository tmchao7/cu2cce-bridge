# cu2cce-bridge 发布前收口审计（2026-09-02）

## 结论

本文件是发布前对 `cucc`、CMake、Make、Ninja 和实际产物契约的收口记录。
参考实现固定为本地 `cu-bridge-muxi` `6ab1076` / `3.8.0`；机制对齐不等于伪造
MACA 后端能力，Ascend 产物以结构和运行时依赖契约判断。

最新 Ascend 服务器回归由维护者报告：

```text
FAIL=0
DEVICE_HEALTHY=1
TEST_BLOCKED_BY_DEVICE=0
```

服务器证据以 `tools/real_test/server_cmd.txt` 的 full profile 输出为准；本机 macOS 不能替代 CANN/NPU 验证；本轮本地静态/mock 全量测试为 210 tests。

## 四层对齐结果

| 层 | 入口/机制 | 产物或证据 | Ascend 边界 |
|---|---|---|---|
| cucc | `bin/cucc` → `tools/cu2cce_driver.py` → `ccec` | `.o`、`.d`、ELF executable/shared object、`NEEDED`/RPATH | `dav-*` 由 `device_arch.sh` 解析；`sm_*` 只作兼容宏/诊断 |
| CMake | `cmake_cce` configure mock + build 真实 `cucc` | CMake metadata、`.o/.a/.so/ELF`、install/export | `CUDA::cudart` 表示 bridge runtime；不提供伪造完整 CUDA Toolkit |
| Make | `make_cce` + manifest fake toolkit | `.o/.d/.a/.so/ELF`、clean 生命周期 | host link 通过 bridge/CANN runtime，不能依赖 MACA 库名 |
| Ninja | `ninja_cce` + 同一 persistent shim | `build.ninja`、`rules.ninja`、同类最终产物 | configure/build 两阶段仍使用同一 `CUDA_DIR` |

产物契约位于 `config/artifact_contract.json`，检查器位于
`tools/real_test/verify_artifacts.py`。禁止产生 `.cce`、`.ptx`、`.cubin`、`.fatbin`
和 `.maca`；不要求 Ascend 与 MACA ELF 字节级一致。

## Code review / simplifier 收口

- `RuleDriver` 的 phase、规则转换、注入排序、最终命令构造已分层；规则优先级保持
  `reject → replace → drop → forward`。
- `pre_cce`、Make、Ninja 使用直接 argv 执行，不使用 `eval` 重构用户参数。
- fake toolkit 生命周期统一由 `fake_toolkit_env.sh` + `build_shim_tree.py` 驱动，避免
  三套手工软链逻辑漂移。
- 直接 `cucc` 与构建 wrapper 共用 `device_arch.sh`；host-only 编译的
  `-I/-include` 值不会误触发 CUDA phase；缺失 `-Xlinker` value 明确报错。
- artifact verifier 只检查实际 target boundary：`.cu` target 验证 compiler，host app
  验证 bridge runtime，避免把合法 host link metadata 当成 CUDA compiler 缺失。

## 目录与文档约定

```text
bin/                 用户入口（cucc）
cmake_modules/       CMake FindCUDA/FindCUDAToolkit 兼容层
config/              版本化规则、shim 和 artifact contract
include/ src/        bridge/runtime 与 experimental BLAS 实现
examples/             可运行样例和构建兼容载体
third_party/          必要的 NVIDIA helper 头
tools/real_test/      唯一真机回归入口与诊断脚本
docs/                 架构、范围、对比、参考和归档文档
```

- `docs/03_Comparison/` 记录机制差异、产物证据和真机结论。
- `docs/archive/` 保存已完成计划和历史设计，不在顶层创建新的 `plans/` 目录。
- `build/`、`.cu2cce-cache/`、`.mimosa/`、`.worktrees/` 及日志均为本地/运行时内容，
  不进入版本库。
- `README.md` 面向项目入口，`QUICKSTART.md` 面向用户操作，`docs/README.md` 面向
  文档导航；不要把临时服务器命令另存为新的版本文件。

## 发布前命令

本机可执行：

```bash
python3 tools/validate_cucc_rules.py config/cu2cce_rules.json
python3 -m unittest discover -s tests -p 'test_*.py' -v
bash tests/mock_acl/syntax_check.sh
git diff --check
```

服务器必须执行：

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git switch main
git fetch origin main
git pull --ff-only origin main
bash tools/real_test/clean.sh
CU2CCE_REGRESSION_REMOTE=origin CU2CCE_REGRESSION_PROFILE=full \
  bash tools/real_test/server_cmd.txt
```

`multi_cu_diagnostic` 的 duplicate registration 仍属于预期的 Ascend backend
`DIAGNOSTIC_ONLY`，不应通过链接器选项伪装为支持。默认 CMake shim 为兼容历史
入口保留持久目录；并发或多 bridge 版本构建应显式设置不同的
`CU2CCE_CMAKE_SHIM_DIR`，或设置 `CU2CCE_SHIM_ISOLATE=1`，避免共享状态互相覆盖。
