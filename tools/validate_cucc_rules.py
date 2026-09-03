#!/usr/bin/env python3
"""Validate the canonical ``bin/conf.json`` cucc policy document."""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path
from typing import Any, Dict, List

ACTIONS = {"forward", "drop", "replace", "inject", "reject"}
PHASES = ("all", "compile", "link", "compile_and_link")
CANONICAL_TOP = {
    "schema_version", "data", "skipstring", "all", "compile", "link",
    "compile_and_link", "condition", "not_found", "cu2cce",
}
CAPABILITY_STATUSES = {"supported_by_bridge", "experimental", "unsupported", "map_available"}
CAPABILITY_ACTIONS = {"bridge_inject", "warn_no_automatic_replacement", "drop", "reject"}


def fail(errors: List[str], message: str) -> None:
    errors.append(message)


def validate_rule(rule: Any, action: str, path: str, errors: List[str]) -> None:
    if not isinstance(rule, dict):
        fail(errors, f"{path}: rule must be an object")
        return
    if action in {"forward", "drop", "replace", "reject"} and "pattern" not in rule and "match" not in rule:
        fail(errors, f"{path}: {action} requires pattern")
    if action == "replace" and not rule.get("split_value") and "to" not in rule and not rule.get("takes_value"):
        fail(errors, f"{path}: replace requires to, takes_value, or split_value")
    if action == "inject":
        tokens = rule.get("tokens")
        if not isinstance(tokens, list) or not tokens or not all(isinstance(x, str) for x in tokens):
            fail(errors, f"{path}: inject requires a non-empty string tokens list")
    if action == "reject" and not rule.get("reason"):
        fail(errors, f"{path}: reject requires reason")
    for key in ("takes_value", "optional_value", "preserve_value"):
        if key in rule and not isinstance(rule[key], bool):
            fail(errors, f"{path}: {key} must be boolean")
    if rule.get("optional_value") and not rule.get("takes_value"):
        fail(errors, f"{path}: optional_value requires takes_value=true")
    if rule.get("preserve_value") and "to" not in rule:
        fail(errors, f"{path}: preserve_value requires to")
    if "when" in rule and not isinstance(rule["when"], str):
        fail(errors, f"{path}: when must be string")


def _canonical_view(data: Dict[str, Any], errors: List[str]) -> Dict[str, Any]:
    """Expose the canonical reference-shaped document as validation sections."""
    missing = CANONICAL_TOP - set(data)
    for key in sorted(missing):
        fail(errors, f"missing top-level key: {key}")
    extension = data.get("cu2cce")
    if not isinstance(extension, dict):
        fail(errors, "cu2cce: must be an object")
        extension = {}
    reject_map = extension.get("reject", {})
    if not isinstance(reject_map, dict):
        fail(errors, "cu2cce.reject: must be an object")
        reject_map = {}
    view: Dict[str, Any] = {
        "schema_version": data.get("schema_version"),
        "frontend": extension.get("frontend", {}),
        "queries": [{"pattern": item} for item in data.get("skipstring", [])],
        "condition": data.get("condition", []),
        "not_found": data.get("not_found", {}),
        "capabilities": extension.get("capabilities", {}),
        "hostcc": extension.get("hostcc", {}),
        "regex_cleanup": extension.get("regex_cleanup", []),
        "diagnostics": extension.get("diagnostics", {}),
    }
    for phase in PHASES:
        section = data.get(phase, {})
        if not isinstance(section, dict):
            section = {}
        view[phase] = {
            "forward": section.get("forward", []),
            "drop": section.get("filter", []),
            "replace": section.get("replacer", []),
            "inject": section.get("adder", []),
            "reject": reject_map.get(phase, []),
        }
    return view


