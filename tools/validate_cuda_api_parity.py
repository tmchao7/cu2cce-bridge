#!/usr/bin/env python3
"""Validate complete CUDA Runtime/device adaptor inventory coverage.

A reference cu-bridge adaptor is an inventory source, not an instruction to
copy MACA implementation names. Every reference CUDA entry must be either an
active cu2cce adaptor mapping or a disabled RESERVED_NPU entry in the parity
inventory header.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Dict, Iterable, List, Mapping

_DEFINE_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)\b(?:\([^)]*\))?\s*(.*)$")
_CUDA_NAME_RE = re.compile(r"^(?:cuda[A-Za-z0-9_]*|make_cuda[A-Za-z0-9_]*)$")
_MACA_RHS_RE = re.compile(r"(?:\bmc[A-Z]|\bmaca\b|\bmxcc\b|runtime_cu|symbol_cu|libmcruntime)", re.I)
_WCUDA_DECL_RE = re.compile(r"\b(wcuda[A-Za-z0-9_]+)\s*\(")


def _logical_lines(text: str) -> Iterable[str]:
    current = ""
    for raw in text.splitlines():
        line = raw.rstrip()
        if current:
            current += line.lstrip()
        else:
            current = line
        if current.endswith("\\"):
            current = current[:-1] + " "
            continue
        yield current
        current = ""
    if current:
        yield current


def resolve_reference(path: Path) -> Path:
    """Accept either the reference adaptor file or a cu-bridge checkout."""
    if path.is_dir():
        return path / "include/bridge/runtime/cuda_to_maca_mcr_adaptor.h"
    return path


def parse_cuda_defines(path: Path) -> Dict[str, str]:
    """Return CUDA/make_cuda macro names and their right-hand sides."""
    path = resolve_reference(path)
    text = path.read_text(encoding="utf-8") if path.is_file() else ""
    result: Dict[str, str] = {}
    for line in _logical_lines(text):
        match = _DEFINE_RE.match(line)
        if not match:
            continue
        name, rhs = match.groups()
        if _CUDA_NAME_RE.match(name):
            result[name] = rhs.strip()
    return result


def active_maca_backend_names(active: Mapping[str, str]) -> List[str]:
    return sorted(name for name, rhs in active.items() if _MACA_RHS_RE.search(rhs))


def parse_runtime_header_wrappers(path: Path | None) -> List[str]:
    """Return typed wcuda function declarations visible in the public header.

    The mapping adaptor also maps CUDA constants and aliases.  Those do not
    require function declarations, so this audit reports only active macro
    targets that are actual ``wcuda*`` callable declarations.
    """
    if path is None or not path.is_file():
        return []
    return sorted(set(_WCUDA_DECL_RE.findall(path.read_text(encoding="utf-8"))))


def load_status_inventory(path: Path) -> Mapping[str, object]:
    if not path.is_file():
        return {}
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}
    return value if isinstance(value, dict) else {}


def audit(reference: Path, implementation: Path, inventory: Path,
          status_inventory: Path | None = None,
          runtime_header: Path | None = None) -> Dict[str, object]:
    reference_entries = parse_cuda_defines(reference)
    active_entries = parse_cuda_defines(implementation)
    reserved_entries = parse_cuda_defines(inventory)

    reference_names = set(reference_entries)
    active_names = set(active_entries) & reference_names
    reserved_names = set(reserved_entries) & reference_names
    missing = sorted(reference_names - active_names - reserved_names)
    duplicate = sorted(active_names & reserved_names)

    status_data = load_status_inventory(status_inventory) if status_inventory else {}
    status_entries = status_data.get("entries", {}) if isinstance(status_data, dict) else {}
    if not isinstance(status_entries, dict):
        status_entries = {}
    status_missing = sorted(reference_names - set(status_entries)) if status_inventory else []
    runtime_header_wrappers = parse_runtime_header_wrappers(runtime_header)
    runtime_header_wrapper_set = set(runtime_header_wrappers)
    typed_wrapper_statuses = {"ACTIVE_ADAPTED", "UNSUPPORTED_TYPED"}
    active_wrapper_entries = sorted(
        name for name in active_names
        if active_entries[name].split()[0] in runtime_header_wrapper_set
    )
    runtime_header_missing_entries = sorted(
        name for name, entry in status_entries.items()
        if name in active_names
        and isinstance(entry, dict)
        and entry.get("status") in typed_wrapper_statuses
        and isinstance(entry.get("cu2cce_target"), str)
        and entry["cu2cce_target"].startswith("wcuda")
        and entry["cu2cce_target"] not in runtime_header_wrapper_set
    )

    status_counts = {
        "ACTIVE_DIRECT": 0, "ACTIVE_ADAPTED": 0, "UNSUPPORTED_TYPED": 0,
        "RESERVED_NPU": 0, "DIAGNOSTIC_ONLY": 0,
    }
    for value in status_entries.values():
        if isinstance(value, dict):
            status = value.get("status")
            if status in status_counts:
                status_counts[status] += 1

    return {
        "reference_entries": len(reference_names),
        "runtime_header_declared_wrappers": len(runtime_header_wrappers),
        "runtime_header_active_wrapper_entries": active_wrapper_entries,
        "runtime_header_missing_entries": runtime_header_missing_entries,
        "active_entries": len(active_names),
        "typed_unsupported_entries": status_counts["UNSUPPORTED_TYPED"],
        "reserved_entries": len(reserved_names),
        "diagnostic_entries": status_counts["DIAGNOSTIC_ONLY"],
        "status_inventory_missing_entries": status_missing,
        "status_counts": status_counts,
        "missing_entries": missing,
        "duplicate_entries": duplicate,
        "active_maca_backend_names": active_maca_backend_names(
            {name: active_entries[name] for name in active_names}
        ),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument(
        "--implementation", type=Path,
        default=root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h",
    )
    parser.add_argument(
        "--inventory", type=Path,
        default=root / "include/bridge/runtime/cuda_to_cce_reserved_parity.h",
    )
    parser.add_argument(
        "--status-inventory", type=Path,
        default=root / "config/cuda_api_parity_inventory.json",
    )
    parser.add_argument(
        "--runtime-header", type=Path,
        default=root / "include/bridge/runtime/cuda_runtime_wrapper.h",
        help="public runtime wrapper header used for a callable surface audit",
    )
    parser.add_argument("--format", choices=("json", "text"), default="text")
    args = parser.parse_args()
    result = audit(
        args.reference, args.implementation, args.inventory,
        args.status_inventory, args.runtime_header,
    )
    if args.format == "json":
        print(json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True))
    else:
        for key in (
            "reference_entries", "active_entries", "typed_unsupported_entries",
            "reserved_entries", "diagnostic_entries",
            "runtime_header_declared_wrappers",
        ):
            print(f"{key}={result[key]}")
        for key in ("missing_entries", "duplicate_entries", "active_maca_backend_names",
                    "status_inventory_missing_entries", "runtime_header_missing_entries"):
            print(f"{key}={len(result[key])}")
            for name in result[key]:
                print(f"  {name}")
    return 0 if not (
        result["missing_entries"]
        or result["duplicate_entries"]
        or result["active_maca_backend_names"]
        or result["status_inventory_missing_entries"]
        or result["runtime_header_missing_entries"]
    ) else 1


if __name__ == "__main__":
    raise SystemExit(main())
