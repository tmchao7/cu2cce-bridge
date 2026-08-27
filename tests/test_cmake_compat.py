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
        # Diagnostic samples must be compiled by the harness before any run
        # attempt: earlier server runs printed empty [DIAG-ONLY] tails because
        # only vectorAdd/matrixMul were in the required "--target" list.
        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("for sample in simpleStreams simpleMultiCopy asyncAPI; do", server_cmd)
        self.assertIn('cmake --build "$EXTERNAL_BUILD" --target "$sample"', server_cmd)
        self.assertIn("cuda_samples_${sample}.build.log", server_cmd)
        self.assertIn("cuda_samples_diag_${sample}.log", server_cmd)
        ext_cmake = (ROOT / "examples/external/cuda_samples_project/CMakeLists.txt").read_text(encoding="utf-8")
        # Every sample target is swept AFTER add_subdirectory: upstream
        # per-target CUDA_ARCHITECTURES lists (asyncAPI requests sm_61..sm_120)
        # otherwise make CMake emit a per-arch .cubin pipeline the puresimt
        # backend cannot consume.
        self.assertIn("cu2cce_sample_targets", ext_cmake)
        for prop in ("CUDA_ARCHITECTURES OFF", "CUDA_SEPARABLE_COMPILATION OFF",
                     "CUDA_CUBIN_COMPILATION OFF", "CUDA_FATBIN_COMPILATION OFF"):
            self.assertIn(prop, ext_cmake)
        # CMake rejects CUDA_CUBIN/FATBIN_COMPILATION on non-OBJECT targets at
        # configure time even when set to OFF; the harness must type-guard the
        # sweep so executables never see those properties.
        self.assertIn('STREQUAL "OBJECT_LIBRARY"', ext_cmake)
        self.assertGreater(ext_cmake.index("CUDA_SEPARABLE_COMPILATION OFF"),
                           ext_cmake.index("add_subdirectory"))

    def test_simt_adaptor_supplies_host_minmax_templates(self) -> None:
        # The server diagnostic run proved simpleStreams/simpleMultiCopy fail
        # because CANN simt_api/math_functions.h marks integer min/max
        # __device__-only ("call to __device__ function from __host__
        # function"). Value-returning templates are the only overload shape
        # that adds viable host candidates without colliding with CANN's
        # static non-template declarations or changing device resolution.
        adaptor = (ROOT / "include/bridge/kernel/simt_adaptor.h").read_text(encoding="utf-8")
        self.assertIn("__CU2CCE_NO_HOST_ARITHMETIC_OVERLOADS__", adaptor)
        self.assertIn("inline T max(T left, T right)", adaptor)
        self.assertIn("inline T min(T left, T right)", adaptor)

    def test_bridge_runtime_surface_for_diagnostic_samples(self) -> None:
        # Second server diagnostic round peeled two more layers:
        #   simpleStreams ran until cudaEventRecord(start_event, 0) and died
        #     with ACL 100000 because a nullptr stream was rejected outright;
        #   simpleMultiCopy still failed to compile on cudaHostAllocDefault
        #     and _cceDeviceProp_t.deviceOverlap/.asyncEngineCount;
        #   asyncAPI exited silently, so the harness must report exit status.
        cpp = (ROOT / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        adaptor = (ROOT / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        wrapper = (ROOT / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        types = (ROOT / "include/bridge/runtime/cuda_wrapper_types.h").read_text(encoding="utf-8")
        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")

        self.assertIn("wcuda_resolve_stream", cpp)
        for flag in ("cudaHostAllocDefault", "cudaHostAllocPortable",
                     "cudaHostAllocMapped", "cudaHostAllocWriteCombined"):
            self.assertIn(flag, adaptor)
        self.assertIn("inline aclError wcudaHostAlloc(T **ptr", wrapper)
        for field in ("deviceOverlap", "asyncEngineCount"):
            self.assertIn(field, types)
        self.assertIn("failed, exit=$diag_status", server_cmd)

    def test_regression_profiles_are_wired(self) -> None:
        # The harness supports three profiles. fast (default) keeps the daily
        # feedback loop short; external targets wrapper/runtime iteration;
        # full preserves the historical whole-suite behavior.
        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn('CU2CCE_REGRESSION_PROFILE:-fast', server_cmd)
        self.assertIn("fast|external|full", server_cmd)
        for helper in ("profile_runs()", "cmake_case_core()", "cmake_case_full_only()"):
            self.assertIn(helper, server_cmd)
        for marker in ("mark_timing puresimt_suite", "mark_timing cmake_suite",
                       "mark_timing make_ninja_suite", "mark_timing external_samples",
                       "mark_timing env_bridge_rebuild", "mark_timing cucc_direct"):
            self.assertIn(marker, server_cmd)
        # Skipped sections must be visible and must never fake a PASS badge.
        for skipped in ("PURESIMT_SECTION_SKIPPED", "CUCC_RULES_SECTION_SKIPPED",
                        "CMAKE_CCE_SECTION_SKIPPED", "BUILD_COMPAT_SECTION_SKIPPED",
                        "CUDA_SAMPLES_SECTION_SKIPPED"):
            self.assertIn(skipped, server_cmd)
        self.assertIn("[SKIP] external NVIDIA CUDA Samples section", server_cmd)
        # fast keeps only the core puresimt subset; every legacy case stays in
        # the file guarded by profile checks so full remains complete.
        fast_filter = "vectorAdd|matrixMul|event_minimal|multiKernel_minimal|atomicAdd_minimal|math_minimal"
        self.assertIn(fast_filter, server_cmd)
        for case in ("cmake_demo", "cmake_compat", "cmake_compat_legacy",
                     "cmake_large_project", "cmake_build_compat", "cmake_ninja"):
            self.assertIn(case, server_cmd)
        self.assertGreaterEqual(server_cmd.count("profile_runs"), 10)

    def test_crt_math_compat_surface_is_forward_only(self) -> None:
        # Batch A: crt header family ships forward-first compat for the
        # common CUDA include surface. No fabricated device math — missing
        # symbols must surface as compile errors naming themselves.
        crt = ROOT / "include/crt"
        for name in ("host_defines.h", "math_functions.h", "device_functions.h",
                     "common_functions.h", "vector_types.h", "builtin_types.h"):
            self.assertTrue((crt / name).is_file(), name)
        self.assertTrue((ROOT / "include/device_launch_parameters.h").is_file())

        # Chain-included headers must be INERT unless every symbol is proven:
        # only the three safe forwarders travel with <cuda_runtime.h>. Match
        # directives per line, not substrings, so explanatory comments cannot
        # trip these checks.
        import re as _re
        runtime_chain = (ROOT / "include/cuda_runtime.h").read_text(encoding="utf-8")
        for hdr in ("crt/host_defines.h", "crt/common_functions.h", "crt/math_functions.h"):
            self.assertIsNotNone(
                _re.search(r'^\s*#\s*include\s+<' + _re.escape(hdr) + r'>',
                           runtime_chain, _re.MULTILINE),
                f"missing expected forwarder include: {hdr}")
        for line in runtime_chain.splitlines():
            self.assertIsNone(_re.search(r'^\s*#\s*include\s+.*vector_functions_compat', line),
                              f"unproven factory layer must stay out of the chain: {line!r}")

        # Keyword-vs-macro lesson: ccec treats __host__/__device__ as language
        # keywords; #define-fallbacks silently strip execution spaces.
        host_defines = (crt / "host_defines.h").read_text(encoding="utf-8")
        for line in host_defines.splitlines():
            self.assertIsNone(_re.search(r'^\s*#\s*define\s+__(?:host|device)__', line),
                              f"forbidden execution-space macro redefinition: {line!r}")

        math_hdr = (crt / "math_functions.h").read_text(encoding="utf-8")
        self.assertIn("__has_include(<simt_api/math_functions.h>)", math_hdr)
        self.assertIn("UNSUPPORTED", math_hdr)  # double-precision boundary

        vectors = (crt / "vector_types.h").read_text(encoding="utf-8")
        # Server evidence: bisheng injects the whole vector-type family as
        # ext_vector_type typedefs; bridge-side structs collided. The file
        # must stay documentation-only (no redefinitions, no dim3).
        self.assertNotIn("typedef struct", vectors)
        self.assertNotIn("dim3", vectors)
        self.assertIn("ext_vector_type", vectors)  # records the native proof

        compat = (crt / "vector_functions_compat.h").read_text(encoding="utf-8")
        # Stay doc-only until the extended inventory lists the exact factory
        # surface. Lock the doc-only state by forbidding any real declaration
        # machinery, not substrings (evidence prose legitimately names types
        # like double2).
        for line in compat.splitlines():
            self.assertIsNone(_re.search(r'^\s*(template\s*<|typedef\b|#define(?! __CU2CCE_CRT_VECTOR_FUNCTIONS_COMPAT_H__))', line),
                              f"compat layer must stay inert for now: {line!r}")

        # Ascend device has no double at all; the probe must stay float-only.
        carrier = (ROOT / "examples/puresimt/device_math_minimal.cu").read_text(encoding="utf-8")
        for forbidden in ("double2", "make_float3", "make_float4"):
            self.assertNotIn(forbidden, carrier)
        # Round-4 platform fact: standalone "__device__" is rejected by the
        # bisheng SIMT front-end. The probe therefore inlines its op-chain
        # into the __global__ kernel until the canonical device-helper
        # spelling is proven by the inventory evidence step. Comments and
        # strings are stripped before scanning so the documented fact does
        # not trip its own guard.
        code_only = _re.sub(r"/\*.*?\*/", "", carrier, flags=_re.S)
        code_only = _re.sub(r"//[^\n]*", "", code_only)
        for line in code_only.splitlines():
            self.assertIsNone(_re.search(r"\b(__device__|__host__)\b", line),
                              f"probe must avoid qualified helpers entirely: {line!r}")
        self.assertRegex(code_only, r"__global__\s+void\s+probe_kernel")

        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("inventory_simt_api.sh", server_cmd)
        self.assertIn("simt_api_inventory.log", server_cmd)
        self.assertIn("device_math_minimal.cu", server_cmd)
        self.assertTrue((ROOT / "tools/real_test/inventory_simt_api.sh").is_file())

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
