#!/usr/bin/env python3
"""Produce a backend-aware cu-bridge parity report.

The report compares mechanisms, not backend-specific library names.  A MACA
feature with no validated Ascend equivalent is classified explicitly instead of
being reported as a missing implementation.  The tool is intentionally
read-only: it inspects both repositories and writes JSON or Markdown only to
stdout.
"""
from __future__ import annotations

import argparse
import json
import subprocess
from collections import Counter
from pathlib import Path
from typing import Any, Dict, Iterable, List, Mapping

STATUSES = (
    "ALIGNED",
    "EQUIVALENT_NPU",
    "MISSING",
    "NOT_APPLICABLE_NPU",
    "DIAGNOSTIC_ONLY",
)


def _git_short_head(path: Path) -> str:
    try:
        return subprocess.check_output(
            ["git", "-C", str(path), "rev-parse", "--short", "HEAD"],
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return "unknown"


def _read(path: Path) -> str:
    return path.read_text(encoding="utf-8") if path.is_file() else ""


def _json(path: Path) -> Mapping[str, Any]:
    try:
        value = json.loads(_read(path))
    except (OSError, json.JSONDecodeError):
        return {}
    return value if isinstance(value, dict) else {}


def _rule_count(rules: Mapping[str, Any], section: str, action: str) -> int:
    value = rules.get(section, {})
    if not isinstance(value, dict):
        return 0
    entries = value.get(action, [])
    return len(entries) if isinstance(entries, list) else 0


def _reference_replacer_count(conf: Mapping[str, Any]) -> int:
    entries = conf.get("all", {})
    if not isinstance(entries, dict):
        return 0
    replacer = entries.get("replacer", [])
    return len(replacer) // 2 if isinstance(replacer, list) else 0


def _walk_objects(value: Any) -> Iterable[Mapping[str, Any]]:
    if isinstance(value, dict):
        yield value
        for child in value.values():
            yield from _walk_objects(child)
    elif isinstance(value, list):
        for child in value:
            yield from _walk_objects(child)


def _reference_cuda_library_alias_count(*assets: Mapping[str, Any]) -> int:
    return sum(
        1
        for asset in assets
        for obj in _walk_objects(asset)
        if str(obj.get("ln", "")).startswith((
            "libcuda", "libcudart", "libcublas", "libnvToolsExt",
            "libcufft", "libcudnn", "libcusparse", "libcurand",
        ))
    )


def _item(
    item_id: str,
    status: str,
    reference: str,
    implementation: str,
    note: str,
) -> Dict[str, str]:
    if status not in STATUSES:
        raise ValueError(f"unknown status: {status}")
    return {
        "id": item_id,
        "status": status,
        "reference": reference,
        "implementation": implementation,
        "note": note,
    }


def build_report(repo: Path, reference: Path) -> Dict[str, Any]:
    rules = _json(repo / "config/cu2cce_rules.json")
    layout = _json(repo / "config/shim_layout.json")
    conf = _json(reference / "bin/conf.json")
    ref_link = _json(reference / "tools/link.json")
    ref_stub = _json(reference / "tools/mock/stub_link.json")

    cmake = _read(repo / "tools/cmake_cce")
    make = _read(repo / "tools/make_cce")
    ninja = _read(repo / "tools/ninja_cce")
    pre = _read(repo / "tools/pre_cce")
    hostcc = _read(repo / "tools/hostcc_cce")
    driver = _read(repo / "tools/cu2cce_driver.py")
    cmake_ref = _read(reference / "tools/cmake_maca")
    cmake_mock_ref = _read(reference / "tools/cmake_mock")
    make_ref = _read(reference / "tools/make_maca")
    ninja_ref = _read(reference / "tools/ninja_maca")
    pre_ref = _read(reference / "tools/pre_makefile")
    cucc_ref = _read(reference / "bin/cucc")

    reference_filter_count = len(conf.get("all", {}).get("filter", [])) if isinstance(conf.get("all"), dict) else 0
    reference_query_count = len(conf.get("skipstring", [])) if isinstance(conf.get("skipstring"), list) else 0
    current_query_count = len(rules.get("queries", [])) if isinstance(rules.get("queries"), list) else 0
    reference_alias_count = _reference_cuda_library_alias_count(ref_link, ref_stub)
    current_alias_count = len(layout.get("cuda_lib_aliases", [])) if isinstance(layout.get("cuda_lib_aliases"), list) else 0

    sections: Dict[str, List[Dict[str, str]]] = {
        "cmake_make": [
            _item(
                "cmake_entry",
                "ALIGNED" if cmake and cmake_ref else "MISSING",
                "cmake_maca -> cmake_mock/cmake_assist",
                "cmake_cce -> persistent CUDA-shaped shim",
                "Explicit cmake_cce is the NPU entry; system cmake is not replaced.",
            ),
            _item(
                "cmake_two_phase",
                "ALIGNED" if "--build" in cmake and "nvcc_entry.sh" in cmake_mock_ref + cmake else "MISSING",
                "configure mock, build real make_maca/ninja_maca",
                "configure mock_nvcc, build cucc through generator wrapper",
                "The backend compiler remains ccec, not mxcc.",
            ),
            _item(
                "make_wrapper",
                "ALIGNED" if make and make_ref else "MISSING",
                "make_maca builds link.json toolkit tree",
                "make_cce builds manifest-driven toolkit tree",
                "Manifest-based implementation is the NPU equivalent.",
            ),
            _item(
                "ninja_wrapper",
                "ALIGNED" if ninja and ninja_ref else "MISSING",
                "ninja_maca rebuilds shim and exports environment",
                "ninja_cce repoints persistent shim and runs ninja",
                "Direct ninja lifecycle is audited separately from CMake dispatch.",
            ),
            _item(
                "pre_wrapper",
                "ALIGNED" if pre and pre_ref else "MISSING",
                "pre_makefile prepares CUDA environment",
                "pre_cce prepares CUDA-shaped Ascend environment",
                "Direct argv execution is safer than reference eval string reconstruction.",
            ),
            _item(
                "host_link_carrier",
                "EQUIVALENT_NPU" if hostcc and "hostcc" in rules else "MISSING",
                "bin/gnu links through mxcc and MACA libraries",
                "hostcc_cce keeps g++ and injects bridge/CANN libraries",
                "Literal mxcc copying is invalid on Ascend.",
            ),
            _item(
                "system_cmake_replacement",
                "NOT_APPLICABLE_NPU",
                "USE_CMAKE_MACA=1 replaces system cmake",
                "No system replacement; cmake_cce remains explicit",
                "Avoids root requirement, global mutation, and restore failure risk.",
            ),
            _item(
                "shim_manifest",
                "ALIGNED" if layout and ref_link and ref_stub else "MISSING",
                "link.json + stub_link.json",
                "config/shim_layout.json + build_shim_tree.py",
                f"Reference CUDA alias entries: {reference_alias_count}; current manifest aliases: {current_alias_count}.",
            ),
        ],
        "cucc": [
            _item(
                "options_file",
                "ALIGNED" if "--options-file" in driver else "MISSING",
                "--options-file recursive expansion",
                "safe shlex response/options expansion",
                "No shell evaluation; nesting is depth-limited.",
            ),
            _item(
                "query_skipstring",
                "ALIGNED" if current_query_count >= reference_query_count - 2 else "MISSING",
                f"{reference_query_count} reference skipstring queries",
                f"{current_query_count} configured queries plus driver banner handling",
                "-E and -V remain explicit follow-up parity items if not represented by rules.",
            ),
            _item(
                "rule_pipeline",
                "EQUIVALENT_NPU" if _rule_count(rules, "all", "drop") and _rule_count(rules, "all", "replace") else "MISSING",
                f"{reference_filter_count} filters / {_reference_replacer_count(conf)} replacer pairs",
                f"{_rule_count(rules, 'all', 'drop')} drops / {_rule_count(rules, 'all', 'replace')} replacements plus NPU policy",
                "Rules preserve semantics where ccec has an equivalent and reject/drop the rest honestly.",
            ),
            _item(
                "architecture_policy",
                "EQUIVALENT_NPU" if "CU2CCE_NPU_ARCH" in _read(repo / "tools/device_arch.sh") else "MISSING",
                "CUCC_TARGETS -> --offload-arch for MACA",
                "CU2CCE_NPU_ARCH/device_arch.sh -> --cce-aicore-arch=dav-*",
                "sm_* is metadata or compatibility macro input, never a ccec target.",
            ),
            _item(
                "library_policy",
                "EQUIVALENT_NPU" if "capabilities" in json.dumps(rules) else "MISSING",
                "CUDA libraries map to MACA library names",
                "validated bridge/CANN mapping, experimental cublas, map_to diagnostics",
                "No unvalidated CUDA library is presented as an Ascend equivalent.",
            ),
            _item(
                "device_artifacts",
                "NOT_APPLICABLE_NPU",
                "PTX/CUBIN/FATBIN and MACA device-link paths",
                "explicit reject for NVIDIA artifacts and unvalidated RDC/device-link",
                "puresimt produces Ascend code through ccec and has no NVIDIA artifact path.",
            ),
            _item(
                "observability",
                "ALIGNED" if "CU2CCE_TRACE_FILE" in driver and "CU2CCE_DEBUG" in driver else "MISSING",
                "debug echo and generated command trace",
                "CU2CCE_DEBUG, CU2CCE_TRACE_FILE, --dryrun, --proxy-mode",
                "Trace records argv decisions without source contents.",
            ),
        ],
    }
    summary = Counter(item["status"] for items in sections.values() for item in items)
    for status in STATUSES:
        summary.setdefault(status, 0)
    return {
        "schema_version": 1,
        "reference": {
            "path": str(reference.resolve()),
            "commit": _git_short_head(reference),
            "version": "3.8.0" if _git_short_head(reference) == "6ab1076" else "unknown",
        },
        "implementation": {
            "path": str(repo.resolve()),
            "commit": _git_short_head(repo),
        },
        "reference_assets": {
            "skipstring": reference_query_count,
            "filter": reference_filter_count,
            "replacer_pairs": _reference_replacer_count(conf),
            "link_json_keys": len(ref_link),
            "stub_link_json_keys": len(ref_stub),
            "cuda_library_aliases": reference_alias_count,
        },
        "sections": sections,
        "summary": dict(summary),
    }


def markdown(report: Mapping[str, Any]) -> str:
    lines = [
        "# cu-bridge parity audit",
        "",
        f"Reference: `{report['reference']['commit']}` ({report['reference']['version']})",
        f"Implementation: `{report['implementation']['commit']}`",
        "",
    ]
    for section, items in report["sections"].items():
        lines.extend([f"## {section}", "", "| ID | Status | Reference | Implementation | Note |", "|---|---|---|---|---|"])
        for item in items:
            lines.append("| {id} | {status} | {reference} | {implementation} | {note} |".format(**item))
        lines.append("")
    lines.append("## Summary")
    lines.append("")
    for status, count in report["summary"].items():
        lines.append(f"- `{status}`: {count}")
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True, help="cu-bridge checkout")
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--format", choices=("json", "markdown"), default="json")
    args = parser.parse_args()
    report = build_report(args.repo.resolve(), args.reference.resolve())
    if args.format == "markdown":
        print(markdown(report), end="")
    else:
        print(json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
