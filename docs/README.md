# cu2cce-bridge 文档导航

## 推荐阅读顺序

**新手**：仓库根 `README.md`（二进制安装树和入口）→ `QUICKSTART.md`（完整操作步骤）。
发布前先看 `03_Comparison/RELEASE_READINESS_2026-09-02.md`；维护定位先看
[`REPOSITORY_FILE_GUIDE.md`](REPOSITORY_FILE_GUIDE.md)。

**进阶**（按序）：

1. `02_Architecture/01_global_overview.md`：架构全局认知（10 分钟全景图 + 仓库地图）。
2. `02_Architecture/03_cucc_driver.md`：cucc 编译驱动详解（五阶段/规则五动作/注入链/FP64 策略/CLI 模式）。
3. `02_Architecture/04_shadow_headers.md`：shadow 头机制与 libc 双模式（含 crt 前向层与两个宿主/设备同名陷阱）。
4. `02_Architecture/05_cmake_design.md`：CMake 设计及其能力（三层漏斗/能力矩阵/设计要点）。
5. `02_Architecture/06_cmake_principle_flow.md`：CMake 原理与完整工作流（一次 configure 的逐层拆解 + 真实命令 trace + 失败模式表）。
6. `02_Architecture/07_cucc_adaptability.md`：cucc 能力现状与反硬编码论证（三层适配模型：153 条 JSON 规则 / 策略层清单 / 开关层全表；"新增 flag 零代码"实操）。
7. `02_Architecture/02_deep_dive.md`：运行时深度解析（BRIDGE_INIT/默认流语义/事件注册表/launch 路径/平台事实清单）。
8. `03_Scope/SUPPORTED_SCOPE.md`：支持范围总览（能力正面清单）。
9. `03_Scope/LIMITS_AND_BOUNDARIES.md`：边界与限制（反面清单唯一权威入口，含证据与"为什么不绕过"）。
10. `03_Comparison/`：各能力矩阵、算子/API/构建总表、发布前收口审计（PURESIMT / CUDA_OPERATOR_API_SUPPORT / BUILD_SYSTEM / CUCC_OPTION / BLAS / mapping / CURRENT_STATE / RELEASE_READINESS）。
11. `01_Design/ASCEND_RESEARCH_NOTES.md`：CANN/SIMT/runtime 事实库。
12. `reference/asc-devkit-simt/`：Ascend SIMT API 参考头和示例。
13. `REPOSITORY_FILE_GUIDE.md`：当前每个 Git 跟踪文件的职责、调用链和定位方法。
14. `tools/real_test/diag/`：历史 runtime context 排障工具和案例。

## 目录结构

```text
docs/
├── 01_Design/            ASCEND_RESEARCH_NOTES（事实库）、CUDA_TO_CCE_PORTING_PLAN（历史方案）
├── 02_Architecture/      架构五篇（全局/深度/驱动/shadow/cmake）← 架构认知权威入口
├── 03_Scope/             支持范围总览 + 边界与限制 ← 范围/边界权威入口
├── 03_Comparison/        能力矩阵与对比（真机证据）
├── archive/              已完成计划、review 记录与 v1 设计文档存档
└── reference/            Ascend SIMT API 参考
```

## 文档边界

- `02_Architecture/` 面向原理、机制与内部实现（当前权威版本；v1 存档在 archive/）。
- `03_Scope/` 面向"能用/不能用"（汇总层，逐项证据指向 03_Comparison 矩阵）。
- `03_Comparison/` 面向功能状态、产物证据、真机结论和发布审计。
- `REPOSITORY_FILE_GUIDE.md` 面向维护者的文件级导航；每次新增/移动文件必须同步更新。
- `03_Comparison/CUDA_OPERATOR_API_SUPPORT_MATRIX.md` 是当前算子、Kernel 写法、API 映射和构建能力总表。
- 已完成的大项目与 review 计划归档在 `docs/archive/`，例如
  `docs/archive/CUDA_LARGE_PROJECT_COMPATIBILITY_PLAN.md`。
- `01_Design/ASCEND_RESEARCH_NOTES.md` 面向底层事实和 API 核查。

## 发布卫生

顶层不保留临时 `plans/` 目录；已完成计划归档到 `docs/archive/`。`build/`、日志、
`.mimosa/`、`.worktrees/` 和 `.cu2cce-cache/` 只属于本地或真机运行时，均由
`.gitignore` 排除。新增功能文档先更新本导航，再更新根 `README.md` 或
`QUICKSTART.md`，避免出现多个互相矛盾的入口。
