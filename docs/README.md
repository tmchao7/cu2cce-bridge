# cu2cce-bridge 文档导航

## 推荐阅读顺序

**新手**：仓库根 `QUICKSTART.md`（拿到项目 → 构建产物 → 产物在哪 → cucc
跑单文件 → cmake_cce 跑大项目）。

**进阶**（按序）：

1. `02_Architecture/01_global_overview.md`：架构全局认知（10 分钟全景图 + 仓库地图）。
2. `02_Architecture/03_cucc_driver.md`：cucc 编译驱动详解（五阶段/规则五动作/注入链/FP64 策略/CLI 模式）。
3. `02_Architecture/04_shadow_headers.md`：shadow 头机制与 libc 双模式（含 crt 前向层与两个宿主/设备同名陷阱）。
4. `02_Architecture/05_cmake_design.md`：CMake 设计及其能力（三层漏斗/能力矩阵/设计要点）。
5. `02_Architecture/02_deep_dive.md`：运行时深度解析（BRIDGE_INIT/默认流语义/事件注册表/launch 路径/平台事实清单）。
6. `03_Scope/SUPPORTED_SCOPE.md`：支持范围总览（能力正面清单）。
7. `03_Scope/LIMITS_AND_BOUNDARIES.md`：边界与限制（反面清单唯一权威入口，含证据与"为什么不绕过"）。
8. `03_Comparison/`：各能力矩阵（PURESIMT / BUILD_SYSTEM / CUCC_OPTION / BLAS / mapping / CU2CCE_VS_CUBRIDGE_REVIEW / CURRENT_STATE）。
9. `01_Design/ASCEND_RESEARCH_NOTES.md`：CANN/SIMT/runtime 事实库。
10. `reference/asc-devkit-simt/`：Ascend SIMT API 参考头和示例。
11. `tools/real_test/diag/`：历史 runtime context 排障工具和案例。

## 目录结构

```text
docs/
├── 01_Design/            ASCEND_RESEARCH_NOTES（事实库）、CUDA_TO_CCE_PORTING_PLAN（历史方案）
├── 02_Architecture/      架构五篇（全局/深度/驱动/shadow/cmake）← 架构认知权威入口
├── 03_Scope/             支持范围总览 + 边界与限制 ← 范围/边界权威入口
├── 03_Comparison/        能力矩阵与对比（真机证据）
├── archive/              v1 设计文档存档（被 02_Architecture 取代）
└── reference/            Ascend SIMT API 参考
```

## 文档边界

- `02_Architecture/` 面向原理、机制与内部实现（当前权威版本；v1 存档在 archive/）。
- `03_Scope/` 面向"能用/不能用"（汇总层，逐项证据指向 03_Comparison 矩阵）。
- `03_Comparison/` 面向功能状态与真机证据明细。
- `01_Design/ASCEND_RESEARCH_NOTES.md` 面向底层事实和 API 核查。
