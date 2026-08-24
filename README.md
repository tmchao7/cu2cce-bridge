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

## 兼容性矩阵

完整的 CUDA→Ascend puresimt 证据矩阵、23 个真机用例、API 状态和 BLAS 边界见：
[PURESIMT_COMPATIBILITY_MATRIX.md](docs/03_Comparison/PURESIMT_COMPATIBILITY_MATRIX.md)。

## Quickstart：运行你自己的 `.cu`

下面示例假设你在 Ascend 真机上工作。建议把自己的工程放在仓库外，避免和
`examples/puresimt/` 的回归样例混在一起，例如：

```text
/home/t50063365/workspace/repos/cu2cce-demo/
├── vector_add.cu
└── build/
```

### 1. 准备源码

`vector_add.cu` 保持普通 CUDA 写法，不需要添加 ACL、Ascend C、`__simt_vf__`
或 `cce::async_invoke`：

```cpp
#include <cuda_runtime.h>
#include <stdio.h>

#define N 1024

__global__ void vectorAdd(const float *a, const float *b, float *c, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) c[i] = a[i] + b[i];
}

int main(void) {
    float hA[N], hB[N], hC[N];
    for (int i = 0; i < N; ++i) {
        hA[i] = (float)i;
        hB[i] = 2.0f;
    }

    float *dA = nullptr, *dB = nullptr, *dC = nullptr;
    cudaMalloc((void **)&dA, sizeof(hA));
    cudaMalloc((void **)&dB, sizeof(hB));
    cudaMalloc((void **)&dC, sizeof(hC));
    cudaMemcpy(dA, hA, sizeof(hA), cudaMemcpyHostToDevice);
    cudaMemcpy(dB, hB, sizeof(hB), cudaMemcpyHostToDevice);

    vectorAdd<<<(N + 255) / 256, 256>>>(dA, dB, dC, N);
    if (cudaGetLastError() != cudaSuccess) return 1;
    if (cudaDeviceSynchronize() != cudaSuccess) return 1;
    if (cudaMemcpy(hC, dC, sizeof(hC), cudaMemcpyDeviceToHost) != cudaSuccess) return 1;

    for (int i = 0; i < N; ++i) {
        if (hC[i] != (float)i + 2.0f) return 1;
    }
    printf("user vectorAdd OK (N=%d)\n", N);
    cudaFree(dA); cudaFree(dB); cudaFree(dC);
    cudaDeviceReset();
    return 0;
}
```

### 2. 拉取、准备 CANN 和构建 bridge

```bash
cd /home/t50063365/workspace/repos

git clone git@gitcode.com:tmchao7/cu2cce-bridge.git
cd cu2cce-bridge
git checkout main

export ASCEND_TOOLKIT_HOME=/usr/local/Ascend/ascend-toolkit/latest
export ASCEND_HOME_PATH=/usr/local/Ascend/latest
source /usr/local/Ascend/latest/set_env.sh
export ASCEND_OPP_PATH=/usr/local/Ascend/cann-9.2.0/opp

bash tools/real_test/env_probe.sh
bash tools/real_test/build_bridge.sh
```

如果仓库已经存在，只需：

```bash
cd /home/t50063365/workspace/repos/cu2cce-bridge
git pull --ff-only gitcode main
source /usr/local/Ascend/latest/set_env.sh
bash tools/real_test/build_bridge.sh
```

### 3. 用 `cucc` 编译和链接

推荐使用两步法：

```bash
export CUCC_PATH=/home/t50063365/workspace/repos/cu2cce-bridge/build/real_test/install
export CUDA_PATH=$CUCC_PATH
export PATH=$CUCC_PATH/bin:$PATH

mkdir -p /home/t50063365/workspace/repos/cu2cce-demo/build

cucc -c /home/t50063365/workspace/repos/cu2cce-demo/vector_add.cu \
    -o /home/t50063365/workspace/repos/cu2cce-demo/build/vector_add.o

cucc /home/t50063365/workspace/repos/cu2cce-demo/build/vector_add.o \
    -o /home/t50063365/workspace/repos/cu2cce-demo/build/vector_add
```

`cucc` 会自动注入 CUDA 兼容头、CANN SIMT 头、运行库和 rpath；`.cu` 会直接
交给 ccec，不会生成 Python converter 或 Ascend kernel 包装源文件。

### 4. 运行

```bash
/home/t50063365/workspace/repos/cu2cce-demo/build/vector_add
```

预期输出：

```text
user vectorAdd OK (N=1024)
```

### 5. 用户代码的边界

- 可以继续使用 CUDA `.cu`、CUDA runtime API、CUDA kernel 语法和 `<<<>>>`。
- 不要在用户源码中直接调用 `acl*`、书写 `__simt_vf__`、`__global__[aicore]`
  或 `cce::async_invoke`。
- 修改 bridge 的 `src/` 或 `include/` 后，需要重新执行 `build_bridge.sh`。
- `tools/real_test/clean.sh` 只清理仓库的 `build/real_test`，不会删除仓库外的
  `cu2cce-demo`。

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
