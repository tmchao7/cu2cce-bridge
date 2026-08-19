# cu2cce-bridge 技术方案：把 cu-bridge 的迁移方法搬到昇腾 CANN/CCE

> 状态：v0.3（D0 已定稿：昇腾 950 系列 dav-c310 → 路线 A）
> 对照基准：cu-bridge 3.8.0（沐曦 MXMACA）
> 目标栈：华为昇腾 NPU，CANN 工具链，Ascend C / AI Core SIMT；bisheng `--npu-arch=dav-c310`
> 说明：本机（macOS）无昇腾硬件、未安装 CANN，文档事实基于 CANN 官方文档检索（2026-08），需在真机上二次核对。

---

## 0. 结论先行（TL;DR）

- **目标**：让 GPU 程序员只用 CUDA 写一份源码，生成跑在昇腾 NPU 上的目标文件。
- **重大前提变化（v0.2 调研结论）**：昇腾 **Ascend 950PR / 950DT 架构的 AI Core 原生支持 SIMT 编程模型**——`__global__`、`threadIdx/blockIdx/blockDim/gridDim`、`<<<>>>` 语法糖、warp=32、共享内存、原子操作、同步原语一应俱全（见 §2.2）。因此 **cu-bridge 的三条迁移线在 SIMT 型号上可以几乎 1:1 照搬**，原 v0.1 假设"昇腾非 SIMT、必须加 kernel 转换层"**只在老型号（无 SIMT）上成立**。
- **核心决策 D0（已定稿 2026-08-07）：目标硬件 = 昇腾 950 系列（Ascend 950PR/950DT），bisheng 架构参数 `--npu-arch=dav-c310`。** 950 系列 AI Core **原生支持 SIMT 编程模型**（见 §2.2），因此锁定 **路线 A**：
  - **路线 A（已锁定）**：三条线照搬 + 一个"SIMT 适配宏表"（`__syncthreads`→`asc_syncthreads`、`__shared__`→`__ubuf__` 等），kernel 由 bisheng（clang 系）原生编译。**难度大幅降低**，与 cu-bridge 实现几乎同构。
  - ~~路线 B（910B/910C 老架构，无 SIMT）~~：已排除（目标非此架构）；若未来要支持老型号再启用 SIMT→tile 转换层。
- **参考资产（调研新发现）**：社区已有 `caas-cli`（CUDA→CANN 源码转换命令行工具，含 API 映射表）、`ops-blas`（开源昇腾 BLAS）、`cann/asc-devkit`（官方算子开发工程，含 SIMT 示例）。cu2cce-bridge 可作为这些资产的"编译器驱动 + 宏表 + 兼容头"层的整合者。

| cu-bridge 三线 | 路线 A（SIMT 型号） | 路线 B（老型号） |
|---|---|---|
| ① Host Runtime API（宏表+wcuda* wrapper→mc*） | 照搬，实现换 aclrt* | 照搬，实现换 aclrt* |
| ② Driver API（cu*→mc*） | 缩水（昇腾无同形 Driver） | 缩水 |
| ③ 设备侧 kernel | bisheng 原生编译 + SIMT 适配宏表 | **需 SIMT→Ascend C tile 转换层** |

---

## 1. cu-bridge 三线迁移机制回顾（对照基准）

（保持 v0.1 内容，见 git 历史。要点：① 宏表 `#define cudaMalloc wcudaMalloc` → wrapper 声明 → wrapper 实现调 mc*；② `cu*`→`mc*` 调用转换；③ kernel 由 mxcc（clang 系）原生编译，**这是 cu-bridge 最省事也最依赖"目标编译器认 CUDA 语法"的一环**；④ 工程胶水：bin/cucc 驱动 + make_maca/cmake_maca + cmake mock + ptx2cpp。）

---

## 2. 昇腾目标栈事实核对（2026-08 调研结果）

### 2.1 Host 侧 Runtime = AscendCL（ACL）✅ 已确认

- 头文件：`acl/acl_rt.h`、`acl/acl_base.h`、`acl/acl_cblas.h`（BLAS）。
- 内存：`aclrtMalloc` / `aclrtFree` / `aclrtMemcpy` / `aclrtMemcpyAsync`；方向枚举 `aclrtMemcpyKind`。
- 设备：`aclrtSetDevice` / `aclrtGetDeviceCount` / `aclrtGetDeviceProperties`。
- 流/事件：`aclrtCreateStream` / `aclrtDestroyStream` / `aclrtCreateEvent` / `aclrtRecordEvent` / `aclrtSynchronizeStream`。
- 错误码：`aclError`（`ACL_SUCCESS`）。
- **Kernel 启动（C3 已确认）**：三种方式——
  1. `kernel_name<<<blockDim, l2ctrl, stream>>>(args...)`（语法糖，l2ctrl 固定 nullptr）；
  2. `ACLRT_LAUNCH_KERNEL(kernel_name)(blockDim, stream, args...)` 宏；
  3. 显式加载：`aclrtLoadBinary` → `aclrtBinaryGetFunction` → `aclrtLaunchKernel(funcHandle, blockDim, argsData, argsSize, stream)`（argsData 为**设备内存**参数区）。
  - 编译时自动生成 `aclrtlaunch_{kernel}.h` 提供启动 API。`blockDim` 取值 [1,65535]，按 Vector(AIV)/Cube(AIC) 核数设置。

