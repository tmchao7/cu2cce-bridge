# 与 cu-bridge 的 CMake/Make 机制对齐审查（2026-08-28；第五批次 2026-08-30 增补）

> 背景：设计要求"实现方案与沐曦 cu-bridge 保持近似（后端不同：GPU vs NPU），
> 并能逐文件讲清自己的代码"。本文基于对 cu-bridge-muxi v3.8.0（git `6ab1076`）
> 的逐文件深读与其用户手册（docs/02_User_Manual），逐机制对照本桥实现，
> 给出：已对齐 / 有意分歧（+理由）/ 真实差距（+修复）/ 代码导读。
> 机制细节的完整事实库见 cu-bridge 仓库；本文只引用与本桥相关的部分。
> 第五批次（2026-08-30）的设计原理剖析与逐项借鉴决策见
> `CUBRIDGE_CMAKE_MAKE_DESIGN_ANALYSIS.md`（下称"分析文档"）。

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
| 双阶段：configure=mock、build=真 cucc | cmake_mock + make_maca 重建 CUDA_DIR | **cmake_cce --build 分流**：刷新 shim `nvcc→cucc` 后分发 `make_cce/ninja_cce` | 已对齐（2026-09-01，server build 强制验证） |
| cmake --build 分流 | deal_build_param → make_maca/ninja_maca（按 build.ninja） | **第五批次补齐**：cmake_cce --build 解析 --target/--parallel/-- 后分发 make_cce/ninja_cce | 已对齐（2026-08-30） |
| 探测应答 | mock_nvcc.py 四分支（子串嗅探） | mock_nvcc.py 五分支（多 `--simple-preprocess`，支持 CMake≥3.24） | 已对齐+扩展 |
| ABI 探针 | 宿主 g++ 现编 CMakeCUDACompilerABI.cpp（保留 INFO 串） | **第五批次补齐同款**：shipped 替身源码现编（可执行形态真链接、运行报无设备），占位仅作回退 | 已对齐（2026-08-30，无预编译工件） |
| CompilerId 探针 | 伪 trace + 复制预编译 a.out ELF | 占位对象 + `--cmake-identity` 宏（无预编译 ELF） | 有意分歧：我们不携带平台特定二进制工件，真机验证走真实编译路径 |
| 编译器自检 | 0 字节 CMakeTestCUDACompiler 覆盖 CMAKE_ROOT 跳过 | **不 patch CMake 模块**：mock 对 configure 期编译调用保留真编译回退（自检是真证据） | 有意分歧（更严格） |
| 规则固化 | 无语言钩子（patched 模块未接线）；USE_CMAKE_MACA 二进制替换 hack | `CMAKE_USER_MAKE_RULES_OVERRIDE`（CMake 官方扩展点）+ `CU2CCE_CMAKE_NO_OVERRIDE` 回退（≈其 cmake_assist） | 有意分歧：官方钩子可维护性远优于二进制替换 |
| 假工具包 lib 软链族 | link.json/stub_link.json（50+ 条） | **lib64 软链族已补 + manifest 化**：libcudart/libcudart_static/libcudadevrt/libcuda.so(.1) → 桥运行时；第五批次扩展 libcublas.so(.1)/libnvToolsExt.so(.1)（EXPERIMENTAL），布局统一由 tools/link.json + tools/mock/virtualDir.py 驱动 | 已对齐（本桥不映射 cufft/cusparse/cudnn 等名——诚实边界） |
| 假工具包清单形式 | link.json（pre_make）+ stub_link.json（virtual_cmake） | **tools/link.json 单一清单 + tools/mock/virtualDir.py 单一构建器**（cmake_cce/make_cce/pre_cce 三入口共用，第五批次消灭三处硬编码） | 已对齐（2026-08-30） |
| 查询直通（skipstring） | conf.json skipstring：探测旗标整条命令绕过规则引擎直通 mxcc | **第五批次补齐**：rules `queries` 段（-dumpversion/-dumpmachine/-print-*），命中即 phase=query 原样直通 ccec | 已对齐（2026-08-30） |
| `-std` 去重 | 用户 flag 透传，无默认冲突 | **第五批次修复**：用户 `-std=` 存在时抑制注入 `-std=gnu++17`（`no_user_std` 条件） | 已对齐（2026-08-30） |
| `__CUDA_ARCH__` 供给 | -gencode/-arch → `-DNV_ARCH_*` + `-Xdevice -D__CUDA_ARCH__`（device-pass 限定） | **机制级借鉴（opt-in）**：drop 前提取架构值；`CU2CCE_ARCH_MACRO=auto` 注入 `-D__CUDA_ARCH__=<值>`（nvcc 编码 major*100+minor*10），显式 `CU2CCE_CUDA_ARCH` 优先；默认 off（ccec 单遍无 -Xdevice 等价物，全局定义会改变 host `#ifdef __CUDA_ARCH__` 分支——风险记录于 PARITY_AUDIT） | 有意分歧（opt-in + 风险留档，2026-08-30） |
| `.cu` 自动预包含 cuda_runtime.h | condition：.cu 追加 `--include cuda_runtime.h` | **第五批次补齐**：`source_is_cuda_and_runtime_header` 条件注入 `-include cuda_runtime.h`（先于 simt_adaptor.h；桥内 `__cuda_runtime.h` 已自含 adaptor 且有 guard） | 已对齐（2026-08-30） |
| 宿主编译器劫持 | bin/gnu 包装器（链接走 mxcc） | hostcc_cce（第五批次规则化：`cu2cce_driver.py --hostcc-mode` 消费 rules `hostcc.link.inject`）；链接器保持 g++（语义由桥运行时承接） | 已对齐（规则面 2026-08-30；链接器保留 NPU 适配） |
| Make 入口 | make_maca（假目录+PATH+make；make.log 哨兵） | make_cce（同构 + manifest 化布局；clean 目标词元匹配修复 `cleanpack` 误删） | 已对齐 |
| 并行隔离 | USE_WCUDA_FOR_MULTI_JOB（$PPID 后缀） | **第五批次补齐**：`CU2CCE_SHIM_ISOLATE=1`（cmake_cce/make_cce/pre_cce 统一） | 已对齐（2026-08-30） |
| 假工具包默认持久化 | CUBRIDGE_HOME=$HOME（持久，configure/build 共享） | **默认持久且按 build 目录隔离**：`${CU2CCE_HOME:-${CUBRIDGE_HOME:-$HOME}}/cu2cce-bridge/cmake-virtual-toolkit-<build-key>`（CU2CCE_CMAKE_SHIM_DIR 显式覆盖） | 已对齐（2026-09-02：双阶段共享同一 shim，同时避免跨项目覆盖） |
| configure 包裹器 | pre_makefile（configure 脚本包裹，FAQ Q5） | **pre_cce**（同构 + hostcc_cce 统一——第五批次前是身份软链，纯 g++ 链接拿不到注入） | 已对齐（2026-08-28；2026-08-30 统一 hostcc） |
| 设备架构探测助手 | multi_target.sh + get_device_arch.sh（CUCC_TARGETS → mx-smi → 提示） | **tools/device_arch.sh**（CU2CCE_NPU_ARCH/CUCC_NPU_ARCH/CUCC_TARGETS/CUCC_TARGETS_FROM_DEVICE → npu-smi 表映射 → cache → default + tip），cmake_cce/make_cce/ninja_cce/pre_cce 接线 | 已对齐（2026-09-01，NPU 单 dav-* target 约束） |
| CUPTI 路径布局 | 虚拟目录含 extras/CUPTI/{include,lib64} | tools/link.json 同款空目录布局（路径探测兼容） | 已对齐（2026-08-28） |
| nvcc 兼容软链 | FAQ Q2 手工软链 workaround | **安装树提供普通文件 `bin/nvcc` wrapper → `cucc`**（无安装阶段软链） | 已对齐+自动化 |
| CUDA Toolkit 发现 | 无 FindCUDAToolkit（文档教用户绕开 CUDA 语言） | FindCUDAToolkit + CUDA::cudart/toolkit 别名族（真机验证） | **本桥领先** |
| 参数适配 | conf.json（replacer/adder/filter/skipstring）+ gomxccbin 正则 | bin/conf.json（filter/replacer/adder/skipstring + Ascend cu2cce 扩展）单一 in-process 驱动 | 机制不同目标相同；无 Go 工具依赖 |
| 库名重写 | 40+ 条 -lcudart→-lmcruntime 等 | -lcudart 丢弃 + 桥运行时注入；虚拟工具链 `lib64` 另备 libcublas/libnvToolsExt 名（EXPERIMENTAL，走桥内实验面） | 语义等价（后端库面不同） |
| 设备架构 | CUCC_TARGETS / mx-smi | CU2CCE_NPU_ARCH / CUCC_TARGETS(dav-*) / npu-smi 自动探测 | 已对齐（Ascend 单 target 约束） |
| CUDA 版本仿真 | CUCC_CUDA_VERSION → -DCUDA_VERSION | 同名开关 CUCC_CUDA_VERSION | 已对齐（同名） |
| 调试 | WCUDA_DEBUG / CUBRIDGE_LOG_LEVEL | WCUDA_DEBUG / CU2CCE_DEBUG / CU2CCE_TRACE_FILE | 已对齐+扩展 |

