#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
mock_nvcc —— 无 CUDA Toolkit 时回答 CMake 的 CUDA 编译器探测。

仿照 cu-bridge 的 tools/mock/mock_nvcc.py（Apache-2.0），目标从 mxcc 换成 cucc（bisheng）。

CMake 在探测 CUDA 编译器时会调用 `nvcc` 数次：
  1. `nvcc __cmake_determine_cuda`  → 打印 CUDA 环境信息（CMake 解析 #$ 标记）
  2. `nvcc --version`                → 打印版本（CMake 提取 CUDA 版本）
  3. 编译 `CMakeCUDACompilerId.cu`   → 生成 a.out 证明编译器可用
其余参数（真实编译 .cu）透传给 cucc → bisheng。

用法：作为 nvcc 的 mock 入口（由 cmake_cce 建立软链，或 PATH 前置）。
"""
import os
import re
import sys
import subprocess


def mock_version(cuda_version: str) -> str:
    """伪造 nvcc --version 输出，让 CMake 认为存在 CUDA 工具链。"""
    major, minor = cuda_version[:2], cuda_version[2:4]
    # CUDA 版本号格式为 11.6（不是 11.06）；minor 个位数时不补零
    return f"""nvcc: NVIDIA (R) Cuda compiler driver
Copyright (c) 2005-2022 NVIDIA Corporation
Built on {major}.{int(minor)}.r{major}.{int(minor)}/compiler.30794723_0
Cuda compilation tools, release {major}.{int(minor)}, V{major}.{int(minor)}.55
"""


def mock_determine_cuda() -> str:
    """伪造 __cmake_determine_cuda 探测输出（CMake 依赖 #$ 标记解析路径）。"""
    cuda_path = os.environ.get("CUDA_PATH", "/usr/local/cuda")
    nvcc_dir = os.path.join(cuda_path, "bin")
    return f"""#$ _NVVM_BRANCH_=nvvm
#$ _SPACE_=
#$ _CUDART_=cudart
#$ _HERE_={nvcc_dir}
#$ _THERE_={nvcc_dir}
#$ _TARGET_SIZE_=
#$ _TARGET_DIR_=
#$ _TARGET_DIR_=targets/x86_64-linux
#$ TOP={cuda_path}
#$ NVVMIR_LIBRARY_DIR={cuda_path}/nvvm/libdevice
#$ LD_LIBRARY_PATH={cuda_path}/lib64:
#$ PATH={cuda_path}/bin:{os.environ.get('PATH','')}
#$ INCLUDES="-I{cuda_path}/include"
#$ LIBRARIES=  "-L{cuda_path}/lib64"
#$ CUDAFE_FLAGS=
#$ PTXAS_FLAGS=
nvcc fatal   : Don't know what to do with '__cmake_determine_cuda'
"""


def mock_compiler_id() -> str:
    """伪造 CMakeCUDACompilerId 编译的详细 trace（CMake 只关心能走到链接出 a.out）。
    真实做法：直接调用 cucc 编译 CMakeCUDACompilerId.cu，产出 a.out。"""
    # 让真实编译器处理：bisheng 能编译这个探测文件
    # （CMake 的 CUDACompilerId.cu 是空 main，bisheng 原生可编译）
    return None


def main():
    args = sys.argv[1:]
    cuda_version = os.environ.get("CUCC_CUDA_VERSION", "11060")

    # 1) CMake 探测：__cmake_determine_cuda
    if len(args) >= 1 and "__cmake_determine_cuda" in args[0]:
        sys.stdout.write(mock_determine_cuda())
        return 0

    # 2) CMake 探测：--version
    if any(a == "--version" for a in args):
        sys.stdout.write(mock_version(cuda_version))
        return 0

    # 3) CMake 探测：编译 CMakeCUDACompilerId.cu / CMakeCUDACompilerABI
    if any("CompilerId" in a or "CMakeCUDACompilerABI" in a for a in args):
        # 找到源 .cu 文件，用真实 cucc 编译（bisheng 能编空 main）
        src = next((a for a in args if a.endswith(".cu") or a.endswith(".cpp")), None)
        out = None
        for i, a in enumerate(args):
            if a in ("-o", "--output", "-c") and i + 1 < len(args):
                out = args[i + 1]
                break
        if src:
            cmd = ["cucc", src]
            if out:
                cmd += ["-o", out]
            # cucc 在 PATH 中（cmake_cce 已把伪 CUDA_DIR/bin 前置）
            r = subprocess.run(cmd)
            return r.returncode

    # 4) 其余：透传给真实 cucc
    cmd = ["cucc"] + args
    r = subprocess.run(cmd)
    return r.returncode


if __name__ == "__main__":
    sys.exit(main())
