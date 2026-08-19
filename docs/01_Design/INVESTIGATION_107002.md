# 问题交接：cudaMalloc 107002 排查（最小示例 minimal 失败，vectorAdd 通过）

> 2026-08-19 深夜整理。**本文档自包含**：新对话的 agent 读本文档 + 仓库即可接手，
> 不需要本对话的其他上下文。如嫌啰嗦直接看「5. 下一步实验清单」。
>
> **接手第一步**：让用户提供 §6.3 的 `bf16x2_add_test` host 源码（能跑的对照），
> 同时按 §5.1 给 bridge_init 加 debug 打印重跑 minimal 拿返回码。

## 1. 项目一句话背景

cu2cce-bridge：用户写**纯 CUDA** 代码（`.cu`），经 `bin/cucc`（驱动）→
`tools/kernel_convert.py`（自动转昇腾 SIMT VF 模板）→ `ccec`（CANN 自带编译器，
`--cce-aicore-arch=dav-c310`）编译 → 链接 → 跑在昇腾 NPU（davinci0）上。
host 侧 CUDA API 由 `src/bridge/runtime/src/cuda_runtime_wrapper.cpp` 的
`wcuda*` 函数映射到 ACL（`aclrt*`）。宏表在
`include/bridge/runtime/cuda_to_cce_mcr_adaptor.h`。

## 2. 现象（时间线）

- **2026-08-19 已达成**：`bash tools/real_test/run_tests.sh all` **8/8 PASS**
  - `host_compile/link/run` PASS（host 冒烟：cudaGetDeviceCount/cudaSetDevice/
    cudaMalloc/cudaMemcpy/cudaMemset/cudaDeviceGetAttribute/cublasCreate 全部真机运行 OK）
  - `vectoradd_compile/link/run` PASS：**纯 CUDA vectorAdd 在 davinci0 上运行，
    输出 `vectorAdd OK (N=1024)` 数值校验通过**（kernel 零改写端到端成立）。
- **问题**：`examples/minimal/minimal.cu`（同样纯 CUDA，但 host 侧**直接
  cudaMalloc**，没有先调 cudaGetDeviceCount）手动编译运行：
  ```
  cucc -c examples/minimal/minimal.cu -o /tmp/minimal.o
  cucc /tmp/minimal.o -o /tmp/minimal
  /tmp/minimal
  → cudaMalloc failed: 107002
  ```
- 已尝试但**无效**的东西：
  1. `source /usr/local/Ascend/latest/set_env.sh`（环境变量路径）→ 无效，仍 107002
  2. `bridge_init` 里 `aclrtSetDevice(0)` **失败重试 5 次 × 100ms**（commit 2281326）→ 无效
- 编译期大量 `warning: ... 'linker' input unused` / `-x cce after last input` 是
  **正常现象**（cucc 把链接参数也传给了编译阶段，bisheng 提示未使用），不影响结果。

## 3. 已确认的事实（排除项）

| 事实 | 证据 |
|---|---|
| ACL 初始化 + 设备枚举是通的 | vectorAdd 输出 `device count = 1`（= `wcudaGetDeviceCount` → `aclrtGetDeviceCount` 成功） |
| ccec 编译/链接/运行链路 OK | vectorAdd 8/8 PASS、minimal 编译链接成功（只挂在运行期 cudaMalloc） |
| host 侧 CUDA API 桥（含 cudaMalloc）**在部分代码路径可用** | vectorAdd 里 cudaMalloc 三个缓冲都成功 |
| 驱动库路径不是主因 | source set_env.sh 前后行为一致 |
| 时序重试不是主因 | 5×100ms 重试后仍 107002 |

**关键差异（minimal vs vectorAdd）**：vectorAdd 的 main 里**第一个 wcuda API 是
`cudaGetDeviceCount`**（先触发了完整的 aclInit + 设备枚举流程），之后才
cudaMalloc；minimal **第一个 API 就是 cudaMalloc**（`bridge_init` 里
aclInit → aclrtSetDevice(0) 一气呵成后立刻分配）。两条路径编译参数、链接库、
wrapper 库完全相同。

## 4. 107002 语义与候选根因（按概率排序）

`107002` = `ACL_ERROR_RT_MEMORY_ALLOCATION`（设备内存分配失败）——
具体的 aclInit/aclrtSetDevice 返回码目前**被静默吞掉**（`bridge_init` 里
`(void)ret` / 重试失败不打印），这是当前最大的信息缺口。

