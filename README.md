# cu2cce-bridge

CUDA→昇腾 NPU（CANN）的兼容层，参考 `cu-bridge` 的“用 CUDA 一门语言跑多硬件”
理念，目标是让原始 NVIDIA CUDA `.cu` 文件不改源码即可在 Ascend 950 / CANN 上运行。

## 当前正式路线：main / puresimt

Ascend 950 的 AI Core 支持 SIMT。main 不再走 Python kernel 转换或新同构包装路线，
而是让 CANN ccec 直接解析 CUDA SIMT 语法：

```text
原始 CUDA .cu
  → ccec -x asc --enable-simt --cce-aicore-arch=dav-c310-vec
  → CUDA runtime compatibility headers
  → cuda* → wcuda* → aclrt*
  → Ascend NPU
```

main 不生成 `__simt_vf__`、`__global__[aicore]`、`cce::async_invoke` 包装源文件。
`include/bridge/kernel/simt_adaptor.h` 只提供必要的 SIMT 原语别名。

## 当前进度（2026-08-24）

- main 已切换为 puresimt 直通：`cucc` 不再调用 Python kernel converter，不生成
  legacy `.cce`/kernel wrapper；直通检查已在真机通过。
- 当前已验证通过 23 个 CUDA/runtime/SIMT 用例，覆盖算术、逻辑、比较、转换、
  shared/global memory、原子、归约、warp、vote、scan、transpose、stencil、event、
  pitched copy、multi-kernel pipeline、3D indexing、动态 shared memory、fence、
  FP16/half2 与 BF16/BF16x2。
- 当前唯一验收标志仍是 `PURESIMT_REGRESSION_PASS`；BLAS bridge 保留为 experimental
  secondary capability，不阻塞 puresimt。

## 真机回归

唯一入口：

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git pull --ff-only gitcode main
bash tools/real_test/server_cmd.txt
```

当前回归包括：

- NVIDIA 官方 `vectorAdd`
- NVIDIA 官方 shared-memory `matrixMul`
- 最小 vector add
- 最小 tiled matrix multiply
- `atomicAdd`/`atomicMin`/`atomicMax`
- shared-memory tree reduction + cross-block atomic add
- FP16/half2 and BF16/BF16x2 conversion round-trips

成功标志：

```text
FAIL=0
DEVICE_HEALTHY=1
PURESIMT_REGRESSION_PASS
```

## 目录

- `include/`：CUDA runtime、CUDA BLAS 兼容头、SIMT 原语别名和 shadow headers。
- `src/bridge/runtime/`：`cuda* → wcuda* → aclrt*`。
- `src/bridge/blas/`：experimental cublas→aclblas，独立于 puresimt 验收。
- `bin/cucc`：nvcc 兼容驱动；`.cu` 直接传给 ccec，不做 Python lowering。
- `examples/puresimt/`：维护中的 CUDA examples。
- `tools/real_test/server_cmd.txt`：唯一主线真机回归入口。
- `tools/real_test/diag/`：非主线 ACL/runtime 排障工具。
- `docs/reference/asc-devkit-simt/`：Ascend SIMT API 参考资料。

## 本地限制

本机 macOS 没有 CANN/NPU，不能本地构建或运行 Ascend 产物。允许执行静态检查：

```bash
bash -n bin/cucc tools/real_test/server_cmd.txt tools/real_test/diag/*.sh
git diff --check
bash tests/mock_acl/syntax_check.sh
```

## 历史路线

旧的新同构/文本 lowering 实验位于 `exp/new-homogeneous-v1` 分支；main 不再维护该
路线。BLAS bridge 仍保留，但属于 experimental secondary capability。
