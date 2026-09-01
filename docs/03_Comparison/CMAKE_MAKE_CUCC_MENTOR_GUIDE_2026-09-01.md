# cu2cce-bridge：CMake / Make / cucc 原理讲解与 cu-bridge 对齐指南

**参考实现：** `/Users/tmchao7/repos/09_expLearn/cu-bridge`，commit `6ab1076`，version `3.8.0`

**目标后端：** Ascend 950 / CANN 9.x / ccec SIMT

## 一、先讲清项目定位

cu2cce-bridge 不是把 CUDA kernel 文本翻译成另一种源代码，而是让 CANN 的 `ccec` 直接接收 CUDA SIMT 源文件，再用兼容头、运行时 bridge 和构建 wrapper 补齐 CUDA 工程所需要的外形。

当前主链：

```text
CUDA .cu
  -> cmake_cce / make_cce / 直接 cucc
  -> 参数和环境适配
  -> ccec -x asc --enable-simt --cce-aicore-arch=dav-c310-vec
  -> Ascend NPU
```

与 `cu-bridge` 的共同点是“构建系统尽量不改用户工程，只把 CUDA 工具链入口接到后端编译器”。区别是：

- `cu-bridge` 的后端是 MACA `mxcc`，最终使用 MACA runtime/library；
- cu2cce-bridge 的后端是 Ascend `ccec`，使用 `dav-*` 架构、CANN runtime、`libascendcl.so` 和 bridge runtime；
- 因此机制可以对齐，MACA 专属参数和库语义不能直接复制。

## 二、CMake 原理

### 2.1 `cu-bridge` 的 CMake 机制

参考入口：

```text
cu-bridge/tools/cmake_maca
  -> cmake_mock 或 cmake_assist
  -> CMake compiler detection
  -> cmake --build 分流到 make_maca/ninja_maca
```

CMake configure 期会探测：

- CUDA compiler identity；
- CUDA toolkit root；
- ABI；
- device linker/fatbinary 工具；
- 版本和架构信息。

`cu-bridge` 用 mock nvcc 回答这些探测，避免 configure 阶段真的调用 CUDA 编译器；build 阶段再把同一个 shim 中的 `nvcc` 指向真实 cucc/mxcc wrapper。

关键设计是双阶段：

```text
configure: nvcc -> mock
build:     nvcc -> real backend compiler
```

### 2.2 cu2cce 的 CMake 实现

入口：

```text
/Users/tmchao7/repos/09_expLearn/cu2cce-bridge/tools/cmake_cce
```

configure 期：

```text
cmake_cce
  -> tools/fake_toolkit_env.sh
  -> tools/build_shim_tree.py
  -> config/shim_layout.json
  -> CUDA_DIR/bin/nvcc -> tools/nvcc_entry.sh
  -> tools/mock/mock_nvcc.py
```

build 期：

```text
cmake_cce --build <build-dir>
  -> 识别 build.ninja 或 Makefiles
  -> ninja_cce / make_cce
  -> CUDA_DIR/bin/nvcc -> bin/cucc
  -> cucc -> tools/cu2cce_driver.py
  -> ccec
```

CMake 需要 fake toolkit 的原因不是为了伪造 CUDA 功能，而是为了让 CMake 找到它期望的目录和文件：

```text
CUDA_DIR/bin/nvcc
CUDA_DIR/include
CUDA_DIR/lib64
CUDA_DIR/targets/<host-triple>/include
CUDA_DIR/targets/<host-triple>/lib
CUDA_DIR/lib64/cmake/CUDAToolkit
```

当前实现使用一个 manifest：

```text
config/shim_layout.json
```

由一个 builder 统一创建：

```text
tools/build_shim_tree.py
```

这样 `cmake_cce`、`make_cce`、`ninja_cce`、`pre_cce` 不会分别维护三份软链逻辑。

### 2.3 为什么不照搬 `USE_CMAKE_MACA=1`

`cu-bridge` 有一种可选模式会备份系统 `cmake`，把它替换成 wrapper，再在结束后恢复。cu2cce 不采用这种方式：

- 需要 root 或 sudo；
- 会修改系统命令；
- 中途异常可能导致系统 CMake 没有恢复；
- 当前显式 `cmake_cce` 加 persistent shim 已经提供同等构建机制。

这是“机制等价、入口方式有意不同”的例子。

## 三、Make 原理

### 3.1 `cu-bridge` 的 Make 机制

`make_maca` 的核心做法：

1. 找到或创建临时 `CUBRIDGE_HOME/cu-bridge`；
2. 根据 `link.json` 创建 CUDA-shaped toolkit；
3. 把 `nvcc`、host compiler、CUDA 库名指向 MACA 实现；
4. 设置 `CUDA_PATH` 和 `PATH`；
5. 执行原始 `make`。

