---
description: 对照 cu-bridge 的某个文件，产出 cu2cce-bridge 移植要点并更新 mapping.md
argument-hint: <cu-bridge 文件路径，如 include/bridge/runtime/cuda_runtime_wrapper.h>
---

# /compare

对照开发入口：给定 cu-bridge 的一个文件路径，调用 porting-mapper 产出移植要点。

用法：
```
/compare include/bridge/runtime/cuda_runtime_wrapper.h
```

流程：
1. 校验路径存在于 `../cu-bridge/`。
2. 读 cu-bridge 源文件 + `docs/03_Comparison/mapping.md` 当前状态。
3. 产出移植要点（照搬/换符号/重写/跳过）。
4. 更新 mapping.md 对应行状态为"移植中"。
