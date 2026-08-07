# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目定位

cu2cce-bridge（仿照 cu-bridge 的"用 CUDA 一门语言跑多硬件"理念，目标平台从沐曦 MXMACA 换成**华为昇腾 NPU / CANN**）：让 GPU 程序员只用 CUDA 编程语言写一个应用程序，就能生成跑在昇腾 NPU 上的目标文件。

**本机（macOS）没有昇腾硬件，也不安装 CANN，无法本地构建/运行**。本仓库在此只做代码阅读与开发（当前任务：仿照 cu-bridge 搭建 cu2cce-bridge 基础框架）。`cucc`/`make_cce` 等工具依赖 Linux + `ASCEND_TOOLKIT_HOME`（/usr/local/Ascend）下的 bisheng 编译器（昇腾的 clang 系编译器）与 ACL 库，切勿在本机执行构建。

**参考仓库**：`../cu-bridge`（沐曦开源，Apache-2.0，当前版本 3.8.0）。cu2cce-bridge 的目录结构与代码骨架刻意与 cu-bridge 对齐，便于逐文件对照移植。对照状态见 `docs/03_Comparison/mapping.md`。

## 核心原理（与 cu-bridge 的差异）

cu-bridge 的三条迁移线在昇腾上的对应（详见 `docs/01_Design/CUDA_TO_CCE_PORTING_PLAN.md`）：

1. **主机侧 Runtime API：宏符号替换 + wrapper 函数**（可近乎 1:1 照搬）
   - 用户代码 `#include <cuda_runtime.h>` → `include/cuda_runtime.h` → `include/__cuda_runtime.h` → `include/bridge/runtime/cuda_to_cce_mcr_adaptor.h`（宏表）
   - adaptor 宏表用 `#define cudaMalloc wcudaMalloc` 把 CUDA 符号替换为 cu2cce 的 `wcuda*` wrapper（`include/bridge/runtime/cuda_runtime_wrapper.h`）
   - `wcuda*` 实现在 `src/bridge/runtime/src/cuda_runtime_wrapper.cpp`，内部调用昇腾 ACL 原生 API（`wcudaMalloc` → `aclrtMalloc`，`wcudaMemcpy` → `aclrtMemcpy`，`wcudaGetDeviceCount` → `aclrtGetDeviceCount`）
   - 关键差异：**返回类型/句柄类型换成 acl 系**（`cudaError_t`→`aclError`、`cudaStream_t`→`aclrtStream`、`cudaEvent_t`→`aclrtEvent`）；`cudaDeviceProp` 由 `_cceDeviceProp_t` 模拟（昇腾无同形结构）
2. **主机侧 Driver API（cuMalloc 等）**：昇腾无同形 CUDA Driver API，映射表大幅缩水，**当前未做**（Phase A2+）
3. **设备侧 kernel**：这是 cu-bridge 与 cu2cce 的最大差异
   - cu-bridge 靠 mxcc（clang 系）原生认识 CUDA SIMT kernel 语法，**不需要转换**
   - 昇腾 bisheng 编译器认识 `__global__ __aicore__` + `<<<>>>` 语法；且 **Ascend 950PR/950DT 架构（`--npu-arch=dav-310`）的 AI Core 原生支持 SIMT 编程模型**（warp=32、`threadIdx`/`blockIdx`、`asc_syncthreads`/`asc_shfl` 等），与 CUDA 高度同构
   - **D0 已定（2026-08-07）：目标硬件 = 950 系列 dav-310 → kernel 直通编译（路线 A）**，仅需一个 SIMT 适配宏表（`__syncthreads`→`asc_syncthreads`、`__shared__`→`__ubuf__` 等）；老型号（910B/910C，无 SIMT）才需要 SIMT→tile 转换层（已排除）
   - **当前 Phase A1 未处理 kernel 启动（`<<<>>>`→aclrtLaunchKernel），留待 Phase A3**

## 关键目录