## 3. 产物对照

| | cu-bridge | 本桥 |
|---|---|---|
| 用户工程内 | 标准 `.o`/目标产物，无中间件 | 同 |
| 桥临时物 | `$CUBRIDGE_HOME/_nvcc/`（命令脚本）、`~/cu-bridge`（假工具包） | 虚拟工具链目录（默认 `~/cu2cce-bridge/cmake-virtual-toolkit-<build-key>`，持久按 build 隔离；或 `CU2CCE_CMAKE_SHIM_DIR`）、`$CU2CCE_HOME/cu2cce-bridge/`（make 假工具包） |
| 链接执行者 | mxcc | g++（+ 桥运行时 .so） |
| 安装树 | bin/include/extras/cmake_module/tools + 库装进 `$MACA_PATH/lib` | bin/include/lib/config/cmake_modules/tools（自包含前缀）+ `bin/nvcc` 软链 |

## 4. 代码导读路线

原理白话版与 configure 时序图见
`docs/01_架构设计/06_cmake_principle_flow.md`；本节给出按数据流顺序的
代码阅读地图（每步给出文件与关键函数）：

1. **configure 入口**：`tools/cmake_cce` —— 环境预检 → build 分流
   （`--build/--install` 时 虚拟工具链 nvcc→cucc；`--build` 再按 build.ninja 分发
   make_cce/ninja_cce，deal_build_param parity）→ 虚拟工具链布局
   （`tools/mock/virtualDir.py` 消费 `tools/link.json`：bin/include/
   lib64 软链族/targets 三元组/version.txt）→ cmake 参数注入
   （ARCHITECTURES=OFF / RUNTIME_LIBRARY=None / MODULE_PATH 前插 /
   CMAKE_CUDA_COMPILER=shim / USER_MAKE_RULES_OVERRIDE）→ `exec cmake`。
