# 全表面对齐审计（第二轮，2026-08-28）

> 对照范围从 CMake/Make 扩展到**全表面**：cucc 参数面、运行时 API 面、
> 头文件生态、工具链。方法：三个并行审计（逐 flag / 逐函数 / 逐头文件），
> 全部结论锚定两个仓库的具体文件与行号。
> 参照：cu-bridge v3.8.0（MACA/mxcc 后端）；本桥：main@main（CANN/ccec 后端）。
> CMake/Make 机制对齐见 `CMAKE_MAKE_ALIGNMENT_WITH_CUBRIDGE.md`（已完成三轮）。

## 1. cucc 参数面（nvcc → ccec 协议翻译）

### 1.1 本轮补齐的缺口（cu-bridge 有、我们缺 → 已按 NPU 特性落规则）

| flag 族 | 处理 | 落点 |
|---|---|---|
| `-dc` / `--device-c`（CMake separable=ON 会发出！） | **reject**（同 -rdc=true 策略） | compile.reject |
| `-dw` / `--device-w` / `--compile` | **replace → `-c`**（整程序编译是唯一设备模式） | compile.replace |
| `-link` | drop（此前被 `-l*` forward 误吞） | all.drop |
| `-l:libcudart*` / `-l:libcuda.so*` / `-l:libOpenCL*` / `-lculibos` | drop + 理由 | all.drop |
| `--Wno-deprecated-gpu-targets`（双横线拼法） | drop | all.drop |
| `-expt-relaxed-constexpr`（单横线） | drop（ccec/clang15 原生支持） | all.drop |
| `--expt-extended-lambda` 等四种拼法 | drop（ccec/clang15 原生支持扩展 lambda） | all.drop |
| `-flto` / `-flto=*` / `-compress-all` | drop（未在 puresimt 验证/NVIDIA 专属） | all.drop |
| `-extra-device-vectorization`（双拼） / `-allow-unsupported-compiler`（双拼） | drop | all.drop |
| `--diag_suppress=*` / `--diag-suppress=*` | drop | all.drop |
| `-restrict` / `--restrict` / `-cuda` / `-no-canonical-prefixes` / `--source-in-ptx` | drop | all.drop |
| `--linker-options` / `--libdevice-directory`（带值） | drop + 建议用 `-Wl,` | all.drop |
| `-Wdefault-stream-launch` / `-Wmissing-launch-bounds` / `-Wext-lambda-captures-this` | drop（nvcc 专属警告名） | all.drop |
| `--Werror=implicit-fallthrough*` | drop（clang 拒绝 =N 后缀） | all.drop |
| `--offload-arch`（带值/等式） | drop（架构由 CU2CCE_NPU_ARCH 决定） | all.drop |
| `-threads=*` / `-t=*` / `--disable-warnings` / `-x=*` | drop | all.drop |
| `--Werror`（双横线）+ 警告名 | **python 配对消费**（对应 bin/cucc:84-116 的配对逻辑） | driver 主循环 |
| `-I/usr/local/cuda*`（陈旧 CUDA 头路径） | drop（镜像 -L shim 逻辑与其 conf.json 过滤） | driver 主循环 |

### 1.2 语义差异但结果等价（有意保留，不做）

| flag | cu-bridge | 我们 | 评估 |
|---|---|---|---|
| `--ptx` / `nvrtc` / `nvlink` / `ptxas` / `fatbinary` | 静默过滤 | **硬 reject** | 我们更严格：构建停而不是静默产错误二进制 |
| `-Xcompiler` 值修正（xhost→Xhost 等） | bash 拼写修正 | 白名单/黑名单 + 无需 shell 重排 | 我们 by-design 无此类拼写 bug 面 |
| `-arch/-gencode` | 翻译成 `-DNV_ARCH_*` | drop + `--cce-aicore-arch` 由 CU2CCE_NPU_ARCH 决定 | 平台差异，正确 |
| `-lcudart` 等 | replace 成 `-lmcruntime` | drop + 桥运行时注入 | 平台差异 |
| `@file` 展开 | 自制转义器 | shlex + 深度 16 + 报错 | 我们更稳 |
| `__CUDA_ARCH__` | 总是定义 | 仅 `CU2CCE_CUDA_ARCH` 设置时定义 | **注意**：分支于该宏的代码静默走 host 路径；保持显式（用户主动声明） |

### 1.3 我们有、cu-bridge 没有（优势面，保持）

严格模式 `CU2CCE_STRICT_FLAGS`、JSON 决策 trace、FP64 前置策略、库能力
policy（不静默映射）、`-S/--fatbin/--maxrregcount` 等大量带值双 token 形式
的正确消费、`-fPIC` 自动注入、`-std=gnu++17` 默认注入、CMake 协议完整
（simple-preprocess/ABI 中和/-MD 保真）。

## 2. 运行时 API 面（libruntime_cu ~330 函数 + libsymbol_cu 433 cu* 符号 vs 我们 ~160 函数）

### 2.1 本轮补齐

| 项 | 说明 |
|---|---|
| `wcudaGetErrorName` 真名表 | 从恒 "unknown" 改为 16 项 ACL 码→cudaErrorXxx 风格静态表（CUDA 契约：无状态按码查询）；mock 头同步补 `ACL_ERROR_BAD_ALLOC/FEATURE_UNSUPPORTED`（真实 CANN 值 200000/9000） |
| `wcudaGetDeviceFlags` | 新增（返回默认 0 flags，与 SetDeviceFlags no-op、canMapHostMemory=0 一致） |
| NVTX 兼容面 | `include/nvtx3/nvToolsExt.h` + `include/nvToolsExt.h` + `bridge/tools_ext/nvtx_stub.h`：RangePushA/Pop/MarkA/RangeStartA/RangeEnd/PushEx 六入口 no-op shim（NPU 适配：注解面是 aclprof，形状不同，不做假数据） |
| `math_constants.h` | M_PIf 族 + CUDART_INF_F/NAN_F（通用 IEEE 常量，诚实：double 常量标注 host-only） |

