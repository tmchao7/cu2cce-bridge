# cu2cce-bridge 全量 Code Review / Code Simplifier 记录

**日期：** 2026-08-31
**基线：** `main` / `c92a4b3`（本轮为工作区未提交 diff）
**对标：** `/Users/tmchao7/repos/09_expLearn/cu-bridge` / `6ab1076` (`3.8.0`)

## Review 范围

本轮覆盖：

- `bin/cucc` 与 `tools/cu2cce_driver.py`
- `tools/cmake_cce`、`tools/make_cce`、`tools/ninja_cce`、`tools/pre_cce`
- `tools/virtual_toolkit_env.sh`、`tools/mock/virtualDir.py`、`tools/link.json`
- `cmake_modules/FindCUDA.cmake`、`cmake_modules/FindCUDAToolkit.cmake`、CUDA override
- 规则、契约测试、mock syntax check、构建兼容性文档

## Review 判据

本轮采用双轴判断：

1. 机制是否与 `cu-bridge` 对齐：fake CUDA toolkit、CMake configure/build 双阶段、Make/Ninja 包装、query/filter/replacer、host compiler carrier、持久化 shim、规则驱动 cucc。
2. 是否保留 Ascend NPU 的真实边界：`ccec -x asc --enable-simt`、`dav-*` 架构、`aclrt/ascendcl/libruntime` 链接、host g++ 路径、无 CUDA device-link/PTX/CUBIN/FATBIN 等。

不能因为 MACA 有某个能力，就将其伪装为 Ascend 已支持能力。

## Findings 与处理结果

### [P1] `pre_cce` 使用 `eval "$*"` 破坏 argv 边界并执行用户数据 — 已修复

**位置：** `tools/pre_cce`

原实现把完整参数重新拼接为 shell 字符串。带空格的参数会被拆分，带 `$()` 等内容的参数会被再次解释。该行为不属于必须保留的 `cu-bridge` 机制；`pre_cce` 的目标只是准备环境后执行命令，因此 Ascend 版本可以采用更安全的等价机制：

```bash
exec "$@"
```

新增真实 subprocess 回归测试，验证参数 `alpha beta` 与 `$(printf unsafe)` 均以原始 argv 传递，不发生拆分或 shell 展开。

### [P2] `RuleDriver.prepare` 职责过多 — 已简化

**位置：** `tools/cu2cce_driver.py`

原 `prepare` 同时承担 response-file 展开、query bypass、阶段判定、前置诊断、逐参数规则处理、注入排序、conda 链接参数、gcc-toolchain、regex cleanup、trace/debug 输出，方法长度约 216 行。

当前拆成三个私有阶段：

- `_transform_arguments()`：只执行逐参数规则管线并记录 action/warning；
- `_order_injections()`：只处理 `-x asc`、`-I`、`-include` 的确定性排序；
- `_finalize_prepared_command()`：只处理最终参数收尾、链接环境、trace/debug 与 `PreparedCommand` 构造。

`prepare()` 现在只负责流程编排，未改变：

- `reject -> replace -> drop -> forward` 顺序；
- `.cu` 的 `-x asc --enable-simt` 注入；
- bridge include 优先级；
- `__CUDACC__`/`CUDA_VERSION` 宏；
- Ascend `dav-*` 架构注入；
- FP64 device-side 前置诊断；
- CANN runtime 链接参数；
- trace/debug 输出。

### [P2] legacy FindCUDA target 操作重复 — 已简化

**位置：** `cmake_modules/FindCUDA.cmake`

`cuda_add_executable`、`cuda_add_library`、`cuda_compile`、`cuda_wrap_srcs` 中存在重复的 CUDA language enablement、include/link/options 处理。

当前新增：

- `_cu2cce_enable_cuda()`
- `_cu2cce_attach_target()`

公共 legacy API 名称和行为保持不变；library 仍然使用 `PUBLIC` runtime link，executable 仍然使用 `PRIVATE` runtime link；PTX/FATBIN/CUBIN 仍然明确报 Ascend puresimt unsupported。

## 有意不修改的区域

### CMake build 参数解析

`tools/cmake_cce` 中的 `--build` 参数解析仍然相对长，但它承担了 `cu-bridge cmake_mock` 的双阶段 generator dispatch 语义，涉及：

- `--build` / `--build=...`
- `--target` / `-t` 多 target
- `--parallel` / `-j`
- `--config` / `--verbose`
- `--` 后 generator 参数
- `build.ninja` 与 Makefiles 选择

本轮没有把它迁移到另一个脚本或 Python，避免在没有 Ascend 真机构建的情况下改变 CMake 命令转发语义。

### `device_arch.sh` cache 语义

`device_arch.sh` 支持读取外部提供的 `device_arch.cache`，但当前仓库没有自动写 cache 的流程。本轮保留该行为，因为它属于设备探测策略而不是 code simplifier 的必要变更；如果后续需要自动缓存，应单独设计写入时机、并发锁和架构失效策略，不能简单在探测路径中直接覆盖。

### MACA-specific capabilities

没有将以下 MACA 能力假设为 Ascend 能力：

- CUDA device-link / RDC
- PTX/CUBIN/FATBIN
- 完整 CUDA library alias 等价性
- UVM/Managed memory
- 完整 Driver API/module loading
- 多 CUDA translation unit 静态 archive 的无条件聚合

## 变更文件

- `tools/pre_cce`
- `tools/cu2cce_driver.py`
- `cmake_modules/FindCUDA.cmake`
- `tests/test_cmake_compat.py`
- `tests/test_cucc_rules.py`
- `docs/04_对比与验证/CUBRIDGE_CMAKE_MAKE_DESIGN_ANALYSIS.md`
- `docs/04_对比与验证/CODE_REVIEW_2026-08-31.md`
- `docs/99_归档/CODE_REVIEW_SIMPLIFIER_SPEC_2026-08-31.md`
- `docs/99_归档/CODE_REVIEW_SIMPLIFIER_PLAN_2026-08-31.md`

## 验证证据

本机 macOS 只做静态/mock 验证，未执行 CANN/NPU 产物构建：

```text
python3 tools/validate_cucc_rules.py bin/conf.json   PASS
python3 -m py_compile ...                                      PASS
bash -n 22 个 shell entrypoints                                  PASS
bash tests/mock_acl/syntax_check.sh                            PASS=3 FAIL=0
python3 -m unittest discover -s tests -p 'test_*.py' -v      94 tests OK
cmake -P cmake_modules/FindCUDA.cmake                         PASS
git diff --check                                              PASS
```

真机验证仍应使用：

```text
/Users/tmchao7/repos/09_expLearn/cu2cce-bridge/tools/real_test/server_cmd.txt
```

尤其需要在真实 CANN 环境验证 `pre_cce`、CMake legacy helper、host g++ 链接和 `libruntime.so` 的链接顺序。
