# 能力清单

本目录是当前支持边界的事实入口。矩阵中的状态只对明确的源码形态、API 形状和构建场景负责，
不能从“某个样例通过”推断整个 CUDA 生态都兼容。

## 入口文档

| 文档 | 用途 |
|---|---|
| `SUPPORTED_SCOPE.md` | 一页查看当前正面能力清单 |
| `LIMITS_AND_BOUNDARIES.md` | 明确不支持、后端限制和已知问题 |
| `CUDA_OPERATOR_API_SUPPORT_MATRIX.md` | CUDA Kernel 写法、算子载体、Runtime API 和库映射总表 |
| `PURESIMT_COMPATIBILITY_MATRIX.md` | puresimt 语言、头文件和运行时逐项矩阵 |
| `BLAS_COMPATIBILITY_MATRIX.md` | `cublas`/`aclblas` 实验线能力 |
| `CUCC_OPTION_COMPATIBILITY_MATRIX.md` | `cucc` 参数、query、规则动作和 NPU 架构策略 |
| `BUILD_SYSTEM_COMPATIBILITY_MATRIX.md` | CMake/Make/Ninja/pre、构建元数据和产物契约 |

## 如何读状态

- `SUPPORTED`：已经纳入主线承诺，并有自动化或真机证据。
- `EXPERIMENTAL`：可以试用，但只对文档列出的形状负责。
- `DIAGNOSTIC_ONLY`：专门记录 Ascend 后端差异，不计入主线阻塞失败。
- `UNSUPPORTED`：不实现、明确拒绝，或没有可信的 Ascend 等价物。

## 当前重点边界

当前正式编译链是 `ccec -x asc --enable-simt --cce-aicore-arch=dav-*`。因此 `sm_*` 只能用于
兼容宏、trace 或诊断，不能直接传给 `ccec`；PTX/CUBIN/FATBIN、RDC/device-link、device-side
double 和未验证 CUDA 库不能通过伪造 alias 变成“支持”。

构建产物对齐关注类别和依赖：`.o`、`.d`、`.a`、`.so`、ELF executable、`NEEDED`、RPATH/RUNPATH
以及 CMake/Make/Ninja 元数据。Ascend 产物不要求与沐曦 MACA 产物字节级一致。
