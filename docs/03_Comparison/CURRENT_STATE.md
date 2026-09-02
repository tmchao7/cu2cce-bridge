# 当前状态（main / puresimt）

更新时间：2026-09-02

## 结论

详细项目原理文档：[`docs/02_Architecture/01_global_overview.md`](../02_Architecture/01_global_overview.md)；文档总导航见 [`docs/README.md`](../README.md)。

main 已锁定 puresimt：用户写原始 CUDA `.cu`，ccec 通过
`--enable-simt --cce-aicore-arch=dav-c310-vec` 直接编译，host API 通过
`cuda* → wcuda* → aclrt*` bridge 运行。main 不使用 Python kernel converter，不生成
新同构包装代码。

## 真机基线

`tools/real_test/server_cmd.txt` 是唯一验收入口，当前包含 puresimt、cucc 直通、
CMake/Make/Ninja 和产物契约检查，并通过 `CU2CCE_REGRESSION_PROFILE` 支持三档画像：`fast`（默认，
核心子集：官方两样例 + event/multiKernel/atomic/math minimal + CMake 两代表
案例）、`external`（bridge 重编 + 全部外部 NVIDIA 样本，含诊断样本）、
`full`（历史全量）。跳过段输出 `[SKIP]` 与汇总 `*_SECTION_SKIPPED`，不计
FAIL 也不冒充 PASS；验收标志在 full 或 fast 执行段内语义不变。验收标志：

```text
FAIL=0 DEVICE_HEALTHY=1 TEST_BLOCKED_BY_DEVICE=0
PURESIMT_REGRESSION_PASS CMAKE_CCE_REGRESSION_PASS
BUILD_COMPATIBILITY_REGRESSION_PASS CUCC_RULES_REGRESSION_PASS
LARGE_PROJECT_REGRESSION_PASS CUDA_SAMPLES_CMAKE_REGRESSION_PASS
ARTIFACT_CONTRACT_REGRESSION_PASS
```

发布前最新服务器 full profile 由维护者报告 `FAIL=0`、`DEVICE_HEALTHY=1`、
`TEST_BLOCKED_BY_DEVICE=0`；本机只做静态/mock 验证。四层命令与产物说明见：
`docs/03_Comparison/RELEASE_READINESS_2026-09-02.md`。并发/多版本 CMake 构建应显式
设置 `CU2CCE_CMAKE_SHIM_DIR` 或 `CU2CCE_SHIM_ISOLATE=1`。

完整逐用例和 API 状态见：
[PURESIMT_COMPATIBILITY_MATRIX.md](PURESIMT_COMPATIBILITY_MATRIX.md)。

## 代码边界

- 正式：`include/bridge/runtime`、`include/bridge/kernel/simt_adaptor.h`、
  `src/bridge/runtime`、`bin/cucc`、`examples/puresimt`。
- experimental：`src/bridge/blas`、`include/bridge/blas`、`include/cublas_v2.h`。
- 参考：`docs/reference/asc-devkit-simt`。
- 历史：`exp/new-homogeneous-v1` 与 Git history。

## 重要事实

- 目标：Ascend 950 / CANN 9.x。
- 编译器：CANN ccec。
- 必要架构参数：`dav-c310-vec`。
- `__shared__` 在 main SIMT 适配中映射为 `__ubuf__`。
- 同步、warp、原子原语通过 `simt_adaptor.h` 做符号适配。
- 不允许将 `__simt_vf__`、`cce::async_invoke` 或 hidden block 参数写入用户样例。
