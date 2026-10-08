# 架构全局认知（Global Overview）

> 面向第一次接触本仓库的工程师：10 分钟建立全局图景。
> 深入细节见同目录 `02_deep_dive.md` / `03_cucc_driver.md` / `05_cmake_design.md`。

## 1. 一句话定位

**cu2cce-bridge 把原始 CUDA 程序（`.cu` + CMake/Make 工程，源码零改写）迁移到
昇腾 NPU 上编译运行**：编译侧用 CANN `ccec --enable-simt` 直接解析 CUDA SIMT
内核（puresimt 路线），运行侧用 `wcuda* → aclrt*` 兼容层承接 CUDA Runtime API。

## 2. 全景图

```text
┌────────────────────────── 用户侧（不感知桥的存在） ──────────────────────────┐
│  原始 .cu / main.cpp / CMakeLists.txt / Makefile                              │
└──────────────┬───────────────────────────────┬──────────────────────────────┘
               │ cucc（nvcc 兼容驱动）          │ cmake_cce / make_cce / ninja_cce
               ▼                               ▼
┌────────────────────────── 桥接层（本仓库） ─────────────────────────────────┐
│ 编译：bin/conf.json 规则驱动 → ccec -x asc --enable-simt           │
│ 头文件：include/cuda_runtime.h → __cuda_runtime.h（宏映射 cuda*→wcuda*）      │
│         + bridge/kernel/simt_adaptor.h（执行空间/共享内存/warp 原语映射）      │
│         + crt/*（前向兼容层，见 04_shadow_headers.md）                         │
│ 运行时：libcce_bridge_runtime.so（wcuda* → aclrt*，含默认流语义/事件注册表）   │
└──────────────┬──────────────────────────────────────────────────────────────┘
               ▼
        CANN ccec / libascendcl（昇腾 NPU）
```

## 3. 三条主线（对应三个子目录的能力）

| 主线 | 编译面 | 运行面 | 状态 |
|---|---|---|---|
| puresimt 直通 | cucc 规则驱动 + simt_adaptor | wcuda 运行时 | **必需门禁**：23 个真机用例全绿 |
| CMake 大项目 | cmake_cce + mock nvcc + override 模块 | 同上 + FindCUDAToolkit 别名族 | **必需门禁**：6 案例 + 外部官方样本 |
| cuBLAS 实验 | cublas_v2.h 宏映射 | wcublas* → aclblas* | 实验线：三用例真机全绿（opt-in） |

## 4. 仓库地图（拿项目先看哪里）

```text
bin/                    cucc / cmake_cce / make_cce / ninja_cce 入口（安装后产物）
include/                用户可见头：cuda_runtime.h、crt/、cublas_v2.h、shadow/
include/bridge/         桥内部头：runtime wrapper、kernel simt_adaptor、blas
src/bridge/runtime/     wcuda* → aclrt* 实现（默认流/事件注册表/FP64 策略）
src/bridge/blas/        wcublas* → aclblas* 实现（实验线）
bin/conf.json  cucc 参数规则（数据驱动核心）
tools/cu2cce_driver.py    规则解释器（cucc 与 mock nvcc 共用）
tools/mock/mock_nvcc.py   CMake 探测应答器（虚拟工具链 nvcc）
tools/cmake_cce 等包装器   CMake/Make/Ninja 适配入口
cmake_modules/            FindCUDAToolkit / override / FindCUDA
examples/                 各场景样例（puresimt 最小集 / cmake 案例集 / blas）
tools/real_test/          真机回归 harness（三画像）+ 清单/取证工具
docs/                     本文档树
```

## 5. 与 cu-bridge（参照项目）的关系

参照其构建适配机制（mock nvcc、Make/Ninja 包装、wcuda 分层），**不复制**其
MACA 映射与 ptx2cpp 降级管线；本桥选择 puresimt 直通。详细对比见
`../04_对比与验证/CU2CCE_VS_CUBRIDGE_REVIEW.md` 与
`../99_归档/MENTOR_REPORT_2026-08-28.md` §2。

## 6. 阅读路径建议

- 快速上手跑通：仓库根 `QUICKSTART.md`
- 支持范围与边界：`../02_能力清单/`
- 运行时内部机制：`02_deep_dive.md`
- cucc 参数与策略：`03_cucc_driver.md`
