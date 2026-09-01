#!/usr/bin/env python3
"""CMake nvcc protocol shim shared with the normal cucc rule driver."""
from __future__ import annotations

import os
import platform
import sys
from pathlib import Path
from typing import List


def _cuda_version_parts() -> tuple[int, int, int]:
    """Return the configured CUDA-compat release for CMake probes."""
    raw = os.environ.get("CUCC_CUDA_VERSION", "11060")
    try:
        value = int(raw)
        major = value // 1000
        minor = (value // 10) % 100
        build = value % 10
        if major < 0 or minor < 0 or build < 0:
            raise ValueError
        return major, minor, build
    except ValueError:
        return 11, 6, 0


def version() -> str:
    major, minor, build = _cuda_version_parts()
    release = f"{major}.{minor}.{build}"
    return (
        "nvcc: NVIDIA (R) Cuda compiler driver\n"
        f"Cuda compilation tools, release {major}.{minor}, V{release}\n"
    )


def _host_triple() -> str:
    configured = os.environ.get("CU2CCE_HOST_TRIPLE", "").strip()
    if configured:
        return configured
    arch = platform.machine().lower()
    arch = {"amd64": "x86_64", "arm64": "aarch64"}.get(arch, arch)
    system = platform.system().lower() or "linux"
    return f"{arch}-{system}"


def determine_cuda() -> str:
    cuda_path = os.environ.get("CUDA_PATH", "/tmp/cu2cce-cuda-shim")
    target_dir = f"targets/{_host_triple()}"
    return f'''#$ _NVVM_BRANCH_=nvvm
#$ _SPACE_=
#$ _CUDART_=cudart
#$ _HERE_={cuda_path}/bin
#$ _THERE_={cuda_path}/bin
#$ _TARGET_SIZE_=64
#$ _TARGET_DIR_={target_dir}
#$ TOP={cuda_path}
#$ NVVMIR_LIBRARY_DIR={cuda_path}/nvvm/libdevice
#$ LD_LIBRARY_PATH={cuda_path}/lib64:
#$ PATH={cuda_path}/bin:{os.environ.get("PATH", "")}
#$ INCLUDES="-I{cuda_path}/include"
#$ LIBRARIES="-L{cuda_path}/lib64"
'''


def _simple_preprocess_response() -> str:
    """Answer CMake >=3.24's --simple-preprocess CUDA-version probe.

    CMake runs `nvcc --simple-preprocess -x cu ...` and greps for the CUDA
    version string. The shim replies with a fragment CMake parses into the
    configured CUDA-compat version; nothing is compiled."""
    major, minor, build = _cuda_version_parts()
    return (
        f"#define __CUDACC_VER_MAJOR__ {major}\n"
        f"#define __CUDACC_VER_MINOR__ {minor}\n"
        f"#define __CUDACC_VER_BUILD__ {build}\n"
    )


def _is_abi_probe(args: List[str]) -> bool:
    return any("CMakeCUDACompilerABI.cu" in arg or "CMakeCompilerABI.cpp" in arg
               for arg in args)


def _output_arg(args: List[str]) -> str:
    """Return the -o value from the invocation, or an empty string."""
    for i, arg in enumerate(args[:-1]):
        if arg in ("-o", "-MF", "-MT"):
            return args[i + 1]
    return ""


def _abi_source_path() -> Path:
    """Shipped ABI stand-in source, compiled by the host compiler at probe
    time (cu-bridge tools/mock parity — no prebuilt platform artifacts)."""
    return Path(__file__).resolve().parent / "CMakeCUDACompilerABI.cpp"


def _handle_abi_probe(args: List[str], cwd: Path) -> int:
    """Answer CMake's CMakeCUDACompilerABI.cu probe without reaching ccec.

    Preferred path (cu-bridge deal_abi parity): compile the shipped
    CMakeCUDACompilerABI.cpp with the host compiler so the probe binary
    embeds the real INFO:... ABI strings and CMake's binary scan succeeds;
    the compiled program reports "no CUDA devices" so arch autodetection
    fails cleanly. When the stand-in cannot be compiled (no host compiler,
    missing file), fall back to the explicit placeholder object — no fake
    success, matching the no-no-op policy."""
    out = _output_arg(args)
    if not out:
        return 0
    out_path = Path(out)
    if not out_path.is_absolute():
        out_path = cwd / out_path
    out_path.parent.mkdir(parents=True, exist_ok=True)

    source = _abi_source_path()
    if source.is_file():
        import subprocess  # noqa: PLC0415
        use_object_mode = "-c" in args
        try:
            if use_object_mode:
                result = subprocess.run(
                    ["g++", "-c", str(source), "-o", str(out_path)],
                    capture_output=True, text=True, timeout=60, shell=False)
            else:
                result = subprocess.run(
                    ["g++", str(source), "-o", str(out_path)],
                    capture_output=True, text=True, timeout=60, shell=False)
        except (OSError, subprocess.SubprocessError):
            result = None
        if result is not None and result.returncode == 0 and out_path.is_file():
            print("mock_nvcc: ABI probe compiled from shipped "
                  "CMakeCUDACompilerABI.cpp (host g++; no ccec reach)",
                  file=sys.stderr)
            return 0

    out_path.write_bytes(b"\x7fELFcu2cce-abi-placeholder")
    return 0


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
    if "--simple-preprocess" in args:
        print(_simple_preprocess_response(), end="")
        return 0
    if _is_abi_probe(args):
        return _handle_abi_probe(args, Path.cwd())
    # Everything else is a real compile/link/preprocess invocation. In
    # particular -MD/-MF/-MT are CMake's dependency-generation flags attached
    # to every genuine CUDA object rule: they must reach the rule driver and
    # ccec, never be swallowed by the shim.
    return run_driver(args)


if __name__ == "__main__":
    raise SystemExit(main())