# 阶段汇报：CMake 大项目能力 × cucc 能力（对比 cu-bridge）

> 2026-08-28 · 汇报用单一入口文档。
> 环境：Ascend 950DT（`Ascend950DT_9582`）· CANN 9.2.0 · bisheng ccec（clang 15.0.5 内核）。
> 里程碑：`main@51f0382`（build-compat 分支合入，full 全量六门禁全绿后合并）。
> 本文所有结论均以 `tools/real_test/server_cmd.txt` 真机回归与 `docs/04_对比与验证/` 各矩阵为准。

---

## 0. 一页速览

- **CMake 线**：现代 CMake CUDA 项目（含多目录、install/export、Makefiles/Ninja）在 NPU 上**零源码改写**构建+运行，官方 NVIDIA CUDA Samples 四样例进回归；legacy FindCUDA 亦有兼容面。
- **cucc 线**：规则数据驱动的 nvcc 兼容驱动，直通 `ccec --enable-simt`；含 FP64 前置策略、`__device__` 执行空间映射、crt 前向兼容层。
- **cuBLAS 线**（实验）：sgemm/gemmex/gemv 三用例真机全绿。
- **工程化**：三画像回归（fast≈40s / external≈28s / full≈3min）、52 个契约测试、全套失败复盘文档。

---

## 1. CMake 能力：中大型 CUDA 项目上 NPU

### 1.1 能力矩阵（全部真机证据）

| 能力 | 状态 | 证据 |
|---|---|---|
| 现代 CMake（≥3.20）CUDA 语言项目 configure/build/run | `SUPPORTED`（核心） | `examples/cmake_demo` 等 6 案例真机通过 |
| 编译器探测链（compiler ID/ABI/features 探针） | `SUPPORTED`（override 吸收） | `cmake_modules/override/CUDAOverrides.cmake` |
| 多目录 `add_subdirectory` + 跨目录目标依赖 + 生成文件 | `SUPPORTED` | `examples/large_project`，`LARGE_PROJECT_REGRESSION_PASS` |
| install/export（`install(EXPORT)` + 使用方 `find_package`） | `SUPPORTED` | large_project 与 build_compat 安装冒烟 |
| `find_package(CUDAToolkit)` + `CUDA::cudart/toolkit/cudart_static/cudadevrt` | `EXPERIMENTAL` | `cmake_modules/FindCUDAToolkit.cmake` |
| Unix Makefiles / Ninja 生成器 | `EXPERIMENTAL` | `make_cce` / `ninja_cce` 包装器 |
| 官方 NVIDIA CUDA Samples 零改写接入 | vectorAdd/matrixMul 必需 PASS；simpleMultiCopy/simpleStreams DIAG-PASS 复验 | external 段 |
| legacy `FindCUDA` 宏 | `EXPERIMENTAL` | `examples/cmake_compat_legacy` |

### 1.2 工作原理：三层漏斗（不改用户源码）

```text
用户 CMake 工程（原始，含 find_package(CUDAToolkit)/CUDA 语言）
        │  bash tools/cmake_cce -S <工程> -B build
        ▼
① cmake_cce 包装器：布置 CUDA 形状的 shim 目录 + 注入
   -DCMAKE_USER_MAKE_RULES_OVERRIDE[_CUDA]=CUDAOverrides.cmake
        ▼
② mock nvcc（shim cuda/bin/nvcc）：应答 CMake 全部探测
   （--version / --simple-preprocess / ABI.cu 编译探针），
   全部真实调用按规则转发给 cucc
        ▼
③ CUDAOverrides.cmake：固定 compiler/device-linker/fatbinary 指向 shim、
   CMAKE_CUDA_RUNTIME_LIBRARY=None、CUDA_ARCHITECTURES=OFF
        ▼
每个 .cu 对象 → cucc（规则驱动）→ ccec -x asc --enable-simt
host C++ 对象 → 系统 g++；链接 → g++ + 桥运行时
```

关键点：**用户 CMakeLists 一行不改**；不生成中间 `.cce` 文件、不做 kernel
改写（有门禁扫描 legacy lowering 标记）。

### 1.3 用法（三段即可上手）