候选根因（按概率）：
1. **`aclrtSetDevice(0)` 在 minimal 路径下确定性失败**（重试也失败），导致
   `aclrtMalloc` 无设备可分配。为什么失败？可能要求先 `aclrtGetDeviceCount`
   完成设备枚举（vectorAdd 恰好先做了）。**最便宜的实验：bridge_init 里
   aclInit 后先调 aclrtGetDeviceCount 再 aclrtSetDevice(0)**。
2. **`aclrtMalloc` 的 policy 问题**：当前用 `ACL_MEM_MALLOC_HUGE_FIRST`
   （大页优先）。若该环境下大页内存配置不足，可能分配失败 → 试
   `ACL_MEM_MALLOC_NORMAL_ONLY`（普通页）。
3. **ACL 需要显式 context**：部分 CANN 版本 `aclrtSetDevice` 后还需
   `aclrtCreateContext` 才能分配。用户实测能跑的 host 代码（`bf16x2_add_test`）
   是否创建了 context / 先做了什么，是重要参照（见 6.3）。
4. 驱动/固件与 run 库版本不匹配（driver 7.0.t9.0.B880 vs cann 9.2.0）——
   但 vectorAdd 能跑，此概率低。
5. `/dev/davinci0` 权限 / 容器映射问题 —— vectorAdd 能跑，排除。

## 5. 下一步实验清单（按顺序，先便宜的）

### 5.1 【加 debug 打印】让错误无处可藏（必做，10 分钟）
`src/bridge/runtime/src/cuda_runtime_wrapper.cpp` 的 `bridge_init()`：
把 `aclInit` / `aclrtGetDeviceCount` / `aclrtSetDevice(0)` 的返回码
**printf 打印出来**（可挂在 `WCUDA_DEBUG` 环境变量下，与 bin/cucc 的
WCUDA_DEBUG 呼应；或直接无条件打印到 stderr 先跑通）。
然后服务器：`git pull && bash tools/real_test/build_bridge.sh` → 重跑 minimal →
看**每一行返回码具体数值**（107002 上游到底是 100006 还是别的）。

### 5.2 【桥接层顺序实验】（5 分钟，可能直接解决）
`bridge_init()` 里 aclInit 成功后**先 `aclrtGetDeviceCount` 再
`aclrtSetDevice(0)`**（对齐 vectorAdd 的调用顺序），重跑 minimal。
若解决 → 结论：ACL 要求先枚举再设设备，把该顺序固化并注释。

### 5.3 【policy 实验】（5 分钟）
`wcudaMalloc` 把 `ACL_MEM_MALLOC_HUGE_FIRST` 换成 `ACL_MEM_MALLOC_NORMAL_ONLY`
试一次（可临时改 + 重编），重跑 minimal。若解决 → 大页内存问题，定稿用
普通页（或按 size 分流：大块 HUGE、小块 NORMAL）。

### 5.4 【裸 ACL 复现】（隔离 wrapper，判断问题在桥还是 ACL 本身）
写最小 `.cce`（不经过 cucc/wrapper），直接：
```c
aclInit(nullptr); aclrtGetDeviceCount(&n); aclrtSetDevice(0);
aclrtMalloc(&p, 1024, ACL_MEM_MALLOC_HUGE_FIRST);  // 打印每步返回码
```
用用户实测脚本等价参数（`/usr/local/Ascend/latest/bin/ccec -std=gnu++17
-xcce --cce-aicore-arch=dav-c310` + `-lruntime` 等）编译运行，看裸 ACL 能否分配。
- 能 → 问题在 wrapper 的某个行为（顺序/policy/缺调用），回到 5.2/5.3
- 不能 → ACL 层问题（驱动/内存/固件），进一步查大页配置、`cat /proc/meminfo`
  的 HugePages、`ls -l /dev/davinci*`、dmesg

### 5.5 【对照用户实测代码】（最有价值的参照）
用户在**同一台机器**用 `scripts/04_build_exec.sh`（CANN 官方风格编译）跑通过
`bf16x2_add_test`，说明裸 ACL host 代码是通的。**拿到那份 host 代码**（见 6.3），
对照它的初始化顺序（有无 aclrtGetDeviceCount、有无 aclrtCreateContext、
alloc policy、是否设置 device 属性），把 bridge_init 对齐它。

## 6. 关键文件与信息源

