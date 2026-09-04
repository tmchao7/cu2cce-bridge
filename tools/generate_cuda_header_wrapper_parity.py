#!/usr/bin/env python3
"""Generate CUDA public-header and wrapper parity placeholders.

The reference checkout supplies names and declaration text only.  Generated
files are ordinary files (never symlinks) and unsupported declarations remain
commented or disabled so they cannot claim a CANN ABI that is not implemented.
"""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Iterable

FUNC_RE = re.compile(r"\b((?:wcuda|wcu|nvrtc)[A-Za-z0-9_]*)\s*\(")
PUBLIC_API_RE = re.compile(r"\b((?:cuda|cu|cublas|cufft|curand|cusolver|cusparse|cudnn|nccl|npp|nppi|npps|nvjpeg|nvml|nvrtc)[A-Za-z0-9_]*)\s*\(")
PRIVATE_REFERENCE_HEADERS = {"__macro_mxcc.h"}


def guard_for(path: str) -> str:
    return "__CU2CCE_PARITY_" + re.sub(r"[^A-Za-z0-9]", "_", path).upper() + "__"


def reference_declarations(text: str) -> list[str]:
    """Collapse reference declarations into readable comment lines."""
    result: list[str] = []
    current = ""
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith("//") or line.startswith("/*") or line.startswith("*"):
            continue
        current = f"{current} {line}".strip()
        if ";" in line:
            if FUNC_RE.search(current):
                result.append(re.sub(r"\s+", " ", current))
            current = ""
    return result


def _without_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*", "", text)


def wrapper_functions(root: Path) -> dict[str, list[str]]:
    found: dict[str, list[str]] = {}
    for path in sorted(root.rglob("*.h")):
        # Reserved parity headers deliberately contain reference prototypes in
        # comments. They are never active wrappers and must not be reclassified
        # merely because a reference comment itself contains nested /* ... */.
        if path.name.startswith("cuda_to_cce_reserved"):
            continue
        text = _without_comments(path.read_text(encoding="utf-8", errors="ignore"))
        for name in sorted(set(FUNC_RE.findall(text))):
            found.setdefault(name, []).append(str(path.relative_to(root)))
    return found


def cuda_name(wrapper: str) -> str:
    if wrapper.startswith("wcuda"):
        return "cuda" + wrapper[len("wcuda"):]
    if wrapper.startswith("wcu"):
        return "cu" + wrapper[len("wcu"):]
    if wrapper.startswith("wcublas"):
        return "cublas" + wrapper[len("wcublas"):]
    if wrapper.startswith("nvrtc"):
        return wrapper
    return wrapper


ACTIVE_HEADER_NAMES = {
    "__cuda_runtime.h", "builtin_types.h", "common_functions.h",
    "cooperative_groups.h", "cuda.h", "cuda_bf16.h", "cuda_bf16.hpp",
    "cuda_device_api.h", "cuda_fp16.h", "cuda_fp16.hpp", "cuda_runtime.h",
    "cuda_runtime_api.h", "cuda_surface_types.h", "cuda_texture_types.h",
    "cudaTypedefs.h", "device_functions.h", "device_launch_parameters.h",
    "device_types.h", "driver_types.h", "host_defines.h", "library_types.h",
    "math_constants.h", "math_functions.h", "surface_types.h",
    "texture_types.h", "vector_types.h",
}
DIAGNOSTIC_HEADER_NAMES = {
    "cuda_profiler_api.h", "cudaProfiler.h", "cudaProfilerTypedefs.h",
    "nvToolsExt.h", "nvToolsExtCuda.h", "nvToolsExtCudaRt.h",
    "nvToolsExtOpenCL.h", "nvToolsExtSync.h",
}


def header_status(name: str) -> str:
    if name in ACTIVE_HEADER_NAMES:
        return "ACTIVE_ADAPTED"
    if name in DIAGNOSTIC_HEADER_NAMES:
        return "DIAGNOSTIC_ONLY"
    return "RESERVED_NPU"


def route_for(name: str) -> str:
    if name.startswith(("cublas", "cufft", "curand", "cusolver", "cusparse", "cudnn", "nccl", "npp", "nppi", "npps", "nvjpeg", "nvml")):
        return "CANN library route requires a dedicated wrapper and real-device evidence"
    if name.startswith("nvrtc"):
        return "No safe puresimt/PTX runtime equivalent"
    if name.startswith("cu") and not name.startswith("cuda"):
        return "CANN aclrtBinary/module route; no active CUDA Driver ABI"
    return "CANN Runtime/SIMT route must be verified against the installed toolkit"


