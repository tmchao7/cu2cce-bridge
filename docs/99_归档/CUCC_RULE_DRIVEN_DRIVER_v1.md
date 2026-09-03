# cucc 规则驱动编译器适配器

## 1. 目标

`cucc` 的职责是把 CUDA/nvcc 风格的命令行适配为 CANN `ccec` 命令。它只处理：

- 参数展开；
- 编译/链接阶段识别；
- CUDA 参数的保留、丢弃、替换、注入和拒绝；
- CANN/bridge 依赖注入；
- 诊断和 trace。

`cucc` 不读取、解析或改写 CUDA 源文件，也不生成 `.cce`、`__simt_vf__` 或 kernel
包装代码。实际 kernel 编译仍然是：

```text
原始 CUDA .cu → ccec -x asc --enable-simt → Ascend SIMT
```

## 2. 调用链

普通命令：

```text
bin/cucc
  → tools/cu2cce_driver.py
  → ccec
```

CMake 命令：

```text
CMake
  → tools/mock/nvcc_entry.sh
  → tools/mock/mock_nvcc.py
  → tools/cu2cce_driver.py
  → ccec
```

普通 `cucc` 和 CMake `mock_nvcc` 共享同一个参数驱动，避免 configure 阶段和真正构建
阶段使用不同的过滤逻辑。

## 3. 规则文件

默认规则文件：

```text
config/cu2cce_rules.json
```

安装后位置：

```text
$CUCC_PATH/config/cu2cce_rules.json
```

规则文件可以通过环境变量覆盖：

```bash
export CU2CCE_RULES_FILE=/path/to/custom/cu2cce_rules.json
```

优先级为：

```text
CU2CCE_RULES_FILE
  > $CUCC_PATH/config/cu2cce_rules.json
  > 仓库内 config/cu2cce_rules.json
```

规则按以下阶段组织：

```text
all
compile
link
compile_and_link
```

每个阶段可以包含：

```text
forward   直接转发
 drop     丢弃并给出原因
replace   转换为目标参数
inject    追加 cu2cce/ccec 参数
reject    明确拒绝并返回非零
```

## 4. CUDA 参数策略

### 保留

常见编译器参数直接传递：

```text
-I  -D  -U  -O0/-O1/-O2/-O3  -g  -fPIC
-std=  -M*  -W*  -L  -l
```

### 丢弃

没有 Ascend 对应语义、但不影响当前 puresimt 编译的 NVIDIA 参数会警告后丢弃：

```text
-ccbin
--compiler-bindir
--cuda-path
-gencode
--generate-code
-arch=sm_*
--gpu-architecture
--gpu-code
--ptxas-options
-lcudart
-lcudart_static
-lcudadevrt
```

NVIDIA GPU 架构不会被转换成伪造的 Ascend 架构。Ascend 架构只由以下变量控制：

```bash
export CU2CCE_NPU_ARCH=dav-c310-vec
```

### 转换

第一版只保留必要的通用参数转换，例如：

```text
-Xcompiler -fPIC
  → -fPIC
```

不会复制 cu-bridge 面向 MACA 的库名、架构宏或 offload 参数映射。

### 拒绝

无法保持 puresimt 语义的功能直接失败：

```text
--ptx
--cubin
--fatbin
--device-link
-dlink
-rdc=true
--relocatable-device-code=true
nvrtc
nvlink
ptxas
fatbinary
```

### Shared library / PIC

当命令包含 `-shared` 且用户没有提供 `-fPIC` 或 `-Xcompiler -fPIC` 时，驱动会自动
注入 `-fPIC`。ccec puresimt 的 kernel launch TLS 状态和 host registration helper
不能以 non-PIC relocation 生成共享对象；这属于构建参数修复，不是 kernel 源码转换。

### 未验证 CUDA 库

以下库不自动映射到 CANN：

```text
-lcublas
-lcublasLt
-lcufft
-lcusparse
-lcurand
-lcudnn
```

默认行为是警告并保留参数；严格模式下直接失败：

```bash
export CU2CCE_STRICT_FLAGS=1
```

`cublas/aclblas` 仍然属于独立的 experimental secondary capability。

## 5. 阶段识别

驱动统一区分：

```text
compile
link
compile_and_link
```

典型规则：

```text
-c                         → compile
.o 输入                    → link
main.cu -o app             → compile_and_link
main.cu -o app.o           → compile
```

响应文件支持：

```text
@options.rsp
--options-file options.rsp
-options-file options.rsp
```

响应文件使用安全的 shell 风格参数解析，不通过逐行空格切分。

## 6. 诊断

普通模式：

```bash
cucc -c main.cu -o build/main.o
```

调试模式：

```bash
CU2CCE_DEBUG=1 cucc -c main.cu -o build/main.o
```

调试输出包括：

```text
[cu2cce] phase=compile
[cu2cce] drop: -gencode: ...
[cu2cce] replace: -Xcompiler: ...
[cu2cce] inject: --enable-simt
[cu2cce] final command: ... ccec ...
```

保存规则决策：

```bash
CU2CCE_TRACE_FILE=build/cu2cce_trace.json \
cucc -c main.cu -o build/main.o
```

trace 只记录参数决策、阶段和最终命令，不记录 CUDA 源码内容。

兼容旧入口：

```text
WCUDA_DEBUG=1
```

会等价开启 `CU2CCE_DEBUG=1`。

## 7. 大型项目构建入口

- `cmake_cce`：配置现代或 legacy CUDA CMake 工程；
- `make_cce`：为使用 `nvcc` 的 Makefile 建立临时 Toolkit shim；
- `ninja_cce`：执行已经由 `cmake_cce -G Ninja` 配置的 Ninja 构建目录。

这些入口共享 `cu2cce_driver.py` 的参数政策。

## 8. 设计边界

本机制复制的是 cu-bridge 的：

```text
filter/replacer/adder
分阶段规则
配置驱动
参数诊断
```

不复制 cu-bridge 的：

```text
MACA 库映射
MACA 架构宏
MACA offload 参数
MACA 伪 CUDA Toolkit 语义
```

目标后端特有的参数必须先有 CANN/Ascend 证据，再进入规则文件。
