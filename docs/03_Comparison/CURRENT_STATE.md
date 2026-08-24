# 当前状态（main / puresimt）

更新时间：2026-08-24

## 结论

main 已锁定 puresimt：用户写原始 CUDA `.cu`，ccec 通过
`--enable-simt --cce-aicore-arch=dav-c310-vec` 直接编译，host API 通过
`cuda* → wcuda* → aclrt*` bridge 运行。main 不使用 Python kernel converter，不生成
新同构包装代码。

## 真机基线

`tools/real_test/server_cmd.txt` 是唯一验收入口，当前包含 23 个真机用例和 cucc
直通检查。验收标志：

```text
FAIL=0 DEVICE_HEALTHY=1 PURESIMT_REGRESSION_PASS
```

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