def wrapper_target_rel(reference_rel: str) -> str:
    """Map backend-specific reference filenames to neutral CCE paths."""
    rel = reference_rel
    replacements = (
        ("cuda_to_maca_", "cuda_to_cce_"),
        ("pfn_to_maca", "pfn_to_cce"),
        ("_to_mcblasLt_", "_to_cce_"),
        ("_to_mcblasXt_", "_to_cce_"),
        ("_to_mcfftXt_", "_to_cce_"),
        ("_to_mcfft_", "_to_cce_"),
        ("_to_mcrand_", "_to_cce_"),
        ("_to_mctx_", "_to_cce_"),
    )
    for old, new in replacements:
        rel = rel.replace(old, new)
    return rel


def write_placeholder(path: Path, reference_rel: str, api_names: Iterable[str], *, private=False) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        current = path.read_text(encoding="utf-8", errors="ignore")
        if "CUDA compatibility header parity entry" not in current:
            return
    guard = guard_for(str(path.relative_to(path.parents[2])) if len(path.parents) > 2 else path.name)
    lines = [
        f"#ifndef {guard}", f"#define {guard}", "",
        "/* CUDA compatibility header parity entry. */",
        f"/* Reference header: {reference_rel} */",
        "/* This is a cu2cce compatibility surface, not an NVIDIA runtime copy. */",
    ]
    if private:
        lines += ["/* Status: REFERENCE_ONLY / BACKEND_PRIVATE. */",
                  "/* The reference header contains backend-private MACA compiler macros. */"]
    elif api_names:
        lines += ["/* Status: RESERVED_NPU or DIAGNOSTIC_ONLY until each API has a safe CANN ABI. */",
                  "/* API names and future routes are recorded below; no fake library is linked. */"]
        for name in api_names:
            lines += [f"/* CUDA API: {name}", f" * CANN route: {route_for(name)}",
                      " * Status: RESERVED_NPU", " * Future implementation: add a typed wrapper and real CANN probe.", " */"]
    else:
        lines += ["/* Status: RESERVED_NPU; declarations are tracked in the parity inventory. */"]
    lines += ["", f"#endif /* {guard} */", ""]
    path.write_text("\n".join(lines), encoding="utf-8")


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
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[brace:index + 1]
    return ""


def implementation_api_status(wrapper: str, implementation: Path) -> str:
    for path in (implementation / "src").rglob("*.cpp"):
        body = _function_body(path.read_text(encoding="utf-8", errors="ignore"), wrapper)
        if body:
            return (
                "UNSUPPORTED_TYPED"
                if "ACL_ERROR_API_NOT_SUPPORT" in body or "return kUnsupported" in body
                else "ACTIVE_ADAPTED"
            )
    return "ACTIVE_ADAPTED"


