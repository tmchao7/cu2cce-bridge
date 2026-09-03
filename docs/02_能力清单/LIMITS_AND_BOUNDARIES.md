# 边界与限制（Limits & Boundaries）

> 本文档是**反面清单的唯一权威入口**：每条限制都附带真机证据与"为什么
> 不绕过"的理由。能力正面清单见 `SUPPORTED_SCOPE.md`。

## 1. 后端架构边界（昇腾硬件/CANN 决定，桥不可修）

| # | 边界 | 证据 | 桥的行为 |
|---|---|---|---|
| 1 | **device 无 double**：无 FP64 数学、无 doubleN 向量类型 | `unknown type name 'double2'`；"double precision operation is not allowed in aicore function" | FP64 前置策略编译前报错引导手动降级；不伪实现、不静默降精度 |
| 2 | **每 linked library 一个 `.cu` 编译单元**：ccec 对每个 TU 重复生成 `BishengLaunchKernelWithHostArgs` / `__cce_rtLinkedDevBinaryAscPureSimtEagerRegister` | multi_cu_diagnostic 真机 ld duplicate symbol（矩阵固化） | 构建面放行、host 链接失败记 DIAG-ONLY；列为昇腾侧聚合能力议题 |
| 3 | reduction 类样本：跨 warp `thread_block_tile>32` + device double | 上游源码形态 | 排除出构建图（显式声明，不报错不掩盖） |
| 4 | PTX/CUBIN/FATBIN/设备链产物 | cucc 明确 reject | 无降级路径 |
| 5 | `aclblasGemmEx` 的 lda/ldb/ldc、`aclblasGemvEx` 的 (lda,incx,incy) 为保留参数必须 -1；scalar 必须 device | ErrMsg "only support -1"（round-4） | wrapper 在 ACL 边界转换；宿主侧保持 CUDA 语义校验 |
| 6 | aclblasGemmEx 仅 (16F,16F,16F)/(16F,16F,32F)/(8I,8I,32I) 组合 | 真机界定 | 其余 dtype 诚实 NOT_SUPPORT |

## 2. 前端（bisheng ccec）事实边界

| # | 事实 | 后果 | 桥的对策 |
|---|---|---|---|
| 1 | `__host__/__device__/__forceinline__` 是语言关键字非宏 | `#define` 兜底会剥离执行空间声明（曾致误导报错） | 禁止宏重定义（契约测试锁定）；`__device__` 经 `simt_adaptor` 映射到 `__SIMT_DEVICE_FUNCTIONS_DECL__` |
| 2 | 全套 CUDA 向量类型（char1..float4）原生注入（ext_vector_type） | 桥侧 struct 重定义 = typedef 冲突 | 桥零定义，文档化空壳 |
| 3 | `.cu` TU 内注入 device-only `malloc/free` | host 代码裸 malloc/free 报 "call to __device__ function from __host__ function" | 文档化：宿主内存用 cudaMallocHost/std/new[]；样例全部合规 |
| 4 | 独立 `__device__` 拼写不被识别（`__global__` 可） | device helper 需映射 | `__device__ → __SIMT_DEVICE_FUNCTIONS_DECL__`（跟随 aicore/simt_callee 双模式；开关 `CU2CCE_NO_DEVICE_KEYWORD_MAP`） |
| 5 | glibc 2.39 系统头与 ccec 前端存在兼容缝隙 | 大项目默认 shadow libc；`CU2CCE_LIBC_MODE=system` 供规避 | 双模式 + 文档 |

## 3. 桥自身的设计边界（诚实不掩盖）

| # | 边界 | 说明 |
|---|---|---|
| 1 | FP64 扫描为词法启发式 | 宏生成签名可能漏报；浮点字面量（`2.0` 隐式 double）不在范围 |
| 2 | 默认流 = 单条桥内自有流 + 消费点设备同步 | 忠实 legacy 语义；极端高频默认流事件场景有性能成本（正确性优先） |
| 3 | `cudaHostRegister/HostGetDevicePointer/HostUnregister/HostGetFlags` | 返回 ACL_ERROR_API_NOT_SUPPORT + 诊断（声明/映射存在，无伪成功） |
| 4 | BLAS1（axpy/copy/dot/scal/swap/nrm2） | 参数校验后返回 not-support（错误归因一致） |
| 5 | external fetch 仅 http/https 公网 host | validate_fetch_url.py 拒绝环回/私有/保留地址；GitHub allowlist 兜底 |
| 6 | 昇腾公开文档站为 SPA，程序化抓取不可用 | 以 `inventory_simt_api.sh`（本机头枚举）+ 用户侧头文件 dump 替代 |

## 4. 悬置议题（非桥侧，材料齐备）

- asyncAPI 单元素丢失：三变体最小复现未触发；现场索引漂移。完整诊断方向记录在 `tools/real_test/diag/` 和真机回归日志中；原工作区历史材料未随仓库交付。
- 多 `.cu` 聚合：同上，建议提报昇腾团队。

## 5. 修改边界文档的规则

任何新增"不支持/限制"必须：真机证据 + 本文档条目 + 对应矩阵行 +
（如适用）契约测试，四件套同步。

## 8. API 映射的证据边界（2026-09-03）

### 8.1 不能作为支持证据的内容

```text
官方文档中存在函数名
头文件可以被 include
CMake configure 成功
函数返回 ACL_SUCCESS 但实际是 no-op
只完成链接、没有真机执行
```

### 8.2 当前后端明确不伪造的能力

```text
完整 CUDA Driver API
CUDA Graph
PTX/CUBIN/FATBIN
RDC/device-link
UVM 和完整 Host memory mapping
NCCL → HCCL 自动 alias
cuDNN/cuFFT/cuSPARSE/cuRAND 自动 alias
```

### 8.3 Runtime 库名称规则

新版本 CANN 优先使用 `libacl_rt.so`。旧版本若只有 `libascendcl.so`，bridge 可以 fallback 并输出诊断；这不代表两个库在所有 CANN 版本中可以同时加载，也不代表 `libascendcl.so` 是新版本推荐入口。
