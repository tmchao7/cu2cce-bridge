# 交接说明 (HANDOVER)

本文档为项目交接而写，帮助接手人快速理解项目现状、验证方式和注意事项。

## 1. 项目是什么

cu2cce-bridge 是 **CUDA → 昇腾 NPU 的兼容层**。目标是让用户保留原始 `.cu` 源码，
在 Ascend 950 / CANN 9.x 上通过 CUDA 兼容头、host runtime bridge 与 ccec SIMT
前端直接编译运行，无需手写 Ascend C。

核心链路：

```
CUDA .cu 源码
  → bin/cucc (参数解析 + 头文件/库/架构注入)
  → tools/cu2cce_driver.py + bin/conf.json (规则驱动)
  → ccec -x asc --enable-simt --cce-aicore-arch=dav-*
  → CUDA 兼容头 + libcce_bridge_runtime.so + CANN runtime
  → Ascend NPU
```

## 2. 当前状态

- **正式路线**：puresimt。用户写原始 CUDA（`__global__`、`threadIdx`、`<<<>>>`），
  ccec 通过 `--enable-simt` 直接编译，不做源码转换。
- **main 分支**：唯一维护分支，所有 exp/feat/fix 分支已清理。
- **最后验证**：2026-09-04 Ascend 真机 `full` profile 通过（`FAIL=0`）。
- **BLAS**：`src/bridge/blas` 是 experimental，不阻塞主线验收。
- **已退出路线**：Python kernel converter / `__simt_vf__` / hidden block lowering。
  历史实现可在 git log 中追溯。

详细状态快照见 [`04_对比与验证/CURRENT_STATE.md`](04_对比与验证/CURRENT_STATE.md)。

## 3. 如何验证

### 真机回归（唯一验收方式）

在装有 CANN 9.x + Ascend 950 NPU 的服务器上：

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git switch main
git fetch origin main
git pull --ff-only origin main
bash tools/real_test/clean.sh
CU2CCE_REGRESSION_PROFILE=full bash tools/real_test/server_cmd.txt
```

回归画像：
- `fast`（默认）：核心子集，快速反馈
- `external`：外部 NVIDIA Samples + 诊断样本
- `full`：全量回归（里程碑/冻结前使用）

验收标志（全部必须为 1/PASS）：

```
FAIL=0  DEVICE_HEALTHY=1  TEST_BLOCKED_BY_DEVICE=0
PURESIMT_REGRESSION_PASS  CMAKE_CCE_REGRESSION_PASS
BUILD_COMPATIBILITY_REGRESSION_PASS  CUCC_RULES_REGRESSION_PASS
LARGE_PROJECT_REGRESSION_PASS  CUDA_SAMPLES_CMAKE_REGRESSION_PASS
ASCEND_API_MAPPING_REGRESSION_PASS  CUDA_API_PARITY_REGRESSION_PASS
CUDA_HEADER_WRAPPER_PARITY_REGRESSION_PASS  ARTIFACT_CONTRACT_REGRESSION_PASS
```

### 本地（无 NPU）检查

无 CANN/NPU 的机器只做静态检查和 mock：

```bash
git diff --check
bash -n tools/real_test/server_cmd.txt
bash tests/mock_acl/syntax_check.sh   # 如可用
```

**不得将本地 mock 结果冒充真机通过。**

## 4. 关键目录

| 路径 | 用途 |
|---|---|
| `include/` | CUDA 兼容入口、runtime/BLAS wrapper 头、SIMT 原语适配头、shadow 头 |
| `src/bridge/runtime/` | CUDA Runtime → ACL Runtime 实现（`cuda* → wcuda* → aclrt*`） |
| `src/bridge/blas/` | experimental cublas → aclblas，不阻塞主线 |
| `bin/cucc` | 驱动入口，参数解析后直接调用 ccec |
| `bin/conf.json` | cucc 规则配置（库 drop/forward、capability 策略） |
| `tools/cu2cce_driver.py` | cucc 核心逻辑（参数过滤、规则匹配、ccec 调用） |
| `tools/real_test/server_cmd.txt` | **唯一真机回归入口** |
| `tools/real_test/clean.sh` | 清理构建产物 |
| `tools/mock/` | mock_nvcc + virtualDir，供 CMake/Make 探测 |
| `examples/puresimt/` | 官方 NVIDIA Samples + 最小 CUDA operator 用例 |
| `third_party/nvidia_cuda_samples/` | 官方 helper 头，只作 include 依赖 |
| `docs/` | 中文技术文档中心 |
| `config/` | artifact contract、API parity inventory 等配置 |
| `tests/` | 本地可重复的契约测试 |

## 5. 对比基线

唯一有效的沐曦参考仓库：`https://gitee.com/metax-maca/cu-bridge.git`
- 冻结基线：`master` / `6ab1076` / `3.8.0`
- 本地路径（如有）：`../cu-bridge-muxi`
- 运行 `python3 tools/compare_cubridge_parity.py` 时默认使用 `../cu-bridge-muxi`

对比文档见 [`04_对比与验证/`](04_对比与验证/)。

## 6. 注意事项

1. **只 push gitcode**，不要 push GitHub。
2. **真机命令只写 `tools/real_test/server_cmd.txt`**，不要新建轮次命令文件。
3. 代码修改后先 `git diff --check`、`bash -n`；真机验证用 `server_cmd.txt`。
4. **不要把 `__simt_vf__`、`cce::async_invoke` 或 hidden block 参数写入用户样例**——
   这些是已退出路线。
5. `__shared__` 在 main SIMT 适配中映射为 `__ubuf__`；同步/warp/原子原语通过
   `include/bridge/kernel/simt_adaptor.h` 做符号适配。
6. BLAS experimental 失败不改变主线结论。
7. `opencode.json` 授予 AI agent 全权限——如不需要 AI 辅助可删除。
8. `CLAUDE.md` 包含 AI 辅助开发的完整工作纪律，接手后建议保留或按需更新。

## 7. 文档导航

| 需要知道什么 | 去哪里看 |
|---|---|
| 项目如何工作 | [`01_架构设计/`](01_架构设计/) |
| 某个 CUDA API 能不能用 | [`02_能力清单/CUDA_OPERATOR_API_SUPPORT_MATRIX.md`](02_能力清单/CUDA_OPERATOR_API_SUPPORT_MATRIX.md) |
| cucc 参数如何处理 | [`01_架构设计/03_cucc_driver.md`](01_架构设计/03_cucc_driver.md) |
| CMake/Make 能力 | [`02_能力清单/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md`](02_能力清单/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md) |
| 不支持什么 | [`02_能力清单/LIMITS_AND_BOUNDARIES.md`](02_能力清单/LIMITS_AND_BOUNDARIES.md) |
| 某个文件应该改哪里 | [`03_文件目录/REPOSITORY_FILE_GUIDE.md`](03_文件目录/REPOSITORY_FILE_GUIDE.md) |
| 与 cu-bridge 的差异 | [`04_对比与验证/`](04_对比与验证/) |
| 历史方案为什么没继续 | [`99_归档/`](99_归档/) |
| 最终验证证据 | [`04_对比与验证/RELEASE_READINESS_2026-09-04.md`](04_对比与验证/RELEASE_READINESS_2026-09-04.md) |
