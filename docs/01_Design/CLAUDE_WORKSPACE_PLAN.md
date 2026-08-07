# cu2cce-bridge Claude/Agent 工作区方案

> 状态：草稿 v0.1
> 目的：让 Claude Code 在这个新仓库里能**对照 cu-bridge** 高效开发 cu2cce-bridge，不跑偏、不重复调研、危险命令（构建/运行）被正确拦下。

---

## 1. 设计原则

1. **镜像 cu-bridge 目录结构** —— 降低"对照"成本：两边同名文件直接并排看。
2. **对照状态显式化** —— 用 `docs/03_Comparison/mapping.md` 追踪每个模块"源(cu-bridge) → 目标(cu2cce)"的移植状态，这是团队（人 + Claude）共享的进度真相源。
3. **本机只读不构建** —— CLAUDE.md + .claude/settings.json 双重挡板：macOS 无 CANN，任何 `cmake`/`make`/`cucc`/`ccec` 构建类命令默认拒绝。
4. **Claude 负责"对照移植"这种重读取、重结构的活**，人负责决策（D1-D5）与真机验证。

---

## 2. 仓库结构（镜像 cu-bridge，标注与源仓库的对应关系）

```
cu2cce-bridge/                      # ← 新仓库根
├── CLAUDE.md                       # 仿照 cu-bridge/CLAUDE.md 写"昇腾版"
├── README.md
├── LICENSE.txt                     # Apache-2.0（同 cu-bridge）
├── NOTICE.txt
├── docs/
│   ├── 01_Design/                  # ← 本方案 + 架构图（放得下可放 solution_design.png 占位）
│   │   ├── CUDA_TO_CCE_PORTING_PLAN.md
│   │   └── CLAUDE_WORKSPACE_PLAN.md
│   ├── 02_User_Manual/
│   └── 03_Comparison/              # ← 新增：与 cu-bridge 的对照表（开发期核心工作文件）
│       └── mapping.md
├── include/                        # 镜像 cu-bridge/include（CUDA 兼容头）
│   ├── cuda_runtime.h
│   ├── __cuda_runtime.h
│   └── bridge/
│       ├── runtime/
│       │   ├── cuda_runtime_wrapper.h        # 同 cu-bridge，签名不变
│       │   ├── cuda_to_cce_mcr_adaptor.h     # 宏表（cu-bridge 的 cuda_to_maca_mcr_adaptor.h 换值）
│       │   └── ...（cuda_driver_wrapper.h / nvrtc_wrapper.h 按需）
│       └── (blas/fft/rand/... )               # 库，后置
├── src/bridge/runtime/src/
│   └── cuda_runtime_wrapper.cpp               # wcuda* → aclrt*
├── bin/
│   └── cucc                                  # nvcc 兼容驱动 → ccec
├── tools/
│   ├── make_cce / cmake_cce / pre_make       # 对应 make_maca / cmake_maca / pre_make
│   ├── mock/                                 # cmake 探测 mock
│   └── multi_target.sh
├── kernel/                                   # ← 新增：SIMT→Ascend C 转换层（Phase 4）
│   ├── simt2tile/                            # libTooling 或 spike
│   └── templates/                            # aclrtlaunch 启动包装模板
└── .claude/
    ├── agents/
    ├── commands/
    └── settings.json
```

**与原仓库的差异**：
- `tools_src/ptx2cpp/` 不搬（昇腾无 PTX），改由 `kernel/simt2tile` 承接"内联汇编/特殊语法"处理。
- 新增 `docs/03_Comparison/` 与 `kernel/`。
- `cuda_driver_wrapper.*`、`nvrtc_wrapper.*` 视 D 决策后置或裁剪。

---

## 3. CLAUDE.md 大纲（新仓库的，落骨架时写全）

仿照 cu-bridge/CLAUDE.md 的 5 段结构：

1. **项目定位**：cu2cce-bridge —— 让 CUDA 程序生成跑在昇腾 NPU 上的目标文件；当前目标是 CANN/Ascend C。**本机无昇腾硬件、无 CANN，禁止构建/运行**（照抄 cu-bridge 的这句语义）。
2. **核心原理（两条线 + 一个编译器 + 一个转换层）**：
   - Host Runtime API：宏表 + wrapper（`cudaMalloc → wcudaMalloc → aclrtMalloc`）
   - Driver API：`cu*` → acl（缩水）
   - Device kernel：ccec 编译 `__global__ __aicore__`；**与 cu-bridge 的关键差异**：不认 SIMT，需转换层
3. **关键目录**：与 §2 树对应，逐个写用途 + 对照的 cu-bridge 源目录。
4. **环境变量**：`ASCEND_HOME`/`CUCC_PATH`/`CUDA_PATH`（真机）；本机无环境。
5. **调研要点/对照开发约定**：移植时"先查 docs/03_Comparison/mapping.md 再动手"；新增模块先写对照表条目。

---

## 4. `.claude/` 工作区配置

### 4.1 settings.json（项目级，进 git）

```json
{
  "permissions": {
    "deny": [
      "Bash(cmake *)",
      "Bash(make *)",
      "Bash(cu2cce-bridge/**/cucc *)",
      "Bash(ccec *)",
      "Bash(msopgen *)"
    ],
    "allow": [
      "Read(/Users/tmchao7/repos/09_expLearn/cu-bridge/**)",
      "Bash(git status)",
      "Bash(git diff *)"
    ]
  }
}
```

