#!/usr/bin/env python3
"""Build the fake CUDA toolkit tree from config/shim_layout.json.

cu-bridge parity: their link.json (pre_make) and mock/stub_link.json
(virtual_cmake) describe the CUDA-shaped shim in one manifest, consumed by
make_maca/cmake_mock/pre_makefile alike. This builder is the cu2cce
equivalent — a single manifest + one builder shared by cmake_cce, make_cce
and pre_cce, instead of three copies of hand-written mkdir/ln sequences.

Layout contract (see config/shim_layout.json):
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


def _replace_symlink(link: Path, target: str) -> None:
    """Create or replace a symbolic link without touching the target."""
    if link.is_symlink() or link.exists():
        if link.is_symlink() and os.readlink(link) == target:
            return
        if link.is_dir() and not link.is_symlink():
            # A real directory from an older shim generation: only descend
            # for the include fallback case handled by the caller; here the
            # manifest owns the shape, so replace via rename is unsafe for
            # non-empty dirs — clear it first.
            import shutil  # noqa: PLC0415
            shutil.rmtree(link)
        else:
            link.unlink()
    link.parent.mkdir(parents=True, exist_ok=True)
    os.symlink(target, link)


def _atomic_symlink(link: Path, target: str) -> None:
    if link.is_symlink():
        if os.readlink(link) == target:
            return
        link.unlink()
    elif link.exists():
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


def build_shim(
    cuda_dir: Path,
    cucc_path: Path,
    nvcc_target: Path,
    layout_path: Path,
    hostcc_bin_dir: Optional[Path] = None,
    hostcc_mode: str = "wrapper",
    cann_base: str = "",
) -> List[str]:
    layout = json.loads(layout_path.read_text(encoding="utf-8"))
    variables = {
        "CUDA_DIR": str(cuda_dir),
        "CUCC_PATH": str(cucc_path),
        "CANN_BASE": cann_base,
        "HOST_TRIPLE": host_triple(),
        "NVCC_TARGET": str(nvcc_target),
    }

    notes: List[str] = []
    for entry in layout.get("dirs", []):
        (cuda_dir / _substitute(entry, variables)).mkdir(parents=True, exist_ok=True)

    for entry in layout.get("dir_links", []):
        link = cuda_dir / _substitute(entry["link"], variables)
        target = _substitute(entry["target"], variables)
        if entry.get("replace"):
            _replace_symlink(link, target)
        else:
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


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cuda-dir", required=True, type=Path)
    parser.add_argument("--cucc-path", required=True, type=Path)
    parser.add_argument("--nvcc-target", required=True, type=Path)
    parser.add_argument("--hostcc-bin-dir", type=Path, default=None)
    parser.add_argument("--hostcc-mode", choices=("wrapper", "identity"), default="wrapper")
    parser.add_argument("--layout", type=Path, default=None)
    parser.add_argument("--cann-base", default="")
    parser.add_argument("--quiet", action="store_true")
    args = parser.parse_args(argv)

    layout_path = args.layout
    if layout_path is None:
        candidates = [
            args.cucc_path / "config" / "shim_layout.json",
            Path(__file__).resolve().parents[1] / "config" / "shim_layout.json",
        ]
        for candidate in candidates:
            if candidate.is_file():
                layout_path = candidate
                break
        if layout_path is None:
            print("build_shim_tree: shim_layout.json not found", file=sys.stderr)
            return 1

    notes = build_shim(
        cuda_dir=args.cuda_dir,
        cucc_path=args.cucc_path,
        nvcc_target=args.nvcc_target,
        layout_path=layout_path,
        hostcc_bin_dir=args.hostcc_bin_dir,
        hostcc_mode=args.hostcc_mode,
        cann_base=args.cann_base,
    )
    if not args.quiet:
        for note in notes:
            print(f"build_shim_tree: {note}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
