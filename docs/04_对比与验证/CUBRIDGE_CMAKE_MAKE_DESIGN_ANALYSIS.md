# cu-bridge CMake/Make 设计原理剖析与借鉴决策（第五对齐批次，2026-08-30）

> 参考仓库：`/Users/tmchao7/repos/09_expLearn/cu-bridge-muxi`（沐曦 MetaX，SDK 3.8.0，只读）。
> 本文是第五对齐批次的决策记录：先完整剖析 cu-bridge 在 CMake/Make 上的设计原理，
> 再逐项决定"借鉴 / 有意不做"，并记录 NPU（ccec puresimt）后端带来的适配差异。
> 配套阅读：`CMAKE_MAKE_ALIGNMENT_WITH_CUBRIDGE.md`（逐项对齐表）、
> `PARITY_AUDIT_2026-08-28.md`（参数面审计）、`BUILD_SYSTEM_COMPATIBILITY_MATRIX.md`（真机证据矩阵）。

---

## 一、cu-bridge 的设计原理（逐机制剖析）

### 1.0 设计前提：双层转换决定了构建桥的边界

> **术语纠正（2026-08-30，项目作者确认）**：cu-bridge 文档里"若移植到编译器
> 不认 CUDA 语法的平台，需增加第 4 层 kernel 语法转换"的提示**不适用于本项目**。
> cu2cce 的 puresimt 路线前端（`ccec -x asc --enable-simt`）**同样原生认识
> CUDA kernel 语法**（`__global__`/`threadIdx`/`<<<>>>` 直通），与 mxcc 是
> **同构关系**，差异只是很小的改动：`-x asc` 语言选择、`--enable-simt` 开关、
> `simt_adaptor.h` 原语别名（`__shared__→__ubuf__` 等）、FP64/架构宏边界。
> 因此 cu-bridge 的核心论证对本桥同样成立，且"能透传就透传"的参数策略对
> ccec 与对 mxcc 一视同仁。

cu-bridge 的目标编译器 mxcc（clang 系）原生认识 CUDA kernel 语法，因此"构建
系统适配"的全部任务收敛为一句话：

> **把一切 nvcc 调用变成 mxcc 调用，并把 include/lib 指向 cu-bridge/MACA。**

于是整个工程围绕三个关键词展开：**不修改用户工程**、**CUDA 外形的假 SDK**、
**数据驱动的参数翻译**。（主机侧 Runtime API 靠头文件宏替换 + wrapper 动态库，
Driver API 靠 `libcuda.so.1` 符号替身 libsymbol_cu.so——这些在构建期不可见。）

### 1.1 机制一：假 CUDA SDK 树 + PATH 包装（构建适配的物理基础）

- 入口约定（`docs/02_User_Manual/README.md:48-54`）：
  `MACA_PATH` / `CUCC_PATH` / `CUDA_PATH=$CUCC_PATH` / `PATH` 前插
  `${CUCC_PATH}/tools:${CUCC_PATH}/bin`，可 `ln -s cucc nvcc`（FAQ Q2，安装树已自动化）。
- 临时假 SDK（`tools/make_maca:10-126,342-360`）：
  `$CUBRIDGE_HOME/cu-bridge/CUDA_DIR` 按 `tools/link.json` 的 `pre_make` 段搭建——
  `bin/nvcc → cucc`、`bin/{c++,g++,gcc,cc} → gnu 包装器`、`include → cu-bridge/include`、
  `lib64/libcuda*.so → libsymbol_cu.so`、`libcudart* → libmcruntime.so`、
  cublas/cudnn/cufft/... 全套 CUDA 库名 → mc* 库，并补齐
  `targets/$ARCH-$OS/{include,lib}` 三元组（`make_maca:356-360`，PyTorch/FindCUDAToolkit 都会找它）。
- **布局描述是数据（manifest），不是代码**：`tools/link.json`（Make/pre_make 用）+
  `tools/mock/stub_link.json`（CMake mock 用，`virtualDir.py:40-75` 支持环境变量替换）。
  三套包装器消费同一份/同构的清单，而不是各写一遍 mkdir/ln。
