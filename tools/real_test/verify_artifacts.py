#!/usr/bin/env python3
"""Validate CMake/Make/Ninja/cuсc output artifacts on the real server.

The verifier checks output shape, not backend identity: Ascend puresimt output
is expected to be ordinary ELF objects/libraries/programs with CANN/bridge
runtime dependencies, while NVIDIA-only intermediate artifacts are forbidden.
Detailed findings are written to the requested log; the CLI prints one compact
verdict line for use by server_cmd.txt.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, Iterable, List, Mapping, Optional, Sequence, Tuple


class ContractError(ValueError):
    """Invalid artifact contract or command-line contract selection."""


@dataclass
class ArtifactResult:
    ok: bool
    details: List[str] = field(default_factory=list)
    checked: List[str] = field(default_factory=list)

    def format_details(self) -> str:
        return "\n".join(self.details) if self.details else "artifact contract passed"


@dataclass
class _Inspect:
    path: Path
    description: str = ""
    dynamic: str = ""


def load_contract(path: Path) -> Dict[str, Any]:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ContractError(f"cannot load artifact contract {path}: {exc}") from exc
    if not isinstance(data, dict) or data.get("schema_version") != 1:
        raise ContractError(f"{path}: schema_version must be 1")
    if not isinstance(data.get("cases"), dict) or not data["cases"]:
        raise ContractError(f"{path}: cases must be a non-empty object")
    forbidden = data.get("forbidden_suffixes", [])
    if not isinstance(forbidden, list) or not all(isinstance(x, str) for x in forbidden):
        raise ContractError(f"{path}: forbidden_suffixes must be a string list")
    for name, case in data["cases"].items():
        if not isinstance(case, dict):
            raise ContractError(f"cases.{name}: must be an object")
        if not isinstance(case.get("required", []), list):
            raise ContractError(f"cases.{name}.required: must be a list")
    return data


def _run_tool(command: Sequence[str]) -> Tuple[int, str, str]:
    try:
        result = subprocess.run(
            list(command), capture_output=True, text=True, check=False,
        )
    except OSError as exc:
        return 127, "", str(exc)
    return result.returncode, result.stdout, result.stderr


def _matches(root: Path, pattern: str) -> List[Path]:
    if any(char in pattern for char in "*?["):
        return sorted(path for path in root.glob(pattern) if path.exists() or path.is_symlink())
    path = root / pattern
    return [path] if path.exists() or path.is_symlink() else []


def _file_description(path: Path) -> str:
    file_tool = shutil.which("file")
    if file_tool:
        code, stdout, stderr = _run_tool([file_tool, "-b", str(path)])
        if code == 0 and stdout.strip():
            return stdout.strip()
        if stderr.strip():
            return f"file failed: {stderr.strip()}"
    try:
        if path.read_bytes()[:8] == b"!<arch>\n":
            return "ar archive"
    except OSError:
        pass
    return ""


def _readelf_header(path: Path) -> str:
    readelf = shutil.which("readelf")
    if not readelf:
        return ""
    code, stdout, stderr = _run_tool([readelf, "-h", str(path)])
    if code != 0:
        return f"readelf failed: {stderr.strip()}"
    return stdout


def _readelf_dynamic(path: Path) -> str:
    readelf = shutil.which("readelf")
    if not readelf:
        return ""
    code, stdout, stderr = _run_tool([readelf, "-d", str(path)])
    if code != 0:
        return f"readelf failed: {stderr.strip()}"
    return stdout


def _check_kind(path: Path, spec: Mapping[str, Any], description: str) -> List[str]:
    kind = str(spec.get("kind", "file"))
    lower = description.lower()
    errors: List[str] = []
    if kind == "dependency":
        try:
            if not path.read_text(encoding="utf-8", errors="replace").strip():
                errors.append(f"dependency artifact is empty: {path}")
        except OSError as exc:
            errors.append(f"cannot read dependency artifact {path}: {exc}")
        return errors
    if kind == "file":
        return errors
    if kind == "archive":
        try:
            magic = path.read_bytes()[:8]
        except OSError as exc:
            errors.append(f"cannot read archive {path}: {exc}")
            return errors
        if magic != b"!<arch>\n" and "archive" not in lower:
            errors.append(f"expected archive, got {description or 'unknown file type'}: {path}")
        return errors
    require_elf = bool(spec.get("require_elf", True))
    header = _readelf_header(path) if require_elf else ""
    header_lower = header.lower()
    if require_elf and not header:
        errors.append(f"readelf -h unavailable for ELF contract: {path}")
    if kind == "object":
        if require_elf and "elf" not in lower:
            errors.append(f"expected ELF object, got {description or 'unknown file type'}: {path}")
        if require_elf and "relocatable" not in lower and "relocatable" not in header_lower:
            errors.append(f"expected relocatable object, got {description or 'unknown file type'}: {path}")
    elif kind == "shared":
        if require_elf and "elf" not in lower:
            errors.append(f"expected ELF shared object, got {description or 'unknown file type'}: {path}")
        if require_elf and "shared object" not in lower and "shared library" not in lower and "dyn" not in header_lower:
            errors.append(f"expected shared object, got {description or 'unknown file type'}: {path}")
    elif kind == "executable":
        if require_elf:
            if "elf" not in lower:
                errors.append(f"expected ELF executable, got {description or 'unknown file type'}: {path}")
            if "executable" not in lower and "exec" not in header_lower:
                errors.append(f"expected executable, got {description or 'unknown file type'}: {path}")
        elif not os.access(path, os.X_OK):
            errors.append(f"expected executable permission: {path}")
    else:
        errors.append(f"unknown artifact kind {kind!r} for {path}")
    return errors


def _check_dynamic(path: Path, spec: Mapping[str, Any], inspect: _Inspect) -> List[str]:
    needed = [str(value) for value in spec.get("required_needed", [])]
    rpaths = [str(value) for value in spec.get("required_rpath", [])]
    if not needed and not rpaths:
        return []
    dynamic = _readelf_dynamic(path)
    inspect.dynamic = dynamic
    if not dynamic:
        return [f"readelf -d unavailable for dynamic contract: {path}"]
    errors: List[str] = []
    for library in needed:
        if library not in dynamic:
            errors.append(f"missing NEEDED {library}: {path}")
    for rpath in rpaths:
        if rpath not in dynamic:
            errors.append(f"missing RPATH/RUNPATH {rpath}: {path}")
    return errors


def _check_metadata(root: Path, entries: Sequence[Mapping[str, Any]]) -> Tuple[List[str], List[str]]:
    errors: List[str] = []
    checked: List[str] = []
    for entry in entries:
        pattern = str(entry.get("path", ""))
        if not pattern:
            errors.append("metadata entry missing path")
            continue
        matches = _matches(root, pattern)
        if not matches:
            errors.append(f"missing metadata artifact: {root / pattern}")
            continue
        contains = entry.get("contains", [])
        if not isinstance(contains, list):
            errors.append(f"metadata contains must be a list: {pattern}")
            continue
        for path in matches:
            checked.append(str(path))
            try:
                text = path.read_text(encoding="utf-8", errors="replace")
            except OSError as exc:
                errors.append(f"cannot read metadata artifact {path}: {exc}")
                continue
            for token in contains:
                if str(token) not in text:
                    errors.append(f"metadata token {token!r} missing from {path}")
    return errors, checked


def _forbidden_artifacts(root: Path, suffixes: Iterable[str]) -> List[str]:
    forbidden = {suffix.lower() for suffix in suffixes}
    errors: List[str] = []
    if not root.exists():
        return errors
    for path in root.rglob("*"):
        if not path.is_file() and not path.is_symlink():
            continue
        if path.suffix.lower() in forbidden:
            errors.append(f"forbidden artifact: {path}")
    return errors


def verify_case(root: Path, case: Mapping[str, Any], contract: Mapping[str, Any]) -> ArtifactResult:
    root = root.expanduser().resolve()
    details: List[str] = []
    checked: List[str] = []
    if not root.is_dir():
        return ArtifactResult(False, [f"artifact root not found: {root}"], checked)

    details.extend(_forbidden_artifacts(root, case.get("forbidden_suffixes", contract.get("forbidden_suffixes", []))))
    for spec in case.get("required", []):
        if not isinstance(spec, dict) or not spec.get("path"):
            details.append("required artifact entry must contain path")
            continue
        pattern = str(spec["path"])
        matches = _matches(root, pattern)
        if not matches:
            details.append(f"missing required artifact: {root / pattern}")
            continue
        if spec.get("all_matches", False) and not matches:
            details.append(f"required glob has no matches: {root / pattern}")
            continue
        for path in matches:
            checked.append(str(path))
            description = _file_description(path)
            inspect = _Inspect(path, description)
            details.extend(_check_kind(path, spec, description))
            details.extend(_check_dynamic(path, spec, inspect))
    metadata_errors, metadata_checked = _check_metadata(root, case.get("metadata", []))
    details.extend(metadata_errors)
    checked.extend(metadata_checked)
    return ArtifactResult(not details, details, checked)


def verify_contract(contract_path: Path, case_name: str, root: Path, log_path: Optional[Path] = None) -> ArtifactResult:
    contract = load_contract(contract_path)
    cases = contract["cases"]
    if case_name not in cases:
        raise ContractError(f"unknown artifact contract case: {case_name}")
    result = verify_case(root, cases[case_name], contract)
    if log_path:
        log_path.parent.mkdir(parents=True, exist_ok=True)
        lines = [f"case={case_name}", f"root={root.resolve()}", f"ok={int(result.ok)}", f"checked={len(result.checked)}"]
        if result.details:
            lines.extend(result.details)
        else:
            lines.append("artifact contract passed")
        log_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return result


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--contract", type=Path, required=True)
    parser.add_argument("--case", required=True)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--log", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        result = verify_contract(args.contract, args.case, args.root, args.log)
    except ContractError as exc:
        args.log.parent.mkdir(parents=True, exist_ok=True)
        args.log.write_text(f"contract error: {exc}\n", encoding="utf-8")
        print(f"[FAIL] artifact [{args.case}] (details: {args.log})", file=sys.stderr)
        return 2
    if result.ok:
        print(f"[PASS] artifact [{args.case}] contract ({len(result.checked)} checks; details: {args.log})")
        return 0
    print(f"[FAIL] artifact [{args.case}] ({len(result.details)} issue(s); details: {args.log})", file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