### 2.2 设备侧 kernel = AI Core SIMT（C1/C2 ✅ 重大确认）

- **硬件**：Ascend 950PR / 950DT 架构的 AIV 向量核同时支持 SIMD 与 SIMT；**Atlas A2/A3（含 910B/910C）不支持 SIMT**。
- **语法与 CUDA 高度同构**：
  | CUDA | 昇腾 SIMT |
  |---|---|
  | `__global__` | `__global__`（同名） |
  | `__syncthreads()` | `asc_syncthreads()`（头文件 `simt_api/device_sync_functions.h`） |
  | `__shared__` | `__ubuf__` |
  | 全局内存 | `__gm__`（SIMT 下可直接读写 GM，无需 tile 搬移） |
  | `threadIdx/blockIdx/blockDim/gridDim` | 同名内置变量（另有函数形式 `AscendC::Simt::GetThreadIdx()<<0>>()`） |
  | `<<<grid,block,stream>>>` | `<<<blocks_per_grid, threads_per_block, stream>>>`（块内最多 2048 线程，gridDim 乘积 ≤65535，warp=32） |
  | `__shfl_*` | `asc_shfl` |
  | `atomicAdd` 等 | `asc_atomic_add` / `asc_atomic_sub` / `asc_atomic_exch` |
  | `__launch_bounds__` | `__launch_bounds__`（同名） |
  | `sin/cos/exp/half/half2/bfloat16` | 支持 |
- **限制**：仅支持向量计算（**不支持 Cube 矩阵运算 / SIMD+SIMT 融合**）；仅特定型号（Atlas 350 / 推理系列）。
- **编译器（C1）**：bisheng 编译器（`${INSTALL_DIR}/compiler/ccec_compiler/`），**本质是 clang 封装**（`bin/clang`、`lld`、`cce-ld` 等），支持 `-I/-L/-l/-D`、`--npu-arch=dav-<arch>`（新）/ `--cce-soc-version=AscendXXXXY`（旧）、`--std=c++17`、`-xcce`；支持 Host+Device 异构编译（host .cpp + kernel 同文件）。
- **其他执行模式**：`__global__ __aicpu__`（AI CPU 内核）；`ICPU_RUN_KF`（CPU 模拟验证）。

### 2.3 库（C4/C5 部分确认）

- **BLAS（C4 ✅）**：头文件 `acl/ops/acl_cblas.h`，库 `libacl_cblas.so`；`aclblasSgemm`/`aclblasGemmEx`/`aclblasHgemm`/`aclblasS8gemm` 等，handle 与非 handle 两种风格；开源实现 `cann/ops-blas`。→ **cublas 桥有对应物，但签名有差异（需 wrapper 适配），且 GemmEx 数据类型组合受限**。
- **cudnn/curand/cufft（C5）**：昇腾无同名 API 级对应。910B 兼容层方案提到"内置 cuDNN 兼容算子库，2000+ 通用算子"（算子级，非 cudnn API 级）。→ 后置，需逐个定制或跳过。

### 2.4 构建与集成（C6 ✅ 确认）

- 环境：`source ${INSTALL_DIR}/bin/setenv.bash`（或 `PATH+=${INSTALL_DIR}/compiler/ccec_compiler/bin`）。
- 头文件 `${INSTALL_DIR}/include/`，库 `${INSTALL_DIR}/lib64/`。
- CMake：昇腾算子工程用 CMakePresets.json + build.sh；asc-devkit 用 `CMAKE_ASC_ARCHITECTURES` 配置 NPU 架构。→ cucc 驱动需把 `--offload-arch` 翻译成 `--npu-arch=dav-*` / `--cce-soc-version`。

### 2.5 社区参考资产（调研新发现）

| 资产 | 说明 | 与 cu2cce-bridge 的关系 |
|---|---|---|
| `caas-cli` | 开源命令行：`caas analyze`（PyTorch 兼容性分析）/ `caas convert`（.cu→.cpp，API 映射 cudaMalloc→aclrtMalloc 等，含 ascend/cambricon/hygon 多平台映射表） | **可借鉴其 API 映射表**（license 待核实）；cu2cce 可做其"编译器驱动"的替代 |
| `cann/asc-devkit` | CANN 官方算子开发工程，`examples/03_simt_api` 有 SIMT 样例 | 真机验证的现成测试基线 |
| `cann/ops-blas` | 开源昇腾 BLAS | aclblas 实现参照 |
| Triton→Ascend 转换（MindSpore/akg） | 模型级转换 | 参考思路，不直接复用 |