### 6.1 仓库内关键位置
- `src/bridge/runtime/src/cuda_runtime_wrapper.cpp`
  - `bridge_init()` ≈ 38-75 行（aclInit + aclrtSetDevice 重试）
  - `wcudaMalloc` ≈ 492-507 行（`ACL_MEM_MALLOC_HUGE_FIRST`）
  - `wcudaGetDeviceCount` ≈ 66 行（aclrtGetDeviceCount）
- `include/bridge/runtime/cuda_wrapper_internal.h`：BRIDGE_INIT 宏（call_once）
- `tools/real_test/run_tests.sh`：真机测试脚本（49 行 LD_LIBRARY_PATH）
- `examples/vectorAdd/vectorAdd.cu`：**能跑的**纯 CUDA 示例（首调用 GetDeviceCount）
- `examples/minimal/minimal.cu`：**挂掉的**纯 CUDA 示例（首调用 cudaMalloc）

### 6.2 真机环境（已确认）
- CANN 9.2.0（`/usr/local/Ascend/cann-9.2.0`；`/usr/local/Ascend/latest` 是软链）
- driver 7.0.t9.0.B880；`/dev/davinci0` + `/dev/davinci_manager` 存在
- 编译器：`/usr/local/Ascend/latest/bin/ccec`（clang 15.0.5），
  `--cce-aicore-arch=dav-c310`；bisheng 是另一套（不认识 dav-c310，勿用）
- SIMT VF 模板（真机验证）：`__simt_vf__` 计算体 + `__global__[aicore]` 入口 +
  `cce::async_invoke` 启动 + `__gm__` 指针；kernel 转换见 `tools/kernel_convert.py`
- 系统标准库头与 glibc 2.39 不兼容 → `include/shadow/` 遮蔽头方案（勿回退）

### 6.3 需要用户提供（下一 agent 的第一件事）
`bf16x2_add_test.cce`（或 `.cpp`/`.cu`）**的 host 侧源码**（main 函数部分：
aclInit → 设备设置 → 内存分配 → kernel 启动的完整顺序），以及
`scripts/04_build_exec.sh` 的当前内容。这是"能跑的参照"，对照它定位 bridge_init
缺什么。

### 6.4 已有知识（ASCEND_RESEARCH_NOTES §3.6 摘录）
- `aclrtGetVersion` 存在但已废弃（建议 aclSysGetVersion）
- 无独立 `aclblasCreate`（是 CreateHandleFor* 系列）→ cublas 桥自定义 handle 正确
- 用户实测编译命令（04_build_exec.sh）链接参数：
  `-L${CANN_BASE}/runtime/lib64 -L${CANN_BASE}/lib64 -lascendcl -lruntime
  -lstdc++ -lpthread -lm -Wl,-rpath=${CANN_BASE}/runtime/lib64`

## 7. 判定标准（做完哪一步算关闭）

1. minimal 手动跑输出 `minimal OK (N=256)`（与 vectorAdd 同级）
2. 若 5.2 解决：把"先 GetDeviceCount 再 SetDevice"固化进 bridge_init + 文档
3. 若 5.3 解决：定稿 alloc policy 策略 + 文档
4. 无论哪种解法，把根因与修复记回本文件（追加"结论"节），并跑一次
   `bash tools/real_test/run_tests.sh all` 确认 8/8 仍通过

---

## 8. 结论（2026-08-19 定案，issue 关闭）

**根因**：`wcudaMalloc` 把原生调用 `aclrtMalloc` 写在了**会触发 `BRIDGE_INIT` 的
`WCUDA_RETURN` 宏之前**。用户程序首个 API 就是 `cudaMalloc` 时，`aclrtMalloc`
在 `aclInit`（乃至任何 ACL 初始化）**之前**执行 → ACL 未初始化状态下的设备分配
失败，返回 `107002`（`ACL_ERROR_RT_MEMORY_ALLOCATION`）。

**压印顺序铁证**（ROUND 9/12/13/15 每次 `WCUDA_DEBUG=1` 的输出顺序）：

```
[wcuda] wcudaMalloc enter
[wcuda] aclrtMalloc call      ret=107002   ← 打印在 bridge_init 之前！
[bridge_init] aclInit
[bridge_init] aclrtGetDeviceCount
[bridge_init] aclrtSetDevice(0)
```

