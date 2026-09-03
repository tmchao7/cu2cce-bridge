#!/usr/bin/env python3
"""Build the virtual CUDA toolkit directory for the Ascend CANN backend.

The public location and command shape follow cu-bridge's ``virtualDir.py``.
The implementation is intentionally Ascend-aware: the virtual directory only
provides CUDA-shaped entry points while the real compiler remains Bisheng
``ccec`` and the real runtime remains ``libcce_bridge_runtime.so`` plus CANN.
``tools/link.json`` is the Make/pre manifest and ``tools/mock/stub_link.json``
is the CMake configure manifest. The builder intentionally accepts only these
canonical manifests so layout ownership stays unambiguous.

Layout contract (normalized from the manifests):
  - dirs: directories created under the CUDA dir (${HOST_TRIPLE} resolved)
  - dir_links / file_links: symbolic links (parent dirs auto-created)
  - hostcc_names: host compiler entries created in --hostcc-bin-dir, bound
    either to the hostcc_cce wrapper (--hostcc-mode wrapper) or to identity
    /usr/bin links (--hostcc-mode identity); missing system compilers are
    skipped
  - alias_dirs/cuda_lib_aliases/alias_target: CUDA-named runtime aliases
    (libcudart.so, libcuda.so.1, ...) pointing at the bridge runtime
  - files: small generated text files (version banner, CMake config shim)

The builder never touches user sources and never follows an existing shim
destructively: links are replaced atomically (symlink to temp name + rename).
"""
from __future__ import annotations

import argparse
import json
import os
import platform
import sys
from pathlib import Path
from typing import Dict, List, Optional


def host_triple() -> str:
    arch = platform.machine().lower()
    arch = {"amd64": "x86_64", "arm64": "aarch64"}.get(arch, arch)
    system = platform.system().lower() or "linux"
    return f"{arch}-{system}"


def _substitute(value: str, variables: Dict[str, str]) -> str:
    for key, resolved in variables.items():
        value = value.replace("${" + key + "}", resolved)
    return value


