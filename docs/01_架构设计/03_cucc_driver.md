# cucc 编译驱动详解

> `cucc`（bin 入口）→ `tools/cu2cce_driver.py`（`RuleDriver`）。
> 同一个驱动也被 `tools/mock/mock_nvcc.py`（CMake shim）复用——这是
> "CMake 路径与命令行路径行为一致"的保证。设计文档历史版见
> `docs/99_归档/CUCC_RULE_DRIVEN_DRIVER_v1.md`。

## 1. 驱动的职责边界

- **只做参数变换**：nvcc 风格参数 → ccec 风格参数；不读改源码内容
  （唯一例外：FP64 类型策略的只读扫描，见 §5）。
- **绝不伪造**：不支持的参数/类型按规则 reject 或明确 not-support；
  不静默吞错误。

Ascend 架构选择优先使用 `CU2CCE_NPU_ARCH`/`CUCC_NPU_ARCH`；为对齐 cu-bridge，`CUCC_TARGETS` 作为兼容别名接受，但当前 ccec puresimt 一次只允许一个 `dav-*` 架构。未显式设置时，直接 `cucc` 与 `cmake_cce`/`make_cce`/`ninja_cce` 共用 `tools/device_arch.sh` 的 `npu-smi → cache → default` 解析，避免不同入口在不同 Ascend SoC 上选择不一致。`sm_*`/`compute_*` 只用于 CUDA 兼容宏、trace 或诊断，绝不作为 ccec 的目标架构。

## 2. 五阶段模型（phase/mode）

| 输入形态 | phase | 行为 |
|---|---|---|
| `.cu` 带 `-c` | compile | `-x asc --enable-simt` + 注入链 → 产出 `.o` |
| `.cu` 无 `-c` | compile_and_link | 编译 + 链接（自动补桥运行时/CANN 库） |
| 只链接（`.o`/库） | link | 透传 + 库策略 |
| `-E` | query/preprocess passthrough | 原样交给 ccec，不注入 SIMT/runtime 参数；输出语义仍需真机确认 |
| `-M/-MM/-MD` | dependency | 透传（DIAGNOSTIC_ONLY） |
| `-S`/`-ptx`/`-cubin` 等 | unsupported | 明确拒绝（puresimt 不产 NVIDIA 中间产物） |

## 3. 规则文件：`config/cu2cce_rules.json`

规则文件之外，`--options-file` 与 `@response-file` 均由同一个安全的 `shlex` 展开器处理；trace 会记录文件路径，不记录文件内容。工具链查询和 `-E` 命中 `queries` 后跳过规则注入，直接透传给 ccec。

五种动作（按序匹配，命中即停）：

| 动作 | 语义 | 例子 |
|---|---|---|
| `reject` | 直接 DriverError 终止 | `--ptx`、`-cubin`、裸 `-rdc` |
| `drop` | 丢弃 + 警告（可带 takes_value） | `-m64`、`-G`、`--use_fast_math` 组、`-arch=sm_*` |
| `replace` | 参数替换 | 部分拼写差异 |
| `forward` | 明示透传 | `-std=c++17`、`-M*` |
| `inject` | 按条件注入 | `-I ${CUCC_PATH}/include`、`-include simt_adaptor.h` |

条件（context 谓词）由驱动计算：`source_is_cuda`、`has_cucc`、
`has_simt_api`、`system_libc/not_system_libc`、`shared_link_without_pic`、
`has_cuda_arch` 等——**全部数据在 JSON 里，驱动不写死具体 flag**。

修改规则后必须过校验器：`python3 tools/validate_cucc_rules.py config/cu2cce_rules.json`。

## 4. 注入链（compile 阶段，`-x asc` 之后的确定性前缀）

```text
-D __CUDACC__/CUDA_VERSION            ← nvcc 身份兼容
-I ${CUCC_PATH}/include               ← 桥头文件（cuda_runtime.h 链）
-I ${CUCC_PATH}/include/shadow        ← shadow libc（仅非 system 模式）
-I ${ASCEND_TOOLKIT_HOME}/include 与 simt_api 根
-include include/bridge/kernel/simt_adaptor.h   ← 强制注入的 SIMT 适配层
-std=gnu++17 -D __CU2CCE_DEVICE_COMPILATION__
[-D __CU2CCE_SYSTEM_LIBC__ | __CU2CCE_NO_LIBC__]
[-D __CUDA_ARCH__=${CU2CCE_CUDA_ARCH}]（用户显式设置时）
```

## 5. FP64 前置类型策略（`CU2CCE_DEVICE_FP64_POLICY`）

- 触发条件：CUDA 源 + 编译阶段 + 策略非 `allow`。
- 扫描器（只读、保行号、忽略注释/字符串）：定位 `__device__/__global__`
  区域（含参数表与原型）内的 `double/doubleN` 记号。
- 三档：`error`（默认）——ccec 启动**前**终止并给 file:line 清单与
  "手动改 float"指引；`warn`——报告后放行；`allow`——跳过扫描。
- 输出统一带 `[type-policy]` 前缀；后端原生报错
  （"double precision operation is not allowed in aicore function"）
  亦由 `_emit_capability_diagnostics` 归并到同一前缀解释。
- 已知限制（如实记录）：宏生成的签名可能漏报；浮点字面量（如 `2.0`
  隐式 double 语义）不在检查范围。

## 6. 宿主 flag 策略（-Xcompiler 通道）

- 白名单放行：`-fPIC/-fno-exceptions/-pthread/-openmp` 等；
- 已知无意义项 drop + 警告：`-m64/-march/-mtune/-fPIE` 等；
- 其余按规则走；strict 模式（`CU2CCE_STRICT_FLAGS=1`）把未识别长选项与
  未验证库升级为错误。
- 链接阶段：`-lcublas` 等 WARN_FORWARD（不自动替换——诚实边界）；
  `-lcce_bridge_runtime` 由桥注入。

## 7. CLI 模式

```bash
cucc [nvcc 风格参数]                 # 正常编译/链接
cucc --dryrun ...                    # 打印最终 ccec 命令不执行
cucc --proxy-mode ...                # shlex 单行输出（供父包装器消费）
cucc --cmake-identity                # CMake shim 身份
CU2CCE_TRACE_FILE=f.json CU2CCE_DEBUG=1   # 规则决策 JSON / 调试
```

## 8. 与 mock_nvcc 的关系

`mock_nvcc.py` 拦截 CMake 对 nvcc 的探测（--version/--simple-preprocess/
ABI 编译），其余一切真实编译调用**进程内直接调用同一 RuleDriver**——
保证"CMake 看到的行为"与"命令行行为"逐字节一致。
