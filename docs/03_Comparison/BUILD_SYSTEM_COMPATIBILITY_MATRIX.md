# CUDA Build System Compatibility Matrix

本矩阵面向大型 CUDA 项目迁移，和 kernel/operator compatibility matrix 分开维护。
状态必须以测试证据为准。

| 能力 | 状态 | 测试/证据 | 限制 |
|---|---|---|---|
| modern CMake CUDA project | `SUPPORTED` for validated core | `examples/cmake_demo` 真机通过 | 大型项目高级 CUDA property 仍需扩展 |
| multiple `.cu` CMake target | `SUPPORTED` for validated core | `examples/cmake_compat` | 复杂 device symbol resolution 未覆盖 |
| static/shared CUDA library | `EXPERIMENTAL` | `examples/cmake_compat` | 依赖 ccec/CANN 链接语义 |
| object/interface library | `EXPERIMENTAL` | build compatibility suite | 需要更多真机组合 |
| host C++ + CUDA | `SUPPORTED` for validated core | `examples/cmake_compat` | 其他 host toolchain 未覆盖 |
| `find_package(CUDAToolkit)` | `EXPERIMENTAL` | minimal module | 不是完整 NVIDIA Toolkit discovery |
| `CUDA::cudart` | `EXPERIMENTAL` | bridge-backed runtime target | 不代表完整 CUDA Runtime 等价 |
| legacy `FindCUDA` | `EXPERIMENTAL` | `examples/cmake_compat_legacy` | 仅基础宏 |
| install/export/package | `EXPERIMENTAL` | native `cu2cce::runtime` package | 需安装树真机回归 |
| Unix Makefiles | `EXPERIMENTAL` | `examples/build_compat/make` | 通过 `make_cce` |
| Ninja | `EXPERIMENTAL` | CMake Ninja smoke + `ninja_cce` | 需服务器安装 Ninja |
| response file | `EXPERIMENTAL` | `tests/test_cucc_rules.py` | 构建系统组合仍需扩大 |
| dependency generation `-M/-MM/-MD/-MF/-MT` | `DIAGNOSTIC_ONLY` until device proof | driver phase tests | ccec 真实依赖输出需真机确认 |
| query `--version/-V/--help` | `SUPPORTED` for driver | local unit tests | 输出是 cu2cce compatibility identity |
| preprocess `-E` | `EXPERIMENTAL` | driver phase tests | ccec 真实预处理输出需真机确认 |
| CUDA `-gencode` / `-arch` | `DIAGNOSTIC_ONLY` | rule trace | 不映射 fake Ascend architecture |
| PTX/CUBIN/FATBIN | `UNSUPPORTED` | explicit rejection | puresimt 不消费 NVIDIA binary artifacts |
| device-link/RDC | `UNSUPPORTED` for current scope | explicit rejection | 后续需单独验证 ccec 能力 |
| `CUDA::cublas` | `EXPERIMENTAL` secondary | separate BLAS regression | 不属于默认构建兼容主线 |
| `CUDA::cufft/cusparse/curand/cudnn` | `UNSUPPORTED` | no validated target | 不创建虚假 target |

## 维护规则

新增构建场景必须同步：

1. 测试工程；
2. `tools/real_test/server_cmd.txt`；
3. 本矩阵；
4. `README.md`/`QUICKSTART.md` 中的使用边界。