2. **探测应答**：`tools/mock/nvcc_entry.sh`（4 行：exec python3 mock）→
   `tools/mock/mock_nvcc.py main()` 五分支（determine_cuda 伪 `#$` /
   --version / --simple-preprocess / ABI 占位对象 / 其余→`run_driver`）。
3. **真编译翻译**：`tools/cu2cce_driver.py` —— `RuleDriver.prepare()`：
   `queries` 查询直通（phase=query 原样过 ccec）→ `-gencode` 架构提取
   （`extract_cuda_arch`，喂 trace 与 CU2CCE_ARCH_MACRO）→ `_phase()`
   （七阶段）→ **FP64 前置扫描**（`scan_device_double_usage`）→
   逐参数规则循环（reject→replace→drop→forward→未识别警告）→ `_inject()`
   注入链（含 `.cu` 预包含 cuda_runtime.h / `-std` 去重）→ 前缀排序
   （`-x asc` 与 `-include` 置前）→ `run_prepared()`（Popen ccec）。
4. **规则数据**：`bin/conf.json` —— 五动作 + `queries` + `hostcc`
   段；校验器 `tools/validate_cucc_rules.py`。
5. **Make 线**：`tools/make_cce` —— tools/mock/virtualDir.py 构建虚拟 CUDA 工具链目录 →
   PATH/CUDA_PATH → exec make（clean 词元匹配删假目录）；对照
   `tools/ninja_cce`；`tools/pre_cce` 包裹 configure；`tools/hostcc_cce`
   委托 driver `--hostcc-mode`（rules hostcc 段）。
