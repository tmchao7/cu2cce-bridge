---
description: 重扫 mapping.md，列出所有"未动/移植中"条目，建议下一步
argument-hint: (可选) 指定阶段 Phase A1/A2
---

# /sync-status

对照开发进度检查。

流程：
1. 读 `docs/03_Comparison/mapping.md`。
2. 列出所有状态为"未动"或"移植中"的条目。
3. 按当前 Phase 建议下一个该做的条目（"未动"优先，ID 升序）。
4. 若指定了 Phase，只列出该 Phase 的条目。
