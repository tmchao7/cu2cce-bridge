#!/usr/bin/env python3
"""Rule-driven CUDA/nvcc argument adapter for cu2cce-bridge.

This module only transforms compiler arguments. It never rewrites CUDA
source files. The actual compiler remains CANN ccec.

One read-only exception serves diagnostics: sources may be scanned (never
modified) to pre-flight report device-side use of types the Ascend backend
cannot run at all — bisheng has no device-side FP64/double support, so
affected code must be downgraded to float manually by the user (mentor
guidance, 2026-08-27). Policy knob: CU2CCE_DEVICE_FP64_POLICY =
error (default) | warn | allow.
"""
from __future__ import annotations

import fnmatch
import json
import os
import re
import shlex
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, Iterable, List, Mapping, Optional, Sequence, Tuple


class DriverError(RuntimeError):
    """A user-facing command-line policy error."""


# ---------------------------------------------------------------------------
# Device-side FP64 scanning (read-only, line-number preserving)
# ---------------------------------------------------------------------------

_DOUBLE_TOKEN_RE = re.compile(r"\bdouble[1-4]?\b")
_DEVICE_FUNC_SIG_RE = re.compile(r"__global__\b|__device__\b")

# nvcc GPU-architecture tokens embedded in -gencode/-arch/-code style
# arguments ("arch=compute_80,code=sm_80", "sm_90", "compute_120").
_CUDA_ARCH_TOKEN_RE = re.compile(r"(?:compute|sm)_([0-9]{2,3})\b")

# nvcc flags that carry a GPU architecture value (space-separated form; the
# =-joined form is caught by the token regex directly).
_ARCH_VALUE_FLAGS = {
    "-gencode", "--generate-code", "-arch", "--gpu-architecture",
    "--gpu-code", "--cuda-gpu-arch", "--offload-arch",
}


def _cuda_arch_macro_value(digits: str) -> int:
    """CUDA __CUDA_ARCH__ numeric value for a two/three digit sm_ token.

    nvcc encodes the compute capability as major*100 + minor*10: sm_80 -> 800,
    sm_86 -> 860, sm_100 -> 1000. The last digit is the minor version."""
    major, minor = digits[:-1], digits[-1]
    return int(major) * 100 + int(minor) * 10


def _cuda_version_parts(version: str) -> Tuple[str, str, str, str]:
    """__CUDACC_VER_* components derived from the compat CUDA version.

    CUCC_CUDA_VERSION is encoded MMmmB (11060 = 11.6.0); the legacy
    __CUDACC_VER__ spelling is major*10000 + minor*100 + build (110600),
    matching the value nvcc 11.6 defines and the mock identity used."""
    try:
        value = int(version)
    except ValueError:
        return "", "", "", ""
    major, minor, build = value // 1000, (value // 10) % 100, value % 10
    return str(major), str(minor), str(build), str(major * 10000 + minor * 100 + build)


def extract_cuda_arch(args: Sequence[str]) -> Optional[int]:
    """Highest GPU architecture requested on the command line, or None.

    The arch flags themselves are dropped by the rules (Ascend selects its
    target through CU2CCE_NPU_ARCH); this read-only scan recovers the value
    so it can drive __CUDA_ARCH__=... when CU2CCE_ARCH_MACRO=auto (cu-bridge
    bin/cucc not_found.dev_compile parity) and enrich the decision trace."""
    best: Optional[int] = None
    i = 0
    while i < len(args):
        arg = args[i]
        if arg in _ARCH_VALUE_FLAGS and i + 1 < len(args):
            arg = args[i + 1]
        for match in _CUDA_ARCH_TOKEN_RE.finditer(arg):
            value = _cuda_arch_macro_value(match.group(1))
            if best is None or value > best:
                best = value
        i += 1
    return best


def _strip_noncode_preserving_lines(source: str) -> str:
    """Blank out comments and string/char literal CONTENTS.

    Newlines survive untouched so offsets stay valid for error reporting;
    every other removed byte becomes a single space.
    """
    out: List[str] = []
    i = 0
    n = len(source)
    state: Optional[str] = None  # 'block' | 'line' | '"' | "'"
    while i < n:
        ch = source[i]
        nxt = source[i + 1] if i + 1 < n else ""
        if state is None:
            if ch == "/" and nxt == "*":
                state = "block"
                out.append("  ")
                i += 2
                continue
            if ch == "/" and nxt == "/":
                state = "line"
                out.append("  ")
                i += 2
                continue
            if ch in "\"'":
                state = ch
                out.append(ch)  # keep the opening quote
                i += 1
                continue
            out.append(ch)
            i += 1
            continue
        if state == "block":
            if ch == "*" and nxt == "/":
                state = None
                out.append("  ")
                i += 2
                continue
            out.append("\n" if ch == "\n" else " ")
            i += 1
            continue
        if state == "line":
            if ch == "\n":
                state = None
                out.append("\n")
            else:
                out.append(" ")
            i += 1
            continue
        # string or char literal: preserve quotes, blank contents, honor escapes
        if ch == "\\" and i + 1 < n:
            out.extend([" ", " "])
            i += 2
            continue
        if (state == '"' and ch == '"') or (state == "'" and ch == "'"):
            state = None
            out.append(ch)
            i += 1
            continue
        out.append("\n" if ch == "\n" else " ")
        i += 1
    return "".join(out)


def _line_start_offsets(text: str) -> List[int]:
    starts = [0]
    for idx, ch in enumerate(text):
        if ch == "\n":
            starts.append(idx + 1)
    return starts


def scan_device_double_usage(source: str) -> List[Tuple[int, str]]:
    """Return [(line_no, matched_token)] for double usage inside device code.

    Device regions are function bodies opened by a signature containing
    __device__ or __global__ (nested braces inherit). Host-only code —
    including double math in plain C++ functions — never matches. Comments,
    strings, and char literals are ignored. Purely lexical/heuristic:
    macro-generated signatures may be missed; the policy knob's "allow"
    exists for those edge cases.
    """
    cleaned = _strip_noncode_preserving_lines(source)
    line_starts = _line_start_offsets(cleaned)
    structs = list(re.finditer(r"[{};]", cleaned))

    def current_line(offset: int) -> int:
        lo, hi = 0, len(line_starts) - 1
        while lo < hi:
            mid = (lo + hi + 1) // 2
            if line_starts[mid] <= offset:
                lo = mid
            else:
                hi = mid - 1
        return lo + 1

    def collect_hits(begin: int, end: int, findings: List[Tuple[int, str]]) -> None:
        seen_before = {(ln, tok) for ln, tok in findings}
        for match in _DOUBLE_TOKEN_RE.finditer(cleaned[begin:end]):
            tok = match.group(0)
            item = (current_line(begin + match.start()), tok)
            if item not in seen_before:
                seen_before.add(item)
                findings.append(item)

    # Single structural walk: attribute every brace pair to whether it
    # opens/joins device code (signature text since the previous structural
    # character mentions __device__/__global__), collect hits for those
    # signature segments directly (covers parameter lists and prototypes),
    # and record body ranges for interior scanning.
    findings: List[Tuple[int, str]] = []
    stack: List[Tuple[bool]] = []
    device_ranges: List[Tuple[int, int]] = []
    active = 0
    range_begin: Optional[int] = None
    prev_end = 0
    for m in structs:
        tok = m.group(0)
        sig_text = cleaned[prev_end:m.start()]
        sig_is_device = bool(_DEVICE_FUNC_SIG_RE.search(sig_text))
        if tok == "{":
            if sig_is_device:
                collect_hits(prev_end, m.start(), findings)
                if active == 0:
                    range_begin = prev_end
                active += 1
            stack.append((sig_is_device,))
        elif tok == "}":
            if stack:
                (enters,) = stack.pop()
                if enters:
                    active -= 1
                    if active == 0 and range_begin is not None:
                        device_ranges.append((range_begin, m.end()))
                        range_begin = None
        else:  # ';'
            if sig_is_device:
                collect_hits(prev_end, m.start(), findings)
        prev_end = m.end()

    for begin, end in device_ranges:
        collect_hits(begin, end, findings)
    return findings


