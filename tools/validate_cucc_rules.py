#!/usr/bin/env python3
"""Validate the versioned cu2cce argument policy file."""
from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any, Dict, List

ACTIONS = {"forward", "drop", "replace", "inject", "reject"}
PHASES = {"all", "compile", "link", "compile_and_link"}
CAPABILITY_STATUSES = {"supported_by_bridge", "experimental", "unsupported", "map_available"}
CAPABILITY_ACTIONS = {"bridge_inject", "warn_no_automatic_replacement", "drop", "reject"}
REQUIRED_TOP = {"schema_version", "frontend", "all", "compile", "link", "compile_and_link", "capabilities", "diagnostics"}


def fail(errors: List[str], message: str) -> None:
    errors.append(message)


def validate_rule(rule: Any, action: str, path: str, errors: List[str]) -> None:
    if not isinstance(rule, dict):
        fail(errors, f"{path}: rule must be an object")
        return
    if action in {"forward", "drop", "replace", "reject"}:
        if "pattern" not in rule and "match" not in rule:
            fail(errors, f"{path}: {action} requires pattern")
    if action == "replace":
        if not rule.get("split_value") and "to" not in rule and not rule.get("takes_value"):
            fail(errors, f"{path}: replace requires to, takes_value, or split_value")
    if action == "inject":
        tokens = rule.get("tokens")
        if not isinstance(tokens, list) or not tokens or not all(isinstance(x, str) for x in tokens):
            fail(errors, f"{path}: inject requires a non-empty string tokens list")
    if action == "reject" and not rule.get("reason"):
        fail(errors, f"{path}: reject requires reason")
    if "takes_value" in rule and not isinstance(rule["takes_value"], bool):
        fail(errors, f"{path}: takes_value must be boolean")
    if "optional_value" in rule and not isinstance(rule["optional_value"], bool):
        fail(errors, f"{path}: optional_value must be boolean")
    if rule.get("optional_value") and not rule.get("takes_value"):
        fail(errors, f"{path}: optional_value requires takes_value=true")
    if "preserve_value" in rule and not isinstance(rule["preserve_value"], bool):
        fail(errors, f"{path}: preserve_value must be boolean")
    if rule.get("preserve_value") and "to" not in rule:
        fail(errors, f"{path}: preserve_value requires to")
    if "when" in rule and not isinstance(rule["when"], str):
        fail(errors, f"{path}: when must be string")


def validate(path: Path) -> List[str]:
    errors: List[str] = []
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:  # noqa: BLE001
        return [f"cannot parse {path}: {exc}"]
    if not isinstance(data, dict):
        return ["top-level rules value must be an object"]
    missing = REQUIRED_TOP - set(data)
    for key in sorted(missing):
        fail(errors, f"missing top-level key: {key}")
    if data.get("schema_version") != 1:
        fail(errors, f"schema_version must be 1, got {data.get('schema_version')!r}")
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
    # Optional cu-bridge skipstring-parity section: probe queries that bypass
    # the whole rule pipeline and pass through to ccec unchanged.
    if "queries" in data:
        queries = data["queries"]
        if not isinstance(queries, list):
            fail(errors, "queries: must be a list")
        else:
            for index, rule in enumerate(queries):
                if not isinstance(rule, dict) or "pattern" not in rule and "match" not in rule:
                    fail(errors, f"queries[{index}]: requires pattern")
                    continue
                if not isinstance(rule.get("reason", ""), str):
                    fail(errors, f"queries[{index}]: reason must be a string")
    # Optional final-command cleanup layer (gomxccbin/conf_regex.yaml
    # parity): PCRE rules applied to the assembled command arguments.
    if "regex_cleanup" in data:
        import re  # noqa: PLC0415
        cleanup = data["regex_cleanup"]
        if not isinstance(cleanup, list):
            fail(errors, "regex_cleanup: must be a list")
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
    # Optional host compiler wrapper section (bin/gnu parity): phase-scoped
    # actions with the same rule shapes as the main pipeline.
    if "hostcc" in data:
        hostcc = data["hostcc"]
        if not isinstance(hostcc, dict):
            fail(errors, "hostcc: must be an object")
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
        fail(errors, "capabilities: must be an object")
    else:
        for name, entry in capabilities.items():
            path_prefix = f"capabilities.{name}"
            if not isinstance(entry, dict):
                fail(errors, f"{path_prefix}: must be an object")
                continue
            status = entry.get("status")
            if status not in CAPABILITY_STATUSES:
                fail(errors, f"{path_prefix}.status must be one of {sorted(CAPABILITY_STATUSES)}")
            action = entry.get("action")
            if action not in CAPABILITY_ACTIONS:
                fail(errors, f"{path_prefix}.action must be one of {sorted(CAPABILITY_ACTIONS)}")
            if "map_to" in entry and not isinstance(entry["map_to"], str):
                fail(errors, f"{path_prefix}.map_to must be a string")
    if not isinstance(data.get("diagnostics"), dict):
        fail(errors, "diagnostics: must be an object")
    return errors


def main(argv: List[str]) -> int:
    path = Path(argv[1]) if len(argv) > 1 else Path(__file__).resolve().parents[1] / "config/cu2cce_rules.json"
    errors = validate(path)
    if errors:
        for error in errors:
            print(f"cu2cce-rules: ERROR: {error}", file=sys.stderr)
        return 1
    print(f"cu2cce-rules: OK: {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
