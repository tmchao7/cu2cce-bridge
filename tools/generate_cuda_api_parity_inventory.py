#!/usr/bin/env python3
"""Generate disabled RESERVED_NPU CUDA parity entries from cu-bridge."""
from __future__ import annotations

import argparse
import json
from collections import defaultdict
from pathlib import Path
from typing import Dict, Iterable, List

from validate_cuda_api_parity import parse_cuda_defines, resolve_reference


def _category(name: str) -> str:
    if name.startswith("cudaGraph"):
        return "Graph"
    if name.startswith(("cudaGraphics", "cudaGL", "cudaEGL", "cudaVDPAU")):
        return "Graphics_Interop"
    if name.startswith(("cudaIpc", "cudaExternal", "cudaImport", "cudaSignal", "cudaWait")):
        return "IPC_External"
    if name.startswith(("cudaArray", "cudaTexture", "cudaSurface", "cudaResource", "cudaChannel", "cudaBind", "cudaGetTexture", "cudaGetSurface")):
        return "Array_Texture_Surface"
    if name.startswith(("cudaMemPool", "cudaMemRange", "cudaMemRemap", "cudaMemUnremap", "cudaMemAdvise")):
        return "MemoryPool_UVM"
    if name.startswith(("cudaStream", "cudaEvent", "cudaThread")):
        return "Stream_Event_Thread"
    if name.startswith(("cudaDevice", "cudaCtx", "cudaChoose", "cudaSetValid", "cudaOccupancy", "cudaLaunch")):
        return "Device_Context_Launch"
    return "Other_Runtime_Device"


def _safe_comment(text: str) -> str:
    return text.replace("*/", "* /").strip()


def _function_body(source: str, name: str) -> str:
    marker = f"{name}("
    start = source.find(marker)
    if start < 0:
        return ""
    brace = source.find("{", start)
    if brace < 0:
        return ""
    depth = 0
    for index in range(brace, len(source)):
        char = source[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[brace:index + 1]
    return ""


def _active_status(name: str, rhs: str, runtime_source: str) -> str:
    if rhs.startswith("wcuda"):
        body = _function_body(runtime_source, rhs)
        if "ACL_ERROR_API_NOT_SUPPORT" in body:
            return "UNSUPPORTED_TYPED"
        return "ACTIVE_ADAPTED"
    return "ACTIVE_DIRECT"


def generate(
    reference: Path, implementation: Path, output: Path, reference_label: str,
    status_output: Path | None = None,
) -> None:
    reference = resolve_reference(reference)
    reference_entries = parse_cuda_defines(reference)
    active_entries = parse_cuda_defines(implementation)
    root = implementation.resolve().parents[3]
    runtime_source_path = root / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp"
    runtime_source = runtime_source_path.read_text(encoding="utf-8") if runtime_source_path.is_file() else ""
    missing = sorted(set(reference_entries) - set(active_entries))
    groups: Dict[str, List[str]] = defaultdict(list)
    for name in missing:
        groups[_category(name)].append(name)

    lines = [
        "#ifndef __CU2CCE_CUDA_TO_CCE_RESERVED_PARITY_H__",
        "#define __CU2CCE_CUDA_TO_CCE_RESERVED_PARITY_H__",
        "",
        "/*",
        " * GENERATED parity inventory. Source: " + reference_label,
        " *",
        " * These entries intentionally stay under #if 0. They document CUDA",
        " * names present in cu-bridge but not active for the Ascend/CANN backend.",
        " * Do not enable an entry until its CANN public API, type surface and",
        " * real-device contract are implemented and tested.",
        " */",
        "",
    ]
    for category in sorted(groups):
        lines.extend([
            "#if 0",
            f"/* RESERVED_NPU category: {category}",
            " * Future implementation: include/bridge/runtime/ and src/bridge/runtime/.",
            " */",
        ])
        for name in groups[category]:
            rhs = _safe_comment(reference_entries[name])
            lines.extend([
                f"/* reference_maca: {name} -> {rhs} */",
                f"#define {name} CU2CCE_RESERVED_NPU_{name}",
            ])
        lines.extend(["#endif", ""])
    lines.extend([
        "#endif /* __CU2CCE_CUDA_TO_CCE_RESERVED_PARITY_H__ */",
        "",
    ])
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines), encoding="utf-8")

    if status_output is not None:
        entries = {}
        for name in sorted(reference_entries):
            if name in active_entries:
                rhs = active_entries[name]
                status = _active_status(name, rhs, runtime_source)
                target = rhs
            else:
                status = "RESERVED_NPU"
                target = f"CU2CCE_RESERVED_NPU_{name}"
            entries[name] = {
                "status": status,
                "reference_maca": reference_entries[name],
                "cu2cce_target": target,
                "category": _category(name),
            }
        status_output.parent.mkdir(parents=True, exist_ok=True)
        status_output.write_text(json.dumps({
            "schema_version": 1,
            "reference": reference_label,
            "entries": entries,
        }, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument(
        "--implementation", type=Path,
        default=root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h",
    )
    parser.add_argument(
        "--output", type=Path,
        default=root / "include/bridge/runtime/cuda_to_cce_reserved_parity.h",
    )
    parser.add_argument(
        "--status-output", type=Path,
        default=root / "config/cuda_api_parity_inventory.json",
    )
    args = parser.parse_args()
    generate(args.reference, args.implementation, args.output, str(args.reference),
             args.status_output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
