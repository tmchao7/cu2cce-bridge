# cu2cce-bridge 与 cu-bridge 构建层对齐审查

## 结论

当前 `cu2cce-bridge` 没有走回旧的 kernel converter/新同构路线，核心设计仍然正确：

```text
原始 CUDA .cu
  → ccec --enable-simt
  → Ascend SIMT
```

本项目吸收的是 cu-bridge 的构建适配机制，而不是复制 MACA 的目标平台映射。

## 已对齐的机制

| cu-bridge 机制 | cu2cce-bridge 实现 |
|---|---|
| 参数 filter/replacer/adder | `bin/conf.json` 的 forward/drop/replace/inject/reject |
| 编译阶段区分 | `compile` / `compile_and_link` / `link` / `preprocess` / `dependency` |
| nvcc mock | `tools/mock/mock_nvcc.py` |
| CUDA Toolkit 虚拟目录 | `cmake_cce`/`make_cce` shim |
| CMake 包装入口 | `cmake_cce` |
| Make 包装入口 | `make_cce` |
| Ninja 构建入口 | `ninja_cce` |
| 参数调试 | `CU2CCE_DEBUG`、`CU2CCE_TRACE_FILE` |
| 配置校验 | `tools/validate_cucc_rules.py` |

## 有意不复制的内容

以下内容属于 cu-bridge 的 MACA 目标后端，不能直接搬到 CANN：

```text
sm_70/sm_75/sm_80 → MACA 架构宏
-lcudart → -lmcruntime
-lcublas → -lmcblas
-lcufft → -lmcfft
-fgpu-rdc / --maca-link
MACA 专用 CMake compiler modules
```

cu2cce 使用：

```text
CU2CCE_NPU_ARCH
--cce-aicore-arch
--enable-simt
libcce_bridge_runtime.so
CANN runtime
```

未验证的 CUDA library 不会被创建假 target 或自动映射。

## CMake 现状

已通过真机验证：

- modern CUDA CMake；
- 多 `.cu` 项目核心路径；
- static/shared CUDA library 基础组合；
- host C++ + CUDA；
- `CUDA::cudart` bridge target；
- legacy `FindCUDA` 基础宏；
- Unix Makefiles；
- Ninja；
- install/export smoke。

仍属于实验性或未支持：

```text
完整 FindCUDAToolkit
CUDA::cublas/cufft/cusparse/curand/cudnn
CUDA device-link / RDC / separable compilation
PTX/CUBIN/FATBIN
多个 .cu 直接聚合到同一静态 archive 的 ccec registration
```

特别注意：当前 ccec puresimt 每个 `.cu` translation unit 会生成相同名称的 runtime
registration helper。因此多 `.cu` 静态 archive 不是普通 linker flag 问题，不能用
`--allow-multiple-definition` 掩盖。现阶段测试将每个 linked static/shared library
限制为一个 `.cu`，后续若要支持多 `.cu` 聚合，需要单独研究 ccec 的 device registration
聚合机制。

## cucc 现状

当前 `cucc` 已经从硬编码 Shell 转为：

```text
bin/cucc
  → tools/cu2cce_driver.py
  → bin/conf.json
  → ccec
```

CMake mock 与普通 cucc 共享同一驱动，避免两套参数规则漂移。

已覆盖：

```text
response file
query
preprocess/dependency phase detection
compile/link/compile_and_link
-gencode/-arch 诊断
-Xcompiler 转换
-shared 自动注入 -fPIC
PTX/CUBIN/FATBIN/device-link 拒绝
CUDA::cudart runtime 注入
```

仍需继续扩展：

- CMake/CUDA 专用属性的完整语义；
- 复杂 nvcc 选项的参数 arity 和组合形式；
- `-M/-MM/-MD/-MF/-MT` 在真实 ccec 上的依赖文件证据；
- CMake compiler query 与真实 nvcc 输出的更多兼容细节；
- 真实大型 CUDA 项目的命令快照和端到端试点。

## 审查结论

当前没有发现路线性偏差。主要边界是“覆盖面不足”，不是“架构走歪”：

```text
正确：复制 cu-bridge 的适配机制
正确：保留 puresimt 直通
正确：不复制 MACA 具体映射
正确：未验证能力显式标记
需要继续：扩大大型工程构建测试和 CANN 后端语义证据
```
