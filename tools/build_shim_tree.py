#!/usr/bin/env python3
"""兼容入口：转发到 ``tools/mock/virtualDir.py``。

主实现已经按 cu-bridge 的目录习惯迁移到 mock 子目录。本文件保留旧文件名，
但不包含第二套目录创建、清单解析或链接逻辑，也不使用软链接。
"""
from __future__ import annotations

import importlib.util
from pathlib import Path
from types import ModuleType

_IMPL_PATH = Path(__file__).resolve().parent / "mock" / "virtualDir.py"
_SPEC = importlib.util.spec_from_file_location("cu2cce_virtualDir", _IMPL_PATH)
if _SPEC is None or _SPEC.loader is None:  # pragma: no cover
    raise ImportError(f"cannot load {_IMPL_PATH}")
_IMPL: ModuleType = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(_IMPL)

host_triple = _IMPL.host_triple
cuda_version_display = _IMPL.cuda_version_display
load_layout = _IMPL.load_layout
build_virtual_toolkit = _IMPL.build_virtual_toolkit
build_shim = _IMPL.build_virtual_toolkit

def main(argv=None):
    return _IMPL.main(argv)

if __name__ == "__main__":
    raise SystemExit(main())
