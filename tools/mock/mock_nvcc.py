#!/usr/bin/env python3
"""CMake CUDA compiler-detection shim for cucc/ccec.

CMake still speaks the nvcc protocol while cucc performs the real Ascend
puresimt compile/link. NVIDIA-only flags are discarded; ordinary compiler and
linker flags are forwarded to cucc.
"""
import os
import subprocess
import sys
from pathlib import Path


def version() -> str:
    return """nvcc: NVIDIA (R) Cuda compiler driver
Cuda compilation tools, release 11.6, V11.6.0
"""


def determine_cuda() -> str:
    cuda_path = os.environ.get("CUDA_PATH", "/tmp/cu2cce-cuda-shim")
    return f'''#$ _NVVM_BRANCH_=nvvm
#$ _SPACE_=
#$ _CUDART_=cudart
#$ _HERE_={cuda_path}/bin
#$ _THERE_={cuda_path}/bin
#$ TOP={cuda_path}
#$ NVVMIR_LIBRARY_DIR={cuda_path}/nvvm/libdevice
#$ LD_LIBRARY_PATH={cuda_path}/lib64:
#$ PATH={cuda_path}/bin:{os.environ.get("PATH", "")}
#$ INCLUDES="-I{cuda_path}/include"
#$ LIBRARIES="-L{cuda_path}/lib64"
'''


def is_value_option(arg: str) -> bool:
    return arg in {
        "-ccbin", "--compiler-bindir", "-Xcompiler", "-Xlinker",
        "-Xfatbin", "--compiler-options", "--keep-dir", "--cuda-path",
        "--generate-code", "-gencode", "-x",
    }


def filter_args(args):
    out = []
    skip = False
    for arg in args:
        if skip:
            skip = False
            continue
        if is_value_option(arg):
            skip = True
            continue
        if arg.startswith(("-ccbin=", "--compiler-bindir=", "--cuda-path=")):
            continue
        if arg in {
            "-forward-unknown-to-host-compiler", "--forward-unknown-to-host-compiler",
            "--expt-extended-lambda", "--expt-relaxed-constexpr", "--device-c",
            "--device-link", "-dlink", "--cudart", "--cudadevrt",
            "--keep", "--verbose", "--dryrun", "--threads",
            "-nocudalib", "-nocudainc", "--generate-code", "-gencode",
        }:
            continue
        if arg.startswith(("-gencode=", "--generate-code=", "--compiler-options=",
                           "-Xcompiler=", "-Xlinker=", "-Xfatbin=")):
            continue
        if arg.startswith("--generate-code") or arg.startswith("-gencode"):
            continue
        out.append(arg)
    return out


def run_cucc(args, compiler_id=False):
    cucc = Path(os.environ["CUCC_PATH"]) / "bin" / "cucc"
    forwarded = filter_args(args)
    # CMake's CUDA ABI/compiler checks contain a guard that rejects any compiler
    # which does not define __CUDACC__. The real backend is ccec, but the shim
    # must preserve these source-level CUDA compiler identity macros for CMake
    # and for normal CUDA CMake targets.
    cuda_macros = [
        "-D__CUDACC__",
        "-D__CUDACC_VER_MAJOR__=11",
        "-D__CUDACC_VER_MINOR__=6",
        "-D__CUDACC_VER_BUILD__=0",
        "-D__CUDACC_VER__=110600",
    ]
    forwarded = cuda_macros + forwarded
    return subprocess.call([str(cucc), *forwarded])


def main() -> int:
    args = sys.argv[1:]
    if args and "__cmake_determine_cuda" in args[0]:
        print(determine_cuda(), end="")
        return 0
    if "--version" in args:
        print(version(), end="")
        return 0
    compiler_id = any("CompilerId" in arg or "CMakeCUDACompilerABI" in arg
                      for arg in args)
    return run_cucc(args, compiler_id=compiler_id)


if __name__ == "__main__":
    raise SystemExit(main())