6. **运行时承接**：`src/bridge/runtime/src/cuda_runtime_wrapper.cpp`
   （默认流解析/事件注册表/FP64 策略）——链接进 `libcce_bridge_runtime.so`。
7. **行为锁**：`tests/test_cucc_rules.py`（驱动 38 项）+
   `tests/test_cmake_compat.py`（表面 30 项）+ `tests/test_blas_surface.py`。

## 5. 剩余差距（后续项）

> 第五批次（2026-08-30）收口：cmake --build 分流、queries 查询直通、`-std` 去重、
> `-gencode` 架构提取 + `CU2CCE_ARCH_MACRO` opt-in 注入、`.cu` 预包含
> cuda_runtime.h、hostcc 规则化、虚拟工具链清单化（tools/link.json +
> tools/mock/virtualDir.py）、lib64 别名扩展（libcublas/libnvToolsExt，EXPERIMENTAL）、
> pre_cce hostcc 统一、clean 词元匹配、CU2CCE_SHIM_ISOLATE 并行隔离、
> mock ABI 现编替身——决策细节见分析文档 §二。
>
> 第六批次（2026-08-30，同日第二批）收口 cucc 运行时机制：宿主旗标逐 flag
> 过滤（修 `-Xcompiler=...` 等号形式绕过与 mixed 泄漏两个 bug）、`-Werror`
> 族清洗（conf_regex.yaml 清洗规则对齐）与 `-Werror=implicit-fallthrough`
> 规范化重写（bin/cucc:250-280 对齐）、`regex_cleanup` 数据驱动清洗层
> （gomxccbin 等价，种子规则：-fno-canonical-system-headers /
> -Wno-nonnull-compare）、`add_gcc_toolchain` 休眠对齐（cucc:1055-1066，
> `CU2CCE_GCC_TOOLCHAIN=off` 可关）、bin/cucc 的 BASH_XTRACE→CU2CCE_DEBUG
> 传递；并按项目作者确认在分析文档落档前提纠正——**puresimt 前端
> （ccec -x asc --enable-simt）与 mxcc 同为原生认识 CUDA 语法的前端**，
> 差异只是小改动。机制面至此全部收口（分析文档 §4.0）。
>
> 第七批次（2026-08-30，同日第三批）收口参数过滤尾部 + code simplifier：
> `-lnvrtc/-l:libnvrtc*/-lOpenCL` 带警告 drop（原静默 forward）、`--share→
> -shared`、`-Xlinker` 成对透传、var-tracking 族清洗种子、`__CUDACC__/
> __CUDACC_VER_*` 宏族注入（CUCC_CUDA_VERSION 推导，仅 .cu；mock identity
> 退役为 no-op）、`-lcublasLt` 两段式库匹配、cmake_cce build 目录检查、
> `tools/virtual_toolkit_env.sh` 共享库（三入口 ~40 行重复收敛）。
>
> 第八批次（2026-08-30，同日第四批）conf.json filter/replacer 终检收口：
> `--use_fast_math` 四拼写→`-ffast-math`、`-lgomp/-l:libgomp*`→`-lomp`、
> `-static` 诚实 drop、杂项 filter（`g++*`/`/usr/bin/cc`/`--threads=*`/
> `-keep-dir` 族/`--W*` 双横线族/`-diag-suppress` 族）、banner 携带
> `compatible with release` 可解析词组；server_cmd.txt full 画像新增
> 机制探针段（queries 直通 / dryrun 三联 / `cmake_cce --build` 分流），
> 失败即打落对应 badge——机制与参数面对齐至此全部可被真机回归锁定。

