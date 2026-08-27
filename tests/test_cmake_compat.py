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

    def test_stream_and_device_flag_compatibility_is_declared(self) -> None:
        adaptor = (ROOT / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        types = (ROOT / "include/bridge/runtime/cuda_wrapper_types.h").read_text(encoding="utf-8")
        wrapper = (ROOT / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        wrapper_h = (ROOT / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        for flag in ("cudaDeviceBlockingSync", "cudaDeviceMapHost", "cudaEventBlockingSync", "cudaEventDefault"):
            self.assertIn(flag, adaptor)
        self.assertIn("canMapHostMemory", types)
        self.assertIn("wcudaSetDeviceFlags", wrapper)
        # Typed allocation overloads keep unmodified CUDA sample calls working
        # (e.g. matrixMul's cudaMallocHost(&float_ptr, ...) and any host code
        # that uses cudaMalloc(&float_ptr, ...) without a void** cast).
        self.assertIn("inline aclError wcudaMalloc(", wrapper_h)
        self.assertIn("inline aclError wcudaMallocHost(", wrapper_h)

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
        coop = (ROOT / "include/cooperative_groups.h").read_text(encoding="utf-8")
        self.assertIn("simt_api/cooperative_groups.h", coop)
        self.assertIn("struct plus", coop)
        self.assertIn("T reduce", coop)

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
        self.assertIn("vectorAdd", harness)
        self.assertIn("matrixMul", harness)
        # The exclusion is explicit: reduction is listed only inside the
        # excluded-samples section, never in the required/diagnostic profiles.
        self.assertIn("_CU2CCE_EXCLUDED_SAMPLES", harness)
        self.assertIn("Samples/2_Concepts_and_Techniques/reduction", harness)
        self.assertIn("Samples/0_Introduction/vectorAdd", harness)
        self.assertNotIn("_CU2CCE_PROFILE_SAMPLES\n  \"Samples/2_Concepts_and_Techniques/reduction\"", harness)
        # Diagnostic-only samples stay in the build graph but never block the
        # required profile.
        for sample in ("simpleStreams", "simpleMultiCopy", "asyncAPI"):
            self.assertIn(sample, harness)

    def test_external_cuda_samples_profile_is_pinned(self) -> None:
        profile = (ROOT / "examples/external/cuda_samples_project/profile_core.json").read_text(encoding="utf-8")
        fetch = (ROOT / "tools/real_test/fetch_external_cuda_samples.sh").read_text(encoding="utf-8")
        self.assertIn("db3eea23946bca2e90a75eca2b5b3e07158a9e11", profile)
        self.assertIn("CUDA_SAMPLES_SOURCE_DIR", fetch)
        self.assertIn("checkout --detach", fetch)
        self.assertTrue((ROOT / "plans/cuda-large-project-compatibility.md").is_file())

    def test_cuda_language_module_override_is_shipped_and_injected(self) -> None:
        override = ROOT / "cmake_modules/override/CUDAOverrides.cmake"
        self.assertTrue(override.is_file())
        text = override.read_text(encoding="utf-8")
        for token in ("CMAKE_CUDA_COMPILER", "CMAKE_CUDA_DEVICE_LINKER",
                      "CMAKE_CUDA_FATBINARY", "CMAKE_CUDA_RUNTIME_LIBRARY",
                      "CMAKE_CUDA_ARCHITECTURES OFF", "ENV{CUDA_PATH}"):
            self.assertIn(token, text)
        # The wrapper must inject the override by default and honour the
        # NO_OVERRIDE escape hatch.
        cmake_cce = (ROOT / "tools/cmake_cce").read_text(encoding="utf-8")
        self.assertIn("CMAKE_USER_MAKE_RULES_OVERRIDE", cmake_cce)
        self.assertIn("CU2CCE_CMAKE_NO_OVERRIDE", cmake_cce)
        # The top-level CMake install ships cmake_modules/ as a directory
        # (which includes the override/ subtree), so the override module
        # reaches an installed prefix.
        top = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("install(DIRECTORY cmake_modules/", top)

    def test_cudatoolkit_alias_family_targets_are_declared(self) -> None:
        module = (ROOT / "cmake_modules/FindCUDAToolkit.cmake").read_text(encoding="utf-8")
        self.assertIn("CUDA::toolkit", module)
        # The alias family is built by a foreach loop, so the literal target
        # names appear as the loop list and the CUDA::${_alias} pattern.
        self.assertIn("foreach(_alias cudart_static cudadevrt cudart)", module)
        self.assertIn("CUDA::${_alias}", module)
        self.assertIn("INTERFACE_LINK_LIBRARIES CUDA::cudart", module)

    def test_matrix_records_multi_cu_diagnostic_evidence(self) -> None:
        matrix = ROOT / "docs/03_Comparison/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md"
        text = matrix.read_text(encoding="utf-8")
        self.assertIn("multiple definition of", text)
        self.assertIn("BishengLaunchKernelWithHostArgs", text)
        self.assertIn("CU2CCE_CMAKE_NO_OVERRIDE", text)

    def test_build_matrix_exists(self) -> None:
        matrix = ROOT / "docs/03_Comparison/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md"
        text = matrix.read_text(encoding="utf-8")
        for item in ("CMake", "Makefiles", "Ninja", "device-link", "CUDA::cudart"):
            self.assertIn(item, text)

    def test_large_project_carrier_covers_multi_directory_and_export(self) -> None:
        top = (ROOT / "examples/large_project/CMakeLists.txt").read_text(encoding="utf-8")
        for pattern in ("add_subdirectory(comp)", "add_subdirectory(util)",
                        "install(TARGETS", "install(EXPORT largeProjectTargets"):
            self.assertIn(pattern, top)
        self.assertTrue((ROOT / "examples/large_project/comp/kernel.cu").is_file())
        self.assertTrue((ROOT / "examples/large_project/app/main.cpp").is_file())
        # The app links the CUDA kernel library and CUDA::cudart so the host
        # main.cpp can find cuda_runtime.h and the exported kernel symbol.
        self.assertIn("large_kernel_lib", top)
        self.assertIn("CUDA::cudart", top)
        # One .cu per linked library: the CUDA target lives in comp/ and the
        # host-only util/ target must not contain CUDA sources.
        self.assertFalse((ROOT / "examples/large_project/util/store.cpp").read_text(
            encoding="utf-8").strip().endswith(".cu"))
        # The install export must include every target it references.
        for sub in ("comp/CMakeLists.txt", "util/CMakeLists.txt"):
            self.assertIn("EXPORT largeProjectTargets",
                          (ROOT / "examples/large_project" / sub).read_text(encoding="utf-8"))

    def test_fetch_script_rejects_non_public_urls(self) -> None:
        fetch = (ROOT / "tools/real_test/fetch_external_cuda_samples.sh").read_text(encoding="utf-8")
        self.assertIn("http://*|https://*)", fetch)
        validator = ROOT / "tools/real_test/validate_fetch_url.py"
        self.assertTrue(validator.is_file())
        self.assertIn("localhost", validator.read_text(encoding="utf-8"))
        self.assertGreaterEqual(
            (ROOT / "tools/real_test/fetch_external_cuda_samples.sh").read_text(
                encoding="utf-8").count("require_public_https_url"), 2)

    def test_mock_nvcc_short_circuits_preprocess_and_deps(self) -> None:
        mock = (ROOT / "tools/mock/mock_nvcc.py").read_text(encoding="utf-8")
        driver = (ROOT / "tools/cu2cce_driver.py").read_text(encoding="utf-8")
        self.assertIn("--proxy-mode", driver)
        # -MD/-MF/-MT are real dependency-generation flags on genuine CUDA
        # object rules; they must reach the rule driver, not be short-circuited.
        self.assertNotIn('"-MD"', mock)
        self.assertNotIn('"-M"', mock)


if __name__ == "__main__":
    unittest.main()