- 幂等与并行：`make.log` 哨兵表示"假工具包已就绪、可跨次复用"（`make_maca:33-36`）；
  `USE_WCUDA_FOR_MULTI_JOB=1` 时目录追加 `$PPID` 后缀做并行隔离（`cmake_mock:16-20`）；
  `make clean`（按参数判断）删除整个假目录（`make_maca:365-367`）。

**设计意图**：用户工程一行不改——Makefile 里的 `$(NVCC)`、`-lcudart`、
`$CUDA_PATH/include` 全部"自然命中"假 SDK 的替身，命令级差异全部收敛到
`cucc` 一个点上再统一翻译。

### 1.2 机制二：CMake 双阶段协议（configure 骗探测，build 走真编译）

CMake 的 CUDA 语言探测分两步：configure 期做 CompilerId/ABI/Determine 探测，
build 期按 `build.make` 里**记录的绝对编译器路径**逐条发起真实编译。cu-bridge 的
应对是"双阶段"：configure 期让假 nvcc 只应答探测，build 期让同一路径变成真驱动。

- 入口分发（`tools/cmake_maca:34-42`，仅 41 行）：`CUCC_CMAKE_ENTRY=2` → `cmake_mock`，
  否则 `cmake_assist`；查询参数（`--version/-E/--help*`）直接透传真 cmake。
- **build 分流（`tools/cmake_mock:64-131` 的 `deal_build_param`）**：解析
  `--build <dir>`、`--target/-t`（连续多目标）、`--parallel/-j`、`--config`（忽略）、
  `--verbose`（忽略）、`--` 之后透传，按 build 目录有无 `build.ninja` 选择
  `make_maca`/`ninja_maca` 执行。**即：cmake --build 不会绕过 Make/Ninja 包装器**，
  Make/Ninja 拿到与直接调用完全一致的环境准备。
- **mock 探测应答（`tools/mock/mock_nvcc.py` 五分支）**：
  1. `nvcc -v __cmake_determine_cuda` → 打印 `#$ _NVVM_BRANCH_=nvvm ...` 假驱动
     trace 并以 `nvcc fatal : Don't know what to do with ...` 结尾（`mock_nvcc.py:16-63`）
     ——CMake 正是解析 `#$` 行且容忍该退出码；
  2. `--version` → 伪造 NVIDIA 官方 banner `nvcc: NVIDIA (R) Cuda compiler driver /
     Cuda compilation tools, release 11.6, V11.6.55`（`:66-81`）→
     `CMAKE_CUDA_COMPILER_ID=NVIDIA`；
  3. `CMakeCUDACompilerId.cu` → 打印完整假 nvcc 分步 trace（gcc→cicc→ptxas→
     fatbinary→cudafe++→nvlink→g++），并把**预编译 a.out**（真 nvcc 编出的
     CompilerId 程序，`stub_link.json:38-41`）拷到 cwd 供 CMake 运行取
     `INFO:compiler[NVIDIA]`（`:115-158`）；
  4. `CMakeCUDACompilerABI.cu` → **用宿主 g++ 现编替身源码**
     `CMakeCUDACompilerABI.cpp`（保留 `INFO:sizeof_dptr/byte_order` 二进制探针串、
     `cmakeCompilerCUDAArch()` 返回无设备路径，`:161-178`）——不需要平台二进制工件，
     探测期才编译；
  5. 其余一切（真编译/链接/`-MD` 依赖生成）→ 落到真编译路径。
- **改写 CMake 模块路线**（`cmake_module/maca/`，第三条接入路径）：
  `CMakeDetermineCUDACompiler.cmake` 重定义编译器为假 nvcc、把 C/CXX 链接也重定向、
  默认 architectures 70;75;80、`CMAKE_CUDA_COMPILER_ID_FLAGS_ALWAYS=-v` + vendor 正则、
  `_CMAKE_CUDA_RDC_FLAG`、清空隐式库等；另有伪语言 `MACA`（`CMakeDetermineMACACompiler.cmake`）
  与老工程用的完整 `FindCUDA.cmake` 移植版。
- **第四条路（文档推荐最省事）**：`.cu` 当 CXX 编——
  `set_source_files_properties(x.cu PROPERTIES LANGUAGE CXX)` +
  `set(CMAKE_CXX_COMPILER $ENV{CUDA_PATH}/bin/nvcc)`，完全绕开 CMake 的 CUDA 语言。

