# cu-bridge → cu2cce-bridge 逐文件对照表

> 对照基准：`../cu-bridge`（3.8.0，Apache-2.0）
> 状态枚举：`未动` / `移植中` / `已移植` / `跳过`（跳过须写原因）
> 每次会话开始先看本表，决定下一步。新文件先加行再动手。

## Phase A1：host Runtime API 桥（最小核心集）

| # | cu-bridge 源文件 | cu2cce 目标文件 | 状态 | 备注 |
|---|---|---|---|---|
| 1 | include/bridge/runtime/cuda_benchmark_targets.h | include/bridge/runtime/cuda_benchmark_targets.h | 已移植 | 版本模拟 + CUBRIDGE_API，结构一致 |
| 2 | include/bridge/runtime/cuda_wrapper_internal_types.h | include/bridge/runtime/cuda_wrapper_internal_types.h | 已移植 | NV arch 枚举 → SOC 类型枚举（预留） |
| 3 | include/bridge/runtime/cuda_wrapper_internal.h | include/bridge/runtime/cuda_wrapper_internal.h | 已移植 | bridge_init + BRIDGE_INIT 宏 |
| 4 | include/bridge/runtime/cuda_to_maca_mcr_adaptor.h | include/bridge/runtime/cuda_to_cce_mcr_adaptor.h | 移植中 | 宏表 mc*→acl*；错误码/句柄/函数已按真实 acl_base.h 对齐；成功码 ACL_ERROR_NONE |
| 5 | include/bridge/runtime/cuda_runtime_wrapper.h | include/bridge/runtime/cuda_runtime_wrapper.h | 移植中 | wcuda* 声明；含 Device/Peer/Context/Stream/Event/Memory/Error/Version |
| 6 | （cu-bridge 无，昇腾新增） | include/bridge/runtime/cuda_wrapper_types.h | 已移植 | _cceDeviceProp_t 模拟 cudaDeviceProp |
| 7 | src/bridge/runtime/src/cuda_runtime_wrapper.cpp | src/bridge/runtime/src/cuda_runtime_wrapper.cpp | 移植中 | wcuda*→aclrt*；已按真实签名修正（ResetDevice 带参/SocName 返回类型/Memset 4 参/QueryEvent 带 status/无 StreamQuery） |
| 8 | include/cuda_runtime.h | include/cuda_runtime.h | 已移植 | 转发入口 |
| 9 | include/__cuda_runtime.h | include/__cuda_runtime.h | 已移植 | 组装 wrapper+adaptor+ACL 头 |
| 10 | include/cuda_runtime_api.h | include/cuda_runtime_api.h | 已移植 | 转发入口 |
| 11 | CMakeLists.txt | CMakeLists.txt | 已移植 | 构建 cce_bridge_runtime.so，链 ASCEND 库 |

## Phase A2：cucc 驱动 + CMake 探测（骨架）

| # | cu-bridge 源文件 | cu2cce 目标文件 | 状态 | 备注 |
|---|---|---|---|---|
| 12 | bin/cucc | bin/cucc | 移植中 | 骨架版：参数收集→找 bisheng→注入 include/link→调用；TODO 完整参数处理 |
| 13 | tools/make_maca | tools/make_cce | 未动 | 包 make，拦截 nvcc→cucc |
| 14 | tools/cmake_maca | tools/cmake_cce | 未动 | 包 cmake |
| 15 | tools/mock/ + tools/cmake_mock | tools/mock/ | 未动 | 无 CUDA Toolkit 时 mock 编译器探测（需 mock bisheng） |

## 跳过项（昇腾无对应）

| # | cu-bridge 源文件 | 原因 |
|---|---|---|
| 16 | tools_src/ptx2cpp | 昇腾无 PTX 内联汇编；需自行处理 __asm__ |
| 17 | include/bridge/{blas,dnn,fft,rand,...} 库 | Phase A4+，cublas→aclblas 先行（acl/ops/acl_cblas.h） |
| 18 | src/bridge/runtime/src/cuda_driver_wrapper.cpp | 昇腾无同形 Driver API，后置 |