1. Bazel/TF 构建配置（conf_bazel.json / WCUDA_BAZEL_TF_BUILD 等价物）；
2. FindCUDA legacy 宏全集的 ptx/cubin/fatbin 族（明确不支持，诚实 FATAL_ERROR）；
3. gfortran/MPI 包装器（低频；cu-bridge 有 conf_gfortran.json 与 ompi 分派）；
4. CompilerId 预编译 ELF 技巧（刻意不用：不携带平台二进制工件——维持有意分歧）；
5. 头文件生态/cuDriver API/多 `.cu` 归档聚合（SDK 级与昇腾侧议题，见分析文档 §四）。

> 已关闭的历史差距：pre_makefile（→pre_cce）、bin/gnu（→hostcc_cce，第五批次
> 进一步规则化）、multi_target/get_device_arch（→device_arch.sh）、持久虚拟工具链目录、
> lib64 软链族、安装树 nvcc 软链、cmake --build 分流、queries 直通、mock ABI
> 现编——2026-08-28/08-30 两批次落地。

## 6. 维护规则

cu-bridge 机制变化（升级 mock 分支/link.json 结构/新增入口）时：重读本表
→ 差距更新 → 需要对齐的项走"探针取证 → 落层 → 契约测试"标准流程。

## 2026-08-31 parity follow-up

本轮继续按 `cu-bridge` 机制逐项收口，同时保留 Ascend NPU 后端边界：

- `ninja_cce` 现在与 `make_cce`/`pre_cce` 共用 virtual CUDA toolkit 生命周期，统一处理持久虚拟工具链目录、`CUDA_PATH`、`CANN_BASE`、`ASCEND_TOOLKIT_HOME`、`CU2CCE_NPU_ARCH`、`LD_LIBRARY_PATH`、clean 和并行隔离。
- 修正 CMake 虚拟工具链目录 路径一致性：`cmake_cce`、`ninja_cce` 和 `tools/mock/virtualDir.py` 统一使用 `<shim-root>/CUDA_DIR`，避免 CMake 记录的 mock `nvcc` 路径与 build 阶段重指向路径不一致。
- `pre_cce` 和所有构建 wrapper 保持 argv 直接传递，不复制参考脚本的 `eval` 字符串执行风险。
- `-E` 采用 `cu-bridge` 的 query passthrough 机制，但后端仍由 Ascend ccec 负责，实际预处理输出继续以真机证据为准。
- `CUCC_TARGETS` 作为兼容别名接受，但 Ascend ccec puresimt 一次只允许一个 `dav-*` 目标；`sm_*`/`compute_*` 不会被误传为 NPU 架构。
- legacy `FindCUDA` 增加常见 uppercase helper 名称；CUDA math-library adders 在没有已验证 Ascend 等价物时显式失败，不创建伪库链接。
- CUDA 库 alias 继续只覆盖已验证 bridge/CANN 路径；`cufft`、`nvjpeg`、`cupti`、NPP、cuFile、NVML 等进入 capability warning/map-to policy，不直接照搬 MACA 库名。

## 产物对齐（2026-09-01）

本轮把 parity 从“命令能执行”提升为“命令生成的产物可验证”。产物契约位于
`config/artifact_contract.json`，检查器位于 `tools/real_test/verify_artifacts.py`，由
`tools/real_test/server_cmd.txt` 在真实 CANN 服务器上调用。

