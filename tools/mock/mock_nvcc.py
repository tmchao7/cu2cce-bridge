#!/usr/bin/env python3
"""CMake CUDA compiler-detection shim for cucc/ccec.

CMake still speaks the nvcc protocol while cucc performs the real Ascend
puresimt compile/link. CUDA source is never rewritten. NVIDIA-only GPU
architecture and device-toolchain flags are either dropped with a diagnostic
or rejected when CU2CCE_STRICT_CUDA_FLAGS=1 is enabled.
"""
from __future__ import annotations

import os
import shlex
import subprocess
import sys
from pathlib import Path
from typing import Iterable, List


NVIDIA_VALUE_OPTIONS = {
    "-ccbin",
    "--compiler-bindir",
    "--cuda-path",
    "--generate-code",
    "-gencode",
    "-arch",
    "--gpu-architecture",
    "--gpu-code",
    "-Xfatbin",
    "--keep-dir",
    "-x",
}

DROP_FLAGS = {
    "-forward-unknown-to-host-compiler",
    "--forward-unknown-to-host-compiler",
    "--device-c",
    "--cudart",
    "--cudadevrt",
    "--keep",
    "--verbose",
    "--dryrun",
    "--threads",
    "-nocudalib",
    "-nocudainc",
    "--expt-extended-lambda",
    "--expt-relaxed-constexpr",
}

UNSUPPORTED_FLAGS = {
    "--device-link",
    "-dlink",
    "--ptx",
    "--cubin",
    "--fatbin",
    "-rdc=true",
    "--relocatable-device-code=true",
}

CUDA_RUNTIME_LIBS = {
    "-lcudadevrt",
    "-lcudart_static",
    "-lcudart",
    "-lcuda",
}


def debug(message: str) -> None:
    if os.environ.get("CU2CCE_CMAKE_DEBUG", "0") == "1":
        print(f"mock_nvcc: {message}", file=sys.stderr)


def warn(message: str) -> None:
    print(f"cu2cce: {message}", file=sys.stderr)