def generate_reserved_wrapper_header(path: Path, missing: dict[str, list[str]], declarations: dict[str, list[str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "#ifndef __CU2CCE_CUDA_TO_CCE_RESERVED_WRAPPER_PARITY_H__",
        "#define __CU2CCE_CUDA_TO_CCE_RESERVED_WRAPPER_PARITY_H__", "",
        "/* GENERATED CUDA wrapper parity inventory; intentionally inactive. */",
        "/* Every item below is a future unlock point, not an active CANN symbol. */", "",
        "#if 0",
    ]
    for wrapper in sorted(missing):
        name = cuda_name(wrapper)
        sources = ", ".join(missing[wrapper])
        lines += [
            "/*",
            f" * CUDA API: {name}",
            f" * Reference wrapper: {wrapper}",
            f" * Reference files: {sources}",
            f" * CANN route: {route_for(name)}",
            " * Status: RESERVED_NPU",
            " * Future implementation: include/bridge wrapper + typed CANN probe.",
        ]
        for declaration in declarations.get(wrapper, []):
            lines.append(f" * Reference prototype: {declaration}")
        lines += [" */", f"/* {name} -> {wrapper} */", ""]
    lines += ["#endif /* RESERVED_NPU */", "", "#endif /* __CU2CCE_CUDA_TO_CCE_RESERVED_WRAPPER_PARITY_H__ */", ""]
    path.write_text("\n".join(lines), encoding="utf-8")


def generate(reference: Path, implementation: Path, inventory: Path) -> dict[str, object]:
    ref_include = reference / "include"
    impl_include = implementation / "include"
    ref_bridge = ref_include / "bridge"
    impl_bridge = impl_include / "bridge"

    reference_headers = sorted(p.name for p in ref_include.iterdir() if p.is_file())
    implementation_headers = sorted(p.name for p in impl_include.iterdir() if p.is_file())
    header_entries: dict[str, dict[str, object]] = {}
    for name in reference_headers:
        if name in PRIVATE_REFERENCE_HEADERS:
            header_entries[name] = {"implementation": None, "status": "REFERENCE_ONLY", "reason": "backend-private MACA header"}
            continue
        reference_header = ref_include / name
        api_names = sorted(set(PUBLIC_API_RE.findall(reference_header.read_text(encoding="utf-8", errors="ignore"))))
        target = impl_include / name
        header_entries[name] = {
            "implementation": f"include/{name}",
            "status": header_status(name),
            "api_names": api_names,
        }
        write_placeholder(target, f"include/{name}", api_names)

    ref_wrappers = sorted(p.relative_to(reference).as_posix() for p in ref_bridge.rglob("*.h"))
    impl_wrappers = {p.relative_to(implementation).as_posix() for p in impl_bridge.rglob("*.h")}
    reference_functions = wrapper_functions(ref_bridge)
    implementation_functions = wrapper_functions(impl_bridge)
    missing_files: list[str] = []
    wrapper_entries: dict[str, dict[str, object]] = {}
    for rel in ref_wrappers:
        target_rel = wrapper_target_rel(rel)
        target = implementation / target_rel
        existed = target.is_file()
        names = sorted({cuda_name(n) for n in reference_functions if rel.endswith(tuple(reference_functions[n]))})
        placeholder = False
        if existed:
            placeholder = "CUDA compatibility header parity entry" in target.read_text(
                encoding="utf-8", errors="ignore"
            )
        if not existed:
            missing_files.append(rel)
            write_placeholder(target, rel, names)
            placeholder = True
        elif placeholder:
            # Refresh generated comments when reference routes/API lists evolve.
            write_placeholder(target, rel, names)
        wrapper_entries[rel] = {
            "implementation": target_rel,
            "status": "RESERVED_NPU" if placeholder else "ACTIVE_ADAPTED",
        }

    missing_functions = {name: files for name, files in reference_functions.items() if name not in implementation_functions}
    declarations: dict[str, list[str]] = {}
    for path in ref_bridge.rglob("*.h"):
        for declaration in reference_declarations(path.read_text(encoding="utf-8", errors="ignore")):
            for wrapper in missing_functions:
                if re.search(rf"\b{re.escape(wrapper)}\s*\(", declaration):
                    declarations.setdefault(wrapper, []).append(declaration)
    reserved = implementation / "include/bridge/cuda_to_cce_reserved_wrapper_parity.h"
    generate_reserved_wrapper_header(reserved, missing_functions, declarations)

    api_entries: dict[str, dict[str, object]] = {}
    for wrapper, files in sorted(reference_functions.items()):
        name = cuda_name(wrapper)
        if wrapper in implementation_functions:
            status = implementation_api_status(wrapper, implementation)
            impl_files = implementation_functions[wrapper]
        else:
            status = "RESERVED_NPU"
            impl_files = ["include/bridge/cuda_to_cce_reserved_wrapper_parity.h"]
        api_entries[name] = {
            "wrapper": wrapper,
            "reference_files": files,
            "implementation_files": impl_files,
            "status": status,
            "mapping_type": "ADAPTED" if status.startswith("ACTIVE") else "ROUTE_ONLY",
            "cann_route": route_for(name),
            "reference_prototypes": declarations.get(wrapper, []),
        }

    data = {}
    if inventory.is_file():
        try:
            data = json.loads(inventory.read_text(encoding="utf-8"))
        except json.JSONDecodeError:
            data = {}
    data.update({
        "reference_commit": "6ab1076",
        "headers": header_entries,
        "wrappers": wrapper_entries,
        "api_entries": api_entries,
        "libraries": {
            "cublas": {"route": "acl/ops/acl_cblas.h + libacl_cblas.so", "status": "EXPERIMENTAL"},
            "cufft": {"route": "SiP/FFT/aclnn", "status": "ROUTE_ONLY"},
            "curand": {"route": "application/Ascend C implementation", "status": "UNSUPPORTED"},
            "cusolver": {"route": "CANN solver route", "status": "ROUTE_ONLY"},
            "cusparse": {"route": "CANN sparse/aclnn route", "status": "ROUTE_ONLY"},
            "cudnn": {"route": "aclnn/ATB", "status": "ROUTE_ONLY"},
            "nccl": {"route": "hccl/hccl.h + libhccl.so", "status": "ROUTE_ONLY"},
            "npp": {"route": "CANN image/vision operators", "status": "ROUTE_ONLY"},
            "nvjpeg": {"route": "CANN JPEG route", "status": "ROUTE_ONLY"},
            "nvml": {"route": "Ascend device management API", "status": "ROUTE_ONLY"},
            "nvrtc": {"route": "unsupported in puresimt", "status": "REJECT"},
        },
    })
    inventory.parent.mkdir(parents=True, exist_ok=True)
    inventory.write_text(json.dumps(data, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return {
        "reference_header_entries": len(reference_headers),
        "implementation_header_entries": len(implementation_headers),
        "reference_wrapper_entries": len(ref_wrappers),
        "missing_wrapper_files": sum(
            1 for rel, entry in wrapper_entries.items()
            if not (implementation / str(entry["implementation"])).is_file()
        ),
        "reference_api_entries": len(reference_functions),
        "missing_api_entries": len(missing_functions),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--implementation", type=Path, default=root)
    parser.add_argument("--inventory", type=Path, default=root / "config/cuda_api_parity_inventory.json")
    args = parser.parse_args()
    print(json.dumps(generate(args.reference, args.implementation, args.inventory), ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
