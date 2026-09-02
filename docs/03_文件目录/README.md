# 文件目录与问题定位

本目录面向维护者。遇到问题时先按“用户入口 → 规则/环境 → 兼容头 → runtime → 构建 shim →
测试取证”的调用链定位，不要先修改样例绕过错误。

## 快速定位

| 现象 | 首先检查 |
|---|---|
| `cucc` 参数被错误转发 | `tools/cu2cce_driver.py`、`config/cu2cce_rules.json`、`02_能力清单/CUCC_OPTION_COMPATIBILITY_MATRIX.md` |
| `.cu` 编译失败 | `include/cuda_runtime.h`、`include/bridge/kernel/simt_adaptor.h`、`tools/device_arch.sh` |
| CMake configure 失败 | `tools/cmake_cce`、`tools/mock/mock_nvcc.py`、`cmake_modules/override/CUDAOverrides.cmake` |
| CMake build 找不到 `nvcc`/`cucc` | `tools/fake_toolkit_env.sh`、`tools/build_shim_tree.py`、`tools/ninja_cce` |
| Make/Ninja 参数或 clean 异常 | `tools/make_cce`、`tools/ninja_cce`、`tools/pre_cce` |
| 运行时 ACL 错误 | `src/bridge/runtime/`、`include/bridge/runtime/`、真机日志 |
| `.a/.so/ELF/.d` 产物不对 | `config/artifact_contract.json`、`tools/real_test/verify_artifacts.py`、构建矩阵 |
| 文档路径或 tracked 文件异常 | 本目录的 `REPOSITORY_FILE_GUIDE.md`、`tests/test_repo_hygiene.py` |

## 维护顺序

1. 先在能力矩阵中确认该行为是否属于当前承诺。
2. 用最小样例或现有测试复现，并保存实际命令和产物。
3. 修改最靠近问题的层：规则问题改 JSON，环境问题改 wrapper，语义问题改 bridge/runtime。
4. 更新对应文档、测试和文件职责表，再执行仓库门禁。

完整的逐文件职责、调用链、对标关系和排障入口见
[`REPOSITORY_FILE_GUIDE.md`](REPOSITORY_FILE_GUIDE.md)。
