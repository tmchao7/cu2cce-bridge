# main 工作区约定

## 正式方向

main 只推进 puresimt：原始 CUDA `.cu` 直接交给 ccec；runtime 通过 CUDA-compatible
headers 和 ACL wrapper 兼容。旧新同构/文本 lowering 不在 main。

## 开发循环

1. 阅读/修改 CUDA-compatible headers、runtime bridge 或 SIMT alias。
2. 保持 examples 为普通 CUDA，不写 ACL/Ascend C。
3. 本机执行 `git diff --check`、`bash -n` 和 mock syntax check。
4. 真机操作统一写 `tools/real_test/server_cmd.txt`。
5. push `gitcode/main`，服务器 `git pull --ff-only gitcode main` 后执行回归。

## 目录

- 正式代码：`include/bridge/runtime`、`include/bridge/kernel`、`src/bridge/runtime`、`bin/cucc`。
- experimental：`src/bridge/blas`。
- 样例：`examples/puresimt`。
- 参考资料：`docs/reference/asc-devkit-simt`。
- 排障：`tools/real_test/diag`。
- 历史路线：`exp/new-homogeneous-v1`。
