# cu2cce-bridge 文档中心

本目录是 `cu2cce-bridge` 的中文技术文档中心。项目把 CUDA `.cu` 源码、常用 CUDA Runtime
API 以及 CMake/Make/Ninja 构建入口适配到昇腾 NPU 的 CANN `ccec` + SIMT 路线。

> **文档原则**：文档正文使用中文；`cucc`、`ccec`、`CUDA`、`CANN`、`Ascend`、API、命令行
> 选项、文件名和代码符号保留原文，确保可以直接复制执行或在代码中搜索。

## 推荐阅读顺序

### 使用者

1. 根目录 [`README.md`](../README.md)：项目定位、安装树和边界。
2. 根目录 [`QUICKSTART.md`](../QUICKSTART.md)：从准备 CANN 到运行 CUDA 示例的操作步骤。
3. [`02_能力清单/README.md`](02_能力清单/README.md)：判断 CUDA Kernel、API、库和构建系统是否支持。

### 维护者

1. [`01_架构设计/README.md`](01_架构设计/README.md)：了解编译、头文件、运行时和构建适配的原理。
2. [`03_文件目录/README.md`](03_文件目录/README.md)：按问题定位到代码、脚本、配置和测试。
3. [`03_文件目录/REPOSITORY_FILE_GUIDE.md`](03_文件目录/REPOSITORY_FILE_GUIDE.md)：逐文件职责和调用链。
4. [`04_对比与验证/README.md`](04_对比与验证/README.md)：查看与沐曦 `cu-bridge` 的机制对比、审计和真机证据。

## 目录结构

```text
docs/
├── README.md              本文件：中文导航、术语和维护规则
├── 01_架构设计/           编译链、cucc、CMake、Runtime、兼容头和 CANN 原理
├── 02_能力清单/           Kernel/API/库/cuсc/CMake/Make/Ninja/产物的支持边界
├── 03_文件目录/           仓库文件职责、调用链和故障定位
├── 04_对比与验证/         cu-bridge 对比、产物审计、review、真机回归结论
└── 99_归档/               已完成计划、旧版设计和外部参考资料
```

## 文档权威关系

| 问题 | 唯一优先入口 |
|---|---|
| 项目如何工作 | `01_架构设计/` 中的架构与机制文档 |
| 某个 CUDA Kernel/API 能不能用 | `02_能力清单/CUDA_OPERATOR_API_SUPPORT_MATRIX.md` |
| cucc 参数如何处理 | `02_能力清单/CUCC_OPTION_COMPATIBILITY_MATRIX.md`、`01_架构设计/03_cucc_driver.md` |
| CMake/Make/Ninja 能力和产物 | `02_能力清单/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md` |
| 明确不支持的能力 | `02_能力清单/LIMITS_AND_BOUNDARIES.md` |
| 某个文件应该改哪里 | `03_文件目录/REPOSITORY_FILE_GUIDE.md` |
| 与 cu-bridge 的差异和验证证据 | `04_对比与验证/`（入口：[`README.md`](04_对比与验证/README.md)） |
| 历史方案为什么没有继续 | `99_归档/` |

## 当前正式技术路线

```text
CUDA .cu
  → bin/cucc / tools/cmake_cce / tools/make_cce / tools/ninja_cce
  → tools/cu2cce_driver.py + config/cu2cce_rules.json
  → ccec -x asc --enable-simt --cce-aicore-arch=dav-*
  → CUDA 兼容头 + libcce_bridge_runtime.so + CANN runtime
  → Ascend NPU
```

CMake 的 configure 阶段使用 fake toolkit 和 `mock_nvcc.py` 满足 CMake 探测，build 阶段仍然
调用真实 `cucc`；Make/Ninja/pre wrapper 复用 `fake_toolkit_env.sh` 和
`build_shim_tree.py`。这是对齐 `cu-bridge` 的机制，而不是复制 MACA 的库名和设备产物。

## 状态和证据规则

- `SUPPORTED`：主线实现并有对应测试或真机证据。
- `EXPERIMENTAL`：实现存在，验证范围或 API 形状有限，不能扩大承诺。
- `DIAGNOSTIC_ONLY`：用于暴露后端差异，不阻塞主线验收。
- `UNSUPPORTED`：明确拒绝、返回不支持或没有安全等价物。
- 文档不得把本地 mock 结果写成 CANN/NPU 真机结果；真机入口是
  `tools/real_test/server_cmd.txt`。
- 不为实现“表面 parity”伪造 `mxcc`、MACA runtime、PTX/CUBIN/FATBIN、RDC/device-link
  或未验证的 CUDA 库。

历史计划示例：`docs/99_归档/CUDA_LARGE_PROJECT_COMPATIBILITY_PLAN.md`。

## 维护规则

1. 改变代码能力时，先更新 `02_能力清单/` 对应矩阵，再补测试和实现说明。
2. 改变构建入口或产物时，同时更新构建矩阵、文件目录和真机验证说明。
3. 新增或移动文件时，更新 `03_文件目录/REPOSITORY_FILE_GUIDE.md` 和本导航。
4. 历史计划保留在 `99_归档/`，不要重新创建顶层 `plans/`、临时日志或构建目录。
5. 每次可交付改动完成后，按仓库约定提交并推送 `gitcode/main`；服务器上先同步、清理，再执行
   `tools/real_test/server_cmd.txt`。

## 术语约定

- **puresimt**：让 `ccec` 直接解析 CUDA SIMT 写法的当前正式路线。
- **bridge runtime**：`libcce_bridge_runtime.so` 及其对 CANN runtime 的封装。
- **fake toolkit/shim**：只提供 CUDA 形状的目录、入口和库别名，供构建系统探测或转发；不等同于
  完整 CUDA Toolkit。
- **产物对齐**：对齐 `.o`、`.d`、`.a`、`.so`、ELF executable、构建元数据和动态依赖的类别与语义，
  不要求 Ascend 与 MACA 的 ELF 字节级一致。