**为什么前置 API 都成功、只有直调 cudaMalloc 挂**（ROUND13）：
`wcudaGetDeviceCount`/`wcudaGetDevice`/`wcudaSetDevice` 内部调用的
`aclrtGetDeviceCount`/`aclrtGetDevice` 会**隐式触发 ACL 初始化**，恰好先完成
初始化才轮到 malloc。裸 ACL 探针、ctor_alloc、prog_b/c/d 全成功也是同一原因
——它们都在 malloc 前有会隐式初始化的调用。

**修复**（commit `e34ee02`）：
`wcudaMalloc` 函数体**开头显式 `BRIDGE_INIT()`**（`call_once`、幂等）——
任何调用都先走完 `aclInit → aclrtGetDeviceCount → aclrtSetDevice(0)`
（含调试打印里的 `aclrtCreateContext+SetCurrentContext`）再轮到 `aclrtMalloc`。

**验证**（真机 2026-08-19，ROUND17）：
- `minimal` 手动跑输出 **`minimal OK (N=256)`**，且压印顺序变为
  `[bridge_init]` 全在前、`[wcuda]` 在后（修复前顺序颠倒）
- `bash tools/real_test/run_tests.sh all` **8/8 PASS**（host 冒烟 + vectorAdd
  端到端，与修复前一致，无回归）
- 顺带加固：`wcudaGetDeviceProperties`/`wcudaStreamQuery`/`wcudaEventQuery`
  三个同样「原生调用在 WCUDA_RETURN 之前」的入口也补了 `BRIDGE_INIT()`
  函数头（commit 定稿时一并提交）

**未来防重回**：凡 `wcuda*` 函数体里出现**直接对 `aclrt*`/`acl` 的原生调用**
（不在 `WCUDA_RETURN` 宏内），必须在函数体第一个语句放 `BRIDGE_INIT()`。
排查期加的 size/policy 压印保留（仅 `WCUDA_DEBUG` 输出，无 env 无开销）。

### 排查过程附录（17 轮证据链，压缩版）

| 轮 | 实验 | 结论 |
|---|---|---|
| 1-3 | bridge_init 加打印、调整顺序；minimal 重跑 | aclInit/GetDeviceCount/SetDevice 全 0，仍 107002 |
| 4 | 裸 ACL 探针（同序 5 种组合） | 全部成功，HBM 97GB 空闲 → 排除 ACL/环境 |
| 5 | 库组合探针 base/+runtime/+wrapper/+both + 双 CANN 根核查 | 4 组合全成功、md5 一致 → 排除库链/多根 |
| 6 | echo（无 kernel 子集） | 仍 107002，errMsg 空 → 问题在 host 侧本身 |
| 7 | wrapper DT_NEEDED/INIT_ARRAY/构造器探针 | ctor_alloc 成功、.init_array 仅 frame_dummy → 排除构造器污染 |
| 8 | trunc_raw（不连 wrapper 裸流程）+ gdb/LD_DEBUG | 裸流程成功、库 init 全在 main 前 → 排除顺序/环境/库 init |
| 9 | echo 用 cucc 原样编 | 仍 107002，与 kernel 无关 |
| 10 | smoke 对照（先 GetDeviceCount）PASS + 迭代 malloc 各 6 次成功 | 差异在"首个 API 是否枚举/设设备" |
| 11 | 保留中间产物 .cce | 有/无 kernel 转换差异；smoke 无损、minimal 被转换（后证与根因无关） |
| 12 | echo 改 .cu 无损编 | 仍 107002 → 排除 .cce/.cu、kernel 转换 |
| 13 | 4 个 host-only .cu（直调/先枚举/先 GetDevice/先 SetDevice） | **决定性：A 直调 malloc 107002，B/C/D 前置任意枚举即成功** |
| 14-15 | SetDevice 后加 CreateContext / SetCurrentContext | 均 ret=0 仍 107002 → context 方向排除（CreateContext 不自设当前，且非本因） |
| 16 | wcudaMalloc 打印 size/policy 原值 + C 对照 | size=1024 policy=0 正常 → 参数无恙 |
| 17 | **压印顺序铁证 + wcudaMalloc 开头 BRIDGE_INIT()** | **minimal OK (N=256)，8/8 PASS —— 定案 ✅** |

**遗留（非本 issue 范围）**：`aclrtResetDevice` 在重复调用时返回
`507007`（`ACL_ERROR_RT_DRV_INTERNAL_ERROR`，ROUND4 探针末尾观测到一次，
单次调用正常）；`wcuda` 其余入口的 BRIDGE_INIT 完整性已审计（除上述 4 处
外，其余入口要么无原生调用、要么原生调用已在 WCUDA_RETURN 宏内）。