前置（真机实测必需）：每个新 shell 先
`source /usr/local/Ascend/latest/bin/setenv.bash`，否则 cucc/cmake_cce 调
ccec 与 ACL 库会因路径缺失报错（harness 脚本内部自动 source，手动使用必须先执行）。

① 大项目直接构建（唯一入口包装器）：

```bash
bash tools/cmake_cce -S /path/to/your_cuda_project -B build
cmake --build build -j && ./build/your_app
```

② 常用环境开关：

| 开关 | 作用 | 默认 |
|---|---|---|
| `CU2CCE_NPU_ARCH` | 目标 Ascend 架构（如 dav-c310-vec） | 自动探测 |
| `CU2CCE_LIBC_MODE=system` | 大项目用系统 libc 头（glibc 环境） | shadow |
| `CU2CCE_CMAKE_NO_OVERRIDE=1` | 关闭语言模块 override（回退旁路） | 关闭 |
| `CU2CCE_DEVICE_FP64_POLICY` | device 侧 double：`error`(默认拦截)/`warn`/`allow` | error |
| `CU2CCE_CUDA_ARCH` | 需要时注入 `-D__CUDA_ARCH__=NNN` | 未设 |

③ 回归自检（三画像）：

```bash
CU2CCE_REGRESSION_PROFILE=fast     bash tools/real_test/server_cmd.txt   # 日常，≈40s
CU2CCE_REGRESSION_PROFILE=external bash tools/real_test/server_cmd.txt   # 外部样本，≈28s
CU2CCE_REGRESSION_PROFILE=full     bash tools/real_test/server_cmd.txt   # 全量，≈3min
```

### 1.4 边界（诚实声明，均有证据）

1. **每个 linked library 一个 `.cu` 编译单元**：ccec 对每个 TU 重复生成注册
   符号（`BishengLaunchKernelWithHostArgs` 等），多 `.cu` 静态库链接冲突
   （`multi_cu_diagnostic` 真机复现，已固化为 DIAGNOSTIC_ONLY）——属于
   后端聚合能力缺口，已列为与昇腾侧沟通议题。
2. RDC / device-link / separable compilation 不支持（明确拒绝，不伪装）。
3. device 侧 **double 完全不存在**（连 doubleN 向量类型都没有）——桥层
   FP64 策略会在编译前给出"手动降级为 float"的明确指引。
4. CUDA 专属库：cuBLAS 实验面（见 §2.4）；cufft/cusparse/curand/cudnn 无
   对应、不创建虚假 target。

---

## 2. cucc 能力：与 cu-bridge 对比

### 2.1 定位与架构对比

| 维度 | cu-bridge（参照） | cu2cce-bridge（本桥） |
|---|---|---|
| 内核路径 | 主线含 ptx2cpp/gomxcc 降级管线（PTX→C++） | **puresimt 直通**：ccec `--enable-simt` 原生解析 CUDA，零改写、零中间产物（有门禁扫描兜底） |
| 参数适配 | 代码内过滤/替换逻辑 | **规则数据驱动**：`config/cu2cce_rules.json`（forward/drop/replace/inject/reject 五种动作），驱动只解释规则；支持 strict 模式把未验证项升级为错误 |
| 目标后端 | MACA | CANN ccec/ACL（不复制 MACA 映射，保留真实身份） |
| CMake 适配 | cmake_maca/mock nvcc | 同级能力 + **语言模块 override 模块**（真机验证） |
| 运行时 | wcuda→maca | wcuda→ACL（1500+ 行，含默认流语义/事件注册表/FP64 策略） |

### 2.2 本桥独有/领先的部分

1. **真机证据纪律**：52 个契约测试 + 六门禁回归 + 每条结论带真机日志的
   矩阵文档（cu-bridge 无同等级测试/证据面）。
2. **external 官方样本 harness**：NVIDIA/cuda-samples 固定版本零改写接入
   （fetch URL 安全校验 + 三画像非阻塞诊断 + 28 秒增量运行）。
3. **FP64 前置类型策略**：编译前扫描 device 区域 double 使用，给出
   "手动改 float"指引（error/warn/allow 三档）——昇腾侧确认 device 无
   double 后的官方推荐式用户体验。
