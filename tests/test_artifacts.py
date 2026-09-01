#!/usr/bin/env python3
"""Unit tests for the build artifact contract verifier."""
from __future__ import annotations

import importlib.util
import json
import os
import stat
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "tools/real_test/verify_artifacts.py"


class ArtifactVerifierTest(unittest.TestCase):
    def load_module(self):
        spec = importlib.util.spec_from_file_location("verify_artifacts", MODULE_PATH)
        self.assertIsNotNone(spec)
        self.assertIsNotNone(spec.loader)
        module = importlib.util.module_from_spec(spec)
        import sys
        sys.modules[spec.name] = module
        spec.loader.exec_module(module)
        return module

    def test_contract_manifest_exists_and_has_all_build_system_cases(self) -> None:
        contract = ROOT / "config/artifact_contract.json"
        self.assertTrue(contract.is_file())
        data = json.loads(contract.read_text(encoding="utf-8"))
        self.assertEqual(data["schema_version"], 1)
        for case in (
            "cucc_direct",
            "cmake_demo",
            "cmake_compat",
            "cmake_compat_legacy",
            "cmake_large_project",
            "cmake_build_compat",
            "cmake_build_compat_ninja",
            "cmake_ninja",
            "make_compat",
            "make_gxx_link",
            "cmake_build_compat_install",
        ):
            self.assertIn(case, data["cases"])

    def test_cmake_makefile_cases_check_link_txt_for_bridge_runtime(self) -> None:
        contract = json.loads((ROOT / "config/artifact_contract.json").read_text(encoding="utf-8"))
        for case_name, link_path in (
            ("cmake_demo", "CMakeFiles/cmake_vector_add.dir/link.txt"),
            ("cmake_compat", "CMakeFiles/cmake_compat_app.dir/link.txt"),
            ("cmake_compat_legacy", "CMakeFiles/legacy_vector_add.dir/link.txt"),
            ("cmake_large_project", "CMakeFiles/large_project_app.dir/link.txt"),
            ("cmake_build_compat", "CMakeFiles/build_compat_app.dir/link.txt"),
        ):
            metadata = {entry["path"]: entry.get("contains", []) for entry in contract["cases"][case_name]["metadata"]}
            self.assertIn(link_path, metadata, case_name)
            self.assertIn("libcce_bridge_runtime", " ".join(metadata[link_path]), case_name)

    def test_ninja_build_compatibility_case_matches_makefile_artifact_shape(self) -> None:
        contract = json.loads((ROOT / "config/artifact_contract.json").read_text(encoding="utf-8"))
        make_case = contract["cases"]["cmake_build_compat"]
        ninja_case = contract["cases"]["cmake_build_compat_ninja"]
        self.assertEqual(
            sorted((entry["path"], entry["kind"]) for entry in make_case["required"]),
            sorted((entry["path"], entry["kind"]) for entry in ninja_case["required"]),
        )
        self.assertIn("CMakeFiles/rules.ninja", {entry["path"] for entry in ninja_case["metadata"]})

    def test_server_artifact_failure_prints_one_summary_and_log_tail(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn('echo "[FAIL] artifact [$case_name] (details: $log)"', server)
        self.assertIn('tail -n 30 "$log"', server)
        self.assertNotIn('tail -n 30 "$error"', server)

    def test_ninja_contract_checks_rules_file_for_cuda_compiler(self) -> None:
        contract = json.loads((ROOT / "config/artifact_contract.json").read_text(encoding="utf-8"))
        metadata = contract["cases"]["cmake_ninja"]["metadata"]
        entries = {entry["path"]: entry.get("contains", []) for entry in metadata}
        self.assertIn("build.ninja", entries)
        self.assertIn("CMakeFiles/rules.ninja", entries)
        self.assertIn("nvcc", entries["CMakeFiles/rules.ninja"])
        self.assertNotIn("nvcc", entries["build.ninja"])

    def test_server_harness_wires_blocking_artifact_checks_for_all_build_systems(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("ARTIFACT_CONTRACT=", server)
        self.assertIn("verify_artifacts.py", server)
        for case in (
            "cucc_direct", "cmake_demo", "cmake_compat",
            "cmake_compat_legacy", "cmake_large_project",
            "cmake_build_compat", "cmake_build_compat_ninja", "cmake_ninja",
            "make_compat", "make_gxx_link", "cmake_build_compat_install",
        ):
            self.assertIn('--case "$case_name"', server, case)
        self.assertIn("examples/build_compat/cmake", server)
        self.assertIn("cmake_build_compat_ninja", server)
        self.assertIn("direct ninja_cce build-compat artifact shape matches CMake Makefiles", server)
        self.assertIn("ARTIFACT_CONTRACT_REGRESSION_PASS", server)
        self.assertIn("ARTIFACT_CONTRACT_REGRESSION_FAIL", server)

    def test_verifier_accepts_macho_independent_dependency_and_archive_contract(self) -> None:
        module = self.load_module()
        with tempfile.TemporaryDirectory() as tmp_name:
            root = Path(tmp_name)
            executable = root / "app"
            executable.write_bytes(b"fake executable")
            executable.chmod(executable.stat().st_mode | stat.S_IXUSR)
            archive = root / "libdemo.a"
            archive.write_bytes(b"!<arch>\n")
            dependency = root / "demo.d"
            dependency.write_text("demo.o: demo.cu\n", encoding="utf-8")
            contract = {
                "schema_version": 1,
                "forbidden_suffixes": [".cce", ".ptx"],
                "cases": {
                    "unit": {
                        "required": [
                            {"path": "app", "kind": "executable", "require_elf": False},
                            {"path": "libdemo.a", "kind": "archive"},
                            {"path": "demo.d", "kind": "dependency"},
                        ]
                    }
                },
            }
            result = module.verify_case(root, contract["cases"]["unit"], contract)
            self.assertTrue(result.ok, result.format_details())

    def test_verifier_reports_missing_and_forbidden_artifacts(self) -> None:
        module = self.load_module()
        with tempfile.TemporaryDirectory() as tmp_name:
            root = Path(tmp_name)
            (root / "unexpected.cce").write_text("legacy", encoding="utf-8")
            contract = {
                "schema_version": 1,
                "forbidden_suffixes": [".cce", ".ptx", ".cubin", ".fatbin", ".maca"],
                "cases": {
                    "unit": {
                        "required": [{"path": "missing.o", "kind": "object"}]
                    }
                },
            }
            result = module.verify_case(root, contract["cases"]["unit"], contract)
            self.assertFalse(result.ok)
            details = result.format_details()
            self.assertIn("missing required artifact", details)
            self.assertIn("forbidden artifact", details)

    def test_verifier_can_validate_fake_elf_metadata_with_tool_overrides(self) -> None:
        module = self.load_module()
        with tempfile.TemporaryDirectory() as tmp_name:
            root = Path(tmp_name)
            tools = root / "tools"
            tools.mkdir()
            fake_file = tools / "file"
            fake_file.write_text(
                "#!/bin/sh\n"
                "echo 'ELF 64-bit LSB pie executable, x86-64'\n",
                encoding="utf-8",
            )
            fake_readelf = tools / "readelf"
            fake_readelf.write_text(
                "#!/bin/sh\n"
                "if [ \"$1\" = \"-h\" ]; then\n"
                "  echo 'Type: EXEC (Executable file)'\n"
                "elif [ \"$1\" = \"-d\" ]; then\n"
                "  echo '0x0000000000000001 (NEEDED) Shared library: [libcce_bridge_runtime.so]'\n"
                "  echo '0x000000000000001d (RUNPATH) Library runpath: [/opt/bridge/lib:/usr/local/Ascend/latest/lib64]'\n"
                "fi\n",
                encoding="utf-8",
            )
            for tool in (fake_file, fake_readelf):
                tool.chmod(tool.stat().st_mode | stat.S_IXUSR)
            executable = root / "app"
            executable.write_bytes(b"elf")
            contract = {
                "schema_version": 1,
                "forbidden_suffixes": [],
                "cases": {
                    "unit": {
                        "required": [{
                            "path": "app",
                            "kind": "executable",
                            "require_elf": True,
                            "required_needed": ["libcce_bridge_runtime.so"],
                            "required_rpath": ["/opt/bridge/lib"],
                        }]
                    }
                },
            }
            old_path = os.environ.get("PATH", "")
            os.environ["PATH"] = f"{tools}{os.pathsep}{old_path}"
            try:
                result = module.verify_case(root, contract["cases"]["unit"], contract)
            finally:
                os.environ["PATH"] = old_path
            self.assertTrue(result.ok, result.format_details())


if __name__ == "__main__":
    unittest.main()
