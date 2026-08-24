# tools/real_test —— 真机验证脚本（昇腾服务器 + CANN 9.x）

> 本目录脚本**只在装有 CANN 的昇腾真机上执行**（Linux）。本机（macOS）无 CANN，除 `env_probe.sh`
> 可安全试跑（会优雅地报"未找到 CANN"）外，其余脚本勿在本机执行。

## 分工

| 脚本 | 分工 | 是否执行命令 |
|---|---|---|
| `env_probe.sh` | **只找包的位置**：定位 CANN/bisheng/ACL 头库/simt_api/npu-smi；grep 头文件只读核对 `docs/03_Comparison/CURRENT_STATE.md` 里的待真机确认项；**核对目标架构 `--npu-arch=dav-c310`**（内部 950 系列：dump `__NPU_ARCH__` 宏、比对 simt_api 门控、最小 SIMT kernel 实测编译） | 仅只读查询 + 最小文件实测编译（产物在 /tmp，不触碰仓库构建） |
| `build_bridge.sh` | **真实构建**：cmake 配置 → 编译 → 安装 wrapper 库到 CUCC_PATH | 执行 cmake/make/install |
| `server_cmd.txt` | **main / puresimt 唯一回归入口**：原始 NVIDIA `vectorAdd` + `matrixMul` | 执行 ccec 编译、链接、运行二进制 |
| `run_tests.sh` | 兼容性分步测试：host 冒烟 + NVIDIA vectorAdd/matrixMul，可选 cucc/direct | 执行 cucc/bisheng 编译、链接、运行二进制 |
| `run_all.sh` | 旧的探测 → 构建 → `run_tests.sh` 串联入口 | 以上全部 |

## 用法

```bash
# 推荐：main / puresimt 官方 NVIDIA 样例回归
tools/real_test/server_cmd.txt

# 或分步（排障时）
tools/real_test/env_probe.sh        # 先看环境缺什么
tools/real_test/build_bridge.sh     # 构建，装到 build/real_test/install
tools/real_test/run_tests.sh all    # 跑测试

# 排查驱动问题时，测试阶段绕过 cucc 直接调 bisheng 作对照
tools/real_test/run_tests.sh all --direct
```

产出（默认 `build/real_test/`，已在 .gitignore 内）：

- `probe.env` —— 探测结果（供后续脚本 source）
- `logs/probe.log` —— 探测明细（含 npu-smi info、驱动版本、逐项签名核对）
- `install/` —— bridge 安装目录（即 CUCC_PATH），含 `lib/libcce_bridge_runtime.so`、`include/`、`bin/cucc`
- `logs/*.log` —— 每个编译/链接/运行步骤的日志

## 通过标准

- `env_probe.sh`：退出码 0（硬缺失=0；签名未命中只 WARN，需人工比对 CURRENT_STATE.md 修正代码）。**第 7 节架构核对**：`--npu-arch=dav-c310` 通过最小 SIMT kernel（asc_shfl+asc_syncthreads）编译、且 `__NPU_ARCH__` 宏在 simt_api 门控内 —— 这两条是路线 A 成立的前提，若 WARN 需把 probe.log 第 7 节输出反馈给开发侧
- `build_bridge.sh`：`libcce_bridge_runtime.so`、`include/cuda_runtime.h`、`bin/cucc` 等 6 项产物齐备
- `server_cmd.txt`：输出 `PURESIMT_REGRESSION_PASS`，且官方 vectorAdd 含 `Test PASSED`、matrixMul 含 `Result = PASS`
- `run_tests.sh host`：输出含 `host bridge smoke OK`
- `run_tests.sh vectoradd`：输出含 `Test PASSED`
- `run_tests.sh matrixmul`：输出含 `Result = PASS`

## 已知限制（脚本按仓库当前状态适配）

0. **换行符 / 执行方式（真机教训）**：脚本若经非 git 方式（压缩包/网盘/粘贴/Windows 拷贝）传到服务器，可能被转成 CRLF（Windows git autocrlf、编辑器保存、传输工具都会干这事；**与 SSH 终端无关，换 xshell/PowerShell 都一样**），表现为 `syntax error near unexpected token $'do\r'` 或 `cannot execute: required file not found`（shebang 带 \r，无扩展名文件如 bin/cucc 最易中招）。修复：`sed -i 's/\r$//' <文件>` 或 git 仓库里 `git checkout -- <文件>`。**根治：服务器上只通过 `git pull` 获取代码**（`.gitattributes` 已强制全部脚本 LF，含无扩展名的 bin/cucc）。**执行用 `bash tools/real_test/xxx.sh` 或 `./xxx.sh`，不要用 `source`**（脚本末尾 `exit` 会关闭交互 shell）。
1. **两步法编译**：`bin/cucc` 目前只在命令行出现 `*.o` 时才追加链接库，故统一 `cucc -c x.cu -o x.o` → `cucc x.o -o x`。
2. **CMakeLists 直接引用 .so 文件名**：`libascendcl.so`/`libacl_cblas.so` 若在真机不存在（如只有 `libacl_rt.so`），`build_bridge.sh` 会在预检处提示，需按真机改 `CMakeLists.txt` 第 48-49 行。
3. **SIMT 头来源**：`env_probe.sh` 自动找 `simt_api` 目录；若 kernel 用到 `__syncthreads/__shared__` 等宏而真机未装 asc-devkit，需先补装。
4. **待真机确认的 7 项**（见 `docs/03_Comparison/CURRENT_STATE.md` §5）：probe 脚本已把能 grep 的部分自动核对，其余（pitch 对齐、原子操作地址空间、null stream 语义）需看 vectorAdd/扩展用例的运行结果。
5. **dav-c310 是内部 950 系列架构参数**（导师 2026-08-19 提供；此前误记 dav-310，真机 bisheng 实测不接受，已全部改正），见 `docs/01_Design/ASCEND_RESEARCH_NOTES.md` §3.5。