因此用户 Makefile 中的：

```make
$(NVCC) kernel.cu -o kernel.o
```

不需要改成 `mxcc`。

### 3.2 cu2cce 的 Make 机制

入口：

```text
/Users/tmchao7/repos/09_expLearn/cu2cce-bridge/tools/make_cce
```

流程：

```text
make_cce
  -> 创建 CUDA_DIR
  -> CUDA_DIR/bin/nvcc -> cucc
  -> host compiler -> hostcc_cce
  -> 设置 CUDA_PATH/PATH/CANN 环境
  -> exec make
```

device `.cu`：

```text
nvcc -> cucc -> cu2cce_driver.py -> ccec
```

host `.cpp`：

```text
g++/gcc -> hostcc_cce -> 系统 g++/gcc
```

host link 时追加：

```text
-lcce_bridge_runtime
-lascendcl
-lruntime
```

其中 `libruntime.so` 不能因为 CUDA alias 存在就认为会自动被 GNU ld 解析；真实服务器回归已经证明，Ascend runtime 符号需要在链接行中显式出现。

### 3.3 Ninja 与 Make 为什么需要同样的环境

CMake 的 build tree 会记录 CUDA compiler 的绝对路径。若 Ninja 直接运行时没有重新创建或重指向 shim，就可能出现：

```text
configure 记录的是 mock nvcc
build 仍然调用 mock nvcc
```

所以 `ninja_cce` 与 `make_cce` 共享：

```text
fake_toolkit_env.sh
build_shim_tree.py
CU2CCE_HOME
CU2CCE_CMAKE_SHIM_DIR
CU2CCE_SHIM_ISOLATE
CU2CCE_NPU_ARCH
```

区别只是最终执行 `ninja` 还是 `make`。

## 四、cucc 原理

### 4.1 沐曦 `cucc` 的基本结构

参考 `cu-bridge/bin/cucc` 是 Bash 驱动，主要步骤：

```text
argv
  -> --options-file / @response file
  -> CUDA/MACA target 检查
  -> 根据输入判断 compile/link/compile_and_link
  -> conf.json filter
  -> conf.json replacer
  -> condition/not_found adder
  -> 架构选择
  -> all/compile/link adder
  -> gomxccbin 二次命令清洗
  -> mxcc
```

参考配置主要在：

```text
cu-bridge/bin/conf.json
cu-bridge/bin/conf_regex.yaml
```

特点是：

- `cucc` 不是写死某一个工程；
- 参数通过 filter/replacer/adder 动态处理；
- 同一个驱动支持多个工程和多个 CUDA 参数形态；
- 不支持的选项由配置过滤或替换；
- 最终只把后端 compiler 能理解的参数交给 mxcc。

### 4.2 cu2cce 的动态规则驱动

当前入口：

```text
/Users/tmchao7/repos/09_expLearn/cu2cce-bridge/bin/cucc
```

真正执行：

```text
/Users/tmchao7/repos/09_expLearn/cu2cce-bridge/tools/cu2cce_driver.py
```

规则：

```text
/Users/tmchao7/repos/09_expLearn/cu2cce-bridge/config/cu2cce_rules.json
```

驱动 pipeline：

```text
argv
  -> response/options 展开
  -> query passthrough
  -> phase detection
  -> Ascend NPU arch resolution
  -> FP64 device preflight
  -> reject
  -> replace
  -> drop
  -> forward
  -> host flag filtering
  -> capability policy
  -> injection ordering
  -> regex cleanup
  -> trace/debug
  -> ccec
```

规则优先级固定为：

```text
reject -> replace -> drop -> forward
```

这是动态规则驱动的核心价值：新增一个普通 nvcc 参数时，优先在 JSON 中添加规则，不需要把每个 flag 写成 Python 分支。

### 4.3 为什么当前是 Python 而不是 Bash

`cu-bridge` 的 Bash 方案承担了很多字符串编码和 shell quoting：

```text
空格 -> nvccsign_space
美元 -> nvccsign_dollar
括号 -> nvccsign_lparenthesis/rparenthesis
```

cu2cce 使用 Python argv list 和 `shlex`：

- 参数从始至终保持 list，不需要拼成 shell command string；
- options file 不执行 shell；
- trace 更容易表达结构化 action；
- FP64 扫描和 capability policy 更适合放在 Python；
- 仍然保留 `cu-bridge` 的规则驱动思想。

因此“动态规则驱动”是正确方向，但不要求语法上也必须复制成 Bash。

## 五、三类参数处理

### 5.1 可以直接透传

例如：

```text
-I
-D
-U
-O2/-O3
-std=c++17
-W*（已知 ccec 可接受的形式）
```

### 5.2 可以转换为 ccec 语义

例如：

