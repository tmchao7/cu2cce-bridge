# Ascend 库映射研究（联网调研，2026-08-28）

> 目标：为"cu-bridge 有、我们缺"的库面找到**昇腾官方等价物与映射路线**，
> 并给出证据来源。本文是映射决策的依据文档；shim 实现按需求逐库立项。
> 调研方式：昇腾社区/CANN 文档检索（hiascend 文档站为 SPA，正文经搜索
> 摘要获取，来源链接逐条附后）；安装树核对命令见文末（周一真机执行）。

## 1. 集合通信：NCCL → HCCL（映射路线完整，可信度最高）

HCCL（Huawei Collective Communication Library）是 NCCL 的昇腾对应物，
接口分组一一对应，头文件 `hccl.h`：

| NCCL | HCCL |
|---|---|
| `ncclGetUniqueId` | `hcclGetUniqueId` |
| `ncclCommInitRank` / `ncclCommInitClusterInfo` | `HcclCommInitRootInfo` / `HcclCommInitClusterInfo`（+ `HcclCommInitClusterInfoConfig`） |
| `ncclCommCount` / `ncclCommUserRank` | `HcclGetRankSize` / `HcclGetRankId` |
| `ncclAllReduce` / `ncclBroadcast` / `ncclSend` / `ncclRecv` | `HcclAllReduce` / `HcclBroadcast` / `HcclSend` / `HcclRecv` |
| `ncclConfig_t`（2.x 后期） | `HcclCommConfig`（缓冲区/确定性计算/通信域名） |

开源仓库：gitcode.com/cann/hccl（官方样例含 HcclAlltoAll）。
桥侧动作：`-lnccl` 已在 capabilities 标注 map_to=HCCL；真正的 shim
（nccl.h → Hccl* 转发）按多卡需求立项。

来源：
- https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/920beta1/commlib/hcclug/docs/zh/api_ref/hccl_header_and_lib.md
- https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha003/API/hcclapiref/hcclcpp_07_0016.html
- https://gitcode.com/cann/hccl

## 2. 驱动 API（cu*）的 CANN module 故事：已确认存在

cu-bridge 的 cu* 两层设计依赖 mcModuleLoadData/cuModuleGetFunction。
**CANN 有对应三件套**（8.0.RC3 起）：

| CUDA | Ascend |
|---|---|
| `cuModuleLoadData/Ex`（内存） | `aclrtBinaryLoadFromMem` / `aclrtBinaryLoadFromData` / `aclrtBinaryLoadFromFile` |
| `cuModuleGetFunction` | `aclrtBinaryGetFunction`（按 kernelName 取 funcHandle） |
| `cuLaunchKernel(funcHandle…)` | funcHandle 送 Kernel Launch 接口（含 funcEntry 入口标识） |

支持面：Atlas A3/A2 系列起。**结论**：cu* 两层（wcu 层 + symbol 导出层）
在昇腾有官方路径，未来立项可行——从"需要 CANN module 故事"升级为
"CANN module 故事已确认"。

来源：
- https://www.hiascend.com/document/detail/zh/canncommercial/80RC3/apiref/appdevgapi/aclcppdevg_03_0150.html （aclrtBinaryGetFunction）
- https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/82RC1/API/appdevgapi/aclcppdevg_03_1896.html （aclrtBinaryLoadFromData 支持面）

## 3. FFT：cuFFT → SiP 加速库 / aclRfft1D

两条官方路径：
1. **SiP 信号处理加速库**：`asdFftCreate` 等 Plan 框架接口（创建→执行，
   与 cuFFT Plan 范式同构）；源码 gitcode.com/cann/sip；
2. **算子库融合算子**：`aclRfft1D`（实数 1D FFT 高层封装，ops-math/aol）。

桥侧动作：capabilities 已标注 map_to；shim（cufft.h → SiP Plan 转发）
按需求立项。

来源：
- https://www.hiascend.com/document/detail/zh/canncommercial/83RC1/API/SiPAPI/SIP_API_0028.html
- https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1/API/aolapi/context/aclRfft1D.md
- https://gitcode.com/cann/sip

## 4. BLAS/GEMM：cublas → aclblas（已有实验线）/ ATB

- GEMM 基础面：`aclblasGemmEx`（我们 BLAS 实验线已在真机跑通三用例）；
- Transformer 负载：**ATB（Ascend Transformer Boost）**是官方高性能算子库
  （cublasLt/cuDNN attention 类负载的昇腾对应物）；
- 算子模板库：**CATLASS**（昇腾版 CUTLASS，官方模板库）。

来源：
- https://www.hiascend.com/developer/techArticles/20250604-1 （CATLASS 官方文章）
- https://www.hiascend.com/cann/aol （CANN 算子库）

