# cu2cce-bridge 文档导航

## 推荐阅读顺序

1. `01_Design/PROJECT_ARCHITECTURE.md`：项目原理、设计哲学、编译/运行逻辑和学习入口。
2. `03_Comparison/PURESIMT_COMPATIBILITY_MATRIX.md`：23 个真机用例和 CUDA API 状态矩阵。
3. `03_Comparison/CURRENT_STATE.md`：当前简要状态。
4. `03_Comparison/mapping.md`：与 cu-bridge 的模块映射。
5. `01_Design/ASCEND_RESEARCH_NOTES.md`：CANN/SIMT/runtime 事实库。
6. `reference/asc-devkit-simt/`：Ascend SIMT API 参考头和示例。
7. `历史 runtime context 排障记录`：历史 runtime context 排障案例。

## 文档边界

- `PROJECT_ARCHITECTURE.md` 面向学习、mentor 评审和团队 onboarding。
- `PURESIMT_COMPATIBILITY_MATRIX.md` 面向功能状态和真机证据。
- `ASCEND_RESEARCH_NOTES.md` 面向底层事实和 API 核查。
- - `.claude/`、根目录 `CLAUDE.md` 和 agent/command 文件属于开发协作配置，不属于
  对外项目设计文档，本目录不重复解释它们。