### 1.3 机制三：cucc 四段流水线（nvcc → mxcc 的命令翻译引擎）

`bin/cucc`（1169 行 bash）+ `tools/cu2cce_driver.py`（Python 规则驱动核心）：

1. **参数预处理**（`cucc:23-172`）：`--options-file`/`@file` 响应文件展开并用
   **哨兵 token**（`nvccsign_space/quote/dollar/...`）保住空格与引号（CMake 长
   命令行的关键，`:36-53,154-172`）；`-Xcompiler` 内联修补；`--Werror <name>` 拆对
   并丢弃 nvcc 特有警告名（`:84-116`）；`-DX="a b"` 引号重排（`:118-150`）。
2. **JSON 规则引擎**（`cucc:411-824`，规则全在 `bin/conf*.json`）：
   作用域 `all/compile/link/compile_and_link` + `condition`（按源文件）+
   `not_found`（兜底）+ **`skipstring`（查询直通）**——凡参数命中
   `-dumpversion/-dumpmachine/-print-*` 等探测旗标，**整条命令绕过规则引擎直通
   mxcc**（`conf.json:3-17`、`cucc:839-847,1135-1149`）。四类动作：
   `filterparam`（静默丢弃）、`replaceparam`（替换，元语法 `prefix#*`/`a#b`/
   `*=*`/`&` 多 token 展开）、`adder`（无条件/条件追加）、`skipstring`。
3. **模式判定**（`cucc:849-882`）：`-c` → compile；出现源文件无 `-c` →
   compile_and_link；仅 `.o`/库 → link；**参数含 `*.o` 时追加设备链接旗标**——
   "链接含设备码的目标文件"等同设备链接处理，RDC 语义折叠进链接步。
4. **注入与后处理**：目标架构 `--offload-arch`（优先命令行，其次 `CUCC_TARGETS`/
   `CUCC_TARGETS_FROM_DEVICE`；Ascend 版要求单个合法 `dav-*`，未提供时由
   `device_arch.sh` 经 `npu-smi` 探测，无法探测则使用 `dav-c310-vec` 默认值并打印
   usage tip）；`-gencode/-arch` →
   `-DNV_ARCH_* -Xdevice -D__CUDA_ARCH__=NNN`（**`-Xdevice` 把宏限定在 device
   pass**，host 代码的 `#ifdef __CUDA_ARCH__` 分支不受污染——这是宏语义的关键）；
   `.cu` 条件追加 `--include cuda_runtime.h`（nvcc 自动预包含语义，
   `conf.json:357-427`）；`-imacros __macro_mxcc.h` 打通 `__CUDACC__`；未知参数
   借 `-forward-unknown-to-compiler` 下沉给编译器而不是报错；最后命令串经
   `gomxccbin` + `conf_regex.yaml` 正则清洗（踩坑固化层）再 source 执行。
5. **宿主编译器包装 `bin/gnu`**（c++/g++/gcc/cc/icpc/mpicxx 共用）：compile-only
   走真 gcc（参数过 `conf_gnu.json` 过滤/追加），**link 走 mxcc** 并强制追加
   `-lruntime_cu -lToolsExt_cu`（`gnu:1093-1141`）——链接期统一收口设备链接与
   运行库注入；`conf_gnu.json` 数据驱动，另有 `conf_gfortran.json`（仅链接期补
   `-lmccompiler`）与 `conf_bazel.json`（TF/Bazel 场景）。

### 1.4 机制四：产物约定（下游不断链）

- 目标文件 `.o`、`-MD/-MT/-MF` 依赖文件**原样透传**给 mxcc（clang 原生支持），
  `.d` 由后端产出，CMake 依赖扫描不断；设备链接不产生中间 fatbin.c/link.stub，
  全部折叠进 mxcc 驱动内部；ELF 不做改写——符号级兼容在运行时由
  libsymbol_cu/libruntime_cu 承担。
- 必须保持的"外形约定"：`targets/$TRIPLE/{include,lib}`、`libcuda.so/.1`、
  `libcudart.so(.11.0)/libcudart_static.a/libcudadevrt.a` 等**文件名级替身**、
  CMake 探测路径下的固定文件名（`bin/a.out`、`CMakeCompilerABI.h/cpp`）。

---

## 二、cu2cce-bridge 借鉴决策表（本批次收口的差异）

