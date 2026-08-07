# cu2cce-bridge 审查交接提示词（给 GPT-5.6 / 其他 AI 助手）

> 用法：把本文整段内容（或下方"可复制的提示词"一节）粘贴给目标 AI。它应能独立完成
> "代码审查 + 联网核实昇腾 API" 任务，无需再补充上下文。

---

## 可复制的提示词

你是昇腾 CANN 平台的专家。请对以下项目做一次**全面代码审查 + 联网核实**，重点是确认所有用到的昇腾 API 签名都是 **CANN 9.1.0 / 9.2.0 最新版**，不允许有任何错误。

### 项目背景

**cu2cce-bridge** 仿照 cu-bridge（沐曦，Apache-2.0），把 CUDA 程序迁移到华为昇腾 NPU：
- 目标硬件：**Ascend 950 系列（Ascend 950PR/950DT），bisheng 编译器 `--npu-arch=dav-310`**
- 目标软件栈：**CANN 9.1.0（发布版）/ 9.2.0（用户服务器）**
- 核心机制：三条迁移线
  1. host Runtime API：宏表 + wrapper（`cudaMalloc → wcudaMalloc → aclrtMalloc`）
  2. host Driver API（缩水）
  3. device kernel：AI Core SIMT（`__global__`/`<<<>>>` 由 bisheng 原生编译）+ SIMT 适配宏表
- 现状：约 124 个 `wcuda*` wrapper 已实现，mock 语法验证通过，但**未真机编译**

### 你的任务

1. **审查** `src/bridge/runtime/src/cuda_runtime_wrapper.cpp` 里每一个 `aclrt*`/`acl*` 调用的签名是否与 CANN 9.1.0/9.2.0 真实头文件一致。
2. **联网核实**：用下面的权威来源逐函数比对，找出任何写错/过时/不存在的 API。
3. 输出**错误清单**（含修正建议和来源）+ **待实机确认清单**。

### 必读文件（先读这些建立上下文）

- `CLAUDE.md`（项目说明、设计原则）
- `docs/03_Comparison/CURRENT_STATE.md`（**审查核对基线，最重要**：
  §3 已核实签名、§4 已修正坑、§5 待确认项、§6 完整修正历史（按轮次附来源））
- `docs/01_Design/ASCEND_RESEARCH_NOTES.md`（调研笔记，§1.5 错误码表 / §1.6 签名修正表）
- `docs/01_Design/CUDA_TO_CCE_PORTING_PLAN.md`（技术方案）
- `docs/reference/asc-devkit-simt/`（SIMT 权威头文件：device_sync/warp/atomic_functions.h 等）
- `docs/reference/cann-runtime-kernel-exec.md`（官方 kernel 加载执行文档）
- `src/bridge/runtime/src/cuda_runtime_wrapper.cpp`（**审查重点**）
- `src/bridge/blas/src/cublas_v2_wrapper.cpp`（cublas 桥）
- `include/bridge/runtime/cuda_to_cce_mcr_adaptor.h`（宏表）
- `include/bridge/kernel/simt_adaptor.h`（SIMT 宏表）
- `tests/mock_acl/acl/`（mock 头，注意它与真实头可能有差异）

> **重要**：`CURRENT_STATE.md` §4 已列出 12 个已修正的坑（如 aclrtStreamQuery 存在、
> 成功码用 ACL_SUCCESS、aclblas 无 handle、kernel 4 参启动等），§6 有 6 轮修正历史。
> **不要重复报告这些**，你的价值在发现**尚未列出的新问题**。

### 权威信息来源（联网时按此优先级）

1. **gitcode.com/cann/asc-devkit** — CANN 官方算子开发工程
   - `include/simt_api/`（asc_syncthreads/asc_shfl/asc_atomic_* 真实签名）
   - `examples/03_simt_api/00_introduction/`（hello_world_simt、gather_1d 官方示例）
