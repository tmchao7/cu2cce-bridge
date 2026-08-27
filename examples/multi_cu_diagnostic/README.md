# multi_cu_diagnostic —— 多 .cu 静态库边界诊断载体
#
# 背景：ccec puresimt 每个 .cu translation unit 生成同名 eager-registration
# 辅助符号（BishengLaunchKernelWithHostArgs / __cce_rtLinkedDevBinary*），
# 两个 .cu 放进同一个静态库再由 host 链接时可能 duplicate symbol。
# 本载体用 2 个 .cu + 1 个 static library 复现，用来记录归因，不进 required
# profile。CMake 强制 CUDA_SEPARABLE_COMPILATION OFF（与 external cuda-samples
# harness 一致）。
#
# 期望：
#   - 若后端已支持 TU 聚合：构建 + 运行出 "multi_cu diagnostic OK ..."；
#   - 若仍 duplicate：记录 DIAGNOSTIC_ONLY，不阻塞 required PASS。
# 判据在 tools/real_test/server_cmd.txt 的 multi_cu_diagnostic 段。