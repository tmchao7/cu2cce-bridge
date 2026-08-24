# tools/real_test

## main 主线

`server_cmd.txt` 是 main / puresimt 唯一回归入口。它在 Ascend 真机上执行：

1. 设备健康门禁；
2. CANN 环境与 bridge 重建；
3. 官方 NVIDIA CUDA Samples 与最小 CUDA operator 编译、链接、运行；
4. `PURESIMT_REGRESSION_PASS` 汇总。

```bash
bash tools/real_test/server_cmd.txt
```

## 支持脚本

- `env_probe.sh`：探测 CANN/ccec/设备/头库，生成真机环境信息。
- `build_bridge.sh`：构建并安装 `libcce_bridge_runtime.so`。
- `server_cmd.txt`：正式验收入口。
- `diag/`：107002、ACL 分配和 runtime 组合排障资产，不属于默认回归。

main 只维护 `server_cmd.txt` 主线入口；converter-only 测试已经删除。