2. **gitcode.com/cann/runtime** — CANN 官方运行时组件
   - `docs/zh/api_ref/14_kerne_loading_and_execution.md`（kernel 启动：aclrtLaunchKernelV2 / aclrtLaunchSIMTKernelWithArgsArray）
   - `docs/zh/api_ref/04_device_management.md`（aclrtGetDeviceProperties 等）
3. **昇腾社区 hiascend.com** 官方文档
4. **gitee.com/ascend/pytorch** `third_party/acl/inc/acl/`（ACL 头，注意是裁剪 stub）
5. **用户服务器实机头文件**（最权威）：`/usr/local/Ascend/ascend-toolkit/latest/include/acl/`（CANN 9.2.0）——如果会话能访问

### 铁律

- **严禁臆造 API 名/签名**。查不到确切签名就标注"待实机确认"，不要猜。
- 每个结论必须附**来源**（URL 或文件路径+行号）。
- 区分三档：**已确认**（有源码/官方文档证据）/ **待实机确认** / **推断**。
- 特别留意 CANN 9.x 的**新增/废弃** API：
  - `aclrtQueryEvent` 已废弃 → 用 `aclrtQueryEventStatus` + `aclrtEventRecordedStatus`（注意取值相反）
  - `aclrtLaunchSIMTKernelWithArgsArray`（SIMT 显式启动，950 支持）
  - `aclrtStreamQuery`（真实存在，带 status 输出）
  - `aclrtGetVersion`（9.2.0 完整头中已确认**不存在**；wrapper 返回模拟 CUDA 版本宏）
  - `aclrtGetDeviceProperties`（**不存在**，正确 API 是 `aclrtGetDeviceInfo(deviceId, attr, int64_t*)`）
  - `aclblasGemmEx`/`aclblasGemvEx` 无 handle，直接传 aclrtStream；无 aclblasCreate/Destroy
  - `aclTransType` 是 N/T/NZ/NZ_T（无 C）
  - `aclDataType` 无 double
  - **成功码用 `ACL_SUCCESS`**（`ACL_ERROR_NONE` 将废弃，勿用）
- 若你无法访问用户服务器，明确说明"以下项需用户实机核对"。

### 检查重点清单

1. **wrapper.cpp 每个 aclrt*/acl* 调用的签名**（含参数个数、类型、顺序）
2. **错误码映射**（`cudaError*` → `aclError`，cuda_to_cce_mcr_adaptor.h）
3. **cublas 桥**（无 handle 映射是否正确：`wcublasCreate` 自定义结构存 stream → `aclblasGemmEx`/`aclblasGemvEx` 直接传 stream）
4. **simt_adaptor.h**（asc_shfl 无 mask、asc_atomic_* 带 __ubuf__/__gm__ 地址空间、kernel 启动 4 参 `<<<blocks,threads,dyn_ubuf_size,stream>>>`）
5. **mock 头 vs 真实头**差异（tests/mock_acl/ 是否误导了实现）
6. **待真机确认项复核**（CURRENT_STATE §5 剩余项：pitch 对齐、原子地址空间、RecordEvent null stream）——若能联网查到官方确认就闭环，否则保留"待实机"

### 输出格式

1. **错误清单**（按严重度降序）：函数名 | 我的实现 | 权威签名 | 来源 | 修正建议（含可替换代码）
2. **待实机确认清单**：函数名 | 疑点 | 建议验证方式
3. **正确 API 映射总表**（每个 wcuda* → acl* 的最终结论）
4. 总结：哪些已验证、哪些需要用户实机验证

---

## 交接给用户的重要提醒

- 本项目**在 macOS 上无 CANN，不能本地构建**。审查/联网核实可以，构建/运行必须在昇腾真机。
- 最权威的确认是用户服务器 `/usr/local/Ascend/ascend-toolkit/latest/include/acl/` 下的实际头文件。
- 如果 GPT 会话无法联网（无工具），至少让它读 `docs/reference/` 下的权威头做静态比对。