---

## 3. 工作路径（D0 已定：950 系列 dav-c310 → 路线 A）

### 路线 A — SIMT 型号：cu-bridge 三线照搬（已锁定）

> 前提 ✅：目标硬件是 Ascend 950 系列（950PR/950DT，bisheng `--npu-arch=dav-c310`），AI Core 原生支持 SIMT。

- **Phase A0**：仓库骨架 + 工作区（见 CLAUDE_WORKSPACE_PLAN.md）+ D0 确认。✅ 完成（2026-08-07）。
- **Phase A1**：Host Runtime API 桥。`cuda_to_maca_mcr_adaptor.h` → `cuda_to_cce_mcr_adaptor.h`（宏表结构原样，mc*→acl*）；`cuda_runtime_wrapper.h`（签名不变）+ `cuda_runtime_wrapper.cpp`（实现调 aclrt*）。错误码 `mcError_t`→`aclError`。
  - ✅ 完成：错误码按真实 acl_base.h / rt_error_codes.h 对齐（ACL_ERROR_NONE 成功码）；含 Device/Peer/Context/Stream/Event/Memory/Host/Error/Version 各组 wcuda* 声明+实现；声明↔实现↔宏表三方一致。
  - ⚠️ 剩余：`aclrtGetDeviceProperties`/`aclrtGetVersion` 等 9.x 新 API 的确切签名待真机核对（镜像头为 CANN 7/8 早期）。
  - 验收：host 侧 `.cu` 编译链接后，仅调 host API 即可在昇腾机跑通（kernel 体先置空）。
- **Phase A2**：`bin/cucc` 驱动 + CMake 探测。复用 cucc 的 nvcc 参数解析；`--offload-arch`→`--npu-arch=dav-c310`；找 bisheng（`ccec_compiler/bin`）而非 mxcc；注入 `-I$CUDA_PATH/include`；生成 `aclrtlaunch_{kernel}.h`。移植 make_maca/cmake_maca→make_cce/cmake_cce、tools/mock/。
  - 验收：`cucc vectorAdd.cu -o vectorAdd` 全流程编链成功。
- **Phase A3**：**SIMT 适配宏表 + kernel 直通**。这是路线 A 与 cu-bridge 唯一实质差异：
  - ✅ 已建 `include/bridge/kernel/simt_adaptor.h`：`__syncthreads`→`asc_syncthreads`、`__shared__`→`__ubuf__`、`__threadfence`→`asc_threadfence`；`__shfl_*`/`atomic*` 因签名待真机核对暂注释。
  - `<<<>>>` 启动：昇腾 SIMT 用 `kernel<<<blocks_per_grid, threads_per_block, stream>>>` 语法糖，由 bisheng 原生识别（无需宏替换）；也可走 `ACLRT_LAUNCH_KERNEL` 宏或 `aclrtLaunchKernelV2`。
  - ⚠️ 待真机核对：`asc_shfl`/`asc_atomic_*` 的确切签名、SIMT kernel 的 host 启动头文件（aclrtlaunch_xxx.h）。
  - 先 `ICPU_RUN_KF` CPU 模拟验证，再真机。
  - 验收：vectorAdd / matrixTranspose（SIMT 示例级）在昇腾上跑通且结果正确。
- **Phase A4**：库与多文件工程。cublas→aclblas 子集（参考 ops-blas / acl_cblas.h）；make_cce/cmake_cce 多子工程；PyTorch 编译指南参照 cu-bridge。
- **Phase A5**：非 SIMT 能力补齐（可选）。矩阵/Cube 类 kernel 走 aclblas 或 tile 模型；`__asm__` 内联处理。

### 路线 B — 老型号（910B/910C 等，无 SIMT）：走 v0.1 原案，难度高

- 在路线 A 的 A1-A2（host API 桥 + 驱动）基础上：
- **Phase B3**：**SIMT→Ascend C tile 转换层**（新增第 4 条线）。分模式支持（逐元素/reduction/GEMM 类），libTooling 可行性 spike 先行；不可映射的报"需人工改写"。
- **Phase B4**：库 + 多文件工程。

> 建议：**两条路线的 A1-A2 完全相同**（host 侧与驱动，与硬件型号无关），可先行。A3 vs B3 的分叉点在于 kernel 层，等 D0 确认型号后再定。

---

## 4. 关键技术决策点（需定稿，含推荐）