> NPU 后端（CANN 9.2 / ccec clang15 / `--enable-simt` puresimt）与 mxcc 的
> 关系是**同构前端 + 小改动**（见 §1.0 纠正），真正的取舍来自两点：
> **(a) ccec 不带 NVIDIA 工具链二进制**（无 ptxas/nvlink/fatbin，无公开的
> `-Xdevice` 式 device-pass 限定对应物）；
> **(b) 单遍编译**——`__CUDA_ARCH__` 无法按 nvcc 语义"只在 device pass 定义"。

| # | 差异点（命令/功能/参数/产物） | cu-bridge 做法 | 本批次决策 | 落点 |
|---|---|---|---|---|
| 1 | `cmake_cce --build` 不分流 | `deal_build_param` 分流 make/ninja 包装器 | **借鉴**：解析 `--build/--target/-t/--parallel/-j/--config/--verbose/--`，按 `build.ninja` 分发 `make_cce`/`ninja_cce` | `tools/cmake_cce` |
| 2 | 无查询直通层 | `skipstring` 整条命令绕过规则引擎直通 mxcc | **借鉴**：rules 新增 `queries` 段（`-dumpversion/-dumpmachine/-print-*`），命中即 phase=query 原样直通 ccec（无 `-x asc`/无注入/无 cmake identity） | `bin/conf.json`、`cu2cce_driver.py` |
| 3 | `-std` 双重注入 | 用户 flag 透传 + 无默认冲突 | **修复**：用户已给 `-std=` 时抑制 `-std=gnu++17`（`no_user_std` 条件） | rules compile.inject |
| 4 | `-gencode/-arch` 只丢不识别 | 翻译成 `-DNV_ARCH_*` + `-Xdevice -D__CUDA_ARCH__` | **机制级借鉴**：drop 前提取架构值（compute_XX/sm_XX → NVCC 编码 major*100+minor*10），入 trace；`CU2CCE_ARCH_MACRO=auto` 时注入 `-D__CUDA_ARCH__=<值>`（默认 off，显式 `CU2CCE_CUDA_ARCH` 仍优先）。**有意不做** `-Xdevice` 等价物：ccec 无公开对应机制，且 puresimt 单遍下全局定义会改变 host `#ifdef __CUDA_ARCH__` 分支——故为 opt-in 并记录风险 | `cu2cce_driver.py`（extract_cuda_arch） |
| 5 | `.cu` 无自动预包含 cuda_runtime.h | condition 对 `.cu` 追加 `--include cuda_runtime.h` | **借鉴**：`source_is_cuda_and_runtime_header` 条件注入 `-include cuda_runtime.h`（桥内 `__cuda_runtime.h` 已自含 simt_adaptor.h 且有 include guard，顺序安全；对齐 nvcc 语义） | rules compile.inject |
| 6 | hostcc 链接注入硬编码在脚本 | `conf_gnu.json` 数据驱动 | **借鉴**：rules 新增 `hostcc.link.inject` 段，`hostcc_cce` 薄壳化委托 `cu2cce_driver.py --hostcc-mode`（保留无桥退化语义；链接仍走 g++——NPU 有意保留，见决策 12） | rules、`tools/hostcc_cce`、driver |
| 7 | capabilities 前缀匹配过宽 | 逐库 fnmatch 规则 | **收窄**：精确名/`key_` 前缀给精确提示，其余宽前缀降级为"heuristic prefix match"提示（保留 advisory 覆盖，文案不再误导） | `_handle_library_policy` |
| 8 | mock ABI 探针写 16 字节假 ELF | 宿主 g++ 现编替身源码（保留 INFO 串） | **借鉴（去掉工件）**：新增 `tools/mock/CMakeCUDACompilerABI.cpp/.h`，探测期现编（`-c` 形态编对象、可执行形态真链接，运行输出 "No CUDA devices found."），失败回退占位 | `tools/mock/`、`mock_nvcc.py` |
| 9 | shim 布局三处硬编码重复 | `link.json`/`stub_link.json` 单一清单 | **借鉴**：`tools/link.json` + `tools/mock/virtualDir.py`，cmake_cce/make_cce/pre_cce 共用（幂等原子软链、targets 三元组、version.txt、CUDAToolkit config shim） | 新增两文件、三入口 |
| 10 | shim lib64 只有 5 个 CUDA 库名 | 全套 CUDA 库名软链 | **扩展（实验性）**：`libcublas.so(.1)`、`libnvToolsExt.so(.1)` → 桥运行时（桥内 cublas 面三用例真机绿 + nvtx stub 已存在），矩阵标 EXPERIMENTAL；**有意不做**全量 `-lcu*→CANN 库` 重写表 | tools/link.json |
| 11 | `clean` 子串匹配误删（`cleanpack`） | 参数含 "clean" 删假目录 | **修复（更严）**：按目标名词元匹配 `clean/*clean/clean-*` | make_cce、pre_cce |
| 12 | 无并行隔离选项 | `USE_WCUDA_FOR_MULTI_JOB`（$PPID 后缀） | **借鉴**：`CU2CCE_SHIM_ISOLATE=1` 三入口统一支持 | cmake_cce/make_cce/pre_cce |
| 13 | pre_cce 宿主编译器是身份软链 | gnu 包装器 | **统一**：pre_cce 改用 hostcc_cce（autotools 纯 g++ 链接也能拿到桥运行时注入） | pre_cce |
| 14 | 预编译 CompilerId a.out / 0 字节 CMakeTestCUDACompiler patch | 预编译 ELF 技巧 | **有意不做**：不携带平台二进制工件；CompilerId 走 vendor 正则 + REQUIRE_SUCCESS + mock 真编译回退，真机已验证（对齐文档既有结论维持） | — |
| 15 | RDC/`-dlink` → 设备链接折叠 | `.o` 链接时追加设备链接旗标 | **有意不做**：ccec puresimt 无可分离设备码，维持 reject/drop + CMake 层 `CUDA_SEPARABLE_COMPILATION OFF` | — |
| 16 | 未知 flag `-forward-unknown-to-compiler` | 借该旗标下沉给编译器 | **维持现状 + 真机观察项**：drop（ccec 对该 nvcc 兼容旗标的接受度未知，宁可显式警告；strict 可升级为错误）。同构前端前提下，若真机验证 ccec 接受，可改为透传（§五） | — |
| 17 | 响应文件哨兵 token | `nvccsign_*` 哨兵保引号 | **维持现状**：shlex posix 解析已覆盖同一语义（契约测试锁定） | — |
| 18 | gomxccbin + conf_regex.yaml 正则清洗层 | 踩坑固化的命令串清洗 | **机制已借鉴（第六批次）**：`regex_cleanup` 数据驱动 PCRE 清洗段 + 宿主旗标逐 flag 清洗（见决策 21-23），单进程实现、动作进 trace；后续踩坑只加规则不改代码 | rules `regex_cleanup`、driver |
| 19 | Bazel（conf_bazel.json）/ gfortran（conf_gfortran.json）/ MPI | 场景化 conf + 包装器 | **维持已知差距**（NPU 场景暂无需求，记录于 §四） | — |
| 20 | `USE_CMAKE_MACA=1` 系统 cmake 二进制替换 | 备份/软链/恢复系统 cmake | **有意不做**：官方 `CMAKE_USER_MAKE_RULES_OVERRIDE` 钩子可维护性更优（对齐文档既有结论维持） | — |
| 21 | `-Xcompiler` 宿主旗标清洗粒度 | 逐参数清洗（cucc:55-83 内联 + gomxccbin） | **对齐 + 修 bug**：本桥原"首个决定性 flag 决定整组"会泄漏（`-Xcompiler -fPIC,-march=native` 把 -march 放行），且 `-Xcompiler=...` 等号形式完全绕过过滤——均改为逐 flag 处理 | `_handle_host_flag` |
| 22 | `-Werror` 族清洗（conf_regex.yaml C500S-11780） | 只从 `-Xcompiler` 段剔除裸 `-Werror`（`-Wno-error=cpp` 等合法拼写保留） | **对齐**：`-Xcompiler` 值中的 `-Werror`/`-Werror=<名>` 丢弃；`-Werror=implicit-fallthrough*` 重写为 clang 兼容 `-Werror=implicit-fallthrough`（cucc:250-280 对齐，原 drop 改 replace） | `_handle_host_flag`、rules replace |
| 23 | `add_gcc_toolchain`（cucc:1055-1066） | PATH 上真 gcc 非 /usr/bin/gcc 时追加 `--gcc-toolchain=<prefix>/` | **对齐（休眠机制）**：`_locate_real_gcc` 跳过 cu2cce shim PATH 项；非标准布局追加旗标，真机 /usr/bin/gcc 存在即 no-op；`CU2CCE_GCC_TOOLCHAIN=off` 可关 | driver |
| 24 | 调试状态传递（make_maca↔cucc 的 BASH_XTRACE/VERBOSE/ERREXIT） | shell set -x 状态跨脚本传递 | **对齐**：bin/cucc 将三个 shell 调试变量映射为 `CU2CCE_DEBUG`（make_cce/pre_cce 已导出） | bin/cucc |
| 25 | 库名/链接面过滤补齐（conf.json filter） | `-lnvrtc`、`-lOpenCL` 直接 filter（不等到链接期才炸）；`--share → -shared`；`-Xlinker <arg>` 直通 mxcc | **对齐 + 修丢失**：`-lnvrtc/-l:libnvrtc*/-lOpenCL` 从静默 forward 改为带警告 drop；`--share` replace 为 `-shared`；`-Xlinker` 对由 drop 改为**成对透传 ccec**（clang 原生，原 drop 丢失用户 rpath/链接脚本） | rules、driver |
| 26 | `__CUDACC__` 宏族（`-imacros __macro_mxcc.h`，conf.json all.adder） | mxcc 的 `__MACACC__` 翻译为 `__CUDACC__` + 版本宏 | **对齐（机制级）**：compile 注入 `-D__CUDACC__` + `__CUDACC_VER_MAJOR/MINOR/BUILD/VER`（值由 `CUCC_CUDA_VERSION` 推导，nvcc 编码 MMmmB）；仅 `.cu` TU（nvcc 语义）；mock `--cmake-identity` 前缀退役为 no-op——宏只剩规则文件一个定义点（simplifier） | rules compile.inject、driver |
| 27 | gcc-only 旗标过滤（conf.json filter 的 var-tracking 族） | `-fno/var-tracking(-assignments)` 静默过滤 | **对齐**：regex_cleanup 种子 `^-f(no-)?var-tracking(-assignments)?$`（clang 报 unknown argument）；同时从宿主旗标白名单移除（白名单此前误放行） | rules regex_cleanup、driver |
| 28 | 代码简化（code review 产出） | link.json 单一实现被三个包装器复用 | **对齐同款思想**：`tools/virtual_toolkit_env.sh` 共享库（哨兵 + builder 调用 + host triple），cmake_cce/make_cce/pre_cce 三处 ~40 行重复收敛为一次 source；规则文件去掉 `--expt-extended-lambda`/`--generate-code=*` 重复项；`-lcublasLt` 库匹配改两段式（精确名优先于宽前缀，提示不再错位） | tools/virtual_toolkit_env.sh、三入口、rules、driver |
| 29 | conf.json filter/replacer 终检缺口（第八批） | `-lnvrtc`/`-lOpenCL` filter、`--share→-shared`、`-Xlinker` 直通、`--use_fast_math`→后端拼写、`-lgomp`→自家 OpenMP 运行时、`-static` 因后端限制改写、杂项 filter（`g++*`/`--threads=*`/`/usr/bin/cc`/`-keep-dir`/`--W*` 双横线族/`-diag-suppress`）、banner 可解析 release 词组 | **全部对齐（后端拼写不同）**：drop 带警告 / replace 规范化（`-ffast-math`、`-lomp`、`-shared`）/ `-Xlinker` 成对透传 / `-static` 诚实 drop；合法单词号 clang 警告有意保留 forward；banner 携带 `compatible with release 11.6` | rules、driver |
| 30 | 真机可验证性 | 测试报告 xlsx（200+ 工程） | **harness 内建**：server_cmd.txt full 画像新增机制探针段（queries 直通 / ARCH_MACRO+__CUDACC__+预包含 dryrun / Xlinker+fast-math+nvrtc）与 `cmake_cce --build` 分流探针，失败即打落 CUCC_RULES/CMAKE badge | server_cmd.txt |

