# cu2cce-bridge

把 CUDA 程序迁移到**华为昇腾 NPU（CANN / Ascend C）**的桥接层 —— 仿照 [cu-bridge](https://gitee.com/metax-maca/cu-bridge)（沐曦 MXMACA，Apache-2.0）的"用 CUDA 一门语言跑多硬件"理念。

**目标**：GPU 程序员只用 CUDA 编程语言写一个应用程序，就能生成跑在昇腾 NPU 上的目标文件（编译 / 链接 / 运行时透明替换）。

**目标硬件（D0，2026-08-07 定稿）**：昇腾 950 系列（Ascend 950PR/950DT），bisheng `--npu-arch=dav-c310`。950 系列 AI Core 原生支持 SIMT，kernel 可直通编译（路线 A）。

## 状态

🚧 **开发中（Phase A1-A4 骨架完成，真机验证待做）**：

| Phase | 内容 | 状态 |
|---|---|---|
| A1 | host Runtime API 桥（宏表 + wcuda*→aclrt*，126 个错误码 + 40+ 函数） | ✅ 骨架完成 |
| A2 | cucc 驱动 + make_cce/cmake_cce/mock 工程胶水 | ✅ 骨架完成 |
| A3 | SIMT 适配宏表 + kernel 直通（`__syncthreads→asc_syncthreads` 等） | ✅ 宏表完成，`cudaLaunchKernel` 显式 API 待真机 |
| A4 | cublas→aclblas 最小子集（Sgemm/GemmEx） | ✅ 骨架完成 |

- `include/`：CUDA 兼容头链（`cuda_runtime.h → __cuda_runtime.h → 宏表 → wcuda* 声明`）+ `cublas_v2.h`
- `src/bridge/`：wcuda*→aclrt* 实现 + wcublas*→aclblas 实现
- `bin/cucc` + `tools/`：nvcc 兼容驱动 + make_cce/cmake_cce/mock
- `tests/mock_acl/`：mock ACL 头 + 本机语法验证脚本（`bash tests/mock_acl/syntax_check.sh`，3/3 通过）

⚠️ 本机（macOS）无昇腾硬件/无 CANN，**代码仅供阅读与移植参考，不可本地构建**。需在装有 CANN 的昇腾真机上编译（见 `docs/01_Design/CUDA_TO_CCE_PORTING_PLAN.md`）。真机验证清单见 `docs/03_Comparison/mapping.md`。

## 文档

| 文档 | 内容 |
|---|---|
| [技术方案](docs/01_Design/CUDA_TO_CCE_PORTING_PLAN.md) | 迁移路线 A/B、阶段规划、待拍板决策（D0-D5） |
| [昇腾调研笔记](docs/01_Design/ASCEND_RESEARCH_NOTES.md) | ACL/SIMT/bisheng 事实库（API 签名、来源链接） |
| [Claude 工作区方案](docs/01_Design/CLAUDE_WORKSPACE_PLAN.md) | 目录结构、.claude 配置、对照开发工作流 |
| [对照表](docs/03_Comparison/mapping.md) | 与 cu-bridge 的逐文件移植状态 |

## 环境变量（真机）

```bash
export ASCEND_TOOLKIT_HOME=/usr/local/Ascend/ascend-toolkit/latest
source ${ASCEND_TOOLKIT_HOME}/bin/setenv.bash
export CUCC_PATH=<cu2cce-bridge 安装目录>
export CUDA_PATH=${CUCC_PATH}
```

## License

Apache-2.0（与 cu-bridge 相同）。