### 2.2 已识别、刻意不补（NPU 特性边界，写入 LIMITS）

| 项 | cu-bridge 做法 | 为什么我们不抄 |
|---|---|---|
| 驱动 API `cu*`（433 符号两层设计：wcu 层 + symbol 导出层） | 完整（mcDrv* 映射） | 需要 CANN 的 module/function 加载故事（aclrtLaunchSIMT 直发不需要 module）；一旦 CANN 提供 module API 可按其两层模式补——设计已在文档记录 |
| Managed/UVM 内存（真实 mcMallocManaged） | 真 UVM | Ascend 无 UVM：我们的 plain-alloc 回退 + MemAdvise no-op 是诚实上限 |
| HostRegister/GetDevicePointer | mcHostRegister | ACL 无公开注册 API（平台限制非设计选择）；canMapHostMemory=0 保证优雅降级 |
| Graph 完整族（~85 函数） | mcGraph* 真实现 | aclrt graph 面待验证；现有 8 入口 stub 保持 |
| Occupancy 真实查询 | mcOccupancy* | 我们保守估算（2048 上限）；真机校准后可接 aclrt 查询 |
| NVTX 真注解 | mctx* | aclprof 形状不同；no-op shim 已给（见上） |

### 2.3 我们有、cu-bridge 没有（保持）

默认流三拼写桥内代持 + 事件注册表消费点语义（MACA 原生接受 NULL 流，
他们不需要——这是后端差异的必然）、ACL 全引导 bridge_init、错误码真名表
（他们 GetErrorString 是全静态 switch）、FP64 编译期策略、WCUDA_DEBUG 逐调用
轨迹、`wcudaCtx*` runtime 形态 API。

## 3. 头文件生态（1617 头 vs 29 头）

| 组 | cu-bridge | 我们 | 定性 |
|---|---|---|---|
| thrust/cuda(libcu++)/cccl（1601 文件） | 大头；MetaX 做过 PTX→intrinsic 重写（单文件 3975 行） | 无 | **SDK 级依赖**，超出桥的静态可写范围；依赖 ccec SIMT 后端对模板的支持程度，需单独评估（可能 CANN SDK 有对应物） |
| 库头全集（cublasLt/cufft/cusolver/cusparse/curand/cudnn/npp/nccl/nvjpeg…146 个） | 1 行重定向→bridge adaptor→SDK 库 | 仅 cublas_v2 最小面 | 每个库是独立映射工程（需逐库找 Ascend 等价物），按需求滚动 |
| crt 全集 + math_constants + mma + nvfunctional | 有 | 入口名有；**math_constants.h 本轮补齐**；mma/wmma 双方都不可用 | 数学常量已闭环；wmma 双平台都不支持 |
| nvtx3 | 有（真 shim） | **no-op shim 本轮补齐** | 注解语义 vs 真数据的差异已文档化 |
| fp8/awbarrier/pipeline | 有 | 无 | Hopper 世代特性，CANN SIMT 待评估 |
| cuda-samples/Common | 有 | harness 侧已有（third_party） | 等价 |
| GL/EGL/CL 互操作 | 有 | 无 | 罕见，超出 NPU 场景 |
| driver API 头（cuda.h/driver_types.h） | 有 | 类型面在 bridge/runtime 内 | cu* 函数面见 §2.2 |

## 4. 工具链面

| 工具 | cu-bridge | 我们 |
|---|---|---|
| ptx2cpp（inline PTX 源翻译二进制） | 有（部分指令类） | 无——我们整体拒绝 PTX/inline asm；诚实差距（NPU 后端同样无 asm 路径） |
| gomxccbin + conf_regex.yaml（Go 正则改写命令） | 有 | 无需——单进程 python 驱动内完成全部改写 |
| process_args.py | 有（CUCC_USE_PYTHON 快路径） | N/A（我们本来就是 python） |
| wnvml（Python 设备管理） | 有（依赖 pymxsml） | 无（昇腾用 npu-smi；device_arch.sh 已覆盖架构探测场景） |
| conf_bazel.json / WCUDA_BAZEL_TF_BUILD | 有 | 无（bazel 工程未进当前需求） |
| fetch 安全校验 / 回归三画像 / 证据矩阵 / FP64 策略 / trace | 无 | **我们独有**（保持） |

## 5. 结论

- **未走偏**：架构四要素（头替换 + 宏映射 + 规则驱动编译翻译 + 运行时承接）
  与 cu-bridge 完全同构；差异全部可归因到后端（MACA 有 UVM/完整 graph/
  驱动 API/宿主 mxcc 链接，Ascend 无/待验证/刻意保持 g++ 链接）。
- **本轮静态补齐**：参数面 30+ flag 规则 + `--Werror` 配对 + 陈旧 include 过滤；
  运行时 GetErrorName 真名表 + GetDeviceFlags + NVTX shim + math_constants.h。
- **周一真机验证项**：新增 flag 规则在真实 ccec 下不破坏必需门禁；
  `-dc` reject 不影响（CMake 层已强制 separable OFF）。
- **后续大项**（需独立立项）：头文件生态的 thrust/cccl 评估、驱动 API cu*
  两层设计、graph 族 aclrt 验证、逐库 BLAS 扩展。
