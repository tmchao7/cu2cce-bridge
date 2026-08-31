# CLAUDE.md

## 项目定位

cu2cce-bridge 是 CUDA→昇腾 NPU 的兼容层，目标是让用户保留原始 `.cu` 源码，
在 Ascend 950 / CANN 9.x 上通过 CUDA 兼容头、host runtime bridge 与 ccec SIMT
前端直接编译运行。

本机是 macOS，无 CANN/NPU；本机只做阅读、静态检查和 mock syntax check，不构建
或运行 Ascend 产物。真机命令统一写入 `tools/real_test/server_cmd.txt`。

## main 正式路线：puresimt

- 用户源文件保持普通 CUDA：`__global__`、`threadIdx`、`blockIdx`、`__shared__`、
  `__syncthreads()`、原子操作和 `<<<>>>`。
- ccec 使用：`-x asc --enable-simt --cce-aicore-arch=dav-c310-vec`。
- `.cu` 直接交给 ccec；**main 不运行 Python kernel converter，也不生成中间包装源文件**。
- `include/bridge/kernel/simt_adaptor.h` 只做 CUDA SIMT 原语别名：
  `__shared__→__ubuf__`、`__syncthreads→asc_syncthreads`、CUDA atomics/warp
  collectives→Ascend SIMT API；不改写 kernel 入口、launch 或参数。
- host 侧通过 `cuda* → wcuda* → aclrt*` bridge 工作。

旧的 `__simt_vf__`/`cce::async_invoke` 包装路线和文本 lowering 已退出 main；历史
实现保留在 `exp/new-homogeneous-v1` 分支与 Git 历史中。

## 关键目录

- `include/`：CUDA 兼容入口、runtime/BLAS wrapper 头、SIMT 原语适配头、shadow 头。
- `src/bridge/runtime/`：CUDA Runtime→ACL Runtime 实现。
- `src/bridge/blas/`：独立的 experimental cublas→aclblas 能力，不阻塞 puresimt 验收。
- `bin/cucc`：参数解析、头文件/库/架构注入，然后直接调用 ccec。
- `examples/puresimt/`：官方 NVIDIA Samples + 最小 CUDA operator 用例。
- `third_party/nvidia_cuda_samples/`：官方 helper 头，只作为 include 依赖。
- `tools/real_test/server_cmd.txt`：main 唯一真机回归入口。
- `tools/real_test/diag/`：非主线 ACL/runtime 排障资产。
- `docs/reference/asc-devkit-simt/`：Ascend SIMT API 参考资料，不是转换模板。

## 工作纪律

1. 不在 macOS 构建/运行 CANN。
2. 真机命令只写 `tools/real_test/server_cmd.txt`，不要新建轮次命令文件。
3. 只 push `gitcode`，不要 push GitHub `origin`。
4. 代码修改后执行 `git diff --check`、`bash -n`；能运行时再执行
   `bash tests/mock_acl/syntax_check.sh`。
5. main 的回归标准是 `PURESIMT_REGRESSION_PASS`；BLAS experimental 失败不改变
   puresimt 主线结论。

## 真机主线命令

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git pull --ff-only gitcode main
bash tools/real_test/server_cmd.txt
```

## 交付与真机回归纪律

1. 每次对仓库做出实际代码、测试或文档修改后，必须先完成本地可行验证，再创建 git commit。
2. 提交完成后必须推送到 `gitcode` 远端；默认主线为 `gitcode/main`，不要只停留在本地或只推 `origin`。
3. 每次对话交付时，必须明确告诉用户服务器执行步骤，至少包括：切换到目标分支、从 `gitcode` 更新/拉取、执行 `tools/real_test/clean.sh`、最后执行 `tools/real_test/server_cmd.txt`。
4. 服务器命令应使用当前实际提交对应的分支和提交号；如不是 `main`，不得把 `main` 命令写成默认回归命令。
5. 服务器回归命令模板：

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git switch <目标分支>
git pull --ff-only gitcode <目标分支>
bash tools/real_test/clean.sh
bash tools/real_test/server_cmd.txt
```

6. 如果本地无法运行 CANN/NPU 回归，必须明确说明服务器仍需执行上述命令，不能把本地 mock/syntax/test 结果冒充真机通过。