## 三、本批次后的机制拓扑（终态）

```text
cmake_cce ──configure──> tools/mock/virtualDir.py(tools/link.json) ──> mock_nvcc.py（探测应答/真编译回退）
   │                                        │                        └─> cu2cce_driver.py（--cmake-identity）
   ├──--build──────────deal_build_param─────┴──> make_cce / ninja_cce（build.ninja 选择）
make_cce/pre_cce ──> tools/mock/virtualDir.py ──> PATH+CUDA_PATH ──> make/direct-exec
bin/nvcc / cucc ──> cu2cce_driver.py ──[queries 直通 | 五动作规则 | 注入/排序]──> ccec -x asc --enable-simt
hostcc_cce ──> cu2cce_driver.py --hostcc-mode ──[hostcc.link.inject]──> /usr/bin/g++（链接补桥运行时）
```

## 四、剩余已知差距（有意保留，待需求驱动）

### 4.0 cmake/make 收口状态复核（第六批次）

对 cu-bridge CMake/Make 清单做最后一轮逐项核对，本桥现状：

| cu-bridge 项 | 状态 |
|---|---|
| cmake_maca 查询透传 / CUCC_CMAKE_ENTRY 双模式 | 已收口（cmake_cce 查询透传；`CU2CCE_CMAKE_NO_OVERRIDE` ≈ cmake_assist 回退） |
| 双阶段（configure=mock / build=真驱动） | 已收口（持久 shim + 两阶段重指向） |
| deal_build_param（--build → make/ninja 包装器） | 已收口（第六批次前一批落地） |
| mock 探测四分支 + ABI 现编替身 | 已收口（多 `--simple-preprocess` 分支；ABI 走 shipped 源码现编） |
| link.json / stub_link.json 清单化假 SDK | 已收口（tools/link.json + tools/mock/virtualDir.py，三入口共用） |
| make_maka 幂等哨兵 / clean / 并行隔离 | 已收口（make.log 哨兵；clean 词元匹配；CU2CCE_SHIM_ISOLATE） |
| pre_make / pre_makefile | 已收口（pre_cce，hostcc 统一） |
| bin/gnu 包装器 + conf_gnu.json | 已收口（hostcc_cce + rules hostcc 段；链接器有意保留 g++） |
| multi_target / get_device_arch | 已收口（device_arch.sh；NPU 单 arch 无多目标列表） |
| skipstring 查询直通 | 已收口（rules queries 段） |
| conf.json 五动作 + condition + not_found | 已收口（五动作 + when 条件；not_found 兜底由 arch_macro/queries 承接对应子集） |
| gomxccbin + conf_regex.yaml 清洗层 | 已收口（regex_cleanup + 逐 flag 宿主旗标清洗，第六批次） |
| add_gcc_toolchain | 已收口（休眠机制，第六批次） |
| conf.json filter 库名面（-lnvrtc/-lOpenCL/--share/-Xlinker） | 已收口（第七批次：drop 带警告、replace 规范化、成对透传） |
| `-imacros __CUDACC__` 宏族 | 已收口（第七批次：compile 注入，CUCC_CUDA_VERSION 推导，单一定义点） |
| `-D` 引号转义重排 / 哨兵响应文件 | 等价实现（无 eval 的 argv 列表 + shlex） |
| conda LDFLAGS / CUCC_CUDA_VERSION / WCUDA_DEBUG | 已对齐（同名或同语义） |
| 多架构去重（choose_arch_nvcc） | 等价（单 arch，取 max） |
| CCCL 版本门控 include | 不适用（NPU 无 cccl） |
| conf_bazel / conf_gfortran / MPI / USE_CMAKE_MACA / 预编译 a.out | 维持有意不做/已知差距（见 §四 1-5） |

