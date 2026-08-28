# cucc 能力现状与反硬编码（适配性）设计

> 回应导师要求："cucc 不让硬编码，要兼容性适配性强。"
> 本文回答两件事：**① 现在的能力面到哪了；② 参数适配如何做到"加新东西
> 不改代码"——以及哪些少数策略仍在代码里、为什么、如何外部化。**

## 1. 核心论点：适配分三层，绝大多数改动落在数据层

```text
L1 数据层    config/cu2cce_rules.json —— 153 条规则（新增 flag 只改这里）
             drop 98 / inject 29(compile 17+link 12) / forward 11 /
             reject 11 / replace 4
L2 策略层    驱动内集中策略集（见 §4）：宿主 flag 白名单 36/丢弃 17、
             strict 开关、FP64 扫描策略、阶段判定逻辑
L3 开关层    环境变量（不重编译即改变行为）：见 §5 表
```

判据：**"某个 nvcc 参数遇到 ccec 该怎么处理"属于数据 → L1**；
**"整体编译语义/风险策略"属于策略 → L2**；**"不改代码就能调的行为"
属于开关 → L3**。三层各司其职，避免"什么都塞进代码"或"什么都散进
JSON"两个极端。

## 2. 实操演示：新增一个 nvcc flag 的适配需要几步？

假设 ccec 某版本开始支持 `--extra-device-opt=xxx`，或我们要兼容某个
新出现的 nvcc 选项 `--new-thing`：

```jsonc
// config/cu2cce_rules.json → compile.forward（透传给 ccec 试探）
{ "tokens": ["--new-thing"], "when": "source_is_cuda" }
// 或 drop（确认 ccec 不需要）：
{ "tokens": ["--new-thing"], "when": "source_is_cuda",
  "reason": "ccec 不消费该选项" }
```

```bash
python3 tools/validate_cucc_rules.py config/cu2cce_rules.json   # 校验
cucc --dryrun -c t.cu --new-thing -o t.o                        # 看最终命令
CU2CCE_TRACE_FILE=t.json cucc -c t.cu --new-thing -o t.o        # 留决策 trace
```

**三步、零 Python 改动、零重编译**。当前 153 条规则全部是这么加出来的
（对照 git log：每个 flag 批次 = 一次 JSON 提交）。

## 3. 能力现状快照（compile / link 两阶段）

| 能力 | 机制位置 | 状态 |
|---|---|---|
| nvcc 风格参数 → ccec puresimt（`-x asc --enable-simt`） | 注入链（L1+L2 流程） | `SUPPORTED` |
| `-gencode/-arch=sm_*` 识别与丢弃 | L1 drop | `SUPPORTED`（不伪造 Ascend 架构） |
| PTX/CUBIN/FATBIN/`-S`/device-link 拒绝 | L1 reject | `SUPPORTED` |
| `-M/-MD/-E` 依赖与预处理透传 | L1 forward | `DIAGNOSTIC_ONLY` |
| `-Xcompiler` 宿主选项白名单/丢弃 | L2 策略集（36/17 项） | `SUPPORTED` |
| FP64 device 使用前置扫描（error/warn/allow） | L2 策略 + L3 开关 | `SUPPORTED` |
| strict 模式（未验证长选项升级为错误） | L2 策略 + L3 开关 | `SUPPORTED` |
| `.o` 输出后缀识别、compile_and_link 阶段 | L2 流程 | `SUPPORTED` |
| conda LDFLAGS 透传 / CUDAFLAGS 拼接 | L2 流程 | `SUPPORTED` |
| response file（@file） | L2 流程 | `SUPPORTED` |
| 未识别长选项警告（非 strict 下） | L2 策略 | `SUPPORTED` |

## 4. 哪些仍在代码层（诚实清单 + 外部化路线）

| 策略集 | 规模 | 为什么暂时在代码 | 外部化计划 |
|---|---|---|---|
| 宿主 flag 白名单/丢弃集 | 36+17 项 | 属"编译器无关的宿主选项"语义判断，需与 strict 模式联动 | 规划项：迁入 JSON `host_flags` 节，校验器同步扩展 |
| FP64 扫描策略（区域判定/输出格式） | ~120 行 | 语义扫描（含 ErrMsg 适配）比参数匹配复杂 | 保留代码；策略档位已由 L3 开关暴露 |
| 阶段判定（compile/link/preprocess/…） | 流程逻辑 | 是流程不是数据 | 不外部化 |
| mock 探测应答 | 138 行 | 探测协议是 CMake 行为，非参数数据 | 不外部化 |

结论：**flag 适配 100% 数据驱动；语义策略集中在 L2 且全部有 L3 开关；
流程逻辑（本就必须是代码）不伪装成数据**。

## 5. L3 开关层全表（不重编译改变行为）

| 开关 | 作用 |
|---|---|
| `CU2CCE_NPU_ARCH` | 目标 Ascend 架构（决定 `--cce-aicore-arch`） |
| `CU2CCE_LIBC_MODE` | shadow / system libc 头策略 |
| `CU2CCE_DEVICE_FP64_POLICY` | device double：error / warn / allow |
| `CU2CCE_STRICT_FLAGS` | 未验证长选项/库：警告 → 错误 |
| `CU2CCE_CUDA_ARCH` | 注入 `-D__CUDA_ARCH__=NNN` |
| `CU2CCE_RULES_FILE` | 换一份规则文件（A/B 测试适配） |
| `CU2CCE_DEBUG` / `CU2CCE_TRACE_FILE` | 逐条决策日志 / JSON trace |
| `CUCC_CUDA_VERSION` | 报给宿主代码的 CUDA 版本号 |

## 6. 适配成果台账（本阶段真机沉淀，均落在对应层）

| 平台事实/需求 | 适配动作 | 落点 |
|---|---|---|
| device 无 double | 前置扫描 + 三档策略 | L2+L3 |
| `__device__` 不被前端识别 | 执行空间映射宏 | `simt_adaptor.h`（头文件层） |
| 向量类型原生存在 / `.cu` host 禁裸 malloc | 文档化禁令 | `04_shadow_headers.md` §6 |
| `-Xcompiler` 宿主选项混杂 | 白名单/丢弃集 | L2 |
| `--ptx/-cubin/-S` 等 | reject 规则 | L1 |
| `-MD/-MF/-MT` 依赖输出 | forward 规则 | L1 |
| 多流管线丢失 / 默认流语义 | 运行时消费点实现 | `02_deep_dive.md`（运行时层，非参数层） |

## 7. 可观测性（适配工作的"仪表盘"）

```bash
CU2CCE_DEBUG=1 cucc ...                    # 逐参数决策（drop/forward/inject + 原因）
CU2CCE_TRACE_FILE=t.json cucc ...          # 结构化 trace（inputs/actions/injected/final）
cucc --dryrun ...                          # 只打印最终 ccec 命令
```

任何一次编译的"为什么这样处理"都可回放——这是适配性强弱的最终检验标准。

## 8. 与 mentor 要求的对照总结

| 要求 | 现状 |
|---|---|
| 不让硬编码 | flag 适配 153 条全在 JSON；代码层策略集有清单、有外部化路线 |
| 兼容性适配性强 | 新增 flag 三步零代码；平台事实逐条落层；52 契约测试锁定行为 |
| 可验证 | trace/debug/dryrun 三种回放手段 + 真机六门禁 |
