# cu-bridge CMake/Make 设计原理剖析与借鉴决策（第五对齐批次，2026-08-30）

> 参考仓库：`/Users/tmchao7/repos/09_expLearn/cu-bridge`（沐曦 MetaX，SDK 3.8.0，只读）。
> 本文是第五对齐批次的决策记录：先完整剖析 cu-bridge 在 CMake/Make 上的设计原理，
> 再逐项决定"借鉴 / 有意不做"，并记录 NPU（ccec puresimt）后端带来的适配差异。
> 配套阅读：`CMAKE_MAKE_ALIGNMENT_WITH_CUBRIDGE.md`（逐项对齐表）、
> `PARITY_AUDIT_2026-08-28.md`（参数面审计）、`BUILD_SYSTEM_COMPATIBILITY_MATRIX.md`（真机证据矩阵）。

---

## 一、cu-bridge 的设计原理（逐机制剖析）

### 1.0 设计前提：双层转换决定了构建桥的边界

cu-bridge 的目标编译器 mxcc（clang 系）**原生认识 CUDA kernel 语法**
（`__global__`/`<<<>>>`），因此"构建系统适配"的全部任务收敛为一句话：

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

`bin/cucc`（1169 行 bash）+ `bin/process_args.py`（python3 时替代 bash 规则引擎）：

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
   `CUCC_TARGETS_FROM_DEVICE`，皆无则**报错退出**，`cucc:1085-1107`；
   设备探测 `tools/get_device_arch.sh` 走 mx-smi + sysfs）；`-gencode/-arch` →
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

> NPU 后端（CANN 9.2 / ccec clang15 / `--enable-simt` puresimt 单遍直通）与
> mxcc 的两个根本差异决定了所有取舍：
> **(a) ccec 不认 CUDA 宏体系/架word 体系之外的 NVIDIA 工具链**（无 ptxas/nvlink/
> fatbin，无 `-Xdevice` 式 device-pass 限定的公开对应物）；
> **(b) 单遍编译**——`__CUDA_ARCH__` 无法按 nvcc 语义"只在 device pass 定义"。

