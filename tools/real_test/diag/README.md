# tools/real_test/diag —— 107002 排查留下的真机诊断工具

> 2026-08-19 107002（cudaMalloc 首调失败）排查的 17 轮实验沉淀。
> **结论已定案**（见 历史 runtime context 排障结论：根因是
> `aclrtMalloc` 调用点在 `BRIDGE_INIT` 之前，修复为 wcudaMalloc 开头显式
> `BRIDGE_INIT()`）。本目录工具保留为"可真机复用的排障资产"，不再属于
> 常用回归流程；`server_cmd.txt`/`build_bridge.sh` 等常态脚本仍在
> `tools/real_test/` 根目录。

## 工具清单

| 文件 | 用途 |
|---|---|
| `acl_alloc_probe.cpp` / `run_acl_probe.sh` | 裸 ACL 分配探针：不经 wrapper，直调 aclInit→GetDeviceCount→SetDevice→aclrtMalloc，隔离"ACL vs 桥"。自动找 CANN 头/库，-lascendcl 失败自动切 -lacl_rt |
| `acl_compose_probe.cpp` / `run_acl_compose_probe.sh` | 库组合探针：同一段分配代码 4 种链接（base/+runtime/+wrapper/+both），定位"多带哪个库"导致异常 |
| `acl_diff_probe.cpp` / `run_acl_diff_probe.sh` | 差异定位探针：逐行复刻 minimal 序列并逐步加动作（HBM 查询/device 查询/连续分配/NORMAL policy） |
| `trunc_raw.c` / `run_trunc_raw.sh` | 不链接 wrapper 的裸 ACL 流程对照（逐步打印） |
| `run_iterate_malloc.sh` | 同一进程反复 malloc/free，看失败是否与次数相关 |
| `diag/echo.cce` / `run_minimal_echo.sh` | 无 kernel 的最小 host 子集（cudaStreamCreate+cudaMalloc） |

## 用法提示

- **只在安装 CANN 和 Ascend 设备的 Linux 环境中执行**。
- 脚本内部用绝对路径/CANN 自动探测，仓库内任意目录 `bash tools/real_test/diag/xxx.sh` 可跑。
- 排查新的 host 侧桥问题时可复用；若要用探针改参数，改完保持文件头注释里的
  "判读" 说明同步更新。
- 这些源码属于非主线排障资产；运行产生的二进制被 `.gitignore` 忽略。