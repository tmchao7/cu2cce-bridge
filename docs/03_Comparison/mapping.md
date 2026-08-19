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

## Phase A3：SIMT 适配宏表（路线 A 核心新增）

| # | cu-bridge 源文件 | cu2cce 目标文件 | 状态 | 备注 |
|---|---|---|---|---|
| 19 | （cu-bridge 无，昇腾新增） | include/bridge/kernel/simt_adaptor.h | 已移植 | SIMT↔CUDA 宏表；__syncthreads/__shared__/__threadfence 已映射，__shfl/atomic 待真机核对签名 |

## Phase A2：cucc 驱动 + CMake 探测（骨架）

| # | cu-bridge 源文件 | cu2cce 目标文件 | 状态 | 备注 |
|---|---|---|---|---|
| 12 | bin/cucc | bin/cucc | 已移植 | @file/--options-file 展开、编译单元/阶段识别、注入 cu2cce include + SIMT adaptor、--npu-arch=dav-c310、链接 wrapper 库；bash -n 通过 |
| 13 | tools/make_maca | tools/make_cce | 已移植 | 包 make：建伪 CUDA 目录（bin/nvcc→cucc），设 CUDA_PATH/PATH，调 make |
| 14 | tools/cmake_maca + cmake_mock | tools/cmake_cce + tools/mock/ | 已移植 | 包 cmake：mock nvcc 应答 CMake CUDA 探测（--version/__cmake_determine_cuda/CompilerId），真实编译透传 cucc |
| 15 | tools/mock/ | tools/mock/ | 已移植 | mock_nvcc.py + nvcc_entry.sh；本机已测 --version 与探测应答 |

## Phase A4：库（cublas→aclblas 最小子集）

| # | cu-bridge 源文件 | cu2cce 目标文件 | 状态 | 备注 |
|---|---|---|---|---|
| 20 | include/cublas_v2.h | include/cublas_v2.h | 已移植 | 入口转发 |
| 21 | include/bridge/blas/cublas_v2_wrapper.h | include/bridge/blas/cublas_v2_wrapper.h | 已移植 | cublasSgemm→wcublasSgemm→aclblasGemmEx（类型补全）；Dgemm 昇腾不支持返回 not-support |
| 22 | src/bridge/blas/src/cublas_v2_wrapper.cpp | src/bridge/blas/src/cublas_v2_wrapper.cpp | 已移植 | handle 关联 stream，transC 固定 N，computeType 高精度 |
| 23 | （昇腾新增） | tests/mock_acl/acl/ops/acl_cblas.h | 已移植 | mock aclblas 签名供语法验证 |

## 跳过项（昇腾无对应）

| # | cu-bridge 源文件 | 原因 |
|---|---|---|
| 16 | tools_src/ptx2cpp | 昇腾无 PTX 内联汇编；需自行处理 __asm__ |
| 17 | include/bridge/{dnn,fft,rand,...} 库 | 昇腾无同名 API 级对应，后置；仅 blas 先行 |
| 18 | src/bridge/runtime/src/cuda_driver_wrapper.cpp | 昇腾无同形 Driver API，后置 |
