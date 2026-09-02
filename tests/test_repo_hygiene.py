#!/usr/bin/env python3
"""Release-hygiene contracts for the repository layout and documentation."""
from __future__ import annotations

import subprocess
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class RepositoryHygieneTest(unittest.TestCase):
    def test_release_documentation_has_one_canonical_plan_location(self) -> None:
        self.assertFalse((ROOT / "plans").exists())
        self.assertTrue(
            (ROOT / "docs/archive/CUDA_LARGE_PROJECT_COMPATIBILITY_PLAN.md").is_file()
        )
        self.assertIn(
            "docs/archive/CUDA_LARGE_PROJECT_COMPATIBILITY_PLAN.md",
            (ROOT / "docs/README.md").read_text(encoding="utf-8"),
        )

    def test_personal_tool_configuration_is_not_part_of_release_tree(self) -> None:
        self.assertFalse((ROOT / ".claude").exists())
        self.assertIn(".claude/", (ROOT / ".gitignore").read_text(encoding="utf-8"))

    def test_gitignore_covers_release_and_runtime_outputs(self) -> None:
        ignore = (ROOT / ".gitignore").read_text(encoding="utf-8")
        for pattern in (".DS_Store", "/build/", ".mimosa/", ".worktrees/", "*.log", "*.py[cod]"):
            self.assertIn(pattern, ignore)

    def test_repository_file_guide_lists_all_tracked_paths(self) -> None:
        guide = ROOT / "docs/REPOSITORY_FILE_GUIDE.md"
        text = guide.read_text(encoding="utf-8")
        tracked = subprocess.check_output(["git", "ls-files"], cwd=ROOT, text=True).splitlines()
        missing = [path for path in tracked if f"`{path}`" not in text]
        self.assertEqual(missing, [])

    def test_repository_file_guide_records_cu_bridge_name_mapping(self) -> None:
        text = (ROOT / "docs/REPOSITORY_FILE_GUIDE.md").read_text(encoding="utf-8")
        for item in (
            "cmake_maca", "cmake_cce", "make_maca", "make_cce",
            "ninja_maca", "ninja_cce", "pre_makefile", "pre_cce",
            "bin/gnu", "hostcc_cce", "link.json", "shim_layout.json",
        ):
            self.assertIn(item, text)

    def test_repository_file_guide_covers_maintenance_entrypoints(self) -> None:
        guide = ROOT / "docs/REPOSITORY_FILE_GUIDE.md"
        self.assertTrue(guide.is_file())
        text = guide.read_text(encoding="utf-8")
        for item in (
            "bin/cucc", "tools/cu2cce_driver.py", "tools/cmake_cce",
            "tools/make_cce", "tools/ninja_cce", "tools/pre_cce",
            "config/cu2cce_rules.json", "config/shim_layout.json",
            "config/artifact_contract.json", "tools/real_test/server_cmd.txt",
            "multi_cu_diagnostic", "DIAGNOSTIC_ONLY",
        ):
            self.assertIn(item, text)

    def test_readme_explains_binary_install_tree_usage(self) -> None:
        readme = (ROOT / "README.md").read_text(encoding="utf-8")
        for item in (
            "二进制安装树", "CUCC_PATH", "build/real_test/install",
            "cucc --version", "cucc -c", "cmake_cce --build",
            "CANN/NPU",
        ):
            self.assertIn(item, readme)

    def test_readme_points_to_current_release_status_and_entrypoints(self) -> None:
        readme = (ROOT / "README.md").read_text(encoding="utf-8")
        for link in (
            "QUICKSTART.md",
            "docs/03_Comparison/RELEASE_READINESS_2026-09-02.md",
            "docs/03_Comparison/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md",
            "docs/03_Comparison/CUCC_OPTION_COMPATIBILITY_MATRIX.md",
            "tools/real_test/server_cmd.txt",
        ):
            self.assertIn(link, readme)
        self.assertIn("cmake_cce --build", readme)

    def test_primary_docs_have_no_removed_path_references(self) -> None:
        files = [
            ROOT / "README.md", ROOT / "QUICKSTART.md", ROOT / "CLAUDE.md",
            ROOT / "examples/README.md", ROOT / "docs/README.md",
            ROOT / "docs/03_Comparison/mapping.md",
        ]
        text = "\n".join(path.read_text(encoding="utf-8") for path in files)
        self.assertNotIn("PROJECT_ARCHITECTURE.md", text)
        self.assertNotIn("docs/superpowers/", text)
        self.assertNotIn("plans/cuda-large-project-compatibility.md", text)

    def test_tracked_tree_contains_no_obvious_local_generated_files(self) -> None:
        result = subprocess.run(
            ["git", "ls-files"], cwd=ROOT, capture_output=True, text=True, check=True
        )
        suspicious = [
            line for line in result.stdout.splitlines()
            if line == ".DS_Store"
            or line.endswith("/.DS_Store")
            or line.endswith(".pyc")
            or "/__pycache__/" in line
            or line.startswith("build/")
        ]
        self.assertEqual(suspicious, [])
