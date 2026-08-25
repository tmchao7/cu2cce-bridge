#!/usr/bin/env python3
"""Static contract checks for the CMake/build compatibility surface."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]


class CMakeCompatibilityContractTest(unittest.TestCase):
    def test_cmake_entrypoints_are_installed(self) -> None:
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        for tool in ("tools/cmake_cce", "tools/make_cce", "tools/ninja_cce", "tools/cu2cce_driver.py"):
            self.assertIn(tool, cmake)

    def test_package_exports_native_runtime(self) -> None:
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        config = (ROOT / "cmake/cu2cce-bridgeConfig.cmake").read_text(encoding="utf-8")
        self.assertIn("EXPORT cu2cceTargets", cmake)
        self.assertIn("$<INSTALL_INTERFACE:include>", cmake)
        self.assertIn("cu2cceTargets.cmake", config)

    def test_examples_cover_build_systems(self) -> None:
        self.assertTrue((ROOT / "examples/build_compat/cmake/CMakeLists.txt").is_file())
        self.assertTrue((ROOT / "examples/build_compat/make/Makefile").is_file())
        self.assertIn("nvcc", (ROOT / "examples/build_compat/make/Makefile").read_text(encoding="utf-8"))
        self.assertTrue((ROOT / "examples/cmake_compat/CMakeLists.txt").is_file())
        self.assertTrue((ROOT / "examples/cmake_compat_legacy/CMakeLists.txt").is_file())
        self.assertTrue((ROOT / "include/cooperative_groups.h").is_file())
        self.assertTrue((ROOT / "include/cooperative_groups/reduce.h").is_file())

    def test_unsupported_build_features_are_explicit(self) -> None:
        find_cuda = (ROOT / "cmake_modules/FindCUDA.cmake").read_text(encoding="utf-8")
        rules = (ROOT / "config/cu2cce_rules.json").read_text(encoding="utf-8")
        for feature in ("cuda_compile_ptx", "cuda_compile_cubin", "cuda_compile_fatbin"):
            self.assertIn(feature, find_cuda)
        for feature in ("--device-link", "--ptx", "--cubin", "--fatbin"):
            self.assertIn(feature, rules)

    def test_cudatoolkit_module_propagates_cann_include_dirs(self) -> None:
        module = (ROOT / "cmake_modules/FindCUDAToolkit.cmake").read_text(encoding="utf-8")
        self.assertIn("CANN_BASE}/include", module)
        self.assertIn("ASCEND_TOOLKIT_HOME", module)
        self.assertIn("INTERFACE_INCLUDE_DIRECTORIES", module)
        harness = (ROOT / "examples/external/cuda_samples_project/CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("__CU2CCE_SYSTEM_LIBC__", harness)

    def test_external_cuda_samples_profile_is_pinned(self) -> None:
        profile = (ROOT / "examples/external/cuda_samples_project/profile_core.json").read_text(encoding="utf-8")
        fetch = (ROOT / "tools/real_test/fetch_external_cuda_samples.sh").read_text(encoding="utf-8")
        self.assertIn("db3eea23946bca2e90a75eca2b5b3e07158a9e11", profile)
        self.assertIn("CUDA_SAMPLES_SOURCE_DIR", fetch)
        self.assertIn("checkout --detach", fetch)
        self.assertTrue((ROOT / "plans/cuda-large-project-compatibility.md").is_file())

    def test_build_matrix_exists(self) -> None:
        matrix = ROOT / "docs/03_Comparison/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md"
        text = matrix.read_text(encoding="utf-8")
        for item in ("CMake", "Makefiles", "Ninja", "device-link", "CUDA::cudart"):
            self.assertIn(item, text)


if __name__ == "__main__":
    unittest.main()
