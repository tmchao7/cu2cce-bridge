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


def run_driver(args: List[str]) -> int:
    """Delegate to the shared rule driver without spawning a shell.

    The driver is loaded in-process so the user-supplied argument list is
    passed as a Python list, never through a command line evaluation.
    """
    driver = _driver_path()
    if not driver.is_file():
        print(f"mock_nvcc: rule driver not found at {driver}", file=sys.stderr)
        return 2
    sys.path.insert(0, str(driver.parent))
    from cu2cce_driver import main as driver_main  # noqa: PLC0415
    return driver_main(["--cmake-identity", *args])


def _is_query_only(args: List[str]) -> bool:
    return any(flag in args for flag in ("--version", "-V", "-help", "--help"))


def _is_preprocess_or_dependency(args: List[str]) -> bool:
    # These are compile-phase queries (no object output). Returning them to
    # ccec through the rule driver would run a real (and unnecessary) compile
    # during CMake configuration, so keep them answered by the shim.
    return any(flag in args for flag in ("-E", "-M", "-MM", "-MD", "-MMD", "--pre-process"))


def main() -> int:
    args = sys.argv[1:]
    if "__cmake_determine_cuda" in args:
        print(determine_cuda(), end="")
        return 0
    if _is_query_only(args):
        print(version(), end="")
        return 0
    if _is_preprocess_or_dependency(args):
        # Emit a syntactically complete, empty dependency file (or nothing for
        # -E) so CMake never needs a real preprocessor from the shim. Real
        # source preprocessing stays on ccec through cucc.
        out = None
        if "-MF" in args:
            index = args.index("-MF")
            if index + 1 < len(args):
                out = Path(args[index + 1])
        if out is not None:
            out.parent.mkdir(parents=True, exist_ok=True)
            out.write_text("", encoding="utf-8")
        return 0
    return run_driver(args)


if __name__ == "__main__":
    raise SystemExit(main())