| # | 差异点（命令/功能/参数/产物） | cu-bridge 做法 | 本批次决策 | 落点 |
|---|---|---|---|---|
| 1 | `cmake_cce --build` 不分流 | `deal_build_param` 分流 make/ninja 包装器 | **借鉴**：解析 `--build/--target/-t/--parallel/-j/--config/--verbose/--`，按 `build.ninja` 分发 `make_cce`/`ninja_cce` | `tools/cmake_cce` |
| 2 | 无查询直通层 | `skipstring` 整条命令绕过规则引擎直通 mxcc | **借鉴**：rules 新增 `queries` 段（`-dumpversion/-dumpmachine/-print-*`），命中即 phase=query 原样直通 ccec（无 `-x asc`/无注入/无 cmake identity） | `config/cu2cce_rules.json`、`cu2cce_driver.py` |
| 3 | `-std` 双重注入 | 用户 flag 透传 + 无默认冲突 | **修复**：用户已给 `-std=` 时抑制 `-std=gnu++17`（`no_user_std` 条件） | rules compile.inject |
| 4 | `-gencode/-arch` 只丢不识别 | 翻译成 `-DNV_ARCH_*` + `-Xdevice -D__CUDA_ARCH__` | **机制级借鉴**：drop 前提取架构值（compute_XX/sm_XX → NVCC 编码 major*100+minor*10），入 trace；`CU2CCE_ARCH_MACRO=auto` 时注入 `-D__CUDA_ARCH__=<值>`（默认 off，显式 `CU2CCE_CUDA_ARCH` 仍优先）。**有意不做** `-Xdevice` 等价物：ccec 无公开对应机制，且 puresimt 单遍下全局定义会改变 host `#ifdef __CUDA_ARCH__` 分支——故为 opt-in 并记录风险 | `cu2cce_driver.py`（extract_cuda_arch） |
| 5 | `.cu` 无自动预包含 cuda_runtime.h | condition 对 `.cu` 追加 `--include cuda_runtime.h` | **借鉴**：`source_is_cuda_and_runtime_header` 条件注入 `-include cuda_runtime.h`（桥内 `__cuda_runtime.h` 已自含 simt_adaptor.h 且有 include guard，顺序安全；对齐 nvcc 语义） | rules compile.inject |
| 6 | hostcc 链接注入硬编码在脚本 | `conf_gnu.json` 数据驱动 | **借鉴**：rules 新增 `hostcc.link.inject` 段，`hostcc_cce` 薄壳化委托 `cu2cce_driver.py --hostcc-mode`（保留无桥退化语义；链接仍走 g++——NPU 有意保留，见决策 12） | rules、`tools/hostcc_cce`、driver |
| 7 | capabilities 前缀匹配过宽 | 逐库 fnmatch 规则 | **收窄**：精确名/`key_` 前缀给精确提示，其余宽前缀降级为"heuristic prefix match"提示（保留 advisory 覆盖，文案不再误导） | `_handle_library_policy` |
| 8 | mock ABI 探针写 16 字节假 ELF | 宿主 g++ 现编替身源码（保留 INFO 串） | **借鉴（去掉工件）**：新增 `tools/mock/CMakeCUDACompilerABI.cpp/.h`，探测期现编（`-c` 形态编对象、可执行形态真链接，运行输出 "No CUDA devices found."），失败回退占位 | `tools/mock/`、`mock_nvcc.py` |
| 9 | shim 布局三处硬编码重复 | `link.json`/`stub_link.json` 单一清单 | **借鉴**：`config/shim_layout.json` + `tools/build_shim_tree.py`，cmake_cce/make_cce/pre_cce 共用（幂等原子软链、targets 三元组、version.txt、CUDAToolkit config shim） | 新增两文件、三入口 |
| 10 | shim lib64 只有 5 个 CUDA 库名 | 全套 CUDA 库名软链 | **扩展（实验性）**：`libcublas.so(.1)`、`libnvToolsExt.so(.1)` → 桥运行时（桥内 cublas 面三用例真机绿 + nvtx stub 已存在），矩阵标 EXPERIMENTAL；**有意不做**全量 `-lcu*→CANN 库` 重写表 | shim_layout.json |
| 11 | `clean` 子串匹配误删（`cleanpack`） | 参数含 "clean" 删假目录 | **修复（更严）**：按目标名词元匹配 `clean/*clean/clean-*` | make_cce、pre_cce |
| 12 | 无并行隔离选项 | `USE_WCUDA_FOR_MULTI_JOB`（$PPID 后缀） | **借鉴**：`CU2CCE_SHIM_ISOLATE=1` 三入口统一支持 | cmake_cce/make_cce/pre_cce |
| 13 | pre_cce 宿主编译器是身份软链 | gnu 包装器 | **统一**：pre_cce 改用 hostcc_cce（autotools 纯 g++ 链接也能拿到桥运行时注入） | pre_cce |
| 14 | 预编译 CompilerId a.out / 0 字节 CMakeTestCUDACompiler patch | 预编译 ELF 技巧 | **有意不做**：不携带平台二进制工件；CompilerId 走 vendor 正则 + REQUIRE_SUCCESS + mock 真编译回退，真机已验证（对齐文档既有结论维持） | — |
| 15 | RDC/`-dlink` → 设备链接折叠 | `.o` 链接时追加设备链接旗标 | **有意不做**：ccec puresimt 无可分离设备码，维持 reject/drop + CMake 层 `CUDA_SEPARABLE_COMPILATION OFF` | — |
| 16 | 未知 flag `-forward-unknown-to-compiler` | 借该旗标下沉给编译器 | **维持现状**：`warn_forward`（ccec 对未知 flag 的容忍度未知，宁可显式警告；strict 可升级为错误） | — |
| 17 | 响应文件哨兵 token | `nvccsign_*` 哨兵保引号 | **维持现状**：shlex posix 解析已覆盖同一语义（契约测试锁定） | — |
| 18 | gomxccbin + conf_regex.yaml 正则清洗层 | 踩坑固化的命令串清洗 | **有意不做**：单进程 python 驱动内完成等价清洗（`--Werror` 配对、陈旧 `-I/usr/local/cuda*` 丢弃等已在 driver 内落位） | — |
| 19 | Bazel（conf_bazel.json）/ gfortran（conf_gfortran.json）/ MPI | 场景化 conf + 包装器 | **维持已知差距**（NPU 场景暂无需求，记录于 §四） | — |
| 20 | `USE_CMAKE_MACA=1` 系统 cmake 二进制替换 | 备份/软链/恢复系统 cmake | **有意不做**：官方 `CMAKE_USER_MAKE_RULES_OVERRIDE` 钩子可维护性更优（对齐文档既有结论维持） | — |

## 三、本批次后的机制拓扑（终态）

```text
cmake_cce ──configure──> build_shim_tree.py(shim_layout.json) ──> mock_nvcc.py（探测应答/真编译回退）
   │                                        │                        └─> cu2cce_driver.py（--cmake-identity）
   ├──--build──────────deal_build_param─────┴──> make_cce / ninja_cce（build.ninja 选择）
make_cce/pre_cce ──> build_shim_tree.py ──> PATH+CUDA_PATH ──> make/eval
bin/nvcc / cucc ──> cu2cce_driver.py ──[queries 直通 | 五动作规则 | 注入/排序]──> ccec -x asc --enable-simt
hostcc_cce ──> cu2cce_driver.py --hostcc-mode ──[hostcc.link.inject]──> /usr/bin/g++（链接补桥运行时）
```

## 四、剩余已知差距（有意保留，待需求驱动）

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