_TYPE_POLICY_MESSAGE_HEAD = (
    "[type-policy] cu2cce: device-side FP64/double is unsupported by the "
    "current Ascend SIMT/AICore target. Host-side double remains supported."
)


def _format_device_double_report(path: Path, findings: Sequence[Tuple[int, str]]) -> str:
    lines = [f"{path}:{ln}: type '{tok}' used in device context" for ln, tok in findings[:8]]
    extra = len(findings) - min(len(findings), 8)
    if extra > 0:
        lines.append(f"... {extra} more device-side double site(s)")
    return "\n".join(lines)


@dataclass
class Action:
    argument: str
    action: str
    reason: str = ""
    replacement: List[str] = field(default_factory=list)


@dataclass
class PreparedCommand:
    phase: str
    mode: str
    args: List[str]
    compiler: str
    inputs: List[str]
    actions: List[Action]
    injected: List[str]
    rules_file: str
    source_is_cuda: bool


class RuleDriver:
    def __init__(self, env: Optional[Mapping[str, str]] = None,
                 rules_file: Optional[Path] = None,
                 cmake_identity: bool = False) -> None:
        self.env = dict(os.environ if env is None else env)
        self.repo_root = Path(__file__).resolve().parents[1]
        self.cucc_path = Path(self.env.get("CUCC_PATH", "")).expanduser()
        self.ascend_toolkit_home = self.env.get(
            "ASCEND_TOOLKIT_HOME", "/usr/local/Ascend/ascend-toolkit/latest"
        )
        self.cann_base = self.env.get("CANN_BASE", "/usr/local/Ascend/latest")
        self.cucc_cuda_version = self.env.get("CUCC_CUDA_VERSION", "11060")
        self.npu_arch = self._resolve_npu_arch()
        self.debug_enabled = self.env.get("CU2CCE_DEBUG", "0") == "1" or \
            self.env.get("WCUDA_DEBUG", "0") == "1"
        self.strict = self.env.get("CU2CCE_STRICT_FLAGS", "0") == "1"
        self.libc_mode = self.env.get("CU2CCE_LIBC_MODE", "shadow").lower()
        if self.libc_mode not in {"shadow", "system"}:
            raise DriverError("CU2CCE_LIBC_MODE must be shadow or system")
        self.fp64_policy = self.env.get("CU2CCE_DEVICE_FP64_POLICY", "error").lower()
        if self.fp64_policy not in {"error", "warn", "allow"}:
            raise DriverError("CU2CCE_DEVICE_FP64_POLICY must be error, warn, or allow")
        # __CUDA_ARCH__ derivation (cu-bridge parity, opt-in): by default the
        # macro is only defined through an explicit CU2CCE_CUDA_ARCH value.
        # CU2CCE_ARCH_MACRO=auto additionally derives it from the -gencode /
        # -arch value on the command line. The NPU backend has no separate
        # device pass, so auto mode defines the macro for the whole
        # translation unit — host branches behind `#ifdef __CUDA_ARCH__`
        # take the device path (documented trade-off, see PARITY_AUDIT).
        arch_macro = self.env.get("CU2CCE_ARCH_MACRO", "off").strip().lower()
        if arch_macro in ("on", "1", "true"):
            arch_macro = "auto"
        if arch_macro not in {"off", "auto"}:
            raise DriverError("CU2CCE_ARCH_MACRO must be off or auto")
        self.arch_macro_mode = arch_macro
        gcc_toolchain = self.env.get("CU2CCE_GCC_TOOLCHAIN", "auto").strip().lower()
        if gcc_toolchain in ("on", "1", "true"):
            gcc_toolchain = "auto"
        if gcc_toolchain not in {"", "auto", "off", "0", "false", "none"}:
            raise DriverError("CU2CCE_GCC_TOOLCHAIN must be auto or off")
        self.gcc_toolchain_mode = gcc_toolchain
        self.cuda_arch_value: Optional[int] = None
        self.cmake_identity = cmake_identity
        self.rules_path = self._select_rules_file(rules_file)
        try:
            self.rules = json.loads(self.rules_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            raise DriverError(f"cannot load cu2cce rules: {self.rules_path}: {exc}") from exc
        frontend_defaults = self.rules.get("frontend", {})
        if "CUCC_CUDA_VERSION" not in self.env:
            self.cucc_cuda_version = str(frontend_defaults.get("cuda_version", self.cucc_cuda_version))
        if (
            "CU2CCE_NPU_ARCH" not in self.env
            and "CUCC_NPU_ARCH" not in self.env
            and not self.env.get("CUCC_TARGETS", "").strip()
        ):
            self.npu_arch = str(frontend_defaults.get("default_arch", self.npu_arch))
        if self.rules.get("schema_version") != 1:
            raise DriverError(
                f"unsupported cu2cce rules schema in {self.rules_path}: "
                f"{self.rules.get('schema_version')!r}"
            )
        self.actions: List[Action] = []
        self.injected: List[str] = []
        self.response_files: List[str] = []

    def _resolve_npu_arch(self) -> str:
        """Resolve one Ascend target without treating CUDA sm_* as a device arch.

        ``CUCC_TARGETS`` is accepted as a compatibility alias for cu-bridge's
        environment knob, but ccec puresimt emits one Ascend ``dav-*`` target
        per invocation. Multiple or NVIDIA targets are therefore rejected
        instead of silently selecting an incompatible backend.
        """
        explicit = self.env.get("CU2CCE_NPU_ARCH") or self.env.get("CUCC_NPU_ARCH")
        if explicit:
            if not explicit.startswith("dav-"):
                raise DriverError(
                    f"NPU architecture {explicit!r} is not an Ascend dav-* "
                    "target; CUDA sm_*/compute_* values cannot select ccec"
                )
            return explicit
        raw_targets = self.env.get("CUCC_TARGETS", "").replace(",", " ")
        targets = raw_targets.split()
        if not targets:
            return "dav-c310-vec"
        if len(set(targets)) != 1:
            raise DriverError(
                "CUCC_TARGETS must contain exactly one Ascend dav-* architecture "
                "for a ccec puresimt invocation"
            )
        target = targets[0]
        if not target.startswith("dav-"):
            raise DriverError(
                f"CUCC_TARGETS={target!r} is not an Ascend dav-* architecture; "
                "CUDA sm_*/compute_* targets cannot select ccec"
            )
        return target


    def _select_rules_file(self, explicit: Optional[Path]) -> Path:
        candidates: List[Path] = []
        if explicit is not None:
            candidates.append(explicit.expanduser())
        if self.env.get("CU2CCE_RULES_FILE"):
            candidates.append(Path(self.env["CU2CCE_RULES_FILE"]).expanduser())
        if str(self.cucc_path):
            candidates.append(self.cucc_path / "config" / "cu2cce_rules.json")
        candidates.append(self.repo_root / "config" / "cu2cce_rules.json")
        for candidate in candidates:
            if candidate.is_file():
                return candidate.resolve()
        raise DriverError(
            "cu2cce rules file not found; set CU2CCE_RULES_FILE or install "
            "config/cu2cce_rules.json"
        )

    def _expand_response_files(self, args: Sequence[str], depth: int = 0) -> List[str]:
        if depth > 16:
            raise DriverError("response-file nesting exceeds 16 levels")
        expanded: List[str] = []
        i = 0
        while i < len(args):
            arg = args[i]
            if arg in ("--options-file", "-options-file"):
                if i + 1 >= len(args):
                    raise DriverError(f"{arg} requires a file path")
                path = Path(args[i + 1]).expanduser()
                if not path.is_file():
                    raise DriverError(f"options file not found: {path}")
                try:
                    content = path.read_text(encoding="utf-8")
                    nested = shlex.split(content, posix=True)
                except (OSError, ValueError) as exc:
                    raise DriverError(f"cannot parse options file {path}: {exc}") from exc
                self.response_files.append(str(path.resolve()))
                expanded.extend(self._expand_response_files(nested, depth + 1))
                i += 2
                continue
            if arg.startswith("@") and len(arg) > 1:
                path = Path(arg[1:]).expanduser()
                if path.is_file():
                    try:
                        nested = shlex.split(path.read_text(encoding="utf-8"), posix=True)
                    except (OSError, ValueError) as exc:
                        raise DriverError(f"cannot parse response file {path}: {exc}") from exc
                    self.response_files.append(str(path.resolve()))
                    expanded.extend(self._expand_response_files(nested, depth + 1))
                    i += 1
                    continue
            expanded.append(arg)
            i += 1
        return expanded

    @staticmethod
    def _match(pattern: Any, argument: str) -> bool:
        if isinstance(pattern, str):
            return fnmatch.fnmatchcase(argument, pattern)
        if not isinstance(pattern, dict):
            return False
        kind = pattern.get("kind", "glob")
        value = str(pattern.get("value", ""))
        if kind == "exact":
            return argument == value
        if kind == "prefix":
            return argument.startswith(value)
        if kind == "suffix":
            return argument.endswith(value)
        if kind == "glob":
            return fnmatch.fnmatchcase(argument, value)
        raise DriverError(f"unknown rule matcher kind: {kind}")

    def _rules_for(self, action_name: str, phase: str) -> Iterable[Dict[str, Any]]:
        sections = [self.rules.get("all", {})]
        if phase in ("compile", "preprocess", "dependency"):
            sections.append(self.rules.get("compile", {}))
        elif phase == "link":
            sections.append(self.rules.get("link", {}))
        elif phase == "compile_and_link":
            sections.extend([
                self.rules.get("compile", {}),
                self.rules.get("link", {}),
                self.rules.get("compile_and_link", {}),
            ])
        for section in sections:
            for rule in section.get(action_name, []):
                yield rule

    def _find_rule(self, action_name: str, phase: str, argument: str) -> Optional[Dict[str, Any]]:
        for rule in self._rules_for(action_name, phase):
            if self._match(rule.get("pattern", rule.get("match")), argument):
                return rule
        return None

    @staticmethod
    def _split_forwarded(value: str) -> List[str]:
        value = value.strip().strip('"')
        if not value:
            return []
        if "," in value:
            return [item for item in value.split(",") if item]
        try:
            return shlex.split(value)
        except ValueError:
            return [value]

    def _warn(self, message: str) -> None:
        print(f"cu2cce: {message}", file=sys.stderr)

    def _debug(self, message: str) -> None:
        if self.debug_enabled:
            print(f"[cu2cce] {message}", file=sys.stderr)

    def _record(self, argument: str, action: str, reason: str = "",
                replacement: Optional[List[str]] = None) -> None:
        item = Action(argument, action, reason, replacement or [])
        self.actions.append(item)
        if self.debug_enabled:
            suffix = f": {reason}" if reason else ""
            self._debug(f"{action}: {argument}{suffix}")

    @staticmethod
    def _has_cuda_source(args: Sequence[str]) -> bool:
        if any(Path(arg).suffix == ".cu" for arg in args):
            return True
        for i, arg in enumerate(args[:-1]):
            if arg == "-x" and args[i + 1] in ("cuda", "cu", "c++-cuda"):
                return True
        return False

    @staticmethod
    def _scan_inputs(args: Sequence[str]) -> Tuple[List[str], Optional[str], bool, bool, str]:
        inputs: List[str] = []
        output: Optional[str] = None
        compile_only = False
        has_object = False
        explicit_language = ""
        i = 0
        value_options = {"-o", "-MF", "-MT", "-include", "-x", "-isystem"}
        while i < len(args):
            arg = args[i]
            if arg in value_options:
                if i + 1 < len(args):
                    value = args[i + 1]
                    if arg == "-o":
                        output = value
                    elif arg == "-x":
                        explicit_language = value
                    i += 2
                    continue
                i += 1
                continue
            if arg == "-c":
                compile_only = True
            elif arg.startswith("-o") and len(arg) > 2:
                output = arg[2:]
            elif arg.endswith(".o"):
                has_object = True
                inputs.append(arg)
            elif not arg.startswith("-"):
                if arg.endswith((".cu", ".cpp", ".cc", ".cxx", ".c")):
                    inputs.append(arg)
            i += 1
        return inputs, output, compile_only, has_object, explicit_language

    @staticmethod
    def _phase(args: Sequence[str]) -> Tuple[str, str, List[str], Optional[str], bool]:
        inputs, output, compile_only, has_object, explicit_language = RuleDriver._scan_inputs(args)
        source_is_cuda = RuleDriver._has_cuda_source(args) or explicit_language in ("cuda", "cu", "c++-cuda")
        if "--device-link" in args or "-dlink" in args:
            # Device-link is handled at link time through the injected
            # -lcce_bridge_runtime target; the cucc CMake layer forces
            # CUDA_SEPARABLE_COMPILATION OFF, so a stray -dlink phase is
            # downgraded to a normal link instead of a hard error. The rules
            # file rejects the flags explicitly with a diagnostic reason.
            return "device_link", "link", inputs, output, source_is_cuda
        if "-E" in args:
            return "preprocess", "preprocess", inputs, output, source_is_cuda
        if "-M" in args or "-MM" in args:
            return "dependency", "dependency", inputs, output, source_is_cuda
        if "-S" in args:
            return "compile", "unsupported", inputs, output, source_is_cuda
        if compile_only:
            phase = "compile"
        elif has_object:
            phase = "link"
        elif source_is_cuda and output and output.endswith(".o"):
            phase = "compile"
        elif source_is_cuda:
            phase = "compile_and_link"
        else:
            phase = "link" if has_object else "compile"
        return phase, phase, inputs, output, source_is_cuda

    def _context(self, phase: str, source_is_cuda: bool, args: Sequence[str]) -> Dict[str, bool]:
        simt_root = self.env.get("SIMT_API_ROOT", "")
        has_simt = bool(simt_root and (Path(simt_root) / "simt_api").is_dir())
        if not has_simt:
            for candidate in (
                "/usr/local/Ascend/cann-*/x86_64-linux/asc/include",
                f"{self.ascend_toolkit_home}/x86_64-linux/asc/include",
                "/usr/local/Ascend/cann-*/aarch64-linux/asc/include",
            ):
                # Glob is intentionally handled by the shell-compatible path
                # check in _resolve_simt_root; this fallback is only a marker.
                if "*" not in candidate and (Path(candidate) / "simt_api").is_dir():
                    simt_root = candidate
                    has_simt = True
                    break
        self.simt_root = simt_root
        self.simt_parent = str(Path(simt_root).parent) if simt_root else ""
        has_pic = any(arg in ("-fPIC", "-fpic") for arg in args)
        if not has_pic:
            for index, arg in enumerate(args[:-1]):
                if arg in ("-Xcompiler", "--compiler-options") and "fPIC" in args[index + 1]:
                    has_pic = True
                    break
        shared_link = any(arg == "-shared" or arg.startswith("-shared=") for arg in args)
        user_has_std = any(arg.startswith("-std=") for arg in args)
        has_openmp = any(arg == "-fopenmp" or arg.startswith("-fopenmp=") for arg in args)
        use_gopenmp = self.env.get("CUCC_USE_GOPENMP", "") == "1" and not has_openmp
        explicit_cuda_arch = bool(self.env.get("CU2CCE_CUDA_ARCH", "").strip())
        return {
            "shared_link_without_pic": shared_link and not has_pic,
            "system_libc": self.libc_mode == "system",
            "not_system_libc": self.libc_mode != "system",
            "has_shadow_compat": has_simt and self.libc_mode != "system" and bool(self.cucc_path and (self.cucc_path / "include/shadow").is_dir()),
            "has_cucc": bool(self.cucc_path and self.cucc_path.is_dir()),
            "has_shadow": (self.cucc_path / "include/shadow").is_dir() if self.cucc_path else False,
            "has_samples_common": (self.cucc_path / "third_party/nvidia_cuda_samples/Common").is_dir() if self.cucc_path else False,
            "has_simt_api": has_simt,
            "source_is_cuda": source_is_cuda,
            "source_is_cuda_and_has_adaptor": source_is_cuda and bool(
                self.cucc_path and (self.cucc_path / "include/bridge/kernel/simt_adaptor.h").is_file()
            ),
            # nvcc auto-preincludes cuda_runtime.h for every .cu translation
            # unit (cu-bridge conf.json condition parity: --include
            # cuda_runtime.h). Sources that include it themselves hit the
            # include guard, so this is always safe to inject.
            "source_is_cuda_and_runtime_header": source_is_cuda and bool(
                self.cucc_path and (self.cucc_path / "include/cuda_runtime.h").is_file()
            ),
            "link_stage": phase in ("link", "compile_and_link"),
            "has_cuda_arch": explicit_cuda_arch,
            # CU2CCE_ARCH_MACRO=auto derives the macro from the dropped
            # -gencode/-arch value; mutually exclusive with the explicit env.
            "arch_macro_auto": (
                self.arch_macro_mode == "auto"
                and not explicit_cuda_arch
                and self.cuda_arch_value is not None
            ),
            # cu-bridge forwards a single -std: an explicit user value wins
            # over the injected -std=gnu++17 (avoids duplicate -std flags on
            # the final ccec command line).
            "no_user_std": not user_has_std,
            "use_gopenmp": use_gopenmp,
        }

    def _resolve_simt_root(self) -> None:
        if self.env.get("SIMT_API_ROOT"):
            return
        import glob
        candidates = [
            "/usr/local/Ascend/cann-*/x86_64-linux/asc/include",
            f"{self.ascend_toolkit_home}/x86_64-linux/asc/include",
            "/usr/local/Ascend/cann-*/aarch64-linux/asc/include",
        ]
        for pattern in candidates:
            for path in glob.glob(pattern):
                if (Path(path) / "simt_api").is_dir():
                    self.env["SIMT_API_ROOT"] = path
                    return

    def _variables(self) -> Dict[str, str]:
        self._resolve_simt_root()
        simt_root = self.env.get("SIMT_API_ROOT", "")
        ver_major, ver_minor, ver_build, ver_legacy = _cuda_version_parts(self.cucc_cuda_version)
        return {
            "CUCC_PATH": str(self.cucc_path),
            "ASCEND_TOOLKIT_HOME": self.ascend_toolkit_home,
            "CANN_BASE": self.cann_base,
            "CUCC_CUDA_VERSION": self.cucc_cuda_version,
            # nvcc defines the __CUDACC_VER_* family for .cu translation
            # units; derived from the compat version so CUCC_CUDA_VERSION
            # overrides propagate (empty parts disable the inject rule).
            "CUCC_CUDA_VER_MAJOR": ver_major,
            "CUCC_CUDA_VER_MINOR": ver_minor,
            "CUCC_CUDA_VER_BUILD": ver_build,
            "CUCC_CUDA_VER_LEGACY": ver_legacy,
            "CU2CCE_NPU_ARCH": self.npu_arch,
            "CU2CCE_CUDA_ARCH": self.env.get("CU2CCE_CUDA_ARCH", "").strip(),
            # Derived from the dropped -gencode/-arch flags; empty unless
            # CU2CCE_ARCH_MACRO=auto and an architecture was requested, so
            # the inject rule self-disables otherwise.
            "CU2CCE_CUDA_ARCH_EXTRACTED": str(self.cuda_arch_value) if self.cuda_arch_value is not None else "",
            "SIMT_API_ROOT": simt_root,
            "SIMT_API_PARENT": str(Path(simt_root).parent) if simt_root else "",
        }

    def _expand_token(self, token: str, variables: Mapping[str, str]) -> str:
        result = token
        for key, value in variables.items():
            result = result.replace("${" + key + "}", value)
        return result

    def _when(self, condition: str, context: Mapping[str, bool]) -> bool:
        if not condition or condition == "always":
            return True
        return bool(context.get(condition, False))

    def _inject(self, phase: str, context: Mapping[str, bool]) -> List[str]:
        variables = self._variables()
        tokens: List[str] = []
        for rule in self._rules_for("inject", phase):
            if not self._when(str(rule.get("when", "always")), context):
                continue
            values = [self._expand_token(str(token), variables) for token in rule.get("tokens", [])]
            if any(value == "" for value in values):
                continue
            tokens.extend(values)
            self.injected.extend(values)
            self._record(" ".join(values), "inject", rule.get("reason", ""))
        return tokens

    def _is_cuda_shim_library_path(self, argument: str) -> bool:
        if not argument.startswith("-L"):
            return False
        value = argument[2:].strip('"')
        cuda_path = self.env.get("CUDA_PATH", "")
        if cuda_path and value in {
            f"{cuda_path}/lib64",
            f"{cuda_path}/targets/x86_64-linux/lib",
            f"{cuda_path}/lib64/stubs",
        }:
            return True
        return "cu2cce-cmake-shim-" in value and "/cuda/" in value

    def _handle_library_policy(self, argument: str) -> bool:
        if not argument.startswith("-l"):
            return False
        name = argument[2:]
        capabilities = self.rules.get("capabilities", {})
        # Two passes so an exact capability key (cublasLt) wins over an
        # earlier key's loose prefix (cublas) — the message must name the
        # capability actually matched.
        capability = None
        loose = False
        for key, entry in capabilities.items():
            if entry.get("action") == "bridge_inject":
                continue
            if name == key or name.startswith(key + "_"):
                capability = key
                break
        if capability is None:
            for key, entry in capabilities.items():
                if entry.get("action") == "bridge_inject":
                    continue
                # Prefix match kept as a fallback so unknown variants
                # (-lcufftw, -lcublasLtCore, ...) still get an advisory
                # warning instead of silently forwarding, but the message
                # is honest about being heuristic.
                if name.startswith(key):
                    capability = key
                    loose = True
                    break
        if capability is None:
            return False
        entry = capabilities.get(capability, {})
        map_to = entry.get("map_to", "")
        if loose:
            message = (
                f"CUDA library {argument} resembles capability {capability} "
                "(prefix match); no automatic Ascend mapping; preserving it"
            )
        elif map_to:
            message = (
                f"CUDA library {argument}: Ascend mapping route documented -> {map_to}; "
                "the shim is not implemented, so the reference is preserved as-is"
            )
        else:
            message = f"CUDA library {argument} has no automatic Ascend mapping; preserving it"
        if self.strict:
            raise DriverError(message)
        self._warn(message)
        self._record(argument, "forward", message)
        return True

    # Host compiler flags that a large project propagates through
    # -Xcompiler (CMake CUDA_PROPAGATE_HOST_FLAGS). Anything that pins host
    # ABI/machine specificity is dropped with a warning; generic safe flags
    # are forwarded to ccec unchanged.
    _HOST_FLAG_ALLOWED = frozenset({
        "-fPIC", "-fpic", "-fno-exceptions", "-fexceptions",
        "-fno-rtti", "-frtti", "-fno-strict-aliasing", "-fstrict-aliasing",
        "-fvisibility=hidden", "-fvisibility=default",
        "-fno-omit-frame-pointer", "-fomit-frame-pointer",
        "-fno-common", "-fcommon", "-fno-stack-protector", "-fstack-protector",
        "-fno-stack-protector-all", "-fopenmp", "-pthread",
        "-fno-plt", "-fplt", "-fno-asynchronous-unwind-tables",
        "-fno-unwind-tables", "-fno-delete-null-pointer-checks",
        "-fno-inline", "-fno-inline-functions",
        "-fno-builtin", "-fno-builtin-memcpy",
        "-funwind-tables", "-fno-sanitize=all", "-fno-ident",
        # Note: the gcc-only var-tracking family is NOT here — it is dropped
        # by the regex_cleanup rules (clang rejects it as unknown argument).
    })

    _HOST_FLAG_DROPPED = frozenset({
        "-m64", "-m32", "-mx32", "-march", "-mtune", "-msse",
        "-msse2", "-msse3", "-mavx", "-mavx2", "-mfma", "-mno-sse",
        "-fPIE", "-fpie", "-no-pie", "-pie", "-static",
    })

    def _split_host_flag(self, flag: str) -> Tuple[str, Optional[str]]:
        """Return (base, consumed-value-token or None) for a host flag."""
        stripped = flag.strip()
        for prefix in ("-march=", "-mtune=", "-msse", "-msse2=", "-msse3=",
                       "-mavx=", "-mavx2=", "-mfma=", "-mno-sse", "-fPIE=",
                       "-fpie=", "-no-pie", "-pie", "-static"):
            if stripped.startswith(prefix):
                return prefix.rstrip("="), None
        return stripped, None

    def _conda_link_flags(self) -> List[str]:
        """Conda link flags honoured by cu-bridge-style drivers.

        When the build runs inside a conda environment (CONDA_EXE is set) and
        the caller did not already pass LDFLAGS on the command line, append the
        environment LDFLAGS so conda's sysroot/rpath survives the injection of
        the bridge link flags."""
        if not self.env.get("CONDA_EXE"):
            return []
        if not self.env.get("LDFLAGS"):
            return []
        ldflags = self.env["LDFLAGS"].strip()
        if not ldflags:
            return []
        if any(flag == "LDFLAGS" for action in self.actions for flag in (action.argument,)):
            return []
        try:
            return shlex.split(ldflags, posix=True)
        except ValueError:
            return ldflags.split()

    def _handle_host_flag(self, argument: str, values: Sequence[str]) -> Tuple[bool, List[str]]:
        """Filter -Xcompiler-propagated host flags one by one.

        cu-bridge filters per argument (bin/cucc inline -Xcompiler handling
        plus the conf_regex.yaml cleanup); a whole-group decision would leak
        machine-specific flags that share the group with safe ones
        (-Xcompiler -fPIC -march=native). Returns (handled, kept-flags):
        handled is always True — every flag is classified here so nothing
        falls through to the generic rule pipeline unfiltered.
        """
        kept: List[str] = []
        for flag in values:
            base, _ = self._split_host_flag(flag)
            if base in self._HOST_FLAG_DROPPED or base.startswith("-m"):
                self._warn(
                    f"dropping host-specific flag {flag}: host ABI/machine "
                    "flags are not applied to the Ascend device compile"
                )
                self._record(flag, "drop", f"host-specific flag {flag} is not applied to ccec")
                continue
            if base == "-Werror" or base.startswith("-Werror="):
                if base.startswith("-Werror=implicit-fallthrough"):
                    # cu-bridge bin/cucc:250-280: the nvcc =N level spelling
                    # is rewritten to the clang-compatible form.
                    kept.append("-Werror=implicit-fallthrough")
                    self._record(
                        flag, "replace",
                        "implicit-fallthrough normalized to the clang spelling "
                        "(cu-bridge bin/cucc parity)",
                        ["-Werror=implicit-fallthrough"],
                    )
                    continue
                # conf_regex.yaml C500S-11780 parity: -Werror and nvcc
                # warning names carried inside -Xcompiler are noise for the
                # clang-based backend; drop instead of forwarding.
                self._record(
                    flag, "drop",
                    "-Werror in -Xcompiler values is removed (cu-bridge "
                    "conf_regex.yaml parity)",
                )
                continue
            if base in ("-Wno-nonnull-compare", "-fno-canonical-system-headers"):
                # gcc-only spellings cu-bridge strips from -Xcompiler inline
                # (bin/cucc:55-83); ccec/clang rejects or warns on them.
                self._record(
                    flag, "drop",
                    f"{base} is a gcc-only spelling; filtered (cu-bridge bin/cucc:55-83 parity)",
                )
                continue
            self._record(flag, "forward", f"host flag {flag} forwarded to ccec")
            kept.append(flag)
        return True, kept

    def _find_query_rule(self, args: Sequence[str]) -> Optional[Dict[str, Any]]:
        for rule in self.rules.get("queries", []):
            pattern = rule.get("pattern", rule.get("match"))
            for argument in args:
                if self._match(pattern, argument):
                    return rule
        return None

    def _prepare_query_passthrough(self, expanded: List[str], rule: Dict[str, Any]) -> PreparedCommand:
        """cu-bridge skipstring semantics: a version/toolchain probe bypasses
        the whole rule pipeline and is handed to the backend compiler
        unchanged (no -x asc, no injected includes, no cmake identity) so
        autoconf/libtool-style probes see the compiler's own answers."""
        matched = next(
            argument for argument in expanded
            if self._match(rule.get("pattern", rule.get("match")), argument)
        )
        self._record(matched, "query_passthrough", rule.get("reason", "probe query passes through to ccec"))
        prepared = PreparedCommand(
            phase="query",
            mode="query",
            args=list(expanded),
            compiler=self._compiler_path(),
            inputs=[],
            actions=self.actions,
            injected=[],
            rules_file=str(self.rules_path),
            source_is_cuda=False,
        )
        self._write_trace(prepared)
        return prepared

    def _apply_regex_cleanup(self, args: List[str]) -> List[str]:
        """Final-command cleanup pass (gomxccbin + conf_regex.yaml parity).

        cu-bridge post-processes the assembled command string with regex
        rules (bug-fix layer, e.g. C500S-11780) right before execution. The
        single-process equivalent: data-driven PCRE rules from the rules
        file applied to the final argument list. Patterns are anchored
        seeds; a rule without "replace" drops the argument, one with
        "replace" rewrites it in place. Query passthroughs skip this pass.
        """
        rules = self.rules.get("regex_cleanup") or []
        if not rules:
            return args
        cleaned: List[str] = []
        for arg in args:
            current = arg
            dropped = False
            for rule in rules:
                pattern = str(rule.get("pattern", ""))
                try:
                    matched = re.search(pattern, current)
                except re.error:
                    continue  # validator rejects broken patterns
                if not matched:
                    continue
                if "replace" in rule:
                    try:
                        rewritten = re.sub(pattern, str(rule["replace"]), current)
                    except re.error:
                        continue
                    if rewritten != current:
                        self._record(current, "regex_replace", rule.get("reason", ""), [rewritten])
                        current = rewritten
                else:
                    self._record(current, "regex_drop", rule.get("reason", ""))
                    dropped = True
                    break
            if not dropped:
                cleaned.append(current)
        return cleaned

    def _locate_real_gcc(self) -> Optional[str]:
        """Path of the real host gcc, skipping cu2cce shim PATH entries."""
        if Path("/usr/bin/gcc").is_file():
            return "/usr/bin/gcc"
        for entry in self.env.get("PATH", "").split(os.pathsep):
            if not entry:
                continue
            lowered = entry.lower()
            if "cu2cce" in lowered:
                continue
            if self.cucc_path and entry.rstrip("/") in {
                str(self.cucc_path), str(self.cucc_path / "bin"),
            }:
                continue
            candidate = Path(entry) / "gcc"
            if candidate.is_file():
                return str(candidate)
        return None

    def _gcc_toolchain_flags(self) -> List[str]:
        """cu-bridge bin/cucc add_gcc_toolchain parity (cucc:1055-1066).

        When the gcc visible behind the shim is not the standard /usr/bin/gcc
        (shim wrapper in front, or devtoolset-only machines), append
        --gcc-toolchain=<real gcc prefix>/ so the clang-based backend's host
        pass finds libgcc/std headers. No-op on the standard layout;
        CU2CCE_GCC_TOOLCHAIN=off disables (knob validated in __init__)."""
        if self.gcc_toolchain_mode in ("off", "0", "false", "none"):
            return []
        real_gcc = self._locate_real_gcc()
        if not real_gcc or real_gcc == "/usr/bin/gcc":
            return []
        prefix = Path(real_gcc).parent.parent
        flag = f"--gcc-toolchain={prefix}/"
        self._record(flag, "inject", "real gcc prefix located behind the shim "
                                     "(cu-bridge add_gcc_toolchain parity)")
        return [flag]

    def _transform_arguments(
        self, expanded: Sequence[str], phase: str
    ) -> List[str]:
        """Apply the ordered per-argument rule pipeline.

        Keep this stage free of injection/final-command concerns: it only
        classifies the expanded argv and records the same actions/warnings as
        the original prepare loop.
        """
        result: List[str] = []
        i = 0
        while i < len(expanded):
            argument = expanded[i]
            consumed = 1

            if argument == "-L" and i + 1 < len(expanded) and self._is_cuda_shim_library_path("-L" + expanded[i + 1]):
                self._record(argument + " " + expanded[i + 1], "drop", "temporary CUDA shim library path")
                i += 2
                continue
            if self._is_cuda_shim_library_path(argument):
                self._record(argument, "drop", "temporary CUDA shim library path")
                i += 1
                continue

            if argument == "--Werror":
                # nvcc's double-dash --Werror consumes a warning NAME as the
                # next token; ccec/clang has no such spelling and the bare
                # name would leak into ccec as a stray input (cu-bridge
                # bin/cucc:84-116 implements the same pair logic).
                consumed = 1
                if i + 1 < len(expanded) and not expanded[i + 1].startswith("-"):
                    consumed = 2
                self._warn(
                    f"ignored {argument}: nvcc double-dash warning-control is not "
                    "accepted by ccec; use -Werror=<name> (single dash)"
                )
                self._record(argument, "drop", "nvcc --Werror spelling with warning name")
                i += consumed
                continue

            if argument == "-Xlinker" and i + 1 < len(expanded):
                # clang-native linker option; cu-bridge forwards it to mxcc
                # unchanged. Dropping the pair silently lost user link
                # options (rpath, scripts, ...), so keep it intact.
                self._record(
                    f"{argument} {expanded[i + 1]}", "forward",
                    "clang-native linker option passthrough (cu-bridge parity)",
                )
                result.append(argument)
                result.append(expanded[i + 1])
                i += 2
                continue

            if argument.startswith("-I/usr/local/cuda"):
                # Stale CUDA toolkit include paths: no CUDA toolkit exists on
                # this machine and bridge headers are force-prepended anyway
                # (mirrors the -L shim-path drop above and cu-bridge
                # conf.json's "-I/usr/local/cuda*" filter).
                self._record(argument, "drop", "stale CUDA toolkit include path")
                i += 1
                continue

            if self._handle_library_policy(argument):
                result.append(argument)
                i += 1
                continue

            reject = self._find_rule("reject", phase, argument)
            if reject:
                reason = reject.get("reason", "unsupported CUDA feature")
                self._record(argument, "reject", reason)
                raise DriverError(f"{argument}: {reason}")

            replace = self._find_rule("replace", phase, argument)
            if replace:
                reason = replace.get("reason", "rule replacement")
                values: List[str] = []
                if replace.get("takes_value") and i + 1 < len(expanded):
                    consumed = 2
                    value = expanded[i + 1]
                    if replace.get("split_value"):
                        values = self._split_forwarded(value)
                    elif replace.get("preserve_value"):
                        raw_to = replace.get("to", [])
                        prefix = raw_to if isinstance(raw_to, list) else [raw_to]
                        values = [str(token) for token in prefix] + [value]
                    else:
                        raw_to = replace.get("to", value)
                        values = [str(raw_to)] if not isinstance(raw_to, list) else [str(x) for x in raw_to]
                elif replace.get("preserve_value") and "=" in argument:
                    raw_to = replace.get("to", [])
                    prefix = raw_to if isinstance(raw_to, list) else [raw_to]
                    values = [str(token) for token in prefix] + [argument.split("=", 1)[1]]
                elif replace.get("split_value") and "=" in argument:
                    values = self._split_forwarded(argument.split("=", 1)[1])
                else:
                    raw = replace.get("to", argument)
                    values = [str(raw)] if not isinstance(raw, list) else [str(x) for x in raw]
                # -Xcompiler/--compiler-options carry host flags (CMake
                # CUDA_PROPAGATE_HOST_FLAGS). Drop machine-specific ones that
                # would break the Ascend device compile, keep safe ones. Both
                # the two-token form (-Xcompiler value) and the attached form
                # (-Xcompiler=values) must go through the per-flag filter.
                if argument in ("-Xcompiler", "--compiler-options") or \
                        argument.startswith(("-Xcompiler=", "--compiler-options=")):
                    handled, filtered = self._handle_host_flag(argument, values)
                    if handled:
                        self._record(argument, "replace", reason, filtered)
                        result.extend(filtered)
                        i += consumed
                        continue
                self._record(argument, "replace", reason, values)
                result.extend(values)
                i += consumed
                continue

            drop = self._find_rule("drop", phase, argument)
            if drop:
                reason = drop.get("reason", "filtered by cu2cce rule")
                if drop.get("takes_value") and i + 1 < len(expanded):
                    consumed = 2
                self._warn(f"ignored {argument}: {reason}")
                self._record(argument, "drop", reason)
                i += consumed
                continue

            forward = self._find_rule("forward", phase, argument)
            if forward:
                reason = forward.get("reason", "ordinary compiler argument")
                self._record(argument, "forward", reason)

            if self.strict and argument.startswith("--") and not forward:
                raise DriverError(f"unrecognized strict-mode CUDA option: {argument}")

            # A CUDA library with no validated mapping is preserved with a
            # warning by _handle_library_policy; ordinary -l and compiler
            # options pass through unchanged.
            result.append(argument)
            i += 1

        return result

    def _order_injections(
        self, injections: List[str], source_is_cuda: bool
    ) -> Tuple[List[str], List[str]]:
        """Split injected arguments into a command prefix and suffix.

        CUDA language selection and compatibility include pairs must precede
        user arguments; all other injections stay after them.
        """
        prefix: List[str] = []
        if source_is_cuda:
            # The language selector must precede the CUDA source. Consume the
            # rule-provided pair here so it is emitted exactly once rather
            # than both as a prefix and as a trailing injection.
            try:
                language_index = next(
                    index for index in range(len(injections) - 1)
                    if injections[index:index + 2] == ["-x", "asc"]
                )
            except StopIteration:
                language_index = -1
            if language_index >= 0:
                injections = (
                    injections[:language_index]
                    + injections[language_index + 2:]
                )
            prefix = ["-x", "asc"]

        # Bridge compatibility headers must precede user/CANN include paths.
        # Keep all injected -I/-include pairs in a deterministic front
        # section, while leaving scalar injections in the trailing section.
        include_prefix: List[str] = []
        remaining: List[str] = []
        i = 0
        while i < len(injections):
            if injections[i] in ("-I", "-include") and i + 1 < len(injections):
                include_prefix.extend(injections[i:i + 2])
                i += 2
                continue
            remaining.append(injections[i])
            i += 1
        return [*prefix, *include_prefix], remaining

    def _finalize_prepared_command(
        self,
        result: List[str],
        phase: str,
        mode: str,
        inputs: List[str],
        output: Optional[str],
        source_is_cuda: bool,
    ) -> PreparedCommand:
        """Apply final link/env cleanup and construct the traceable command."""
        if output and output.endswith(".o") and "-c" not in result:
            result.insert(0, "-c")

        compiler = self._compiler_path()
        conda_ldflags = self._conda_link_flags()
        if conda_ldflags and phase in ("link", "compile_and_link"):
            # conda-packaged CUDA projects carry LDFLAGS pointing at the conda
            # sysroot/rpath. The driver's injected link line must observe them;
            # defer to the caller's environment when no explicit LDFLAGS given.
            self._record("LDFLAGS", "forward", "conda LDFLAGS appended to link")
            result.extend(conda_ldflags)

        # Final-command passes (cu-bridge runs them right before executing
        # the backend): gcc-toolchain injection, then data-driven regex
        # cleanup over the assembled command.
        result.extend(self._gcc_toolchain_flags())
        result = self._apply_regex_cleanup(result)
        prepared = PreparedCommand(
            phase=phase,
            mode=mode,
            args=result,
            compiler=compiler,
            inputs=inputs,
            actions=self.actions,
            injected=self.injected,
            rules_file=str(self.rules_path),
            source_is_cuda=source_is_cuda,
        )
        self._write_trace(prepared)
        if self.debug_enabled:
            self._debug(f"phase={phase}")
            self._debug(f"inputs={inputs}")
            self._debug("final command: " + " ".join([compiler, *result]))
        return prepared

    def prepare(self, argv: Sequence[str]) -> PreparedCommand:
        expanded = self._expand_response_files(list(argv))
        # --cmake-identity is accepted for compatibility but is now a no-op:
        # the __CUDACC__/__CUDACC_VER_* macro set moved into the rules-file
        # inject section (source_is_cuda, values derived from
        # CUCC_CUDA_VERSION), which covers the mock probe path and real
        # Makefile-driven cucc compiles alike — one definition site instead
        # of two (simplifier pass, seventh batch).
        query_rule = self._find_query_rule(expanded)
        if query_rule is not None:
            return self._prepare_query_passthrough(expanded, query_rule)

        # Read-only recovery of the GPU architecture the build requested.
        # The arch flags stay dropped; the value only feeds __CUDA_ARCH__
        # derivation (CU2CCE_ARCH_MACRO=auto) and the decision trace.
        self.cuda_arch_value = extract_cuda_arch(expanded)
        phase, mode, inputs, output, source_is_cuda = self._phase(expanded)
        if mode == "unsupported":
            raise DriverError(
                "CUDA assembly output (-S) is not produced by the ccec puresimt "
                "driver; use cucc -c to compile objects"
            )
        self._resolve_simt_root()
        context = self._context(phase, source_is_cuda, expanded)
        if (
            self.arch_macro_mode == "auto"
            and self.cuda_arch_value is None
            and source_is_cuda
            and not self.env.get("CU2CCE_CUDA_ARCH", "").strip()
        ):
            self._warn(
                "CU2CCE_ARCH_MACRO=auto but the command line carries no "
                "-gencode/-arch value; __CUDA_ARCH__ is not defined. Pass "
                "-gencode or set CU2CCE_CUDA_ARCH for code that branches on it."
            )
        if source_is_cuda and self.fp64_policy != "allow" and phase == "compile":
            self._pre_flight_report_device_double(inputs)

        result = self._transform_arguments(expanded, phase)
        injections = self._inject(phase, context)
        prefix, suffix = self._order_injections(injections, source_is_cuda)
        result = [*prefix, *result, *suffix]
        return self._finalize_prepared_command(
            result, phase, mode, inputs, output, source_is_cuda
        )

    def _compiler_path(self) -> str:
        configured = self.env.get("CCEC_PATH", "")
        if configured and Path(configured).is_file():
            return configured
        candidates = [
            f"{self.cann_base}/bin/ccec",
            "/usr/local/Ascend/latest/bin/ccec",
        ]
        import glob
        candidates.extend(glob.glob("/usr/local/Ascend/cann-*/bin/ccec"))
        for candidate in candidates:
            if Path(candidate).is_file():
                return candidate
        return configured or f"{self.cann_base}/bin/ccec"

    def _write_trace(self, prepared: PreparedCommand) -> None:
        path = self.env.get("CU2CCE_TRACE_FILE", "")
        if not path:
            return
        payload = {
            "phase": prepared.phase,
            "mode": prepared.mode,
            "inputs": prepared.inputs,
            "source_is_cuda": prepared.source_is_cuda,
            "libc_mode": self.libc_mode,
            "rules_file": prepared.rules_file,
            "response_files": list(self.response_files),
            "actions": [
                {
                    "argument": action.argument,
                    "action": action.action,
                    "reason": action.reason,
                    "replacement": action.replacement,
                }
                for action in prepared.actions
            ],
            "injected": prepared.injected,
            "final_compiler": prepared.compiler,
            "final_args": prepared.args,
        }
        trace_path = Path(path).expanduser()
        trace_path.parent.mkdir(parents=True, exist_ok=True)
        trace_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    def _pre_flight_report_device_double(self, inputs: Sequence[str]) -> None:
        """Proactively surface device-side double usage before ccec runs.

        Mentor guidance (2026-08-27): unsupported device types must be
        reported with actionable "downgrade to float" advice; the user then
        edits their source manually. error policy stops the build here;
        warn prints and continues (letting ccec produce its own errors);
        allow skips the scan entirely.
        """
        any_findings = False
        for path_text in inputs:
            path = Path(path_text)
            if path.suffix != ".cu":
                continue
            try:
                source = path.read_text(encoding="utf-8", errors="replace")
            except OSError:
                if self.debug_enabled:
                    print(f"[type-policy] could not read {path}; FP64 scan skipped", file=sys.stderr)
                continue
            findings = scan_device_double_usage(source)
            if not findings:
                continue
            any_findings = True
            print(_TYPE_POLICY_MESSAGE_HEAD, file=sys.stderr)
            print(_format_device_double_report(path, findings), file=sys.stderr)
        if not any_findings or self.fp64_policy != "error":
            return
        raise DriverError(
            "[type-policy] build stopped: CU2CCE_DEVICE_FP64_POLICY=error and "
            "device-side double usage was reported above. Downgrade those "
            "sites to float, or rerun with CU2CCE_DEVICE_FP64_POLICY=warn/allow."
        )

    def _emit_capability_diagnostics(self, stderr_text: str) -> None:
        if self.fp64_policy == "allow":
            return
        fp64_markers = (
            "double precision operation is not allowed in aicore function",
            "Input type T only supports",
            "double is not supported",
        )
        if not any(marker in stderr_text for marker in fp64_markers):
            return
        message = (
            "[type-policy] cu2cce: device-side FP64/double is unsupported by the "
            "current Ascend SIMT/AICore target. Host-side double remains supported; change the "
            "device-side value/accumulator to float, or provide an explicit host "
            "fallback/mixed-precision implementation."
        )
        print(message, file=sys.stderr)
        if self.fp64_policy == "warn":
            print("cu2cce: continuing because CU2CCE_DEVICE_FP64_POLICY=warn", file=sys.stderr)

    def run_prepared(self, prepared: PreparedCommand) -> int:
        if not Path(prepared.compiler).is_file():
            print(
                f"cucc: ccec not found at {prepared.compiler}; set CCEC_PATH or install CANN",
                file=sys.stderr,
            )
            return 1
        process = subprocess.Popen(
            [prepared.compiler, *prepared.args],
            env=self.env,
            stdout=None,
            stderr=subprocess.PIPE,
            text=True,
        )
        stderr_lines: List[str] = []
        assert process.stderr is not None
        for line in process.stderr:
            sys.stderr.write(line)
            sys.stderr.flush()
            stderr_lines.append(line)
        return_code = process.wait()
        if return_code != 0:
            self._emit_capability_diagnostics("".join(stderr_lines))
        return return_code

    def run(self, argv: Sequence[str]) -> int:
        return self.run_prepared(self.prepare(argv))


    def run_hostcc(self, args: Sequence[str], name: str, proxy_mode: bool = False) -> int:
        """bin/gnu-parity host compiler wrapper driven by config/cu2cce_rules.json.

        The fake toolkit binds bin/{c++,g++,gcc,cc} to hostcc_cce, which
        delegates here. Pure compiles/preprocess/queries pass through to the
        real system compiler untouched; link commands append the
        hostcc.link.inject rule set (bridge runtime + CANN acl), matching
        cu-bridge's conf_gnu.json link adders. Device sources (.cu) are
        refused: they must go through cucc.
        """
        if not name:
            name = "g++"
        for argument in args:
            if not argument.startswith("-") and Path(argument).suffix == ".cu":
                print(
                    f"cu2cce hostcc: '{argument}' is a CUDA source; compile it "
                    "with cucc (device code never goes through the host compiler)",
                    file=sys.stderr,
                )
                return 2
        compiler = f"/usr/bin/{name}"
        if not Path(compiler).is_file():
            import shutil  # noqa: PLC0415
            resolved = shutil.which(name)
            if not resolved:
                print(f"cu2cce hostcc: host compiler '{name}' not found", file=sys.stderr)
                return 1
            compiler = resolved

        has_c = "-c" in args
        has_e = "-E" in args
        is_query = any(arg in ("--version", "-dumpversion", "-v", "--help") for arg in args)
        has_out = any(arg == "-o" or (arg.startswith("-o") and len(arg) > 2) for arg in args)
        link_phase = not has_c and not has_e and not is_query

        if is_query or has_c or has_e or not has_out:
            # compile-only / preprocess / query / no explicit output: keep the
            # real compiler's own semantics untouched.
            if proxy_mode:
                print(shlex.join([compiler, *args]))
                return 0
            os.execv(compiler, [compiler, *args])

        injections: List[str] = []
        if self.cucc_path and self.cucc_path.is_dir():
            variables = self._variables()
            for rule in self.rules.get("hostcc", {}).get("link", {}).get("inject", []):
                if not self._when(str(rule.get("when", "always")), {
                    "has_cucc": True,
                    "link_stage": True,
                }):
                    continue
                values = [self._expand_token(str(token), variables) for token in rule.get("tokens", [])]
                if any(value == "" for value in values):
                    continue
                injections.extend(values)
                self._record(" ".join(values), "inject", rule.get("reason", "hostcc link inject"))
        if proxy_mode:
            print(shlex.join([compiler, *args, *injections]))
            return 0
        os.execv(compiler, [compiler, *args, *injections])


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = list(sys.argv[1:] if argv is None else argv)
    cmake_identity = False
    proxy_mode = False
    hostcc_mode = False
    hostcc_name = ""
    # Driver-level flags are stripped from the front in any order; the first
    # non-driver token starts the compiler argument list.
    while args:
        if args[0] == "--cmake-identity":
            cmake_identity = True
            args = args[1:]
        elif args[0] == "--proxy-mode":
            proxy_mode = True
            args = args[1:]
        elif args[0] == "--hostcc-mode":
            hostcc_mode = True
            args = args[1:]
        elif args[0] == "--hostcc-name":
            hostcc_name = args[1] if len(args) > 1 else ""
            args = args[2:]
        else:
            break
    if args and args[0] == "--":
        args = args[1:]
    try:
        if hostcc_mode:
            return RuleDriver(cmake_identity=False).run_hostcc(args, hostcc_name, proxy_mode=proxy_mode)
        if "--version" in args or "-V" in args:
            # Banner carries the parseable "compatible with release" token
            # (cu-bridge's -V banner does the same) so scripts probing
            # `nvcc --version` keep working; identity stays honest about
            # being the cu2cce driver.
            print("cucc: cu2cce-bridge rule-driven CUDA driver, "
                  "compatible with release 11.6, V11.6.0")
            print("backend ccec/puresimt; CUCC_CUDA_VERSION controls the emulated release")
            return 0
        if "--help" in args or "-help" in args:
            print("usage: cucc [nvcc-compatible options] input.cu [-o output]")
            print("environment: CU2CCE_NPU_ARCH, CU2CCE_RULES_FILE, CU2CCE_DEBUG, CU2CCE_TRACE_FILE")
            return 0
        dryrun = "--dryrun" in args or "-dryrun" in args
        if dryrun:
            args = [arg for arg in args if arg not in {"--dryrun", "-dryrun"}]
        driver = RuleDriver(cmake_identity=cmake_identity)
        prepared = driver.prepare(args)
        if dryrun:
            print(" ".join([prepared.compiler, *prepared.args]))
            return 0
        if proxy_mode:
            # Emit the prepared command as a single shell-quoted line without
            # executing ccec. The parent wrapper (mock_nvcc or a test harness)
            # decides whether and how to execute the real compile.
            print(shlex.join([prepared.compiler, *prepared.args]))
            return 0
        return driver.run_prepared(prepared)
    except DriverError as exc:
        print(f"cu2cce: error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
