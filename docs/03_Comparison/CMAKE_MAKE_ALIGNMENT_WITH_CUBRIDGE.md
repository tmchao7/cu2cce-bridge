# 与 cu-bridge 的 CMake/Make 机制对齐审查（2026-08-28）

> 背景：导师要求"实现方案与沐曦 cu-bridge 保持近似（后端不同：GPU vs NPU），
> 并能逐文件讲清自己的代码"。本文基于对 cu-bridge v3.8.0（git `6ab1076`）
> 的逐文件深读与其用户手册（docs/02_User_Manual），逐机制对照本桥实现，
> 给出：已对齐 / 有意分歧（+理由）/ 真实差距（+修复）/ 代码导读。
> 机制细节的完整事实库见 cu-bridge 仓库；本文只引用与本桥相关的部分。

## 1. cu-bridge 的 CMake/Make 实现机制（事实摘要）

### 1.1 入口分层

```text
cmake_maca（用户唯一入口）
 ├─ 查询透传：--version/--help/-E → 直接 exec 真 cmake
 ├─ 目标检查：CUCC_TARGETS 或 mx-smi 设备探测（--offload-arch 数据源）
 └─ CUCC_CMAKE_ENTRY 分流：
     ├─ =2 → cmake_mock：虚拟 CUDA_DIR（stub_link.json 生成）+ mock nvcc
     └─ 其他 → cmake_assist：CUDA_DIR 直接软链"发现的工具包"
```

### 1.2 双阶段协议（关键设计）

- **configure 阶段**：shim `CUDA_DIR/bin/nvcc → mock/nvcc_entry.sh →
  mock_nvcc.py`，**只答探测、绝不真编译**（其余调用记日志无输出）。
  探测四分支：`-v __cmake_determine_cuda`（伪 `#$` trace）、`--version`
  （硬编码 11.6 banner）、`CMakeCUDACompilerId.cu`（伪 trace + **复制一个
  预编译真 ELF a.out**，内嵌 `INFO:compiler[NVIDIA]` 等字符串）、
  `Modules/CMakeCUDACompilerABI.cu`（用系统 g++ 编译删减版 ABI 探针）。
  配套手段：其 cmake_module/maca 里有 **0 字节 CMakeTestCUDACompiler.cmake**
  （手工覆盖 CMAKE_ROOT 后跳过编译器自检）。
- **build 阶段**：`cmake --build` 被 deal_build_param 分流到
  `make_maca/ninja_maca`，它们按 `link.json` 的 pre_make 段重建 CUDA_DIR，
  此时 `bin/nvcc → ${CUCC_PATH}/bin/cucc`——**真编译绕过 mock**。
- 另有 `USE_CMAKE_MACA=1` 模式：物理替换系统 cmake 二进制为
  cmake_maca 软链（备份 .bak），Makefile 内再调用 cmake 时回到 assist，
  结束后还原。

### 1.3 假工具包清单（link.json / stub_link.json）

约 50 条 CUDA 名 → MACA 名的软链：`lib64/libcudart.so →
$MACA_PATH/lib/libmcruntime.so`、`libcublas.so → libmcblas.so`、
`libcuda.so.1 → libsymbol_cu.so`、`libnccl.so → libmccl.so`…；
宿主编译器劫持：`bin/{c++,g++,gcc,cc,icpc,mpicxx} → bin/gnu`（gnu 包装器：
纯编译透传真 gcc，**链接走 mxcc**）；`include → ${CUCC_PATH}/include`；
`targets/<arch>-<os>/{include,lib}` 布局。

### 1.4 产物与链接行

用户工程内只有标准 `.o` 与目标产物（无 `.ptx/.cpp/.maca` 中间件）；
桥的临时物在 `$CUBRIDGE_HOME/_nvcc/`（生成的命令脚本，用后即删）与
`$CUBRIDGE_HOME/cu-bridge/`（假工具包，可删）。**最终链接由 mxcc 执行**
（非 g++），link.adder 强制 `-fgpu-rdc --maca-link -lToolsExt_cu
-lruntime_cu -lmcToolsExt`，且 40+ 条 `-l*cuda*/*cublas* → -lmc*` 重写。

### 1.5 文档承诺（对用户的契约）

新入口命令（make_maca/cmake_maca/cucc）；不提供 nvcc（FAQ 给
`ln -s cucc nvcc` 软链 workaround）；**不需要安装 CUDA Toolkit**；
configure 脚本用 `pre_makefile` 包裹；Python 库安装不管（FAQ Q7）。