> 注意：deny 规则是粗略示范，落骨架时用 `update-config` skill 精调（比如构建只允许在明确标注的真机目录里跑，或干脆全部 deny + CLAUDE.md 明文）。

### 4.2 agents（`.claude/agents/*.md`）

| agent | 用途 | 说明 |
|---|---|---|
| `porting-mapper` | 给定 cu-bridge 某个文件 → 产出 cu2cce 对应文件的移植要点（换哪些符号、哪些逻辑要重写） | 这是"对照开发"的主力 agent，读 cu-bridge 源码 + 读 mapping.md，输出结构化要点 |
| `ascend-api-check` | 核对某个 aclrt*/aclblas* API 的确切签名、头文件、枚举值 | 查 CANN 文档/网络/记忆，避免瞎猜 API |
| `simt2tile-designer` | kernel 转换层设计讨论的专用 agent | Phase 4 用，先出可行性 spike 设计 |

### 4.3 commands（`.claude/commands/*.md`）

| command | 动作 |
|---|---|
| `/compare <cu-bridge 文件路径>` | 调 porting-mapper：输出该文件→cu2cce 的移植要点，并**追加一行到 mapping.md** |
| `/sync-status` | 重扫 mapping.md，列出所有"未动/移植中"条目，建议下一步 |

### 4.4 skills（可选，后置）
不一定要自建 skill；先依赖 agents + commands 就够。若发现反复做同一类事（如"生成 aclrtlaunch 头"），再抽成 skill。

---

## 5. 对照开发工作流

### 5.1 mapping.md 格式（docs/03_Comparison/mapping.md）

```markdown
| # | cu-bridge 源文件 | cu2cce 目标文件 | 状态 | 备注 |
|---|---|---|---|---|
| 1 | include/bridge/runtime/cuda_to_maca_mcr_adaptor.h | include/bridge/runtime/cuda_to_cce_mcr_adaptor.h | 未动 | 宏表换值 mc*→acl* |
| 2 | include/bridge/runtime/cuda_runtime_wrapper.h | 同 | 未动 | 签名不变 |
| 3 | src/bridge/runtime/src/cuda_runtime_wrapper.cpp | 同 | 未动 | 实现换 aclrt* |
| ... | bin/cucc | bin/cucc | 未动 | 找 ccec；SOC 型号 |
```

状态枚举：`未动 / 移植中 / 已移植 / 跳过`（跳过要写原因，如"昇腾无对应：ptx2cpp"）。

### 5.2 单次开发循环（建议）

1. 打开新会话 → 读 `docs/03_Comparison/mapping.md` → 挑一个"未动"且当前 Phase 该做的条目。
2. `ll cu-bridge/<对应文件>`（参考源）+ `ll cu2cce/<目标>`（目标现状）→ 必要时 `/compare` 生成要点。
3. 实现 → 更新 mapping.md 状态 → 提交。
4. 每个 Phase 结束：真机验证（如果用户有昇腾环境）。

### 5.3 知识沉淀（Claude memory）

把跨会话有用的、非源码可推导的事实写进 memory（项目类型）：

- 昇腾 ACL 命名规律（`aclrt*`、`aclError`、`aclrtMemcpyKind`）。
- Ascend C kernel 形态（`__global__ __aicore__` + `<<<blockDim, l2ctrl, stream>>>`，无 gridDim）。
- 核心结论："昇腾 AI Core 非 SIMT，kernel 需 tile 化" —— 这是 cu2cce 与 cu-bridge 最本质差异。
- 用户偏好：中文交流、先方案后骨架、对照开发。

---

## 6. 分工建议（人 vs Claude）

| 事项 | 人 | Claude |
|---|---|---|
| D1-D5 决策 | ✅ 拍板 | 给推荐 |
| 逐文件对照移植（宏表/wrapper/cucc 骨架） | | ✅ 主力 |
| API 签名核对 | | ✅ 查文档 |
| SIMT→Ascend C 转换层 spike 设计 | ✅ 确认方向 | ✅ 出方案/原型 |
| mapping.md 维护 | 抽查 | ✅ 维护 |
| 真机构建/运行/跑结果 | ✅ 唯一能做的（有硬件） | 生成命令给用户跑 |
| 构建类命令 | — | 默认拒绝，防误触 |

---

## 7. 落骨架的清单（讨论定稿后执行）

- [ ] 写 `CLAUDE.md`（§3 大纲落全）
- [ ] 写 `README.md`（定位 + 对照 cu-bridge 的关系说明）
- [ ] `.claude/settings.json`（permission 挡板，用 update-config skill 精调）
- [ ] `.claude/agents/{porting-mapper,ascend-api-check,simt2tile-designer}.md`
- [ ] `.claude/commands/{compare,sync-status}.md`
- [ ] `docs/03_Comparison/mapping.md`（按 §5.1 格式铺第一版全表）
- [ ] 目录骨架（§2 树，空文件 + 占位 README）
- [ ] `LICENSE.txt`/`NOTICE.txt`（Apache-2.0）
- [ ] 初始 commit（用户确认后）
