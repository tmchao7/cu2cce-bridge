#!/usr/bin/env python3
"""Validate CUDA public header, wrapper-file and API registration parity."""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Iterable

FUNC_RE = re.compile(r"\b((?:wcu|wcuda|wcublas|nvrtc)[A-Za-z0-9_]*)\s*\(")
MACA_RE = re.compile(r"\b(?:mxcc|maca|runtime_cu|symbol_cu|libmcruntime|mc[A-Z][A-Za-z0-9_]*)\b")
PRIVATE_HEADERS = {"__macro_mxcc.h"}


def funcs(root: Path) -> set[str]:
    found: set[str] = set()
    for p in root.rglob("*.h"):
        text = active_text(p)
        found.update(FUNC_RE.findall(text))
    return found


def public_headers(root: Path) -> set[str]:
    include = root / "include"
    return {p.name for p in include.iterdir() if p.is_file() and p.name not in PRIVATE_HEADERS}


def active_text(path: Path) -> str:
    text = path.read_text(encoding="utf-8", errors="ignore")
    # Remove comments before checking active implementation for forbidden names.
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//.*", "", text)
    return text


def audit(reference: Path | None, implementation: Path, inventory: Path) -> dict[str, object]:
    data = json.loads(inventory.read_text(encoding="utf-8")) if inventory.is_file() else {}
    header_data = data.get("headers", {}) if isinstance(data, dict) else {}
    wrapper_data = data.get("wrappers", {}) if isinstance(data, dict) else {}
    api_data = data.get("api_entries", {}) if isinstance(data, dict) else {}
    if reference is not None:
        ref_headers = public_headers(reference)
        ref_funcs = funcs(reference / "include/bridge")
    else:
        ref_headers = {
            str(name) for name, value in header_data.items()
            if not isinstance(value, dict) or value.get("status") != "REFERENCE_ONLY"
        }
        ref_funcs = {
            str(value.get("wrapper")) for value in api_data.values()
            if isinstance(value, dict) and value.get("wrapper")
        }
    impl_headers = public_headers(implementation)
    impl_funcs = funcs(implementation / "include/bridge")
    reserved = implementation / "include/bridge/cuda_to_cce_reserved_wrapper_parity.h"
    reserved_text = reserved.read_text(encoding="utf-8", errors="ignore") if reserved.is_file() else ""
    reserved_api_names = set(re.findall(r"\* CUDA API: ([A-Za-z_][A-Za-z0-9_]*)", reserved_text))
    reserved_wrapper_names = set(re.findall(r"\* Reference wrapper: ([A-Za-z_][A-Za-z0-9_]*)", reserved_text))
    missing_headers = sorted(ref_headers - impl_headers - {n for n, v in header_data.items() if isinstance(v, dict) and v.get("status") == "REFERENCE_ONLY"})
    missing_wrappers = sorted(path for path, value in wrapper_data.items() if not (implementation / str(value.get("implementation", ""))).is_file())
    # A JSON row alone is not a future unlock point.  Every non-active wrapper
    # must also appear in the committed #if 0 reserved header with its reference
    # prototype/comment, otherwise a regenerated inventory could silently lose
    # the declaration-level maintenance surface.
    missing_apis = sorted(name for name in ref_funcs if name not in impl_funcs and name not in reserved_wrapper_names)
    active_maca: list[str] = []
    for p in (implementation / "include").rglob("*.h"):
        text = active_text(p)
        if MACA_RE.search(text) and "cuda_to_cce_reserved_wrapper_parity.h" not in str(p):
            active_maca.append(str(p.relative_to(implementation)))
    wrapper_values = [str(value.get("wrapper")) for value in api_data.values() if isinstance(value, dict) and value.get("wrapper")]
    duplicate = sorted(name for name in set(wrapper_values) if wrapper_values.count(name) > 1)
    return {
        "reference_header_entries": len(ref_headers),
        "implementation_header_entries": len(impl_headers),
        "reference_wrapper_entries": len(set(wrapper_data)),
        "implementation_wrapper_entries": len({p.relative_to(implementation).as_posix() for p in (implementation / "include/bridge").rglob("*.h")}),
        "reference_api_entries": len(ref_funcs),
        "implementation_api_entries": len(impl_funcs),
        "missing_headers": missing_headers,
        "missing_wrapper_entries": missing_wrappers,
        "missing_api_entries": missing_apis,
        "duplicate_entries": duplicate,
        "active_maca_backend_names": sorted(set(active_maca)),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument(
        "--reference", type=Path,
        help="reference checkout; omit on servers to use the committed inventory snapshot",
    )
    parser.add_argument("--implementation", type=Path, default=root)
    parser.add_argument("--inventory", type=Path, default=root / "config/cuda_api_parity_inventory.json")
    parser.add_argument("--format", choices=("json", "text"), default="text")
    args = parser.parse_args()
    result = audit(args.reference, args.implementation, args.inventory)
    if args.format == "json":
        print(json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True))
    else:
        for key in ("reference_header_entries", "implementation_header_entries", "reference_wrapper_entries", "implementation_wrapper_entries", "reference_api_entries", "implementation_api_entries"):
            print(f"{key}={result[key]}")
        for key in ("missing_headers", "missing_wrapper_entries", "missing_api_entries", "duplicate_entries", "active_maca_backend_names"):
            print(f"{key}={len(result[key])}")
            for item in result[key]:
                print(f"  {item}")
    bad = any(result[k] for k in ("missing_headers", "missing_wrapper_entries", "missing_api_entries", "duplicate_entries", "active_maca_backend_names"))
    return 1 if bad else 0


if __name__ == "__main__":
    raise SystemExit(main())