## 5. 模板库（thrust/libcu++/CCCL/CUTLASS）：生态结论

- **无官方 thrust/libcu++ 移植**（多轮检索确认）；
- 生态答案：CANN Next 的 SIMT 编程模型（官方口径与 CUDA 接口兼容度
  约 95%）+ CATLASS（CUTLASS 对应物）+ CANN 算子库；
- MetaX 的做法（copy+改写 1601 个模板头，单文件 intrinsic 重写 3975 行）
  证明纯手工移植代价极高且强绑定其编译器；昇腾侧正确路线是等/推
  CANN 官方 SIMT 模板库，或按项目逐个把 thrust 用法改写为内核/算子。

来源：
- https://www.hiascend.com/developer/blog/details/02111204181287677435 （CUDA→CANN 迁移指南）
- https://www.hiascend.com/cann/aol

## 6. NVTX：no-op shim 已给（本轮），真注解面为 aclprof

cu-bridge 用 mctx*；昇腾的注解/性能面是 **aclprof**（API 形状不同）。
已交付 no-op shim（编译兼容优先）；如需真注解，走 aclprof 单独接。

## 7. 周一安装树核对清单（确认本机 CANN 9.2.0 实际提供哪些）

```bash
ls /usr/local/Ascend/cann-9.2.0/include/acl/ | head -30
ls /usr/local/Ascend/cann-9.2.0/include/acl/ops/
find /usr/local/Ascend/cann-9.2.0/include -name 'hccl*' -o -name '*fft*' -o -name '*atb*' | head
ls /usr/local/Ascend/cann-9.2.0/lib64/ | grep -iE 'hccl|fft|atb|blas' | head
grep -l aclrtBinaryLoad /usr/local/Ascend/cann-9.2.0/include/acl/acl_rt.h
```

核对结果回填本表：存在的 → 升级为可实现；不存在的 → 维持 honest gap。

## 8. 桥侧已落地

- capabilities 表驱动化（map_to 字段）+ 驱动警告消息带映射路线
  （`-lnccl` 现在会提示 → HCCL 路线，strict 下报错）；
- 本文档作为映射决策依据；shim 实现逐库立项。

## 10. 2026-09-03 官方头文件、库文件与状态校准

本节以昇腾社区 CANN 官方文档为证据入口，状态仍需以服务器实际安装树、编译探针和真机结果共同确认。官方页面中的“存在”只说明接口属于 CANN 开放面，不代表 cu2cce-bridge 已经提供 CUDA 兼容 wrapper。

官方依据：

```text
https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/ascendcopapi/docs/zh/api/api_list.md
https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/latest/API/headerliblist/hfandlf_09_0001.html
```

### 10.1 Runtime 头文件与库文件

| CUDA/bridge 面 | CANN 头文件 | CANN 官方库文件 | bridge 处理 | 状态 |
|---|---|---|---|---|
| 设备、Context、Stream、同步、内存 | `acl/acl_rt.h` | `libacl_rt.so` | `cuda* → wcuda* → aclrt*` | `SUPPORTED/ADAPTED` |
| C++ Runtime 扩展 | `acl/acl_rt_api.h` | `libacl_rt.so` | 当前不自动暴露全部 C++ 扩展 | `NOT_VERIFIED` |
| Dump | `acl/acl_dump.h` | `libascend_dump.so` | 没有 CUDA Dump 直译 | `ROUTE_ONLY` |
| Profiling | `acl/acl_prof.h` | `libmsprofiler.so` | `cudaProfilerStart/Stop` 保持兼容占位 | `COMPAT_NOOP/ROUTE_ONLY` |
| Tensor 数据传输 | `acl/acl_tdt.h` | `libacl_tdt_channel.so` | 当前不映射 CUDA Runtime | `UNSUPPORTED` |
| 共享队列/Buffer | `acl/acl_tdt_queue.h` | `libacl_tdt_queue.so` | 当前不映射 CUDA Runtime | `UNSUPPORTED` |

新版本 CANN 优先使用 `libacl_rt.so`。`libascendcl.so` 仅作为旧版本安装树的 fallback；不能因为旧库名仍存在就把它写成当前官方首选。

### 10.2 Ascend C Device API

官方 Ascend C API 按编程模型分为 SIMT、SIMD、AI CPU 和 Utils 等类别。当前 CUDA kernel 迁移只使用其中经过 ccec puresimt 验证的子集：