4. **执行空间关键字映射**：`__device__` → CANN 自身
   `__SIMT_DEVICE_FUNCTIONS_DECL__` 选择器（自动跟随 aicore/simt_callee
   双模式），经 ErrMsg 取证定案、真机验证。
5. **平台事实清单**（都踩出来并写入文档）：向量类型 bisheng 原生注入、
   `__host__/__device__` 是关键字不可 #define、`.cu` host 代码禁裸
   malloc/free（被劫持为 device 函数）、链式包含头必须惰性、GemvEx
   保留三元组 (lda,incx,incy)=(-1,-1,-1)。

### 2.3 差距（诚实对比）

| 维度 | cu-bridge | 本桥 | 差距定性 |
|---|---|---|---|
| 头文件生态 | ~1617 个头（crt 全量、thrust/cutlass/CCCL/nvtx3） | 21 个核心头 + crt 前向层 | **最大差距**，体力型扩展，方向已被验证 |
| 设备 intrinsic（`__expf` 族之外的 device_functions 全集） | 全量 | 常用面（expf/logf/sqrtf/sinf/cosf/tanhf/powf/fmin/fmax/fabsf 等）真机验证 | 中等，按样本证据滚动补 |
| cuBLAS | 完整映射 mBLAS | 三用例绿（Sgemm/Sgemv/GemmEx）+ 模式 API | 明显，属实验线（3.1 节矩阵） |
| 运行时 API 面 | ~2050 行 symbol runtime | ~1700 行，覆盖核心面 + 默认流语义 | 小差距 |
| 工程纪律/证据 | 相对薄弱 | 契约测试 + 门禁 + 证据矩阵 + 失败复盘 | **本桥占优** |

### 2.4 cuBLAS 实验线快照（`docs/02_能力清单/BLAS_COMPATIBILITY_MATRIX.md`）

- 真机三绿：`cublas_sgemm`（FP32→FP16 staging）、`cublas_gemmex`
  （FP16×FP16→FP32 dtype 显式）、`cublas_gemv`（FP32 原生，**SUPPORTED**，
  契约：保留三元组 (lda,incx,incy)=(-1,-1,-1)）。
- 模式 API：PointerMode 被真实落实（HOST scalar staging 到 device）；Math/
  Atomics 为性能提示语义透传。
- Dgemm / BLAS1 六函数：诚实 UNSUPPORTED（参数校验先行，错误归因一致）。
- 开关：`CU2CCE_BLAS_TEST=1`（独立于必需门禁）。

---

## 3. 汇报现场演示脚本（三条命令）

```bash
# ① 日常回归（约 40s）：必需门禁全貌
CU2CCE_REGRESSION_PROFILE=fast bash tools/real_test/server_cmd.txt

# ② 外部官方样本零改写（约 30s，缓存热）
CU2CCE_REGRESSION_PROFILE=external bash tools/real_test/server_cmd.txt

# ③ cuBLAS 实验线（独立开关）
CU2CCE_BLAS_TEST=1 bash tools/real_test/blas_regression.sh
```

---

## 4. 悬置议题（定性：非桥侧问题，材料已备）

- **asyncAPI 元素丢失**：官方样本 26 轮全数组自增后**恰好 1 个元素少 1**
  （索引逐轮漂移）。桥侧证据链闭环：全部调用返回 0；三种等待节奏的最小
  复现（含逐帧复刻样本的 query 轮询）均无法触发；D2H 前有 device 级同步。
  定性：ccec SIMT 执行层/ACL 可见性缝隙。素材：
  `tools/real_test/diag/`（含可复现诊断资产与触发方向清单；原工作区历史材料不随仓库交付）。给
  昇腾侧的问题清单）。
- **多 `.cu` 静态库聚合**：后端 translation-unit 聚合能力缺口（同上档案
  与构建矩阵），建议作为与昇腾团队的正式沟通项。

---

## 5. 维护规则

本汇报的数据快照对应 `main@51f0382`。能力状态变化时同步：
`docs/04_对比与验证/` 对应矩阵 → 本文档数字 → `CURRENT_STATE.md`。
