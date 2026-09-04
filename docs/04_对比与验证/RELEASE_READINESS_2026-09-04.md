# cu2cce-bridge 最终验证记录（2026-09-04）

## 结论

`main / puresimt` 已完成本轮上线前的本地门禁、代码审查、简化收口和 Ascend 真机
`full` profile 验证。运行时验证以本轮 header/wrapper parity 收口后的主线提交为基线；
本文件仅沉淀证据，**不创建 release，也不创建 release tag**。

## Ascend 真机 full profile 证据

维护者在 Ascend 服务器执行：

```bash
bash tools/real_test/clean.sh
CU2CCE_REGRESSION_PROFILE=full bash tools/real_test/server_cmd.txt
```

最终摘要为：

```text
FAIL=0
DEVICE_HEALTHY=1
TEST_BLOCKED_BY_DEVICE=0
PROFILE=full

PURESIMT_REGRESSION_PASS
CMAKE_CCE_REGRESSION_PASS
BUILD_COMPATIBILITY_REGRESSION_PASS
CUDA_SAMPLES_CMAKE_REGRESSION_PASS
CUCC_RULES_REGRESSION_PASS
LARGE_PROJECT_REGRESSION_PASS
ASCEND_API_MAPPING_REGRESSION_PASS
CUDA_API_PARITY_REGRESSION_PASS
CUDA_HEADER_WRAPPER_PARITY_REGRESSION_PASS
```

`full` profile 同时执行主线产物契约检查；详细日志位于：

```text
build/real_test/puresimt/
build/real_test/puresimt/artifacts/
```

## 本地发布门禁

本地 macOS 不代替真机运行，但已完成可重复的静态/契约检查：

```text
cu2cce-rules: OK
mock ACL syntax: PASS=5 FAIL=0
Python unittest: Ran 304 tests ... OK
Runtime API parity: missing_entries=0
CUDA header/wrapper parity: missing_headers=0
CUDA header/wrapper parity: missing_wrapper_entries=0
CUDA header/wrapper parity: missing_api_entries=0
git diff --check: PASS
```

header/wrapper parity 还校验：

```text
reference_header_entries=144
reference_wrapper_entries=66
reference_api_entries=825
active_maca_backend_names=0
```

## 本轮收口内容

- 修复 header probe 在 `set -u` 下先使用后初始化 `work` 临时目录的问题；
- 修复 parity generator 将 reserved 注释中的 reference prototype 误识别为 active wrapper 的问题；
- 增加 generator 幂等性测试，重复生成不会改变 inventory 或 reserved header；
- 合并 header probe 中重复的 library/interop include 诊断逻辑；
- 补齐 CUDA 公共头、Wrapper 与 API inventory 的维护入口；
- 明确区分 `ACTIVE_ADAPTED`、`UNSUPPORTED_TYPED`、`RESERVED_NPU` 和 `DIAGNOSTIC_ONLY`；
- 不伪造 MACA runtime、CUDA Driver ABI、PTX/CUBIN/FATBIN/RDC 或未验证 CUDA 库链接。

## 发布边界

本次通过说明当前主线回归和已登记的兼容边界可交付；它不意味着所有 CUDA API、所有 CUDA
库、所有设备架构或所有并发/多卡语义已经等价。新增能力仍必须遵循：

```text
CANN 官方头/库证据
→ CUDA 形状的类型与 wrapper
→ 本地契约测试
→ Ascend 真机编译/链接/运行验证
→ 再升级能力状态
```
