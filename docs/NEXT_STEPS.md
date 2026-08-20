# 后续方向 + 维护面 + 薄弱点盘点（2026-08-20）

> 给下一个接手 agent（或 codex）的路线图。详细可执行版见同目录 `HANDOFF_PROMPT.txt`
> （可直接整段粘贴给 LLM）；本文是浓缩版决策页。

## 一、后续方向（按优先级）

### 已完成 ✅ kernel_convert.py 动态 blockDim lowering（B 方案）
- 结论：不能在 `__global__[aicore]` wrapper 中读取 built-in `blockDim`（Bisheng 前端崩溃）；改为由 host launch 隐藏传入 `bx/by/bz`，wrapper 使用 `cce::dim3{bx,by,bz}`。
- 能力：同一 kernel 支持运行时 256/512，多 kernel 独立启动，并支持 2D `32×8×1`；CCE 外层使用标量总线程数 `x*y*z`。
- 真机：vectorAdd、minimal、纯 CUDA dynamicBlockDim 综合样例和 1D/2D 探针全部编译、链接、数值运行通过，`FAIL=0`（2026-08-20）。
- 保留：A 静态推导分支作为 fallback；C Runtime launch lowering 是长期研究方向。

### 方向 1 ⭐ SIMT 原语真机验证（当前显式待确认项）
- 现状：`simt_adaptor.h` 已把 `__syncthreads→asc_syncthreads`、`__shared__→__ubuf__`、
  `__shfl_sync→asc_shfl`、`atomicAdd→asc_atomic_add` 等映射好，**但只验证过 elementwise
  kernel，warp 原语/atomic 从未真机跑过**（CURRENT_STATE.md 待确认 #6 是 ⚠️）。
- 做：reduction（用 __syncthreads+__shared__）或 histogram（atomicAdd）示例，真机跑通+数值校验。
- 价值：把"骨架"变成"多模式可用"，直接关掉一份待确认项。

### 方向 2 cublas 桥真机验证
- 现状：`cublas_v2_wrapper.cpp` 已实现 gemm/gemv 映射（自定义 handle），**从未真机运行**；
  wrapper 链接 libacl_cblas.so（运行期解析）。
- 做：cublasSgemm 小示例，真机校验 C=A×B；注意昇腾分配不自动清零，gemm 前手动初始化 C。

### 方向 3 回归加固
- 把 minimal.cu（首调 cudaMalloc，107002 复退防线）加进 `run_tests.sh` 固定用例并断言输出。

## 二、要维护的

- `tools/real_test/`：env_probe / build_bridge / run_tests / run_all（真机常态化回归，勿破坏）
- **`tools/real_test/server_cmd.txt`**：每轮推送前覆盖成最新服务器命令（单一来源，规则在 CLAUDE.md 开发约定 5）
- `.gitignore`：diag 产物/中间 cce/server_cmd 不需要忽略（它在版本库里）
- `docs/01_Design/ASCEND_RESEARCH_NOTES.md` / `CURRENT_STATE.md`：每次真机新结论要回写
- `00_project_journey/`：PROJECT_JOURNEY / TIMELINE / RESUME_BULLETS / CASE_STUDY /
  HANDOFF_PROMPT（素材截止 commit 要更新）

## 三、薄弱点（接手心里有数）

| 薄弱点 | 说明 |
|---|---|
| kernel 转换器仍是轻量语法重写 | 已支持隐藏 blockDim lowering，但模板 kernel、函数指针 launch、宏生成 launch 与复杂参数声明仍需 AST 版本 |
| host 桥覆盖浅 | 124 个 wcuda 声明只有核心路径 8/8 真机验证；Graph/Texture/Symbol/Peer/Unified/SOMA 多数只有占位或未经运行 |
| glibc 兼容是遮蔽头+宏 | ccec(clang15)+glibc2.39 靠 `include/shadow/`，新头文件可能撞 `_Float32`/`__FILE`，改影子头要回归 8/8 |
| 107002 防复退 | 修复覆盖了 4 个入口，但"原生调用在 WCUDA_RETURN 之前"的模式要新代码守规矩 |
| 链接器告警噪音 | cucc 把链接参数传给编译阶段，`linker input unused` 一堆，别看错（正常） |

## 四、一句话给新手 agent

> 负责 = 在**真机上**让"纯 CUDA + 更复杂 kernel/库调用"继续跑通并回归 8/8；
> 每轮真机命令走 server_cmd.txt 单文件约定；结论回写 docs 与 00_project_journey。