def validate(path: Path) -> List[str]:
    errors: List[str] = []
    try:
        raw = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:  # noqa: BLE001
        return [f"cannot parse {path}: {exc}"]
    if not isinstance(raw, dict):
        return ["top-level rules value must be an object"]
    data = _canonical_view(raw, errors)
    if data.get("schema_version") != 1:
        fail(errors, f"schema_version must be 1, got {data.get('schema_version')!r}")
    if not isinstance(raw.get("data"), str) or not raw.get("data"):
        fail(errors, "data must be a non-empty string")
    if not isinstance(raw.get("skipstring"), list) or not all(isinstance(x, str) for x in raw.get("skipstring", [])):
        fail(errors, "skipstring must be a list of strings")
    if not isinstance(data.get("frontend"), dict):
        fail(errors, "cu2cce.frontend must be an object")
    if not isinstance(data.get("condition"), list):
        fail(errors, "condition must be a list")
    if not isinstance(data.get("not_found"), dict):
        fail(errors, "not_found must be an object")

    for phase in PHASES:
        section = data.get(phase)
        if not isinstance(section, dict):
            fail(errors, f"{phase}: must be an object")
            continue
        seen: Dict[str, str] = {}
        for action in section:
            if action not in ACTIONS:
                fail(errors, f"{phase}: unknown action {action}")
                continue
            rules = section[action]
            if not isinstance(rules, list):
                fail(errors, f"{phase}.{action}: must be a list")
                continue
            for index, rule in enumerate(rules):
                validate_rule(rule, action, f"{phase}.{action}[{index}]", errors)
                if isinstance(rule, dict):
                    pattern = rule.get("pattern", rule.get("match"))
                    if isinstance(pattern, str):
                        previous = seen.get(pattern)
                        if previous and previous != action and {previous, action} & {"drop", "replace", "reject"}:
                            fail(errors, f"{phase}: conflicting actions for pattern {pattern!r}: {previous}/{action}")
                        seen[pattern] = action

    for index, rule in enumerate(data.get("queries", [])):
        if not isinstance(rule, dict) or "pattern" not in rule:
            fail(errors, f"queries[{index}]: requires pattern")
    cleanup = data.get("regex_cleanup")
    if not isinstance(cleanup, list):
        fail(errors, "cu2cce.regex_cleanup must be a list")
    else:
        for index, rule in enumerate(cleanup):
            if not isinstance(rule, dict):
                fail(errors, f"regex_cleanup[{index}]: must be an object")
                continue
            pattern = rule.get("pattern")
            if not isinstance(pattern, str) or not pattern:
                fail(errors, f"regex_cleanup[{index}]: requires a string pattern")
                continue
            try:
                re.compile(pattern)
            except re.error as exc:
                fail(errors, f"regex_cleanup[{index}]: invalid regex: {exc}")
            if not isinstance(rule.get("reason", ""), str):
                fail(errors, f"regex_cleanup[{index}]: reason must be a string")
            if "replace" in rule and not isinstance(rule["replace"], str):
                fail(errors, f"regex_cleanup[{index}]: replace must be a string")

    hostcc = data.get("hostcc")
    if not isinstance(hostcc, dict):
        fail(errors, "cu2cce.hostcc must be an object")
    else:
        for phase, section in hostcc.items():
            if phase == "comment":
                continue
            if not isinstance(section, dict):
                fail(errors, f"hostcc.{phase}: must be an object")
                continue
            for action, rules in section.items():
                if action not in ACTIONS:
                    fail(errors, f"hostcc.{phase}: unknown action {action}")
                    continue
                if not isinstance(rules, list):
                    fail(errors, f"hostcc.{phase}.{action}: must be a list")
                    continue
                for index, rule in enumerate(rules):
                    validate_rule(rule, action, f"hostcc.{phase}.{action}[{index}]", errors)

    capabilities = data.get("capabilities")
    if not isinstance(capabilities, dict):
        fail(errors, "cu2cce.capabilities must be an object")
    else:
        for name, entry in capabilities.items():
            prefix = f"capabilities.{name}"
            if not isinstance(entry, dict):
                fail(errors, f"{prefix}: must be an object")
                continue
            if entry.get("status") not in CAPABILITY_STATUSES:
                fail(errors, f"{prefix}.status must be one of {sorted(CAPABILITY_STATUSES)}")
            if entry.get("action") not in CAPABILITY_ACTIONS:
                fail(errors, f"{prefix}.action must be one of {sorted(CAPABILITY_ACTIONS)}")
            if "map_to" in entry and not isinstance(entry["map_to"], str):
                fail(errors, f"{prefix}.map_to must be a string")
    if not isinstance(data.get("diagnostics"), dict):
        fail(errors, "cu2cce.diagnostics must be an object")
    return errors


def main(argv: List[str]) -> int:
    path = Path(argv[1]) if len(argv) > 1 else Path(__file__).resolve().parents[1] / "bin/conf.json"
    errors = validate(path)
    if errors:
        for error in errors:
            print(f"cu2cce-rules: ERROR: {error}", file=sys.stderr)
        return 1
    print(f"cu2cce-rules: OK: {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
