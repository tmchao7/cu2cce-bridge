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
            "bridge_install",
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
            "cmake_large_project_install",
        ):
            self.assertIn(case, data["cases"])

    def test_bridge_install_contract_covers_runtime_and_package_surface(self) -> None:
        data = json.loads((ROOT / "config/artifact_contract.json").read_text(encoding="utf-8"))
        case = data["cases"]["bridge_install"]
        required = {entry["path"] for entry in case["required"]}
        for path in (
            "lib/libcce_bridge_runtime.so",
            "bin/cucc",
            "bin/nvcc",
            "lib/cmake/cu2cce-bridge/cu2cceTargets.cmake",
            "lib/cmake/cu2cce-bridge/cu2cce-bridgeConfig.cmake",
            "cmake_modules/FindCUDA.cmake",
            "config/cu2cce_rules.json",
        ):
            self.assertIn(path, required)
        metadata = {entry["path"]: entry.get("contains", []) for entry in case.get("metadata", [])}
        self.assertIn("lib/cmake/cu2cce-bridge/cu2cceTargets.cmake", metadata)
        self.assertIn("cu2cce::runtime", " ".join(metadata["lib/cmake/cu2cce-bridge/cu2cceTargets.cmake"]))

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

    def test_install_export_contracts_check_exported_target_names(self) -> None:
        data = json.loads((ROOT / "config/artifact_contract.json").read_text(encoding="utf-8"))
        for case, names in {
            "cmake_build_compat_install": ("build_compat_app", "build_compat_static", "build_compat_shared"),
            "cmake_large_project_install": ("large_project_app", "large_kernel_lib", "large_util_lib"),
        }.items():
            metadata = data["cases"][case]["metadata"]
            tokens = {token for entry in metadata for token in entry.get("contains", [])}
            self.assertTrue(set(names).issubset(tokens), case)

    def test_ninja_build_compatibility_case_matches_makefile_artifact_shape(self) -> None:
        contract = json.loads((ROOT / "config/artifact_contract.json").read_text(encoding="utf-8"))
        make_case = contract["cases"]["cmake_build_compat"]
        ninja_case = contract["cases"]["cmake_build_compat_ninja"]
        self.assertEqual(
            sorted((entry["path"], entry["kind"]) for entry in make_case["required"]),
            sorted((entry["path"], entry["kind"]) for entry in ninja_case["required"]),
        )
        self.assertIn("CMakeFiles/rules.ninja", {entry["path"] for entry in ninja_case["metadata"]})

    def test_optional_blas_harness_clears_output_directories_too(self) -> None:
        blas = (ROOT / "tools/real_test/blas_regression.sh").read_text(encoding="utf-8")
        self.assertIn('find "$OUT" -mindepth 1 -maxdepth 1 -exec rm -rf -- {} +', blas)
        self.assertNotIn('rm -f "$OUT"/*', blas)

    def test_server_failure_paths_do_not_cat_unbounded_run_logs(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertNotIn('cat "$run_log"', server)
        self.assertNotIn('cat "$OUT/make_gxx_link.run.log"', server)
        self.assertIn('tail -n 30 "$run_log"', server)
        self.assertIn('tail -n 30 "$OUT/make_gxx_link.run.log"', server)

    def test_server_summary_does_not_claim_skipped_puresimt_pass(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("if profile_runs \"fast,full\"; then\n        echo 'PURESIMT_REGRESSION_PASS'", server)

    def test_server_harness_usage_comment_is_valid_for_origin_checkout(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("git fetch origin main", server)
        self.assertIn("git pull --ff-only origin main", server)
        self.assertIn("bash tools/real_test/clean.sh", server)
        self.assertNotIn("git fetch gitcode main   #", server)

    def test_server_harness_resolves_gitcode_or_origin_remote(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn('CU2CCE_REGRESSION_REMOTE', server)
        self.assertIn('git remote get-url gitcode', server)
        self.assertIn('git remote get-url origin', server)
        self.assertIn('git fetch "$REGRESSION_REMOTE" "$REGRESSION_BRANCH"', server)
        self.assertIn('git pull --ff-only "$REGRESSION_REMOTE" "$REGRESSION_BRANCH"', server)

    def test_server_harness_fetches_before_fast_forward_pull(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertLess(
            server.index('git fetch "$REGRESSION_REMOTE" "$REGRESSION_BRANCH"'),
            server.index('git pull --ff-only "$REGRESSION_REMOTE" "$REGRESSION_BRANCH"'),
        )

    def test_server_install_failure_output_is_bounded(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertNotIn('cat "$OUT/cmake-install.log"', server)
        self.assertIn('tail -n 30 "$OUT/cmake-install.log"', server)

    def test_server_harness_does_not_run_stale_checkout_after_git_update_failure(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn('if ! git fetch "$REGRESSION_REMOTE" "$REGRESSION_BRANCH"; then', server)
        self.assertIn('if ! git pull --ff-only "$REGRESSION_REMOTE" "$REGRESSION_BRANCH"; then', server)
        self.assertIn('refusing to run regression on stale checkout', server)
        self.assertIn('exit 1', server)

    def test_server_harness_clears_previous_output_directories(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn('find "$OUT" -mindepth 1 -maxdepth 1 -exec rm -rf -- {} +', server)
        self.assertNotIn('rm -f "$OUT"/*', server)

    def test_server_diagnostic_logs_stay_compact_on_terminal(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn('tail -n 12 "$diag_log"', server)
        self.assertNotIn('tail -n 60 "$diag_log"', server)
        self.assertNotIn('cat "$OUT/async_repro_diag.run.log"', server)

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
            "cmake_compat_legacy", "cmake_large_project", "cmake_large_project_install",
            "cmake_build_compat", "cmake_build_compat_ninja", "cmake_ninja",
            "make_compat", "make_gxx_link", "cmake_build_compat_install",
        ):
            self.assertIn('--case "$case_name"', server, case)
        self.assertIn("examples/build_compat/cmake", server)
        self.assertIn("cmake_build_compat_ninja", server)
        self.assertIn("direct ninja_cce build-compat artifact shape matches CMake Makefiles", server)
        self.assertIn("ARTIFACT_CONTRACT_REGRESSION_PASS", server)
        self.assertIn("artifact_check bridge_install", server)
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

    def test_verifier_rejects_non_makefile_dependency_artifacts(self) -> None:
        module = self.load_module()
        with tempfile.TemporaryDirectory() as tmp_name:
            root = Path(tmp_name)
            dependency = root / "bad.d"
            dependency.write_text("compiler diagnostics only\n", encoding="utf-8")
            contract = {
                "schema_version": 1,
                "forbidden_suffixes": [],
                "cases": {"unit": {
                    "required": [{"path": "bad.d", "kind": "dependency"}]
                }},
            }
            result = module.verify_case(root, contract["cases"]["unit"], contract)
            self.assertFalse(result.ok)
            self.assertIn("dependency", result.format_details())

    def test_verifier_rejects_malformed_metadata_contract_entries(self) -> None:
        module = self.load_module()
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            contract = tmp / "bad.json"
            contract.write_text(json.dumps({
                "schema_version": 1,
                "cases": {"bad": {"required": [], "metadata": ["not-an-object"]}},
            }), encoding="utf-8")
            log = tmp / "bad.log"
            with self.assertRaises(module.ContractError):
                module.verify_contract(contract, "bad", tmp, log)

    def test_verifier_requires_exact_dynamic_dependency_and_rpath_entries(self) -> None:
        module = self.load_module()
        with tempfile.TemporaryDirectory() as tmp_name:
            root = Path(tmp_name)
            tools = root / "tools"
            tools.mkdir()
            fake_file = tools / "file"
            fake_file.write_text("#!/bin/sh\necho 'ELF 64-bit LSB pie executable, x86-64'\n", encoding="utf-8")
            fake_readelf = tools / "readelf"
            fake_readelf.write_text(
                "#!/bin/sh\n"
                "if [ \"$1\" = \"-h\" ]; then\n"
                "  echo 'Type: EXEC (Executable file)'\n"
                "elif [ \"$1\" = \"-d\" ]; then\n"
                "  echo 'NEEDED [libcce_bridge_runtime.so.fake]'\n"
                "  echo 'RUNPATH [/opt/bridge/library-extra]'\n"
                "fi\n",
                encoding="utf-8",
            )
            for tool in (fake_file, fake_readelf):
                tool.chmod(tool.stat().st_mode | stat.S_IXUSR)
            (root / "app").write_bytes(b"elf")
            contract = {
                "schema_version": 1,
                "forbidden_suffixes": [],
                "cases": {"unit": {
                    "required": [{
                        "path": "app",
                        "kind": "executable",
                        "required_needed": ["libcce_bridge_runtime.so"],
                        "required_rpath": ["/opt/bridge/library"],
                    }]
                }},
            }
            old_path = os.environ.get("PATH", "")
            os.environ["PATH"] = f"{tools}{os.pathsep}{old_path}"
            try:
                result = module.verify_case(root, contract["cases"]["unit"], contract)
            finally:
                os.environ["PATH"] = old_path
            self.assertFalse(result.ok)
            self.assertIn("missing NEEDED", result.format_details())
            self.assertIn("missing RPATH/RUNPATH", result.format_details())

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