## 2. 逐项对齐表

| 机制 | cu-bridge | 本桥 | 状态 |
|---|---|---|---|
| 用户入口 | cmake_maca（查询透传/目标检查/分流） | cmake_cce（查询透传/注入/shim） | 已对齐 |
| 双阶段：configure=mock、build=真 cucc | cmake_mock + make_maca 重建 CUDA_DIR | **cmake_cce --build 分流**：刷新 shim `nvcc→cucc` 后 exec cmake --build | 已对齐（2026-08-28 起） |
| 探测应答 | mock_nvcc.py 四分支（子串嗅探） | mock_nvcc.py 五分支（多 `--simple-preprocess`，支持 CMake≥3.24） | 已对齐+扩展 |
| CompilerId 探针 | 伪 trace + 复制预编译 a.out ELF | 占位对象 + `--cmake-identity` 宏（无预编译 ELF） | 有意分歧：我们不携带平台特定二进制工件，真机验证走真实编译路径 |
| 编译器自检 | 0 字节 CMakeTestCUDACompiler 覆盖 CMAKE_ROOT 跳过 | **不 patch CMake 模块**：mock 对 configure 期编译调用保留真编译回退（自检是真证据） | 有意分歧（更严格） |
| 规则固化 | 无语言钩子（patched 模块未接线）；USE_CMAKE_MACA 二进制替换 hack | `CMAKE_USER_MAKE_RULES_OVERRIDE`（CMake 官方扩展点）+ `CU2CCE_CMAKE_NO_OVERRIDE` 回退（≈其 cmake_assist） | 有意分歧：官方钩子可维护性远优于二进制替换 |
| 假工具包 lib 软链族 | link.json/stub_link.json（50+ 条） | **lib64 软链族已补**：libcudart.so/libcudart_static.a/libcudadevrt.a/libcuda.so(.1) → 桥运行时（cmake shim 与 make_cce 双侧） | 已对齐（本桥不映射 cublas 名——实验线诚实边界） |
| 宿主编译器劫持 | bin/gnu 包装器（链接走 mxcc） | 不劫持：宿主编译/链接用系统 g++（桥运行时是 .so，配合 lib64 软链即可） | 有意分歧：链接器保持标准 g++，语义由桥运行时承接 |
| Make 入口 | make_maca（假目录+PATH+make；make.log 哨兵） | make_cce（同构：make.log 哨兵**已创建**/容错清理/CUDA_DIR 布局/targets 三元组派生/宿主编译器 passthrough） | 已对齐 |
| 假工具包默认持久化 | CUBRIDGE_HOME=$HOME（持久，configure/build 共享） | **默认持久**：`${CU2CCE_HOME:-${CUBRIDGE_HOME:-$HOME}}/cu2cce-bridge/cmake-shim`（CU2CCE_CMAKE_SHIM_DIR 显式覆盖） | 已对齐（2026-08-28 静态补齐：双阶段的 build 切换依赖持久 shim） |
| 宿主编译器 passthrough 软链 | pre_cmake 段：bin/{c++,g++,gcc,cc} → /usr/bin/* | cmake shim 与 make_cce 均提供同款 passthrough（仅当 /usr/bin 存在） | 已对齐（2026-08-28 静态补齐） |
| configure 包裹器 | pre_makefile | 未实现（后续项） | 差距（低频场景） |
| nvcc 兼容软链 | FAQ Q2 手工软链 workaround | **安装树自动提供 `bin/nvcc → cucc`**（install CODE） | 已对齐+自动化 |
| CUDA Toolkit 发现 | 无 FindCUDAToolkit（文档教用户绕开 CUDA 语言） | FindCUDAToolkit + CUDA::cudart/toolkit 别名族（真机验证） | **本桥领先** |
| 参数适配 | conf.json（replacer/adder/filter）+ gomxccbin 正则 | cu2cce_rules.json（153 条，五动作）单一 in-process 驱动 | 机制不同目标相同；无 Go 工具依赖 |
| 库名重写 | 40+ 条 -lcudart→-lmcruntime 等 | -lcudart 丢弃 + 桥运行时注入（cublas 实验线） | 语义等价（后端库面不同） |
| 设备架构 | CUCC_TARGETS / mx-smi | CU2CCE_NPU_ARCH / probe 自动探测 | 已对齐 |
| CUDA 版本仿真 | CUCC_CUDA_VERSION → -DCUDA_VERSION | 同名开关 CUCC_CUDA_VERSION | 已对齐（同名） |
| 调试 | WCUDA_DEBUG / CUBRIDGE_LOG_LEVEL | WCUDA_DEBUG / CU2CCE_DEBUG / CU2CCE_TRACE_FILE | 已对齐+扩展 |

## 3. 产物对照

| | cu-bridge | 本桥 |
|---|---|---|
| 用户工程内 | 标准 `.o`/目标产物，无中间件 | 同 |
| 桥临时物 | `$CUBRIDGE_HOME/_nvcc/`（命令脚本）、`~/cu-bridge`（假工具包） | shim 目录（默认 `~/cu2cce-bridge/cmake-shim`，持久；或 `CU2CCE_CMAKE_SHIM_DIR`）、`$CU2CCE_HOME/cu2cce-bridge/`（make 假工具包） |
| 链接执行者 | mxcc | g++（+ 桥运行时 .so） |
| 安装树 | bin/include/extras/cmake_module/tools + 库装进 `$MACA_PATH/lib` | bin/include/lib/config/cmake_modules/tools（自包含前缀）+ `bin/nvcc` 软链 |

## 4. 代码导读（向导师讲解自己代码的路线）

原理白话版与 configure 时序图见
`docs/02_Architecture/06_cmake_principle_flow.md`；本节给出按数据流顺序的
代码阅读地图（每步给出文件与关键函数）：

1. **configure 入口**：`tools/cmake_cce` —— 环境预检（L20-28）→ build 分流
   （新增：`--build/--install` 时 shim nvcc→cucc）→ shim 布局（L57+：bin/
   include/lib64 软链族/targets 三元组/version.txt）→ cmake 参数注入
   （L100+：ARCHITECTURES=OFF / RUNTIME_LIBRARY=None / MODULE_PATH 前插 /
   CMAKE_CUDA_COMPILER=shim / USER_MAKE_RULES_OVERRIDE）→ `exec cmake`。
2. **探测应答**：`tools/nvcc_entry.sh`（4 行：exec python3 mock）→
   `tools/mock/mock_nvcc.py main()` 五分支（determine_cuda 伪 `#$` /
   --version / --simple-preprocess / ABI 占位对象 / 其余→`run_driver`）。
3. **真编译翻译**：`tools/cu2cce_driver.py` —— `RuleDriver.prepare()`：
   `_scan_inputs` → `_phase()`（七阶段）→ **FP64 前置扫描**
   （`scan_device_double_usage`，error/warn/allow）→ 逐参数规则循环
   （reject→replace→drop→forward→未识别警告）→ `_inject()` 注入链 →
   前缀排序（`-x asc` 与 `-include` 置前）→ `run_prepared()`（Popen ccec）。
4. **规则数据**：`config/cu2cce_rules.json` —— 五动作 153 条；校验器
   `tools/validate_cucc_rules.py`。
5. **Make 线**：`tools/make_cce` —— 假工具包布局（nvcc→cucc、lib64 软链
   族、targets 三元组）→ PATH/CUDA_PATH → exec make；对照 `tools/ninja_cce`。
6. **运行时承接**：`src/bridge/runtime/src/cuda_runtime_wrapper.cpp`
   （默认流解析/事件注册表/FP64 策略）——链接进 `libcce_bridge_runtime.so`。
7. **行为锁**：`tests/test_cucc_rules.py`（驱动 30 项）+
   `tests/test_cmake_compat.py`（表面 18 项）+ `tests/test_blas_surface.py`。

## 5. 剩余差距（后续项，均低频）

1. `pre_makefile` 等价的 configure 脚本包裹器（cu-bridge 用于 configure 类
   自动脚本的场景）；
2. `bin/gnu` 宿主编译器包装器（我们评估为非必需：桥运行时 .so + lib64 软链
   已覆盖纯 g++ 链接场景）；
3. Bazel/TF 构建配置（conf_bazel.json 等价物）；
4. FindCUDA legacy 宏面（我们有基础 FindCUDA.cmake，宏集合少于其版本）；
5. mock 的 CompilerId 预编译 ELF 技巧（我们刻意不用：不携带平台二进制）。

## 6. 维护规则

cu-bridge 机制变化（升级 mock 分支/link.json 结构/新增入口）时：重读本表
→ 差距更新 → 需要对齐的项走"探针取证 → 落层 → 契约测试"标准流程。