| # | 决策 | 选项 | 推荐 |
|---|---|---|---|
| **D0** | **目标硬件型号** | 950PR/950DT 系（SIMT）vs 910B/910C 系（无 SIMT） | ✅ **已定（2026-08-07）：950 系列 dav-c310 → 路线 A** |
| D1 | kernel 策略 | A：bisheng 直编+适配宏表；B：tile 转换层；C：CPU 模拟先行 | ✅ **A（950 系列 SIMT，bisheng 直编 + 适配宏表）** |
| D2 | 转换层技术路线 | clang libTooling / python AST / 正则 | 路线 A 不需要（已排除路线 B） |
| D3 | 库的取舍 | cublas→aclblas 先行，其余后置 | cublas→aclblas（有现成实现参照） |
| D4 | wrapper 命名 | 保留 `wcuda*` vs 改名 `wacl*` | **保留 `wcuda*`**（host 侧 API 签名与 cu-bridge 完全一致，多平台统一） |
| D5 | 与 cu-bridge 关系 | 镜像重写 vs git fork | 镜像重写（Apache-2.0 均可；代码主体同源但 target 全换） |

---

## 5. 与 cu-bridge 的逐模块差异表

| cu-bridge 模块 | cu2cce-bridge 对应 | 工作方式 | 风险 |
|---|---|---|---|
| `include/__cuda_runtime.h` | 同构 | 换 include 列表 | 低 |
| `cuda_to_maca_mcr_adaptor.h` | `cuda_to_cce_mcr_adaptor.h` | 宏表结构原样，值换 acl | 低（机械） |
| `cuda_runtime_wrapper.h` | 同构 | 签名不变 | 低 |
| `cuda_runtime_wrapper.cpp` | 同构 | mc* → aclrt* | 中 |
| `cuda_driver_wrapper.*` | 缩水 | 昇腾无同形 Driver API | 中（后置） |
| `bin/cucc` | 同构 | 找 bisheng；`--offload-arch`→`--npu-arch=dav-*`；生成 aclrtlaunch 头 | 中 |
| `tools/make_maca`/`cmake_maca` | `make_cce`/`cmake_cce` | 复制改名 | 低 |
| `tools/mock/` + cmake_mock | 同构 | mock nvcc → bisheng | 中 |
| `tools_src/ptx2cpp` | **不需要**（无 PTX） | 改处理 `__asm__` 内联 | - |
| mxcc 原生编译 kernel | **bisheng 直编 + SIMT 适配宏表**（路线 A） | 宏表 + 启动桥 | **中（本方案核心新增）** |
| （原 v0.1）SIMT→tile 转换层 | 仅路线 B 需要 | libTooling | 高 |
| 库（blas/dnn/fft/...） | 部分 | cublas→aclblas 先行 | 高 |

---

## 6. 环境变量与运行约束（写进 CLAUDE.md 用）

本机（macOS）无昇腾硬件、无 CANN，**不可构建/运行**，仅做代码阅读与调研。真机环境建议：

```bash
# 昇腾/CANN 环境（真机）
source /usr/local/Ascend/ascend-toolkit/latest/bin/setenv.bash
export ASCEND_HOME=/usr/local/Ascend
export ASCEND_TOOLKIT_HOME=${ASCEND_HOME}/ascend-toolkit/latest
# cu2cce-bridge 安装位置
export CUCC_PATH=...        # 复用 cu-bridge 的 CUCC_PATH 语义
export CUDA_PATH=${CUCC_PATH}
```

---

## 7. 开放问题清单（讨论用）

1. ~~D0（目标硬件型号）~~ ✅ 已定：950 系列 dav-c310 → 路线 A（2026-08-07）。
2. D4 命名：wrapper 保留 `wcuda*` 还是改名？（我推荐保留）
3. D5 仓库关系：镜像重写 vs git fork？（我推荐镜像重写）
4. caas-cli 的 license 与代码结构是否可借鉴？（可作为一个对照参考并排读，若用其映射表需确认 license）
5. 真机验证：可用的昇腾 950 系列环境有没有？（`--npu-arch=dav-c310` 是否被当前 bisheng 版本接受，A3 的 kernel 直通就靠它验证）

---

## 8. 调研来源（2026-08）

- 昇腾社区 CANN 文档（hiascend.com）：Ascend C 算子开发、AI Core SIMT 编译、AscendCL Runtime API、aclrtLaunchKernel、aclblas。
- asc-devkit（gitcode.com/cann/asc-devkit）：SIMT 编程模型概述、03_simt_api 示例、asc_syncthreads。
- ops-blas（gitcode.com/cann/ops-blas）：昇腾 BLAS 开源实现。
- 社区：caas-cli、910B CUDA 兼容层方案、CUDA→Ascend 迁移工具文章。