| 产物层 | cu-bridge | cu2cce-bridge / Ascend NPU | 判定 |
|---|---|---|---|
| CUDA device compile | `.o`，由 mxcc 生成 | `.o`，由 `ccec -x asc --enable-simt` 生成 | 结构等价 |
| dependency output | `.d` / `-MD/-MF/-MT` | `.d` / `-MD/-MF/-MT` | 机制等价，真实内容由服务器验证 |
| static library | `.a` | `.a`，由 CMake 或 `ar` 归档 | 结构等价 |
| shared library | `.so` | `.so`，带 bridge/CANN 运行时依赖 | 结构等价，依赖后端不同 |
| executable | ELF executable | ELF executable，链接 `libcce_bridge_runtime.so` 和 CANN runtime | NPU-aware 等价 |
| build metadata | `CMakeFiles`、`build.make`、`build.ninja`、`link.txt` | 同类 CMake/Make/Ninja 元数据，CUDA compiler 指向 shim/cuсc | 结构等价 |
| runtime search path | MACA lib paths | bridge/CANN lib paths，必要时包含 `libruntime.so` 所在目录 | 后端适配 |
| forbidden intermediates | 用户工程不依赖 `.maca` 中间源 | 禁止 `.cce/.ptx/.cubin/.fatbin/.maca` | 明确禁止 |
| bridge SDK internals | `runtime_cu.so`、`symbol_cu.so` | `libcce_bridge_runtime.so` + `libascendcl.so`/`libruntime.so` | 不做字节/符号复制 |

当前 blocking artifact cases 覆盖：

- 直接 `cucc` 的 `.o/.d/executable/.so`；
- CMake demo、multi-CU、legacy FindCUDA、large project、build-compat、Ninja；
- Make static/shared/executable 和 plain g++ link carrier；
- CMake install/export 文件；
- 禁止中间物扫描、ELF/archive 类型、`NEEDED` 和 `RPATH/RUNPATH`。

成功路径只在终端打印一行 artifact summary，详细结果写入
`build/real_test/puresimt/artifacts/`；失败路径输出有限 tail，不隐藏产物检查原因。

## 2026-09-02 发布前收口

本轮在不复制 MACA 后端能力的前提下补齐了几项机制边界：

- `cmake_cce` 接受显式 `CCEC_PATH`，并兼容版本化 CANN 安装目录；`--config` 在
  Ninja Multi-Config 存在 `build-<config>.ninja` 时传为 `-f`，`--clean-first` 先执行
  generator clean。
- Make/Ninja/pre/CMake 的 `LD_LIBRARY_PATH` 拼接不再产生尾部空路径；Make/pre 同样
  导出 `CUDA_HOME` 和 `runtime/lib64`，保证各入口环境一致。
- 直接 `cucc` 与 wrapper 复用 `device_arch.sh`；`.cu.cc`、`-code=sm_*` 和绝对 CUDA
  toolkit/stub library 路径按 CUDA 输入/过滤策略处理，不能泄漏给 Ascend ccec。
- artifact verifier 的 `kind: file` 要求普通文件或有效 symlink；legacy FindCUDA
  在 `REQUIRED` 发现阶段校验最小 bridge，并提供常用版本/runtime 兼容变量。
- 顶层 README/Quickstart/文档导航只保留单一操作入口；已完成计划和 review spec
  统一归档到 `docs/99_归档/`，构建/扫描状态由 `.gitignore` 排除。

仍有意保留的限制：默认 persistent virtual toolkit 是历史兼容行为；并发或多 bridge 版本构建
必须通过 `CU2CCE_CMAKE_SHIM_DIR` 或 `CU2CCE_SHIM_ISOLATE=1` 隔离。Ascend 多 `.cu`
静态库重复 registration、PTX/CUBIN/FATBIN、RDC/device-link 和未验证 CUDA library
不会因 parity 目标被伪装成已支持能力。
