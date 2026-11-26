# 对比与验证

本目录记录与沐曦 `cu-bridge` 的机制对比、逐项 parity 审计、code review、设计评审和真机
验收结论。这里的“对齐”指职责、调用链、参数转发和产物类别对齐，不表示 Ascend 复制
MACA 的编译器、库名或设备产物。

## 文档索引

| 文档 | 用途 |
|---|---|
| `CMAKE_MAKE_ALIGNMENT_WITH_CUBRIDGE.md` | CMake/Make/Ninja/pre wrapper 的机制对照 |
| `CUBRIDGE_CMAKE_MAKE_DESIGN_ANALYSIS.md` | 对 cu-bridge 构建策略的逐机制分析 |
| `CU2CCE_VS_CUBRIDGE_REVIEW.md` | 两个项目的当前实现差异审查 |
| `CURRENT_STATE.md` | 当前 `main / puresimt` 状态快照 |
| `RELEASE_READINESS_2026-09-04.md` | 当前最终验证、产物契约、API/header parity 与真机 full profile 证据（不创建 release/tag） |
| `mapping.md` | CUDA、cu-bridge 和 Ascend 映射摘要 |

历史过程文档（`CODE_REVIEW_*`、`PARITY_AUDIT_*`、`RELEASE_READINESS_2026-09-02`）已归档至 [`../99_归档/`](../99_归档/)。

## 验证优先级

1. 代码和规则文件是实现事实。
2. `tests/` 是本地可重复的契约证据。
3. `tools/real_test/server_cmd.txt` 是 Ascend 真机验收入口。
4. 本目录的矩阵和审计记录解释证据及限制。
5. 归档文档只用于历史追溯，不覆盖当前结论。

## 结果分类

- **主线阻塞通过**：会影响 `FAIL=0` 和正式回归标志的测试通过。
- **诊断通过/失败**：`DIAG-ONLY`，用于记录后端差异，不覆盖主线结论。
- **未支持能力**：按文档明确拒绝或告知限制，不通过伪造库、参数或中间产物绕过。
- **待真机确认**：本地静态/mock 已通过，但尚未取得对应 CANN/NPU 证据。