**结论：cu-bridge 的 CMake/Make 与 cucc 运行时机制面已全部收口**——除 NPU 明确
不适用项与维持"有意不做"的四项外，无遗留机制差距；后续差异只会来自真机踩坑
（增补 rules 规则即可，无需改驱动代码）。

### 4.1 差距清单

1. **Bazel/TF 场景**（conf_bazel.json / `WCUDA_BAZEL_TF_BUILD` 等价物）；
2. **gfortran/MPI 包装器**（conf_gfortran.json + ompi 分派；NPU 场景 Fortran 需求未出现）；
3. **FindCUDA legacy 宏全集**的 ptx/fatbin/cubin 族（永久 FATAL_ERROR，诚实不支持）；
4. **头文件生态**（cu-bridge ~1617 头 vs 本桥 29 头）与 cuDriver API 面——SDK 级依赖，另立项；
5. **多 `.cu` 聚合进同一静态 archive**（ccec TU registration 聚合能力，昇腾侧议题，
   真机 duplicate symbol 证据见 BUILD_SYSTEM_COMPATIBILITY_MATRIX.md）。

## 五、真机验证清单（本次新增行为的 server_cmd.txt 观察点）

| 行为 | 观察点 | 预期 |
|---|---|---|
| `.cu` 预包含 cuda_runtime.h | puresimt 全量回归（现有例子全部显式 include，主要看无回归） | 六 badge 不回退 |
| mock ABI 现编 | `cmake_cce` configure 日志出现 "ABI probe compiled from shipped ..."，CMake ABI 步骤无 exec-format 噪音 | CMAKE_CCE_REGRESSION_PASS |
| `--build` 分流 | 任一例子改用 `cmake_cce --build build -jN` 跑一次 | 与 `cmake --build` 等价 |
| `CU2CCE_ARCH_MACRO=auto` | `cucc -c t.cu -gencode arch=compute_80,code=sm_80 -o t.o` + WCUDA_DEBUG=1 | 命令行含 `-D__CUDA_ARCH__=800` |
| queries 直通 | `cucc -print-file-name=cudart` | ccec 原生应答，无注入 |
| hostcc 规则化 | `make_gxx_link` 载体 + 任何纯 g++ 链接例子 | 链接命令尾部含 `-lcce_bridge_runtime -lascendcl` |
| lib64 别名扩展 | `make_gxx_link` 加 `-lcublas` 变体（EXPERIMENTAL） | 链接解析到桥运行时，运行期 cublas 三用例语义不变 |
| 清理/隔离 | `make_cce clean` / `make_cce cleanpack` / `CU2CCE_SHIM_ISOLATE=1` 并行两作业 | 前者删假目录、后者不删、隔离各自生效 |
| `-Werror` 族清洗 | `-Xcompiler=-fPIC,-Werror,-Wno-error=cpp` 场景（CMake 传播） | 裸 `-Werror` 丢弃、`-Wno-error=cpp` 保留、无回归 |
| implicit-fallthrough 重写 | 带 `-Werror=implicit-fallthrough=2` 的工程 | ccec 收到 `-Werror=implicit-fallthrough`（非 =N） |
| regex_cleanup 层 | 直达 `-fno-canonical-system-headers` / 逗号列表中的 gcc-only 旗标 | 丢弃且 trace 有 `regex_drop` 记录 |
| add_gcc_toolchain | 真机 `/usr/bin/gcc` 存在 | no-op（休眠）；`sh -c 'command -v gcc'` 非标准布局时确认 `--gcc-toolchain` 出现在 ccec 命令行 |
| `-forward-unknown-to-compiler` 探针 | 真机 `cucc -c t.cu -forward-unknown-to-compiler -o t.o`（CCEC_PATH 指向真 ccec） | 若 ccec 接受则考虑从 drop 改透传（同构前端前提下合理）；拒绝则维持 drop |
| `__CUDACC__` 宏族 | 带 `#ifdef __CUDACC__` 的用户 kernel 编译 + `ccec` 内建宏行为观察 | 注入值与 ccec 自身定义（若有）一致无 redefinition 告警；`.cpp` TU 不出现 |
| `-Xlinker` 透传 | 带 `-Xlinker -rpath=...` 的链接 | ccec 侧 rpath 生效；compile 阶段仅 unused-argument 告警 |
| 库名 drop 提示 | `-lnvrtc` 参与 | 立即警告（非链接期失败），strict 模式报错 |
| fast-math / OpenMP 映射 | `--use_fast_math` → `-ffast-math`；`-lgomp` → `-lomp` | dryrun 探针锁定（server_cmd full 画像），FP/并行语义由调用方选择 |
| 机制探针段 | server_cmd full 画像 `[2.75/7]` 探针块 + `[5/7]` 分流探针 | 三组探针任一失败即 `CUCC_RULES_PASS=0`/`CMAKE_PASS=0`，badge 不再能掩盖机制回归 |