```text
--use_fast_math -> -ffast-math
-lgomp -> -lomp
--share -> -shared
-Werror=implicit-fallthrough=N -> -Werror=implicit-fallthrough
```

### 5.3 Ascend 没有等价物

例如：

```text
--ptx
--cubin
--fatbin
-rdc=true
--device-link
nvlink
ptxas
fatbinary
```

这些不能照搬 `cu-bridge` 的：

```text
-fgpu-rdc
--maca-link
```

因为这些是 MACA backend 的 device-link 机制，不是 Ascend ccec puresimt 机制。cu2cce 应该明确 reject，而不是伪造成功。

## 六、架构参数的核心差异

`cu-bridge`：

```text
CUCC_TARGETS
  -> --offload-arch=<MACA arch>
```

可以处理多 target。

cu2cce：

```text
CU2CCE_NPU_ARCH / CUCC_NPU_ARCH / CUCC_TARGETS
  -> --cce-aicore-arch=dav-*
```

当前 ccec puresimt 一次只允许一个 Ascend target：

```text
dav-c310-vec
 dav-c100-vec
```

`sm_80`、`sm_90`、`compute_80` 只能用于：

- `__CUDA_ARCH__` 兼容宏；
- trace；
- diagnostic；

不能作为 NPU compiler target。

## 七、库映射的核心差异

`cu-bridge` 可以把：

```text
-lcublas -> -lmcblas
-lcufft -> -lmcfft
-lnccl -> -lmccl
```

因为 MACA SDK 提供对应库。

cu2cce 不能根据库名直接生成：

```text
-lcublas -> 某个假想 Ascend 库
```

当前策略是：

| CUDA 库 | cu2cce 行为 |
|---|---|
| cudart/cuda | bridge runtime / Ascend runtime |
| cublas | experimental wrapper |
| nccl | HCCL route diagnostic |
| cufft/cufftw | CANN FFT route diagnostic |
| cusparse | CANN sparse route diagnostic |
| curand | Ascend random route diagnostic |
| cudnn | ATB/CANN route diagnostic |
| cusolver | CANN solver route diagnostic |
| NVML | npu-smi route diagnostic |
| NVRTC/OpenCL | unsupported/drop |

这就是“同样的兼容层机制”，但不是“同样的后端库名替换”。

## 八、CMake legacy helper 如何处理

`cu-bridge` 还提供 `gfortran`、`mpicc`、`mpicxx` 等 host carrier。cu2cce 不把它们接到 ccec device frontend，而是通过：

```text
tools/gfortran_cce
tools/mpi_cce
```

保留普通 host-only 编译，同时对 `.cu/.cuh` 直接报错并提示使用 `cucc`。这是对 `cu-bridge` 入口机制的安全等价实现，不是声称 Ascend 已支持 Fortran CUDA 或 MPI device compilation。Bazel/TF 仍需单独的项目级 wrapper 设计。

## 十、当前真实差距

当前审计基线为：

```text
Reference: cu-bridge 6ab1076 / 3.8.0
```

主要差距：

1. `FindCUDA.cmake` 还有参考实现中的少量低频 helper 未完整复刻；
2. fake toolkit CUDA library alias 只有已验证的少量集合；
3. Bazel、gfortran、MPI wrapper 尚未建立；
4. 参考 `conf.json` 的部分低频 filter/replacer 仍需要逐项确认；
5. 当前 CANN/NPU 后端不支持的能力必须继续保持 diagnostic/reject；
6. 这些差距不能通过复制 MACA 参数来“伪修复”。

当前 parity audit：

```text
/Users/tmchao7/repos/09_expLearn/cu2cce-bridge/docs/03_Comparison/PARITY_AUDIT_2026-09-01.md
```

## 十一、向 mentor 汇报时的推荐表达

可以按下面的顺序讲：

1. **先讲共同抽象：** CUDA 工程不改，构建入口和参数交给 bridge 适配；
2. **再讲 CMake：** configure 用 mock 探测，build 用真实后端，靠 fake toolkit 保持 CUDA 外形；
3. **再讲 Make/Ninja：** 通过 `CUDA_PATH/PATH` 把 `nvcc` 接到 `cucc`，host compiler 走对应 carrier；
4. **再讲 cucc：** 通过规则文件动态处理 argv，而不是为单个工程写死；
5. **再讲 NPU 差异：** ccec、`dav-*`、CANN runtime、g++ link、无 PTX/device-link；
6. **最后讲证据：** 单测、dryrun、shim tests 和服务器 `server_cmd.txt` 的真实回归结果。

一句话总结：

> 我们借鉴的是 cu-bridge 的“CUDA-shaped shim + 双阶段构建 + 动态规则驱动”机制，而不是把 MACA 的 compiler/linker/library 选项原样搬到 Ascend；后端不支持的能力通过规则和诊断显式分层。