def cuda_version_display() -> str:
    """Format CUCC_CUDA_VERSION for the toolkit-shaped version.txt."""
    raw = os.environ.get("CUCC_CUDA_VERSION", "11060")
    try:
        value = int(raw)
        major = value // 1000
        minor = (value // 10) % 100
        build = value % 10
        if major < 0 or minor < 0 or build < 0:
            raise ValueError
        return f"{major}.{minor}.{build}"
    except ValueError:
        return "11.6.0"


def _atomic_symlink(link: Path, target: str) -> None:
    if link.is_symlink():
        if os.readlink(link) == target:
            return
        link.unlink()
    elif link.exists():
        # A previous interrupted/older shim build may have left a real
        # directory where the manifest now requires a link (for example
        # targets/<triple>/include).  This path is inside the generated fake
        # toolkit, so remove the stale directory instead of calling
        # Path.unlink() and failing with IsADirectoryError.
        if link.is_dir():
            import shutil  # noqa: PLC0415
            shutil.rmtree(link)
        else:
            link.unlink()
    link.parent.mkdir(parents=True, exist_ok=True)
    # Random suffix + os.symlink (which never clobbers) + rename: an attacker
    # cannot pre-place the temporary link, and a collision just retries.
    for _ in range(8):
        tmp = link.with_name(f"{link.name}.tmp{os.urandom(6).hex()}")
        try:
            os.symlink(target, tmp)
        except FileExistsError:  # pragma: no cover - collision retry
            continue
        os.rename(tmp, link)
        return
    raise OSError(f"cannot create temporary symlink next to {link}")


def _merge_layout(base: Dict[str, object], overrides: Dict[str, object]) -> Dict[str, object]:
    """Merge manifest metadata without duplicating the layout implementation."""
    merged = dict(base)
    for key, value in overrides.items():
        if key in {"comment", "schema_version", "backend", "compiler", "device_arch", "runtime"}:
            continue
        merged[key] = value
    return merged


def load_layout(layout_path: Path, _seen: Optional[set] = None) -> Dict[str, object]:
    """Normalize old and cu-bridge-shaped manifests to the builder layout.

    ``tools/link.json`` stores the common layout under ``pre_make``. The CMake
    ``stub_link.json`` points at that file through ``virtual_cmake.base_manifest``
    so the two manifests do not drift into two implementations.
    """
    path = layout_path.expanduser().resolve()
    seen = set() if _seen is None else set(_seen)
    if path in seen:
        raise ValueError(f"manifest inheritance cycle: {path}")
    seen.add(path)
    data = json.loads(path.read_text(encoding="utf-8"))
    if isinstance(data.get("pre_make"), dict):
        return dict(data["pre_make"])
    virtual_cmake = data.get("virtual_cmake")
    if isinstance(virtual_cmake, dict):
        base_name = virtual_cmake.get("base_manifest")
        if base_name:
            base_path = (path.parent / str(base_name)).resolve()
            base = load_layout(base_path, seen)
            overrides = virtual_cmake.get("overrides", {})
            return _merge_layout(base, overrides if isinstance(overrides, dict) else {})
        layout = virtual_cmake.get("layout")
        if isinstance(layout, dict):
            return dict(layout)
    # Canonical manifests are link.json (pre_make) and stub_link.json
    # (virtual_cmake); no legacy centralized layout is accepted here.
    return data


def build_virtual_toolkit(
    cuda_dir: Path,
    cucc_path: Path,
    nvcc_target: Path,
    layout_path: Path,
    hostcc_bin_dir: Optional[Path] = None,
    hostcc_mode: str = "wrapper",
    cann_base: str = "",
) -> List[str]:
    # Resolve all CLI paths before writing symlinks. Otherwise a relative
    # target such as `install/bin/cucc` is interpreted relative to the link
    # location inside the generated toolkit and becomes dangling.
    cuda_dir = cuda_dir.expanduser().resolve()
    cucc_path = cucc_path.expanduser().resolve()
    nvcc_target = nvcc_target.expanduser().resolve()
    layout_path = layout_path.expanduser().resolve()
    if hostcc_bin_dir is not None:
        hostcc_bin_dir = hostcc_bin_dir.expanduser().resolve()
    try:
        layout = load_layout(layout_path)
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        raise ValueError(f"cannot load virtual toolkit manifest {layout_path}: {exc}") from exc
    variables = {
        "CUDA_DIR": str(cuda_dir),
        "CUCC_PATH": str(cucc_path),
        "CANN_BASE": cann_base,
        "HOST_TRIPLE": host_triple(),
        "NVCC_TARGET": str(nvcc_target),
        "CUCC_CUDA_VERSION_DISPLAY": cuda_version_display(),
    }

    notes: List[str] = []
    for entry in layout.get("dirs", []):
        (cuda_dir / _substitute(entry, variables)).mkdir(parents=True, exist_ok=True)

    for entry in layout.get("dir_links", []):
        link = cuda_dir / _substitute(entry["link"], variables)
        target = _substitute(entry["target"], variables)
        # All manifest-owned links use the same atomic replacement path;
        # the old replace/non-replace split duplicated identical cleanup.
        _atomic_symlink(link, target)
        notes.append(f"dir link {link} -> {target}")

    # cu-bridge make_maca parity: a whole-tree include link can silently fail
    # across filesystem boundaries, so verify and fall back to per-file links.
    include_link = cuda_dir / "include"
    if layout.get("per_file_include_fallback", True) and include_link.is_symlink():
        if not (include_link / "cuda_runtime.h").exists():
            include_link.unlink()
            include_link.mkdir(parents=True)
            for item in sorted(Path(_substitute("${CUCC_PATH}", variables), "include").iterdir()):
                _atomic_symlink(include_link / item.name, str(item))
            notes.append("include fallback: per-file links")

    for entry in layout.get("file_links", []):
        link = cuda_dir / _substitute(entry["link"], variables)
        target = _substitute(entry["target"], variables)
        _atomic_symlink(link, target)
        notes.append(f"file link {link} -> {target}")

    if hostcc_bin_dir is not None:
        hostcc_bin_dir.mkdir(parents=True, exist_ok=True)
        configured_wrapper = _substitute(str(layout.get("hostcc_target", "")), variables)
        wrapper = Path(configured_wrapper) if configured_wrapper else cucc_path / "tools" / "hostcc_cce"
        if not wrapper.is_file():
            wrapper = cucc_path / "tools" / "hostcc_cce"
        for name in layout.get("hostcc_names", []):
            if hostcc_mode == "wrapper" and wrapper.is_file():
                target = str(wrapper)
            elif Path(f"/usr/bin/{name}").exists():
                target = f"/usr/bin/{name}"
            else:
                continue
            _atomic_symlink(hostcc_bin_dir / name, target)
            notes.append(f"hostcc {name} -> {target}")

        # Low-frequency cu-bridge carriers are separate from hostcc_cce:
        # Fortran/MPI stay host-only and reject CUDA device sources instead of
        # pretending that Ascend ccec or an MPI wrapper is their frontend.
        for entry in layout.get("host_tool_wrappers", []):
            names = entry.get("names", [])
            if isinstance(names, str):
                names = [names]
            wrapper_target = _substitute(str(entry.get("target", "")), variables)
            for name in names:
                if hostcc_mode == "wrapper" and wrapper_target and Path(wrapper_target).is_file():
                    target = wrapper_target
                elif Path(f"/usr/bin/{name}").exists():
                    target = f"/usr/bin/{name}"
                else:
                    continue
                _atomic_symlink(hostcc_bin_dir / name, target)
                notes.append(f"host tool {name} -> {target}")

    alias_target = _substitute(layout.get("alias_target", ""), variables)
    for alias_dir in layout.get("alias_dirs", []):
        base = cuda_dir / _substitute(alias_dir, variables)
        base.mkdir(parents=True, exist_ok=True)
        for alias in layout.get("cuda_lib_aliases", []):
            _atomic_symlink(base / alias, alias_target)
            notes.append(f"alias {base / alias} -> {alias_target}")

    for entry in layout.get("files", []):
        path = cuda_dir / _substitute(entry["path"], variables)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(_substitute(entry["content"], variables), encoding="utf-8")
        notes.append(f"wrote {path}")
    return notes


# Compatibility API name used by older integrations.
build_shim = build_virtual_toolkit


def _reference_command(argv: List[str]) -> Optional[int]:
    """Handle cu-bridge-compatible ``virtualDir.py cmake/make -r`` calls."""
    if not argv or argv[0] not in {"cmake", "make"}:
        return None
    command = argv[0]
    parser = argparse.ArgumentParser(prog="virtualDir.py " + command)
    parser.add_argument("command", choices=("cmake", "make"))
    parser.add_argument("-r", "--root", required=True, type=Path)
    parser.add_argument("--cucc-path", type=Path, default=None)
    parser.add_argument("--cann-base", default=os.environ.get("CANN_BASE", "/usr/local/Ascend/latest"))
    parser.add_argument("--hostcc-mode", choices=("wrapper", "identity"), default="wrapper")
    args = parser.parse_args(argv)
    env_cucc = os.environ.get("CUCC_PATH", "").strip()
    cucc_path = (args.cucc_path or (Path(env_cucc) if env_cucc else None) or
                 Path(__file__).resolve().parents[2]).expanduser().resolve()
    target = (cucc_path / "tools" / "mock" / "nvcc_entry.sh" if command == "cmake"
              else cucc_path / "bin" / "cucc")
    manifest = (cucc_path / "tools" / "mock" / "stub_link.json" if command == "cmake"
                else cucc_path / "tools" / "link.json")
    try:
        notes = build_virtual_toolkit(
            # Reference virtualDir.py treats -r as the toolkit root itself.
            # The wrapper helpers use the explicit --cuda-dir form and keep
            # their existing <root>/CUDA_DIR layout for CMake cache stability.
            cuda_dir=args.root,
            cucc_path=cucc_path,
            nvcc_target=target,
            layout_path=manifest,
            hostcc_bin_dir=args.root.parent / "bin",
            hostcc_mode=args.hostcc_mode,
            cann_base=args.cann_base,
        )
    except (OSError, ValueError, KeyError) as exc:
        print(f"virtualDir.py: {exc}", file=sys.stderr)
        return 1
    for note in notes:
        print(f"virtualDir.py: {note}")
    return 0


def main(argv: Optional[List[str]] = None) -> int:
    actual_argv = list(sys.argv[1:] if argv is None else argv)
    reference_result = _reference_command(actual_argv)
    if reference_result is not None:
        return reference_result

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cuda-dir", required=True, type=Path)
    parser.add_argument("--cucc-path", required=True, type=Path)
    parser.add_argument("--nvcc-target", required=True, type=Path)
    parser.add_argument("--hostcc-bin-dir", type=Path, default=None)
    parser.add_argument("--hostcc-mode", choices=("wrapper", "identity"), default="wrapper")
    parser.add_argument("--layout", "--manifest", dest="layout", type=Path, default=None)
    parser.add_argument("--cann-base", default="")
    parser.add_argument("--quiet", action="store_true")
    args = parser.parse_args(actual_argv)

    layout_path = args.layout
    if layout_path is None:
        candidates = [
            args.cucc_path / "tools" / "link.json",
            args.cucc_path / "tools" / "mock" / "stub_link.json",
            Path(__file__).resolve().parents[2] / "tools" / "link.json",
            Path(__file__).resolve().parents[2] / "tools" / "mock" / "stub_link.json",
        ]
        layout_path = next((candidate for candidate in candidates if candidate.is_file()), None)
        if layout_path is None:
            print("virtualDir.py: virtual toolkit manifest not found", file=sys.stderr)
            return 1

    try:
        notes = build_virtual_toolkit(
            cuda_dir=args.cuda_dir,
            cucc_path=args.cucc_path,
            nvcc_target=args.nvcc_target,
            layout_path=layout_path,
            hostcc_bin_dir=args.hostcc_bin_dir,
            hostcc_mode=args.hostcc_mode,
            cann_base=args.cann_base,
        )
    except (OSError, ValueError, KeyError) as exc:
        print(f"virtualDir.py: {exc}", file=sys.stderr)
        return 1
    if not args.quiet:
        for note in notes:
            print(f"virtualDir.py: {note}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