def unsupported(message: str) -> None:
    warn(message)
    if os.environ.get("CU2CCE_STRICT_CUDA_FLAGS", "0") == "1":
        raise RuntimeError(message)


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
#$ _TARGET_SIZE_=64
#$ _TARGET_DIR_=targets/x86_64-linux
#$ TOP={cuda_path}
#$ NVVMIR_LIBRARY_DIR={cuda_path}/nvvm/libdevice
#$ LD_LIBRARY_PATH={cuda_path}/lib64:
#$ PATH={cuda_path}/bin:{os.environ.get("PATH", "")}
#$ INCLUDES="-I{cuda_path}/include"
#$ LIBRARIES="-L{cuda_path}/lib64"
'''


def _split_forwarded(value: str) -> Iterable[str]:
    """Split -Xcompiler/-Xlinker payloads without shell-evaluating them."""
    value = value.strip().strip('"')
    if not value:
        return []
    if "," in value:
        return [item for item in value.split(",") if item]
    try:
        return shlex.split(value)
    except ValueError:
        return [value]


def _is_shim_lib_path(arg: str, cuda_path: str) -> bool:
    if not arg.startswith("-L"):
        return False
    path = arg[2:].strip('"')
    if cuda_path and path in {
        f"{cuda_path}/lib64",
        f"{cuda_path}/targets/x86_64-linux/lib",
        f"{cuda_path}/lib64/stubs",
    }:
        return True
    return "cu2cce-cmake-shim-" in path and "/cuda/" in path


def filter_args(args: List[str]) -> List[str]:
    out: List[str] = []
    cuda_path = os.environ.get("CUDA_PATH", "")
    i = 0
    while i < len(args):
        arg = args[i]

        if arg in {"-Xcompiler", "--compiler-options"}:
            if i + 1 < len(args):
                values = list(_split_forwarded(args[i + 1]))
                out.extend(values)
                debug(f"forward {arg}: {values}")
                i += 2
                continue
            warn(f"ignored {arg} without a value")
            i += 1
            continue

        if arg.startswith(("-Xcompiler=", "--compiler-options=")):
            values = list(_split_forwarded(arg.split("=", 1)[1]))
            out.extend(values)
            debug(f"forward {arg.split('=', 1)[0]}: {values}")
            i += 1
            continue

        if arg in {"-Xlinker", "-Xfatbin"} or arg.startswith(("-Xlinker=", "-Xfatbin=")):
            value = ""
            if "=" not in arg and i + 1 < len(args):
                value = args[i + 1]
                i += 1
            unsupported(f"ignored NVIDIA linker/fatbin option {arg} {value}".rstrip())
            i += 1
            continue

        if arg in UNSUPPORTED_FLAGS:
            unsupported(f"CUDA build feature {arg} is unsupported on Ascend puresimt")
            i += 1
            continue

        if arg in NVIDIA_VALUE_OPTIONS:
            if i + 1 < len(args):
                value = args[i + 1]
                if arg in {"--cuda-path", "-x"}:
                    # These are CMake protocol details, not user CUDA
                    # semantics. cucc determines the real language/path.
                    debug(f"filtered CMake protocol option {arg} {value}")
                else:
                    warn(f"ignored NVIDIA-specific option {arg} {value}")
                i += 2
            else:
                warn(f"ignored NVIDIA-specific option {arg} without a value")
                i += 1
            continue

        if arg.startswith("--cuda-path="):
            debug(f"filtered CMake protocol option {arg}")
            i += 1
            continue

        if arg.startswith((
            "-ccbin=", "--compiler-bindir=", "--cuda-path=",
            "--generate-code=", "-gencode=", "-arch=",
            "--gpu-architecture=", "--gpu-code=", "--keep-dir=",
        )):
            warn(f"ignored NVIDIA-specific option {arg}")
            i += 1
            continue

        if arg in DROP_FLAGS:
            warn(f"ignored NVIDIA-specific option {arg}")
            i += 1
            continue

        if arg in CUDA_RUNTIME_LIBS:
            # CMake adds these during its NVIDIA CUDA ABI probe. cucc injects
            # the bridge/CANN runtime itself during the real link stage.
            debug(f"filtered CUDA runtime library {arg}")
            i += 1
            continue

        if _is_shim_lib_path(arg, cuda_path):
            debug(f"filtered CUDA shim library path {arg}")
            i += 1
            continue

        if arg.startswith("-L") and "cu2cce-cmake-shim-" in arg:
            debug(f"filtered temporary CUDA shim library path {arg}")
            i += 1
            continue

        out.append(arg)
        i += 1

    return out


def run_cucc(args: List[str]) -> int:
    cucc_path = os.environ.get("CUCC_PATH")
    if not cucc_path:
        warn("CUCC_PATH is not set; cannot invoke cucc")
        return 2
    cucc = Path(cucc_path) / "bin" / "cucc"
    if not cucc.is_file():
        warn(f"cucc not found at {cucc}")
        return 2

    forwarded = filter_args(args)
    # CMake's compiler checks contain a guard that rejects a compiler which
    # does not define __CUDACC__. These identity macros are for CMake/source
    # compatibility only; ccec remains the real compiler.
    cuda_macros = [
        "-D__CUDACC__",
        "-D__CUDACC_VER_MAJOR__=11",
        "-D__CUDACC_VER_MINOR__=6",
        "-D__CUDACC_VER_BUILD__=0",
        "-D__CUDACC_VER__=110600",
    ]
    command = [str(cucc), *cuda_macros, *forwarded]
    debug("exec: " + " ".join(shlex.quote(item) for item in command))
    return subprocess.call(command)


def main() -> int:
    args = sys.argv[1:]
    try:
        if args and "__cmake_determine_cuda" in args[0]:
            print(determine_cuda(), end="")
            return 0
        if "--version" in args:
            print(version(), end="")
            return 0
        return run_cucc(args)
    except RuntimeError as exc:
        print(f"mock_nvcc: error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
