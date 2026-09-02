# 架构设计

本目录回答“代码为什么这样组织、一次编译/运行到底发生了什么”。阅读时先看全局，再按功能
深入；不要把 `99_归档/` 中的旧 converter 或 MACA 方案当作当前实现。

## 文档索引

| 文档 | 内容 |
|---|---|
| `01_global_overview.md` | 全局架构、三条主线和仓库地图 |
| `02_deep_dive.md` | 初始化、默认流、事件、同步、Kernel launch 和调试开关 |
| `03_cucc_driver.md` | `cucc` 的 phase、规则动作、参数注入和 query 语义 |
| `04_shadow_headers.md` | CUDA 兼容头、shadow libc、crt 前向层和 host/device 陷阱 |
| `05_cmake_design.md` | CMake 的 shim、mock 探测、override 和真实构建分流 |
| `06_cmake_principle_flow.md` | 一次 CMake configure/build 的逐层时序和失败模式 |
| `07_cucc_adaptability.md` | 规则驱动、策略层、可观测性和新增参数 SOP |
| `ASCEND_RESEARCH_NOTES.md` | CANN/SIMT/runtime 的事实记录 |
| `ASCEND_LIBRARY_MAPPING.md` | CUDA 库到 CANN/bridge 的映射策略 |
| `CUDA_TO_CCE_PORTING_PLAN.md` | 当前 puresimt 路线的迁移原则和验收边界 |

## 核心数据流

```text
用户 .cu / CMakeLists.txt / Makefile
        │
        ├─ cucc → RuleDriver → ccec
        └─ cmake_cce/make_cce/ninja_cce
             → fake toolkit + shim → 同一 cucc
        │
        ├─ CUDA 兼容头 / SIMT adaptor
        ├─ wcuda* → aclrt* bridge runtime
        └─ CANN / Ascend NPU
```

关键设计是“探测可以 mock，正式编译不能 mock”：CMake configure 只回答工具链身份和 ABI 探针，
真正的 `.cu` 编译、链接和最终产物仍由 `cucc`、`ccec` 和 host linker 完成。
