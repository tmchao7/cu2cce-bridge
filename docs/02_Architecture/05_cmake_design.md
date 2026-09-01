# CMake 设计与能力

> 回答：**不改动用户 CMakeLists 的前提下，CMake 如何把 CUDA 工程编译到
> 昇腾上？** 涉及：`tools/cmake_cce`、`tools/mock/mock_nvcc.py`、
> `cmake_modules/override/CUDAOverrides.cmake`、`cmake_modules/FindCUDAToolkit.cmake`。

## 1. 三层漏斗

```text
① tools/cmake_cce（Bash 入口）
   布置 CUDA 形状 shim 目录（bin/nvcc→nvcc_entry.sh→mock_nvcc.py）
   注入 -DCMAKE_USER_MAKE_RULES_OVERRIDE[_CUDA]=CUDAOverrides.cmake
   导出 CUDA_PATH/CANN_BASE/CU2CCE_NPU_ARCH/LIBC_MODE 等
        ▼
② mock nvcc（探测应答器）
   --version / --simple-preprocess(CMake≥3.24) / CMakeCUDACompilerABI.cu
   / __cmake_determine_cuda 全部短路应答；
   其余真实编译调用 → 进程内 RuleDriver → ccec
        ▼
③ CUDAOverrides.cmake（CMake 内置探测前生效）
   CMAKE_CUDA_COMPILER/DEVICE_LINKER/FATBINARY ← shim
   CMAKE_CUDA_RUNTIME_LIBRARY=None、CUDA_ARCHITECTURES=OFF
   vendor 正则接受 shim 的 NVIDIA-compat 身份
```

逃生开关：`CU2CCE_CMAKE_NO_OVERRIDE=1` 回退旧旁路（纯 `-DCMAKE_CUDA_*`）。

## 2. 用户视角：三条等价路径

前置（真机实测必需）：每个新 shell 先
`source /usr/local/Ascend/latest/bin/setenv.bash`，否则 ccec/ACL 库路径缺失，
`cucc -c/-o` 与 `cmake_cce` 内部调用会报错（harness 脚本会自动 source，
手动使用时必须自己执行）。

```bash
# ① 推荐入口（自动布置 shim + override）
bash tools/cmake_cce -S <工程根> -B build
bash tools/cmake_cce --build build --parallel

# ② 自定义安装前缀
CUCC_PATH=/opt/cu2cce bash tools/cmake_cce -S <工程> -B build

# ③ 大型项目常用组合
CU2CCE_LIBC_MODE=system bash tools/cmake_cce -S <工程> -B build -DCMAKE_BUILD_TYPE=Release
bash tools/cmake_cce --build build --parallel
```

CMakeLists 本身保持原样（`project(... LANGUAGES CXX CUDA)`、
`add_executable`、`find_package(CUDAToolkit)` 均可用）。

## 3. 能力矩阵（真机）

| 能力 | 状态 |
|---|---|
| 现代 CMake CUDA 工程（探测/对象规则/链接） | `SUPPORTED`（6 案例真机） |
| 多目录 + install/export + 跨目录依赖 | `SUPPORTED`（large_project） |
| Makefiles / Ninja | `EXPERIMENTAL`（make_cce / ninja_cce） |
| `find_package(CUDAToolkit)` 与 `CUDA::cudart/toolkit/cudart_static/cudadevrt` | `EXPERIMENTAL`（桥运行时承接） |
| NVIDIA CUDA Samples 零改写 | 必需：vectorAdd/matrixMul；诊断：simpleStreams/simpleMultiCopy 已 PASS、asyncAPI pending |
| multi `.cu` 静态库 | `DIAGNOSTIC_ONLY`（后端注册符号聚合缺口，证据固化） |
| RDC/device-link/separable | 不支持（明确拒绝） |

## 4. 设计要点（为什么这样做）

1. **override 注入而非替换 CMake**：CMAKE_USER_MAKE_RULES_OVERRIDE 在
   内置探测前生效，让 CMake 自己的流程跑通——升级 CMake 版本时兼容面
   由 CMake 维护，我们只固定与 ccec 相关的少数规则。
2. **mock 只答探测、真编译走驱动**：探测应答与实际编译共享同一
   RuleDriver，杜绝"探测通过、编译行为不一致"。
3. **属性清扫按 TYPE 守卫**：`CUDA_CUBIN/FATBIN_COMPILATION` 只能出现在
   OBJECT 目标上（CMake 硬校验），executable 上设了直接 configure 失败
   （真机教训）；普通可执行文件由 `CUDA_ARCHITECTURES OFF` 移除按架构
   产物管线。
4. **外部样本构建树增量复用**：源码缓存在 `build/` 之外的
   `.cu2cce-cache/`（逃过 clean.sh），`cmake --build --parallel`；构建树
   不再每轮删除，坏树自动回退全量 reconfigure。

## 5. 边界与升级路径

- 上游样本/项目自设 `CUDA_ARCHITECTURES sm_XX` 会被强制回 OFF（NVIDIA
  架构在 Ascend 无意义，且会触发 `.cubin` 产物管线）；
- 每个_linked library 一个 `.cu` 编译单元（后端注册符号聚合缺口）；
- 与 Ascend 侧沟通项与完整边界清单见 `../03_Scope/`。

## 6. 历史文档

本文件取代 `docs/archive/BUILD_COMPATIBILITY_MODEL_v1.md`；override 机制
的设计过程记录在 `docs/archive/` 与 `plans/cuda-large-project-compatibility.md`。
