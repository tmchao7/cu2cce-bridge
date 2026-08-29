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