| Ascend C 面 | 典型头文件 | 库依赖 | 与 CUDA 的关系 |
|---|---|---|---|
| SIMT | `simt/asc_simt.h` | 通常不单独链接运行库 | 用于当前 `simt_adaptor.h` 的核验依据 |
| SIMD C | `c_api/asc_simd.h` | Device 编译期能力 | 不是 CUDA Runtime API 的直接替换 |
| Tensor API | `tensor_api/tensor.h` | Device 编译期能力 | 需要重写为 Ascend C Tensor 编程 |
| TPipe/TQue | `basic_api/kernel_basic_intf.h` 等 | Device 编译期能力 | 不由 CUDA wrapper 自动生成 |
| 高阶 API | `adv_api/kernel_api.h`、`adv_api/tiling_api.h` | Device/Host 分工 | 属于 Ascend C 原生开发路线 |

当前 `asc_*` 名称只有在服务器 CANN 头文件和 ccec 编译探针均确认后才可标记为 `SUPPORTED`。

### 10.3 通信、数学和高层算子库

| CUDA 库 | 官方 Ascend 路线 | 当前状态 | 处理原则 |
|---|---|---|---|
| NCCL | `hccl/hccl.h`、`libhccl.so` | `ROUTE_ONLY/NOT_VERIFIED` | 不自动把 `-lnccl` 改成 `-lhccl` |
| cuBLAS | `acl/ops/acl_cblas.h`、`libacl_cblas.so` | `EXPERIMENTAL` | 仅保留已验证 Gemm/Gemv 形状 |
| cuFFT | SiP、FFT 算子、`aclnn` 路线 | `ROUTE_ONLY` | 不生成 `cufft.h` 的伪实现 |
| cuDNN | `aclnn`、ATB 等高层路线 | `ROUTE_ONLY` | 不自动生成 `CUDA::cudnn` |
| CUTLASS | CATLASS、Ascend C | `ROUTE_ONLY` | 需要按工程重写模板和布局 |
| NVTX | `acl/acl_prof.h`、Msprofiler | `COMPAT_NOOP/ROUTE_ONLY` | 真 profiling 不等同 NVTX 注解 |

`aclnn` 是 CANN 单算子 API，不是 CUDA kernel 语法转换结果；CUDA kernel、CUDA Runtime 和 `aclnn` 调用必须分别评审。

### 10.3.1 CUDA 头文件与 Wrapper parity 边界

`cuda_to_cce_mcr_adaptor.h` 只覆盖 Runtime 宏替换，不能替代 CUDA 头文件树和 Driver/Interop/库
Wrapper 的登记。本项目额外维护 `config/cuda_api_parity_inventory.json`：它记录参考 CUDA header、
reference wrapper、cu2cce 文件、CANN 路线、当前状态与 future implementation location。

- `cuda.h`、Runtime/SIMT、FP16/BF16 等核心入口可转发到现有 CANN/bridge surface；
- Graph、CUDA Driver、EGL/GL/VDPAU、NVRTC 和未验证库 API 保留 CUDA 形状，但不宣称存在 Ascend ABI；
- 复杂原型位于 `cuda_to_cce_reserved_wrapper_parity.h` 的 `#if 0`，供后续 CANN 能力到位后恢复；
- `cufft`、`cudnn`、`nccl` 等仅登记到 SiP/aclnn/HCCL 等迁移路线，绝不自动改写为真实库链接。

### 10.4 Runtime/device 替换表登记与服务器证据

`cuda_to_cce_mcr_adaptor.h` 以 CANN/bridge 的实际路径维护 active 映射；
`cuda_to_cce_reserved_parity.h` 以 `#if 0` 保存参考表中尚无安全 Ascend 等价物的
`RESERVED_NPU` 项。提交的 `config/cuda_api_parity_inventory.json` 是 reference 名称和当前状态的
可复现快照，因此不需要在服务器额外克隆参考仓库即可运行：

```bash
python3 tools/validate_cuda_api_parity.py \
  --implementation include/bridge/runtime/cuda_to_cce_mcr_adaptor.h \
  --inventory include/bridge/runtime/cuda_to_cce_reserved_parity.h \
  --runtime-header include/bridge/runtime/cuda_runtime_wrapper.h \
  --status-inventory config/cuda_api_parity_inventory.json
```

该命令校验 `missing_entries=0`、`duplicate_entries=0`、active 映射不含 MACA 后端名称，
以及 `ACTIVE_ADAPTED`/`UNSUPPORTED_TYPED` 的 `wcuda*` 入口在公开 wrapper 头中有类型化声明。
它是**登记完整性**证据，不是对 Graph、IPC、UVM、纹理或 CUDA Driver ABI 的实现声明。
`server_cmd.txt` 将它与 Runtime 链接探针、core SIMT blocking 编译以及
experimental/legacy/FP16/BF16 diagnostic-only 编译一起记录。
