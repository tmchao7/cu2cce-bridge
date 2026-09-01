#!/usr/bin/env python3
"""Static contract checks for the CMake/build compatibility surface."""
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class CMakeCompatibilityContractTest(unittest.TestCase):
    def test_cubridge_parity_audit_reports_backend_aware_sections(self) -> None:
        # Stage-0 baseline: compare the checked-in cu-bridge reference assets
        # with the NPU implementation without treating MACA-only mappings as
        # missing Ascend features.
        audit = ROOT / "tools/compare_cubridge_parity.py"
        self.assertTrue(audit.is_file())
        result = subprocess.run(
            [
                "python3", str(audit),
                "--reference", str(ROOT.parent / "cu-bridge"),
                "--format", "json",
            ],
            capture_output=True, text=True, check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        import json
        report = json.loads(result.stdout)
        self.assertEqual(report["reference"]["commit"], "6ab1076")
        self.assertIn("cmake_make", report["sections"])
        self.assertIn("cucc", report["sections"])
        statuses = report["summary"]
        self.assertIn("EQUIVALENT_NPU", statuses)
        self.assertIn("NOT_APPLICABLE_NPU", statuses)
        self.assertGreater(statuses["ALIGNED"], 0)
        self.assertGreater(statuses["MISSING"], 0)
        self.assertGreater(report["reference_assets"]["cuda_library_aliases"], 0)
        self.assertFalse(report["reference_assets"]["missing_findcuda_macros"])
        self.assertTrue(report["reference_assets"]["missing_link_names"])

    def test_cmake_entrypoints_are_installed(self) -> None:
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        for tool in ("tools/cmake_cce", "tools/make_cce", "tools/ninja_cce", "tools/cu2cce_driver.py", "tools/compare_cubridge_parity.py"):
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
        self.assertIn('cmake --build "$EXTERNAL_BUILD" --parallel "$EXTERNAL_JOBS" --target "$sample"', server_cmd)
        # Regression guard: prefixing a shell FUNCTION with env yields
        # "env: 'run_with_timeout': No such file or directory" (exit 127)
        # and silently swallows every diagnostic run. Diagnostics force
        # WCUDA_DEBUG=1 so failing samples leave per-call bridge traces.
        self.assertIn("WCUDA_DEBUG=1 run_with_timeout", server_cmd)
        self.assertNotIn('env WCUDA_DEBUG=', server_cmd)
        self.assertIn("tail -n 60 \"$diag_log\"", server_cmd)
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
        # Legacy default-stream semantics, consumption-point form (round-8
        # regression lesson: syncing the device at RECORD time broke
        # asyncAPI's mid-pipeline default-stream events). Events record
        # freely; the device sync happens where results are consumed.
        self.assertIn("wcuda_is_bridge_default", cpp)
        self.assertIn("wcuda_event_mark_default", cpp)
        self.assertIn("wcuda_event_is_default", cpp)
        record_block = cpp[cpp.index("aclError wcudaEventRecord"):cpp.index("aclError wcudaEventRecordWithFlags")]
        self.assertNotIn("aclrtSynchronizeDevice()", record_block)
        sync_block = cpp[cpp.index("aclError wcudaEventSynchronize"):cpp.index("aclError wcudaEventQuery")]
        self.assertIn("aclrtSynchronizeDevice()", sync_block)
        query_block = cpp[cpp.index("aclError wcudaEventQuery"):cpp.index("aclError wcudaEventElapsedTime")]
        self.assertIn("aclrtSynchronizeDevice()", query_block)
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
        # Round-8 performance work: the source cache must survive clean.sh
        # (it lives outside build/), the build tree is incremental, and
        # compilation is parallel.
        self.assertIn('CU2CCE_EXTERNAL_CACHE_DIR:-$PWD/.cu2cce-cache', server_cmd)
        self.assertNotIn('EXTERNAL_SOURCE="$OUT/external/nvidia_cuda_samples"', server_cmd)
        self.assertIn('cmake --build "$EXTERNAL_BUILD" --parallel "$EXTERNAL_JOBS"', server_cmd)
        self.assertIn("EXTERNAL_JOBS=", server_cmd)
        self.assertNotIn('rm -rf "$EXTERNAL_BUILD"', server_cmd)
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
        # Round-5 evidence + round-6 fix: standalone "__device__" was rejected
        # by the bisheng front-end; simt_adaptor now maps it onto CANN's own
        # __SIMT_DEVICE_FUNCTIONS_DECL__ selector (escape hatch:
        # CU2CCE_NO_DEVICE_KEYWORD_MAP). The probe MUST exercise the mapped
        # spelling end-to-end.
        adaptor = (ROOT / "include/bridge/kernel/simt_adaptor.h").read_text(encoding="utf-8")
        self.assertIn("#define __device__ __SIMT_DEVICE_FUNCTIONS_DECL__", adaptor)
        self.assertIn("CU2CCE_NO_DEVICE_KEYWORD_MAP", adaptor)
        code_only = _re.sub(r"/\*.*?\*/", "", carrier, flags=_re.S)
        code_only = _re.sub(r"//[^\n]*", "", code_only)
        for line in code_only.splitlines():
            self.assertIsNone(_re.search(r"\b(__host__)\b", line),
                              f"probe must stay device-only in helpers: {line!r}")
        self.assertRegex(code_only, r"__device__\s+float\s+probe_chain")
        self.assertRegex(code_only, r"__global__\s+void\s+probe_kernel")

        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("inventory_simt_api.sh", server_cmd)
        self.assertIn("simt_api_inventory.log", server_cmd)
        # Round-6+: the math probe was PROMOTED into the required [3/7]
        # gate after consecutive [DIAG-PASS] server rounds; it is no longer
        # a standalone diagnostic segment.
        self.assertIn("'device_math_minimal.cu|device_math_minimal OK'", server_cmd)
        self.assertIn("vectorAdd|matrixMul|event_minimal|multiKernel_minimal|atomicAdd_minimal|math_minimal|device_math_minimal",
                      server_cmd)
        self.assertNotIn("device_math_minimal (diagnostic-only probe)", server_cmd)
        self.assertIn("double_policy_diag (diagnostic-only type-policy demo)", server_cmd)
        self.assertIn("async_repro_diag (diagnostic-only backend-semantics probe)", server_cmd)
        self.assertTrue((ROOT / "examples/puresimt/async_repro_diag.cu").is_file())
        # the repro must expose mismatch counts (not just pass/fail) so the
        # Ascend-side issue has quantitative evidence
        repro = (ROOT / "examples/puresimt/async_repro_diag.cu").read_text(encoding="utf-8")
        self.assertIn("MISMATCH count=", repro)
        self.assertIn("variantB event-per-round", repro)
        # variantC replicates the exact asyncAPI cadence: query-spin between
        # launches (the field issue only ever appeared on that path)
        self.assertIn("variantC query-spin", repro)
        self.assertIn("cudaEventQuery(evC)", repro)
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

    def test_cmake_build_phase_and_lib_symlinks_align_with_cubridge(self) -> None:
        # cu-bridge alignment (2026-08-28): two-phase protocol and the CUDA-
        # named lib64 symlink family. Their cmake_maca answers configure
        # probes via mock_nvcc and repoints CUDA_DIR/bin/nvcc -> cucc for the
        # build phase; their link.json pre_make section creates lib64/libcudart
        # -> runtime symlinks so plain-g++ linked Make projects resolve
        # -lcudart. Our cmake_cce/make_cce replicate both; since the fifth
        # parity batch the shim TREE itself is built from the shared manifest
        # config/shim_layout.json by tools/build_shim_tree.py (their
        # link.json/stub_link.json single-manifest parity).
        cmake_cce = (ROOT / "tools/cmake_cce").read_text(encoding="utf-8")
        self.assertIn('cmake_cce: build phase: shim nvcc ->', cmake_cce)
        self.assertIn('ln -sfn "$CUCC_PATH/bin/cucc" "$CUDA_DIR/bin/nvcc"', cmake_cce)
        # Persistent shim default (their CUBRIDGE_HOME semantics) is what
        # makes the build-phase re-point effective across processes.
        self.assertIn('${CU2CCE_HOME:-${CUBRIDGE_HOME:-$HOME}}/cu2cce-bridge/cmake-shim', cmake_cce)
        # Configure phase binds the shim nvcc to the mock through the shared
        # helper (re-pointing here makes repeated configure/build cycles on
        # one persistent shim deterministic). Since the seventh batch the
        # sentinel/builder logic lives in tools/fake_toolkit_env.sh.
        self.assertIn('cu2cce_prepare_fake_toolkit "$SHIM_ROOT" "$CUCC_PATH/tools/nvcc_entry.sh"',
                      cmake_cce)
        helper = (ROOT / "tools/fake_toolkit_env.sh").read_text(encoding="utf-8")
        self.assertIn('--hostcc-mode "$hostcc_mode"', helper)
        self.assertIn('echo 1 > "$dst/make.log"', helper)
        # The CUDA-named runtime alias family lives in the shared manifest.
        layout = (ROOT / "config/shim_layout.json").read_text(encoding="utf-8")
        for lib in ("libcudart.so", "libcudart_static.a", "libcudadevrt.a",
                    "libcuda.so", "libcuda.so.1", "libcublas.so",
                    "libcublas.so.1", "libnvToolsExt.so"):
            self.assertIn(lib, layout)
        make_cce = (ROOT / "tools/make_cce").read_text(encoding="utf-8")
        # targets triple derivation lives in the helper (uname machine/system,
        # amd64/arm64 normalization).
        self.assertIn("amd64", helper)
        self.assertIn("aarch64", helper)
        self.assertIn('"libcudart.so"', layout)
        self.assertIn('lib64/cmake/CUDAToolkit', layout)
        # make.log sentinel is created by the helper (persistent fake toolkit);
        # make_cce sources it and binds the shim nvcc to the real cucc.
        self.assertIn('. "${CUCC_PATH}/tools/fake_toolkit_env.sh"', make_cce)
        self.assertIn('cu2cce_prepare_fake_toolkit "$DST_WCUDA" "$CUCC_PATH/bin/cucc"', make_cce)
        self.assertIn('PATH="$DST_WCUDA/bin:$CUDA_DIR/bin:$PATH"', make_cce)

    def test_shim_layout_manifest_is_single_source_for_three_entries(self) -> None:
        # Fifth parity batch: cmake_cce/make_cce/pre_cce used to carry three
        # hand-written copies of the mkdir/ln sequences (cu-bridge keeps ONE
        # manifest: tools/link.json + tools/mock/stub_link.json). All three
        # entries must consume config/shim_layout.json — since the seventh
        # batch indirectly, through the sourced helper
        # tools/fake_toolkit_env.sh (sentinel + builder invocation).
        builder = ROOT / "tools/build_shim_tree.py"
        helper = ROOT / "tools/fake_toolkit_env.sh"
        self.assertTrue(builder.is_file())
        self.assertTrue(helper.is_file())
        helper_text = helper.read_text(encoding="utf-8")
        self.assertIn("tools/build_shim_tree.py", helper_text)
        for entry in ("tools/cmake_cce", "tools/make_cce", "tools/pre_cce"):
            text = (ROOT / entry).read_text(encoding="utf-8")
            self.assertIn("tools/fake_toolkit_env.sh", text, entry)
            self.assertIn("cu2cce_prepare_fake_toolkit", text, entry)
        for section in ("dirs", "dir_links", "file_links", "hostcc_names",
                        "cuda_lib_aliases", "alias_dirs", "alias_target", "files"):
            self.assertIn(f'"{section}"', (ROOT / "config/shim_layout.json").read_text(encoding="utf-8"))

    def test_device_arch_rejects_cuda_targets_and_accepts_one_dav_target(self) -> None:
        script = ROOT / "tools/device_arch.sh"
        env = dict(os.environ)
        env.pop("CU2CCE_NPU_ARCH", None)
        env.pop("CUCC_NPU_ARCH", None)
        accepted = subprocess.run(
            ["bash", str(script)],
            env={**env, "CUCC_TARGETS": "dav-c310-vec"},
            capture_output=True, text=True, check=False,
        )
        self.assertEqual(accepted.returncode, 0, accepted.stderr)
        self.assertEqual(accepted.stdout.strip(), "dav-c310-vec")
        rejected = subprocess.run(
            ["bash", str(script)],
            env={**env, "CUCC_TARGETS": "sm_80"},
            capture_output=True, text=True, check=False,
        )
        self.assertNotEqual(rejected.returncode, 0)
        self.assertIn("dav-*", rejected.stderr)

    def test_build_wrappers_share_validated_npu_arch_resolution(self) -> None:
        helper = (ROOT / "tools/fake_toolkit_env.sh").read_text(encoding="utf-8")
        self.assertIn("cu2cce_resolve_npu_arch()", helper)
        for name in ("tools/cmake_cce", "tools/make_cce", "tools/ninja_cce", "tools/pre_cce"):
            text = (ROOT / name).read_text(encoding="utf-8")
            self.assertIn("cu2cce_resolve_npu_arch", text, name)
            self.assertNotIn("|| echo dav-c310-vec", text, name)

    def test_ninja_cce_rebuilds_the_same_persistent_npu_shim(self) -> None:
        # The shared builder materializes <shim-root>/CUDA_DIR. Direct ninja
        # invocation must use that same path as cmake_cce --build; using a
        # different lowercase /cuda path silently leaves nvcc bound to mock.
        ninja = (ROOT / "tools/ninja_cce").read_text(encoding="utf-8")
        cmake = (ROOT / "tools/cmake_cce").read_text(encoding="utf-8")
        self.assertIn('$SHIM_ROOT/CUDA_DIR', ninja)
        self.assertIn('$SHIM_ROOT/CUDA_DIR', cmake)
        self.assertNotIn('CUDA_DIR="$SHIM_ROOT/cuda"', ninja)
        self.assertNotIn('CUDA_DIR="$SHIM_ROOT/cuda"', cmake)
        for text in (ninja,):
            self.assertIn("tools/fake_toolkit_env.sh", text)
            self.assertIn("cu2cce_prepare_fake_toolkit", text)
            self.assertIn("CU2CCE_SHIM_ISOLATE", text)
            self.assertIn('export CUDA_PATH=', text)
            self.assertIn('CANN_BASE=', text)
            self.assertIn('export CANN_BASE', text)
            self.assertIn('ASCEND_TOOLKIT_HOME=', text)
            self.assertIn('export ASCEND_TOOLKIT_HOME', text)
            self.assertIn('clean|*clean|clean-*', text)

    def test_make_cce_clean_matching_preserves_cleanpack_and_removes_clean(self) -> None:
        # Token-level clean matching must mirror cu-bridge's cleanup intent
        # without deleting a persistent shim for an unrelated target such as
        # cleanpack.
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            fake_make = fake_bin / "make"
            fake_make.write_text("#!/bin/sh\nexit 0\n", encoding="utf-8")
            fake_make.chmod(0o755)
            home = tmp / "home"
            env = dict(os.environ)
            env.update({
                "CUCC_PATH": str(ROOT),
                "CU2CCE_HOME": str(home),
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
            })
            keep = home / "cu2cce-bridge"
            result = subprocess.run(
                [str(ROOT / "tools/make_cce"), "cleanpack"],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertTrue(keep.exists())

            result = subprocess.run(
                [str(ROOT / "tools/make_cce"), "clean"],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertFalse(keep.exists())

    def test_ninja_cce_exports_npu_environment_and_builds_shim(self) -> None:
        # Functional direct-entry test: Ninja must receive the same NPU
        # environment as make_cce and the generated nvcc path must point to
        # the real cucc, not the configure mock.
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            log = tmp / "ninja.log"
            fake_ninja = fake_bin / "ninja"
            fake_ninja.write_text(
                "#!/bin/sh\n"
                "{ printf '%s\n' \"CUDA_PATH=$CUDA_PATH\" \"CUDA_HOME=$CUDA_HOME\" \"CANN_BASE=$CANN_BASE\" \"ASCEND_TOOLKIT_HOME=$ASCEND_TOOLKIT_HOME\" \"CU2CCE_NPU_ARCH=$CU2CCE_NPU_ARCH\"; printf 'ARG=%s\n' \"$@\"; } > \"$NINJA_LOG\"\n",
                encoding="utf-8",
            )
            fake_ninja.chmod(0o755)
            shim_root = tmp / "shim"
            env = dict(os.environ)
            env.update({
                "CUCC_PATH": str(ROOT),
                "CU2CCE_CMAKE_SHIM_DIR": str(shim_root),
                "CANN_BASE": str(tmp / "cann"),
                "ASCEND_TOOLKIT_HOME": str(tmp / "ascend"),
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "NINJA_LOG": str(log),
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
            })
            result = subprocess.run(
                [str(ROOT / "tools/ninja_cce"), "-j", "2", "all"],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(
                log.read_text(encoding="utf-8").splitlines(),
                [
                    f"CUDA_PATH={shim_root / 'CUDA_DIR'}",
                    f"CUDA_HOME={shim_root / 'CUDA_DIR'}",
                    f"CANN_BASE={tmp / 'cann'}",
                    f"ASCEND_TOOLKIT_HOME={tmp / 'ascend'}",
                    "CU2CCE_NPU_ARCH=dav-c310-vec",
                    "ARG=-j",
                    "ARG=2",
                    "ARG=all",
                ],
            )
            self.assertEqual(
                os.readlink(shim_root / "CUDA_DIR/bin/nvcc"),
                str(ROOT / "bin/cucc"),
            )

    def test_cmake_cce_accepts_attached_parallel_forms(self) -> None:
        # CMake accepts both --parallel=4 and -j4. The wrapper must preserve
        # the requested parallelism when dispatching to make_cce/ninja_cce.
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            cann_bin = tmp / "cann/bin"
            cann_bin.mkdir(parents=True)
            (cann_bin / "ccec").write_text("#!/bin/sh\nexit 0\n", encoding="utf-8")
            (cann_bin / "ccec").chmod(0o755)
            install = tmp / "install"
            (install / "bin").mkdir(parents=True)
            (install / "tools").mkdir()
            (install / "bin/cucc").write_text("#!/bin/sh\nexit 0\n", encoding="utf-8")
            (install / "bin/cucc").chmod(0o755)
            (install / "tools/fake_toolkit_env.sh").write_text(
                (ROOT / "tools/fake_toolkit_env.sh").read_text(encoding="utf-8"),
                encoding="utf-8",
            )
            (install / "tools/device_arch.sh").write_text(
                (ROOT / "tools/device_arch.sh").read_text(encoding="utf-8"),
                encoding="utf-8",
            )
            (install / "tools/device_arch.sh").chmod(0o755)
            log = tmp / "generator.log"
            fake_make = install / "bin/make_cce"
            fake_make.write_text(
                "#!/bin/sh\n"
                "for arg in \"$@\"; do printf 'ARG=%s\n' \"$arg\" >> \"$GENERATOR_LOG\"; done\n",
                encoding="utf-8",
            )
            fake_make.chmod(0o755)
            build_dir = tmp / "build with spaces"
            build_dir.mkdir()
            env = dict(os.environ)
            env.update({
                "CUCC_PATH": str(install),
                "CANN_BASE": str(tmp / "cann"),
                "ASCEND_TOOLKIT_HOME": str(tmp / "ascend"),
                "CU2CCE_CMAKE_SHIM_DIR": str(tmp / "shim"),
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "GENERATOR_LOG": str(log),
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
            })
            for parallel in ("--parallel=4", "-j4"):
                log.unlink(missing_ok=True)
                result = subprocess.run(
                    [str(ROOT / "tools/cmake_cce"), "--build", str(build_dir), parallel],
                    env=env, capture_output=True, text=True, check=False,
                )
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(log.read_text(encoding="utf-8").splitlines(), ["ARG=-j", "ARG=4"])

    def test_cmake_cce_build_dispatches_to_real_make_or_ninja_wrapper(self) -> None:
        # Exercise the wrapper boundary without CANN: fake ccec only satisfies
        # the entry gate, while fake make/ninja records the argv it receives.
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            cann_bin = tmp / "cann/bin"
            cann_bin.mkdir(parents=True)
            (cann_bin / "ccec").write_text("#!/bin/sh\nexit 0\n", encoding="utf-8")
            (cann_bin / "ccec").chmod(0o755)
            log = tmp / "generator.log"
            for name in ("make", "ninja"):
                tool = fake_bin / name
                tool.write_text(
                    "#!/bin/sh\n"
                    "printf '%s\n' \"GENERATOR=$0\" > \"$GENERATOR_LOG\"\n"
                    "for arg in \"$@\"; do printf 'ARG=%s\n' \"$arg\" >> \"$GENERATOR_LOG\"; done\n",
                    encoding="utf-8",
                )
                tool.chmod(0o755)
            env = dict(os.environ)
            env.update({
                "CUCC_PATH": str(ROOT),
                "CANN_BASE": str(tmp / "cann"),
                "ASCEND_TOOLKIT_HOME": str(tmp / "ascend"),
                "CU2CCE_CMAKE_SHIM_DIR": str(tmp / "cmake-shim"),
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "GENERATOR_LOG": str(log),
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
            })

            ninja_build = tmp / "ninja-build"
            ninja_build.mkdir()
            (ninja_build / "build.ninja").write_text("# fake\n", encoding="utf-8")
            result = subprocess.run(
                [str(ROOT / "tools/cmake_cce"), "--build", str(ninja_build),
                 "--target", "app", "--parallel", "2", "--", "--verbose"],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            ninja_lines = log.read_text(encoding="utf-8").splitlines()
            self.assertTrue(ninja_lines[0].endswith("/ninja"), ninja_lines)
            self.assertEqual(ninja_lines[1:], ["ARG=-j", "ARG=2", "ARG=app", "ARG=--verbose"])

            make_build = tmp / "make-build"
            make_build.mkdir()
            result = subprocess.run(
                [str(ROOT / "tools/cmake_cce"), "--build", str(make_build),
                 "--target", "app", "--parallel", "3"],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            make_lines = log.read_text(encoding="utf-8").splitlines()
            self.assertTrue(make_lines[0].endswith("/make"), make_lines)
            self.assertEqual(make_lines[1:], ["ARG=-j", "ARG=3", "ARG=app"])

    def test_cmake_cce_build_phase_dispatches_to_make_ninja_entries(self) -> None:
        # deal_build_param parity (cu-bridge cmake_mock:64-131): cmake_cce
        # --build must route through make_cce/ninja_cce (generator chosen by
        # build.ninja presence) instead of exec'ing cmake directly, so the
        # Make/Ninja path gets the same environment preparation.
        cmake_cce = (ROOT / "tools/cmake_cce").read_text(encoding="utf-8")
        self.assertIn('build_dir/build.ninja', cmake_cce)
        self.assertIn('"ninja_cce"', cmake_cce)
        self.assertIn('"make_cce"', cmake_cce)
        self.assertIn('--target|-t', cmake_cce)
        self.assertIn('--parallel|-j', cmake_cce)
        self.assertIn('exec cmake "$@"', cmake_cce)

    def test_make_entries_use_token_level_clean_matching(self) -> None:
        # make_cce used `[[ "$*" == *"clean"* ]]` (substring): a goal like
        # `cleanpack` would delete the fake toolkit. Both make_cce and
        # pre_cce now match clean goals as tokens only.
        for entry in ("tools/make_cce", "tools/pre_cce"):
            text = (ROOT / entry).read_text(encoding="utf-8")
            self.assertNotIn('[[ "$*" == *"clean"* ]]', text)
            self.assertIn("clean|*clean|clean-*", text)

    def test_shim_isolation_env_is_supported_by_all_entries(self) -> None:
        # cu-bridge USE_WCUDA_FOR_MULTI_JOB parity: CU2CCE_SHIM_ISOLATE=1
        # gives each top-level job its own shim ($PPID suffix).
        for entry in ("tools/cmake_cce", "tools/make_cce", "tools/pre_cce"):
            text = (ROOT / entry).read_text(encoding="utf-8")
            self.assertIn("CU2CCE_SHIM_ISOLATE", text, entry)
            self.assertIn('"1"', text)

    def test_host_only_fortran_and_mpi_wrappers_reject_cuda_sources(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            compiler = tmp / "host-compiler"
            log = tmp / "compiler.args"
            compiler.write_text(
                "#!/bin/sh\n"
                "printf '%s\n' \"$@\" > \"$COMPILER_LOG\"\n",
                encoding="utf-8",
            )
            compiler.chmod(0o755)
            env = dict(os.environ)
            env["COMPILER_LOG"] = str(log)

            result = subprocess.run(
                [str(ROOT / "tools/gfortran_cce"), "-c", "hello.f90"],
                env={**env, "CU2CCE_FORTRAN_COMPILER": str(compiler)},
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(log.read_text(encoding="utf-8").splitlines(), ["-c", "hello.f90"])
            rejected = subprocess.run(
                [str(ROOT / "tools/gfortran_cce"), "kernel.cu"],
                env={**env, "CU2CCE_FORTRAN_COMPILER": str(compiler)},
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(rejected.returncode, 2)
            self.assertIn("CUDA source", rejected.stderr)

            mpicc = tmp / "mpicc"
            mpicc.symlink_to(ROOT / "tools/mpi_cce")
            result = subprocess.run(
                [str(mpicc), "-c", "hello.c"],
                env={**env, "CU2CCE_MPICC_COMPILER": str(compiler)},
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(log.read_text(encoding="utf-8").splitlines(), ["-c", "hello.c"])
            rejected = subprocess.run(
                [str(mpicc), "kernel.cu"],
                env={**env, "CU2CCE_MPICC_COMPILER": str(compiler)},
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(rejected.returncode, 2)
            self.assertIn("CUDA source", rejected.stderr)

    def test_shim_manifest_supports_host_only_fortran_and_mpi_wrappers(self) -> None:
        # cu-bridge exposes gfortran/mpicc/mpicxx entries. The Ascend version
        # must expose them without pretending those host tools can compile CUDA
        # device code; the wrapper will reject .cu and delegate host-only work.
        layout = (ROOT / "config/shim_layout.json").read_text(encoding="utf-8")
        builder = (ROOT / "tools/build_shim_tree.py").read_text(encoding="utf-8")
        self.assertIn('"host_tool_wrappers"', layout)
        self.assertIn("host_tool_wrappers", builder)
        for name in ("gfortran_cce", "mpi_cce"):
            self.assertTrue((ROOT / "tools" / name).is_file())
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("tools/gfortran_cce", cmake)
        self.assertIn("tools/mpi_cce", cmake)
        for script in (ROOT / "tools/gfortran_cce", ROOT / "tools/mpi_cce"):
            text = script.read_text(encoding="utf-8")
            self.assertIn("CUDA source", text)
            self.assertIn('exec "$compiler" "$@"', text)

    def test_shim_builder_is_idempotent_and_produces_the_manifest_layout(self) -> None:
        # Functional check of the shared builder (no CANN/ccec needed): the
        # produced tree must carry the toolkit shape, the CUDA-named alias
        # family pointing at the bridge runtime, hostcc entries, and must be
        # idempotent (re-running with a different nvcc target re-points).
        import json
        import os
        import subprocess as sp
        import sys
        import tempfile

        sys.path.insert(0, str(ROOT / "tools"))
        import build_shim_tree  # noqa: PLC0415

        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            cucc = tmp / "install"
            (cucc / "bin").mkdir(parents=True)
            (cucc / "include").mkdir()
            (cucc / "include/cuda_runtime.h").write_text("/* shim */\n", encoding="utf-8")
            (cucc / "lib").mkdir()
            (cucc / "lib/libcce_bridge_runtime.so").write_text("", encoding="utf-8")
            (cucc / "cmake_modules").mkdir()
            (cucc / "cmake_modules/FindCUDAToolkit.cmake").write_text("", encoding="utf-8")
            (cucc / "tools").mkdir()
            (cucc / "tools/hostcc_cce").write_text("#!/bin/sh\n", encoding="utf-8")
            (cucc / "tools/gfortran_cce").write_text("#!/bin/sh\n", encoding="utf-8")
            (cucc / "tools/mpi_cce").write_text("#!/bin/sh\n", encoding="utf-8")
            (cucc / "bin/cucc").write_text("#!/bin/sh\n", encoding="utf-8")

            cuda_dir = tmp / "cuda"
            hostbin = tmp / "hostbin"
            result = sp.run([
                sys.executable,
                str(ROOT / "tools/build_shim_tree.py"),
                "--cuda-dir", str(cuda_dir),
                "--cucc-path", str(cucc),
                "--nvcc-target", str(cucc / "bin/cucc"),
                "--hostcc-bin-dir", str(hostbin),
                "--hostcc-mode", "wrapper",
            ], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)

            self.assertTrue((cuda_dir / "include/cuda_runtime.h").is_file())
            self.assertEqual(os.readlink(cuda_dir / "bin/nvcc"), str(cucc / "bin/cucc"))
            self.assertEqual(os.readlink(cuda_dir / "bin/cucc"), str(cucc / "bin/cucc"))
            self.assertEqual(os.readlink(hostbin / "g++"), str(cucc / "tools/hostcc_cce"))
            self.assertEqual(os.readlink(hostbin / "gfortran"), str(cucc / "tools/gfortran_cce"))
            self.assertEqual(os.readlink(hostbin / "mpicc"), str(cucc / "tools/mpi_cce"))
            self.assertEqual(os.readlink(hostbin / "mpicxx"), str(cucc / "tools/mpi_cce"))
            self.assertEqual(os.readlink(cuda_dir / "lib64/libcudart.so"),
                             str(cucc / "lib/libcce_bridge_runtime.so"))
            self.assertEqual(os.readlink(cuda_dir / "lib64/libcublas.so"),
                             str(cucc / "lib/libcce_bridge_runtime.so"))
            self.assertEqual(os.readlink(cuda_dir / "lib64/stubs/libcuda.so.1"),
                             str(cucc / "lib/libcce_bridge_runtime.so"))
            self.assertTrue((cuda_dir / "version.txt").read_text(encoding="utf-8").startswith("CUDA Version 11.6"))
            self.assertTrue((cuda_dir / "lib64/cmake/CUDAToolkit/CUDAToolkitConfig.cmake").is_file())
            self.assertTrue((cuda_dir / "extras/CUPTI/include").is_dir())
            triple_dir = cuda_dir / "targets" / build_shim_tree.host_triple()
            self.assertTrue(triple_dir.is_dir(), triple_dir)

            # Idempotent re-run re-points the shim nvcc without disturbing
            # the rest of the tree.
            rerun = sp.run([
                sys.executable,
                str(ROOT / "tools/build_shim_tree.py"),
                "--cuda-dir", str(cuda_dir),
                "--cucc-path", str(cucc),
                "--nvcc-target", str(cucc / "tools/nvcc_entry.sh"),
                "--quiet",
            ], capture_output=True, text=True)
            self.assertEqual(rerun.returncode, 0, rerun.stderr)
            self.assertEqual(os.readlink(cuda_dir / "bin/nvcc"), str(cucc / "tools/nvcc_entry.sh"))
            self.assertTrue((cuda_dir / "lib64/libcudart.so").is_symlink())

    def test_mock_abi_probe_compiles_shipped_stand_in(self) -> None:
        # cu-bridge deal_abi parity without prebuilt artifacts: the ABI probe
        # compiles the shipped CMakeCUDACompilerABI.cpp with the host g++ so
        # CMake's binary scan finds the INFO:... strings; the placeholder is
        # only a fallback.
        import importlib.util  # noqa: PLC0415
        mock_path = ROOT / "tools/mock/mock_nvcc.py"
        spec = importlib.util.spec_from_file_location("mock_nvcc_abi2", mock_path)
        assert spec is not None and spec.loader is not None
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        self.assertTrue((ROOT / "tools/mock/CMakeCUDACompilerABI.cpp").is_file())
        self.assertTrue((ROOT / "tools/mock/CMakeCompilerABI.h").is_file())

        cwd = Path(self.tmpdir())
        obj = cwd / "abi-probe.o"
        code = module._handle_abi_probe(
            ["-c", "Modules/CMakeCUDACompilerABI.cu", "-o", str(obj)], cwd)
        self.assertEqual(code, 0)
        self.assertTrue(obj.is_file())
        binary = obj.read_bytes()
        # Platform note: macOS produces Mach-O objects, Linux ELF — the
        # contract is a real host-compiled binary carrying the ABI probe
        # strings, not a specific container format.
        self.assertGreater(len(binary), 64, "expected a real host-compiled object")
        self.assertIn(b"INFO:sizeof_dptr[", binary)
        # Placeholder fallback path still exists for host-compiler-less hosts.
        fallback = cwd / "fallback.o"
        module_orig = module._abi_source_path
        module._abi_source_path = lambda: Path("/nonexistent/CMakeCUDACompilerABI.cpp")
        try:
            self.assertEqual(module._handle_abi_probe(
                ["-c", "Modules/CMakeCUDACompilerABI.cu", "-o", str(fallback)], cwd), 0)
        finally:
            module._abi_source_path = module_orig
        self.assertEqual(fallback.read_bytes(), b"\x7fELFcu2cce-abi-placeholder")

    def tmpdir(self) -> Path:
        import tempfile  # noqa: PLC0415
        return Path(tempfile.mkdtemp(prefix="cu2cce-abi-test-"))

    def test_install_tree_provides_nvcc_symlink(self) -> None:
        # cu-bridge user manual FAQ Q2 workaround, automated: the install tree
        # carries bin/nvcc -> bin/cucc so Makefiles hard-coding `nvcc` work
        # with only $CUCC_PATH/bin on PATH.
        top = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("file(CREATE_LINK cucc", top)
        self.assertIn("bin/nvcc SYMBOLIC", top)

    def test_runtime_parity_surface_round2(self) -> None:
        # Second full-surface audit runtime additions: error-name table,
        # GetDeviceFlags, NVTX no-op shim, math_constants.h.
        cpp = (ROOT / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        header = (ROOT / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        self.assertIn('return entry.name;', cpp)  # stateless per-code name table
        self.assertIn('"cudaErrorNotSupported"', cpp)
        self.assertIn("wcudaGetDeviceFlags(unsigned int *flags)", header)
        self.assertIn("*flags = 0;", cpp)
        for f in ("include/nvtx3/nvToolsExt.h", "include/nvToolsExt.h",
                  "include/bridge/tools_ext/nvtx_stub.h", "include/math_constants.h"):
            self.assertTrue((ROOT / f).is_file(), f)
        nvtx = (ROOT / "include/bridge/tools_ext/nvtx_stub.h").read_text(encoding="utf-8")
        for macro in ("nvtxRangePushA", "nvtxRangePop", "nvtxMarkA",
                      "nvtxRangeStartA", "nvtxRangeEnd"):
            self.assertIn(f"#define {macro}", nvtx)
        self.assertIn("no-op", nvtx)  # honest contract: no fabricated profiling data
        mc = (ROOT / "include/math_constants.h").read_text(encoding="utf-8")
        self.assertIn("M_PIf", mc)
        self.assertIn("CUDART_INF_F", mc)
        self.assertIn("HOST-side use", mc)  # double constants carry the FP64 warning
        # parity audit document exists and records the three comparison tables
        audit = (ROOT / "docs/03_Comparison/PARITY_AUDIT_2026-08-28.md").read_text(encoding="utf-8")
        self.assertIn("-dc", audit)
        self.assertIn("libsymbol_cu", audit)
        self.assertIn("thrust", audit)

    def test_alignment_doc_records_cubridge_mechanism_and_reading_map(self) -> None:
        doc = (ROOT / "docs/03_Comparison/CMAKE_MAKE_ALIGNMENT_WITH_CUBRIDGE.md").read_text(encoding="utf-8")
        for section in ("cmake_maca", "cmake_mock", "cmake_assist", "link.json",
                        "USE_CMAKE_MACA", "预编译真 ELF", "双阶段协议",
                        "逐项对齐表", "代码导读", "剩余差距"):
            self.assertIn(section, doc)
        self.assertIn("docs/02_Architecture/06_cmake_principle_flow.md", doc)

    def test_findcuda_does_not_define_case_colliding_recursive_aliases(self) -> None:
        # CMake command names are case-insensitive. Defining CUDA_ADD_EXECUTABLE
        # after cuda_add_executable replaces the same command and a wrapper
        # call recurses forever (the server failure repeated FindCUDA.cmake:161).
        findcuda = (ROOT / "cmake_modules/FindCUDA.cmake").read_text(encoding="utf-8")
        definitions = re.findall(
            r"^\s*(?:macro|function)\(\s*([A-Za-z_][A-Za-z0-9_]*)",
            findcuda, flags=re.MULTILINE,
        )
        definition_names = [name.lower() for name in definitions]
        for command in (
            "cuda_add_executable", "cuda_add_library", "cuda_compile",
            "cuda_wrap_srcs", "cuda_include_directories",
        ):
            self.assertEqual(definition_names.count(command), 1, command)

    def test_findcuda_supports_common_legacy_helpers_and_rejects_unmapped_libs(self) -> None:
        findcuda = (ROOT / "cmake_modules/FindCUDA.cmake").read_text(encoding="utf-8")
        for macro in (
            "macro(CUDA_GET_SOURCES_AND_OPTIONS",
            "macro(cuda_compile_base",
            "macro(CUDA_BUILD_CLEAN_TARGET",
            "macro(cuda_unset_include_and_libraries",
        ):
            self.assertIn(macro, findcuda)
        with tempfile.TemporaryDirectory() as tmp_name:
            script = Path(tmp_name) / "reject.cmake"
            script.write_text(
                f'include("{ROOT / "cmake_modules/FindCUDA.cmake"}")\n'
                'CUDA_ADD_CUBLAS_TO_TARGET(app)\n',
                encoding="utf-8",
            )
            result = subprocess.run(
                ["cmake", "-P", str(script)],
                capture_output=True, text=True, check=False,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("no validated Ascend equivalent", result.stderr)

    def test_findcuda_exposes_cubridge_legacy_macro_names_with_npu_limits(self) -> None:
        findcuda = (ROOT / "cmake_modules/FindCUDA.cmake").read_text(encoding="utf-8")
        # CMake command names are case-insensitive; lowercase definitions
        # provide both spellings without recursive duplicate aliases.
        for macro in (
            "function(cuda_add_executable target)",
            "function(cuda_add_library target)",
            "macro(cuda_compile out_var)",
            "macro(cuda_wrap_srcs cuda_target format generated_files)",
            "macro(CUDA_ADD_CUBLAS_TO_TARGET",
            "macro(CUDA_ADD_CUFFT_TO_TARGET",
            "macro(CUDA_ADD_CUSOLVER_TO_TARGET",
            "macro(CUDA_ADD_CUSPARSE_TO_TARGET",
            "macro(CUDA_ADD_CURAND_TO_TARGET",
        ):
            self.assertIn(macro, findcuda)
        self.assertIn("no validated Ascend equivalent", findcuda)
        self.assertIn("CUDA_LIBRARIES", findcuda)

    def test_findcuda_helpers_preserve_common_configuration_values(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            script = tmp / "helpers.cmake"
            dep = tmp / "cuda-deps.cmake"
            script.write_text(
                f'include("{ROOT / "cmake_modules/FindCUDA.cmake"}")\n'
                'set(CUDA_configuration_types Debug Release)\n'
                'CUDA_PARSE_NVCC_OPTIONS(_opts DEBUG -O0 RELEASE -O2)\n'
                'CUDA_BUILD_SHARED_LIBRARY(_shared)\n'
                f'CUDA_INCLUDE_NVCC_DEPENDENCIES("{dep}")\n'
                'CUDA_COMPUTE_BUILD_PATH("/tmp/path with spaces/file.o" _build)\n'
                'message(STATUS "opts_debug=${_opts_DEBUG}")\n'
                'message(STATUS "opts_release=${_opts_RELEASE}")\n'
                'message(STATUS "shared=${_shared}")\n'
                'message(STATUS "build=${_build}")\n',
                encoding="utf-8",
            )
            result = subprocess.run(
                ["cmake", "-P", str(script)],
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("opts_debug=-O0", result.stdout)
            self.assertIn("opts_release=-O2", result.stdout)
            self.assertIn("shared=STATIC", result.stdout)
            self.assertTrue(dep.is_file())

    def test_findcuda_covers_remaining_reference_helper_names_with_npu_guards(self) -> None:
        findcuda = (ROOT / "cmake_modules/FindCUDA.cmake").read_text(encoding="utf-8")
        for helper in (
            "macro(CUDA_FIND_HELPER_FILE",
            "macro(CUDA_INCLUDE_NVCC_DEPENDENCIES",
            "macro(cuda_find_library_local_first_with_path_ext",
            "macro(cuda_find_library_local_first",
            "macro(find_library_local_first",
            "macro(CUDA_PARSE_NVCC_OPTIONS",
            "function(CUDA_ADD_CUDA_INCLUDE_ONCE",
            "function(CUDA_BUILD_SHARED_LIBRARY",
            "function(CUDA_COMPUTE_BUILD_PATH",
            "function(_cuda_get_important_host_flags",
            "function(CUDA_COMPUTE_SEPARABLE_COMPILATION_OBJECT_FILE_NAME",
            "function(CUDA_LINK_SEPARABLE_COMPILATION_OBJECTS",
        ):
            self.assertIn(helper, findcuda)
        self.assertIn("CUDA_SEPARABLE_COMPILATION OFF", findcuda)
        self.assertIn("unsupported on Ascend puresimt", findcuda)

    def test_findcuda_uses_shared_legacy_helpers(self) -> None:
        # Keep the legacy CMake surface easy to audit: repeated CUDA-language
        # enablement and target setup should live in private helpers, while
        # Ascend-specific unsupported artifact paths remain explicit.
        findcuda = (ROOT / "cmake_modules/FindCUDA.cmake").read_text(encoding="utf-8")
        self.assertIn("function(_cu2cce_enable_cuda)", findcuda)
        self.assertIn("function(_cu2cce_attach_target", findcuda)
        for public in (
            "function(cuda_add_executable target)",
            "function(cuda_add_library target)",
            "macro(cuda_compile out_var)",
            "macro(cuda_wrap_srcs cuda_target format generated_files)",
        ):
            self.assertIn(public, findcuda)
        for unsupported in (
            "CUDA PTX generation is unsupported on Ascend puresimt",
            "CUDA FATBIN generation is unsupported on Ascend puresimt",
            "CUDA CUBIN generation is unsupported on Ascend puresimt",
        ):
            self.assertIn(unsupported, findcuda)

    def test_findcuda_legacy_macros_and_ninja_two_phase(self) -> None:
        # Fourth alignment batch: legacy FindCUDA macro surface (cuda_compile
        # as a LANGUAGE-CUDA passthrough — real FindCUDA produces objects,
        # so the earlier FATAL_ERROR was over-restrictive), CUDA_NVCC_FLAGS
        # consumption, honest PTX/CUBIN/FATBIN errors, and ninja_cce joining
        # the two-phase protocol (ninja_maca parity).
        findcuda = (ROOT / "cmake_modules/FindCUDA.cmake").read_text(encoding="utf-8")
        self.assertIn("macro(cuda_compile out_var)", findcuda)
        self.assertIn("macro(cuda_wrap_srcs cuda_target format generated_files)", findcuda)
        self.assertIn("set(_nvcc_options ${CUDA_NVCC_FLAGS})", findcuda)
        self.assertIn("enable_language(CUDA)", findcuda)
        for honest in ("macro(cuda_compile_ptx out_var)", "macro(cuda_compile_fatbin out_var)",
                       "macro(cuda_compile_cubin out_var)"):
            self.assertIn(honest, findcuda)

        ninja = (ROOT / "tools/ninja_cce").read_text(encoding="utf-8")
        self.assertIn('CU2CCE_CMAKE_SHIM_DIR:-${CU2CCE_HOME}/cu2cce-bridge/cmake-shim', ninja)
        self.assertIn("ninja_cce: build phase: shim nvcc ->", ninja)
        self.assertIn('cu2cce_prepare_fake_toolkit', ninja)
        self.assertIn('"$CUCC_PATH/bin/cucc"', ninja)

        # plain-g++ link carrier: proves the lib64 symlink family end-to-end
        gxx_dir = ROOT / "examples/build_compat/make_gxx_link"
        self.assertTrue((gxx_dir / "Makefile").is_file())
        makefile = (gxx_dir / "Makefile").read_text(encoding="utf-8")
        self.assertIn("-L$(CUDA_PATH)/lib64 -lcudart", makefile)
        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("make_gxx_link: plain-g++ link with -lcudart -> bridge runtime", server_cmd)

    def test_pre_cce_preserves_argument_boundaries(self) -> None:
        # pre_cce receives an argv vector and must not reconstruct it through
        # eval/$*: values containing spaces or shell syntax are user data, not
        # commands. This is the NPU equivalent of the cu-bridge pre_makefile
        # wrapper contract, with safer direct execution semantics.
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            recorder = tmp / "record_args.py"
            output = tmp / "argv.txt"
            recorder.write_text(
                "#!/usr/bin/env python3\n"
                "import os, sys\n"
                "from pathlib import Path\n"
                "Path(os.environ['ARGV_OUT']).write_text(\n"
                "    chr(10).join(sys.argv[1:]) + chr(10), encoding='utf-8'\n"
                ")\n",
                encoding="utf-8",
            )
            recorder.chmod(0o755)
            env = dict(os.environ)
            env.update({
                "CUCC_PATH": str(ROOT),
                "CU2CCE_HOME": str(tmp / "shim-home"),
                "CANN_BASE": str(tmp / "cann"),
                "ARGV_OUT": str(output),
            })
            result = subprocess.run(
                [str(ROOT / "tools/pre_cce"), str(recorder),
                 "alpha beta", "$(printf unsafe)"],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(
                output.read_text(encoding="utf-8").splitlines(),
                ["alpha beta", "$(printf unsafe)"],
            )

    def test_pre_cce_preserves_wrapped_exit_status(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            command = tmp / "fail.sh"
            command.write_text("#!/bin/sh\nexit 37\n", encoding="utf-8")
            command.chmod(0o755)
            env = dict(os.environ)
            env.update({
                "CUCC_PATH": str(ROOT),
                "CU2CCE_HOME": str(tmp / "shim-home"),
                "CANN_BASE": str(tmp / "cann"),
            })
            result = subprocess.run(
                [str(ROOT / "tools/pre_cce"), str(command)],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 37, result.stderr)

    def test_cubridge_parity_tools_are_shipped(self) -> None:
        # Third alignment batch: multi_target/get_device_arch -> device_arch,
        # pre_makefile -> pre_cce, bin/gnu -> hostcc_cce, CUPTI layout.
        for f in ("tools/device_arch.sh", "tools/pre_cce", "tools/hostcc_cce"):
            self.assertTrue((ROOT / f).is_file(), f)
        top = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("install(PROGRAMS tools/device_arch.sh tools/pre_cce tools/hostcc_cce", top)

        device_arch = (ROOT / "tools/device_arch.sh").read_text(encoding="utf-8")
        # env-first, then npu-smi device query, then cache, then default —
        # mirroring cu-bridge multi_target.sh semantics.
        self.assertIn('CU2CCE_NPU_ARCH', device_arch)
        self.assertIn("npu-smi info", device_arch)
        self.assertIn("dav-c310-vec", device_arch)

        pre_cce = (ROOT / "tools/pre_cce").read_text(encoding="utf-8")
        # Fifth parity batch: the fake toolkit comes from the shared builder;
        # seventh batch: sentinel/builder calls moved into the sourced helper
        # tools/fake_toolkit_env.sh (pre_cce binds the shim nvcc to the real
        # cucc like make_cce, and its host compiler entries go through
        # hostcc_cce).
        self.assertIn('. "${CUCC_PATH}/tools/fake_toolkit_env.sh"', pre_cce)
        self.assertIn('cu2cce_prepare_fake_toolkit "$DST_WCUDA" "$CUCC_PATH/bin/cucc"', pre_cce)
        self.assertIn('exec "$@"', pre_cce)
        self.assertNotIn('eval "$*"', pre_cce)

        hostcc = (ROOT / "tools/hostcc_cce").read_text(encoding="utf-8")
        # bin/gnu parity, NPU-adapted: real compiler for compiles, and the
        # link injection is rule-driven (hostcc.link in cu2cce_rules.json,
        # consumed by the driver's --hostcc-mode). The script keeps its
        # no-bridge degradation to the plain system compiler.
        self.assertIn('exec "/usr/bin/$name" "$@"', hostcc)
        self.assertIn("--hostcc-mode", hostcc)
        rules_text = (ROOT / "config/cu2cce_rules.json").read_text(encoding="utf-8")
        self.assertIn('"hostcc"', rules_text)
        self.assertIn('"-lcce_bridge_runtime"', rules_text)
        self.assertIn("is a CUDA source",
                      (ROOT / "tools/cu2cce_driver.py").read_text(encoding="utf-8"))

        for wrapper in ("tools/cmake_cce", "tools/make_cce"):
            text = (ROOT / wrapper).read_text(encoding="utf-8")
            self.assertIn("tools/fake_toolkit_env.sh", text)
            self.assertIn("cu2cce_resolve_npu_arch", text)
        helper_text = (ROOT / "tools/fake_toolkit_env.sh").read_text(encoding="utf-8")
        self.assertIn("tools/device_arch.sh", helper_text)
        self.assertIn("tools/build_shim_tree.py", helper_text)
        self.assertIn("hostcc_cce",
                      (ROOT / "tools/build_shim_tree.py").read_text(encoding="utf-8"))
        layout_text = (ROOT / "config/shim_layout.json").read_text(encoding="utf-8")
        self.assertIn("extras/CUPTI/include", layout_text)
        # the driver already accepts --options-file (cu-bridge conf.json parity)
        self.assertIn('"--options-file"', (ROOT / "tools/cu2cce_driver.py").read_text(encoding="utf-8"))

    def test_build_parity_keeps_mac_specific_paths_out_of_npu_wrappers(self) -> None:
        # Mechanism parity must not become literal MACA backend copying. The
        # Ascend wrappers use ccec/g++/CANN instead of mxcc, -fgpu-rdc, or
        # --maca-link, and system cmake replacement stays deliberately absent.
        for name in ("tools/cmake_cce", "tools/make_cce", "tools/ninja_cce", "tools/pre_cce"):
            text = (ROOT / name).read_text(encoding="utf-8")
            self.assertNotIn("mxcc", text, name)
            self.assertNotIn("--maca-link", text, name)
            self.assertNotIn("-fgpu-rdc", text, name)
        self.assertNotIn("USE_CMAKE_MACA", (ROOT / "tools/cmake_cce").read_text(encoding="utf-8"))
        self.assertIn("ccec", (ROOT / "tools/cmake_cce").read_text(encoding="utf-8"))

    def test_server_cmd_external_success_paths_use_compact_terminal_summaries(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        # Detailed external/diagnostic output is already redirected to per-case
        # logs. Successful cases must print status plus a log pointer, not dump
        # ACL/wcuda traces into the terminal. Failure branches may still tail
        # logs for diagnosis.
        self.assertIn("[DIAG-PASS] [$sample]", server)
        self.assertNotIn('cat "$diag_log"', server)
        self.assertIn("details: $diag_log", server)
        self.assertIn("details: $run_log", server)

    def test_round9_query_arch_and_options_probes_are_wired_into_the_harness(self) -> None:
        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("-E query passthrough", server_cmd)
        self.assertIn("--options-file", server_cmd)
        self.assertIn("CUCC_TARGETS=dav-c310-vec", server_cmd)
        self.assertIn("direct ninja_cce", server_cmd)
        self.assertIn("CUCC_USE_GOPENMP=1", server_cmd)
        self.assertIn("response_files", (ROOT / "tools/cu2cce_driver.py").read_text(encoding="utf-8"))

    def test_round8_machinery_probes_are_wired_into_the_harness(self) -> None:
        # The real-machine harness locks the batch-5..8 runtime mechanisms
        # without device execution: query passthrough, ARCH_MACRO auto +
        # __CUDACC__ + preinclude via --dryrun greps, -Xlinker/fast-math/
        # nvrtc handling, and the cmake_cce --build dispatch probe.
        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("queries 直通：-dumpmachine 由 ccec 原生应答", server_cmd)
        # 8th-batch fix: the final command is shlex-joined, so inject tokens
        # arrive either attached (-D__CUDA_ARCH__=860) or space-separated
        # (-D __CUDA_ARCH__=860); the probe must accept both spellings.
        self.assertIn('__CUDA_ARCH__=860|__CUDA_ARCH__ = 860', server_cmd)
        self.assertIn("--use_fast_math", server_cmd)
        self.assertIn("-ffast-math", server_cmd)
        self.assertIn("cmake_cce --build 分流 make_cce（deal_build_param parity）", server_cmd)
        # The probes only run in the full profile and must be able to fail
        # the section badge (no silent skips for broken machinery).
        self.assertIn("CU2CCE_ARCH_MACRO=auto", server_cmd)
        self.assertGreaterEqual(server_cmd.count("CUCC_RULES_PASS=0"), 4)

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