- `include/` — CUDA 头文件替代品（cuda_runtime.h、cuda_runtime_api.h、__cuda_runtime.h、bridge/ 等），编译时通过 `-I$CUDA_PATH/include`（CUDA_PATH 指向 cu2cce-bridge 安装目录）注入
- `include/bridge/runtime/` — `cuda_to_cce_mcr_adaptor.h`（核心宏表）、`cuda_runtime_wrapper.h`（wcuda* 声明）、`cuda_wrapper_types.h`（_cceDeviceProp_t）、`cuda_wrapper_internal.h`（bridge_init）、`cuda_benchmark_targets.h`（版本/导出宏）
- `include/bridge/kernel/` — `simt_adaptor.h`（Phase A3：AI Core SIMT 适配宏表，`__syncthreads`→`asc_syncthreads`、`__shared__`→`__ubuf__`）
- `src/bridge/runtime/src/` — `cuda_runtime_wrapper.cpp`（wcuda* → aclrt* 实现）
- `bin/cucc` — nvcc 兼容编译器驱动骨架（Phase A2，解析 nvcc 参数、找 bisheng、注入 include/link、`--npu-arch=dav-*`、`-DCUDA_VERSION`）
- `tools/` — （规划）make_cce / cmake_cce / mock，Phase A2+
- `docs/01_Design/` — 技术方案（CUDA_TO_CCE_PORTING_PLAN.md）、昇腾调研笔记（ASCEND_RESEARCH_NOTES.md）、Claude 工作区方案（CLAUDE_WORKSPACE_PLAN.md）
- `docs/03_Comparison/` — 与 cu-bridge 的逐文件对照表（mapping.md，开发期核心工作文件）

## 环境变量（真机编译/运行时）

- **CANN 版本基准：9.x 系列**（发布版 9.1.0，用户服务器 9.2.0）。API 用 9.x 新增面：`aclrtLaunchKernelV2`、`aclrtCreateEventWithFlag`、AI Core SIMT；不用 8.x 旧命名。
- `ASCEND_TOOLKIT_HOME=/usr/local/Ascend/ascend-toolkit/latest` — CANN 根目录（bisheng 在 `compiler/ccec_compiler/bin/`，ACL 头在 `include/`，库在 `lib64/`）
- `CUCC_PATH` — cu2cce-bridge 安装位置；`CUDA_PATH=${CUCC_PATH}`（让 `-I$CUDA_PATH/include` 落到 cu2cce）
- `CUCC_NPU_ARCH` — bisheng NPU 架构参数（**D0 已定：`dav-310`，对应昇腾 950 系列 Ascend 950PR/950DT**，AI Core 原生支持 SIMT）
- `CUCC_CUDA_VERSION` — 模拟的 CUDA 版本（默认 11060）
- `source ${ASCEND_TOOLKIT_HOME}/bin/setenv.bash` — 配置 CANN 环境

## 构建与使用（仅 Linux + CANN，本机不可执行）

```bash
# 构建 wrapper 库（真机）
export ASCEND_TOOLKIT_HOME=/usr/local/Ascend/ascend-toolkit/latest
source ${ASCEND_TOOLKIT_HOME}/bin/setenv.bash
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=${ASCEND_TOOLKIT_HOME}/tools/cu2cce-bridge ..
make && make install

# 迁移一个 CUDA 工程（真机，Phase A2 完成后）
# cd vectorAdd && cucc vectorAdd.cu -o vectorAdd
```

## 已知差异（移植时最值得关注）

| 差异 | 说明 |
|---|---|
| `aclrtMemcpy` 多一个 `destMax` 参数 | CUDA `cudaMemcpy(dst,src,count,kind)` 无此参数，wrapper 填 count |
| `aclrtMalloc` 有 policy 参数 | 第三参 `aclrtMemMallocPolicy`（大页/普通页）；CUDA 无 |
| ACL 需先 `aclInit` | wrapper 的 bridge_init 中调用一次（惰性初始化） |
| `aclrtMalloc` 不初始化内存 | CUDA 默认清零，昇腾需自行 aclrtMemset（对齐 cu-bridge 的规避策略） |
| 错误码枚举 | `cudaError*` 枚举 → `aclError`（int），目前只映射常用几个，需补全 |
| kernel 启动 | `<<<>>>` → aclrtLaunchKernel（Phase A3）；SIMT 型号可直通 |
| warp/块上限 | SIMT：warp=32、block 内 2048 线程；老型号需 tile 化 |

## 开发约定

1. 仿照 cu-bridge：新文件先对照 `../cu-bridge` 对应文件，在 `docs/03_Comparison/mapping.md` 记一行状态再动手
2. 本机只读不构建：任何 `cmake`/`make`/`cucc`/`bisheng`/`msopgen` 类命令在本机一律禁止执行
3. 新增 wrapper 函数照抄 cu-bridge 声明行，把 `mc*` 类型换成 `acl*` 类型
4. 昇腾 API 事实不确定时先查 `docs/01_Design/ASCEND_RESEARCH_NOTES.md`（已收录 ACL/SIMT 调研笔记），再查官方文档，不要臆造 API 名
