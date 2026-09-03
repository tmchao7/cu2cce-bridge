#!/usr/bin/env python3
"""安全的 cu-bridge 风格 argv 过滤/替换辅助工具。

主线 `cucc` 使用 `tools/cu2cce_driver.py`，不会调用本工具。该入口只是为需要
`bin/process_args.py` 路径的迁移脚本提供无 shell 展开的参数处理能力。
"""
from __future__ import annotations

import argparse
import fnmatch
import json
from typing import Iterable, List, Optional, Sequence


def apply(argv: Iterable[str], filters: Iterable[str], replacements: Iterable[str]) -> List[str]:
    """按 glob 过滤参数，并按 ``old=new`` 做单 token 替换。"""
    filter_patterns = tuple(filters)
    replacement_map = {}
    for item in replacements:
        if "=" in item:
            old, new = item.split("=", 1)
            replacement_map[old] = new
    result = []
    for value in argv:
        if any(fnmatch.fnmatchcase(value, pattern) for pattern in filter_patterns):
            continue
        result.append(replacement_map.get(value, value))
    return result


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--filter", dest="filters", action="append", default=[], metavar="GLOB")
    parser.add_argument("--replace", dest="replacements", action="append", default=[], metavar="OLD=NEW")
    parser.add_argument("--json", action="store_true", help="以 JSON 数组输出，保留 argv 边界")
    parser.add_argument("args", nargs="*", help="要处理的参数")
    ns = parser.parse_args(argv)
    result = apply(ns.args, ns.filters, ns.replacements)
    if ns.json:
        print(json.dumps(result, ensure_ascii=False))
    else:
        # 默认输出仅适合人读；需要继续传递 argv 时请使用 --json。
        print(" ".join(result))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
