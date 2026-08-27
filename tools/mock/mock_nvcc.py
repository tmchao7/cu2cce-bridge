#!/usr/bin/env python3
"""CMake nvcc protocol shim shared with the normal cucc rule driver."""
from __future__ import annotations

import os
import sys
from pathlib import Path
from typing import List


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


def _driver_path() -> Path:
    explicit = os.environ.get("CU2CCE_DRIVER")
    if explicit:
        return Path(explicit)
    cucc_path = os.environ.get("CUCC_PATH", "")
    if cucc_path:
        candidate = Path(cucc_path) / "tools" / "cu2cce_driver.py"
        if candidate.is_file():
            return candidate
    return Path(__file__).resolve().parents[1] / "cu2cce_driver.py"


def _splice_cudaflags(args: List[str], env: "os._Environ|dict|None" = None) -> List[str]:
    """Prepend CUDAFLAGS from the environment, nvcc-style.

    CUDAFLAGS is parsed with the standard-library shlex and never evaluated,
    so no environment-controlled text can be executed. Returns a new list."""
    environ = os.environ if env is None else env
    cudaflags = environ.get("CUDAFLAGS", "")
    if not cudaflags or not cudaflags.strip():
        return args
    import shlex  # noqa: PLC0415
    return [*shlex.split(cudaflags), *args]


def run_driver(args: List[str]) -> int:
    """Delegate to the shared rule driver without spawning a shell.

    The driver is loaded in-process so the user-supplied argument list is
    passed as a Python list, never through a command line evaluation.
    """
    args = _splice_cudaflags(args)
    driver = _driver_path()
    if not driver.is_file():
        print(f"mock_nvcc: rule driver not found at {driver}", file=sys.stderr)
        return 2
    sys.path.insert(0, str(driver.parent))
    from cu2cce_driver import main as driver_main  # noqa: PLC0415
    return driver_main(["--cmake-identity", *args])


def main() -> int:
    args = sys.argv[1:]
    if "__cmake_determine_cuda" in args:
        print(determine_cuda(), end="")
        return 0
    if "--version" in args:
        print(version(), end="")
        return 0
    # Everything else is a real compile/link/preprocess invocation. In
    # particular -MD/-MF/-MT are CMake's dependency-generation flags attached
    # to every genuine CUDA object rule: they must reach the rule driver and
    # ccec, never be swallowed by the shim.
    return run_driver(args)


if __name__ == "__main__":
    raise SystemExit(main())