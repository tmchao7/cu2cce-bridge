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
        # builder (re-pointing here makes repeated configure/build cycles on
        # one persistent shim deterministic).
        self.assertIn('--nvcc-target "$CUCC_PATH/tools/nvcc_entry.sh"', cmake_cce)
        # Host compilers as toolkit-relative entries (their pre_cmake) come
        # from the builder's hostcc modes now.
        self.assertIn('--hostcc-mode wrapper', cmake_cce)
        # The CUDA-named runtime alias family lives in the shared manifest.
        layout = (ROOT / "config/shim_layout.json").read_text(encoding="utf-8")
        for lib in ("libcudart.so", "libcudart_static.a", "libcudadevrt.a",
                    "libcuda.so", "libcuda.so.1", "libcublas.so",
                    "libcublas.so.1", "libnvToolsExt.so"):
            self.assertIn(lib, layout)
        make_cce = (ROOT / "tools/make_cce").read_text(encoding="utf-8")
        # targets triple derivation moved into build_shim_tree.py
        # (uname machine/system, amd64/arm64 normalization).
        builder = (ROOT / "tools/build_shim_tree.py").read_text(encoding="utf-8")
        self.assertIn("amd64", builder)
        self.assertIn("aarch64", builder)
        self.assertIn('"libcudart.so"', layout)
        self.assertIn('lib64/cmake/CUDAToolkit', layout)
        # make.log sentinel is actually created (persistent fake toolkit).
        self.assertIn('> "$DST_WCUDA/make.log"', make_cce)
        self.assertIn('PATH="$DST_WCUDA/bin:$CUDA_DIR/bin:$PATH"', make_cce)
        # make_cce binds the shim nvcc to the real cucc through the builder.
        self.assertIn('--nvcc-target "$CUCC_PATH/bin/cucc"', make_cce)

    def test_shim_layout_manifest_is_single_source_for_three_entries(self) -> None:
        # Fifth parity batch: cmake_cce/make_cce/pre_cce used to carry three
        # hand-written copies of the mkdir/ln sequences (cu-bridge keeps ONE
        # manifest: tools/link.json + tools/mock/stub_link.json). All three
        # entries must now consume config/shim_layout.json via
        # tools/build_shim_tree.py.
        builder = ROOT / "tools/build_shim_tree.py"
        self.assertTrue(builder.is_file())
        for entry in ("tools/cmake_cce", "tools/make_cce", "tools/pre_cce"):
            text = (ROOT / entry).read_text(encoding="utf-8")
            self.assertIn("tools/build_shim_tree.py", text, entry)
            self.assertIn("config/shim_layout.json", builder.read_text(encoding="utf-8"))
        for section in ("dirs", "dir_links", "file_links", "hostcc_names",
                        "cuda_lib_aliases", "alias_dirs", "alias_target", "files"):
            self.assertIn(f'"{section}"', (ROOT / "config/shim_layout.json").read_text(encoding="utf-8"))

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
        self.assertIn('SHIM_ROOT="${CU2CCE_CMAKE_SHIM_DIR:-${CU2CCE_HOME:-${CUBRIDGE_HOME:-$HOME}}/cu2cce-bridge/cmake-shim}"', ninja)
        self.assertIn("ninja_cce: build phase: shim nvcc ->", ninja)
        self.assertIn('ln -sfn "$CUCC_PATH/bin/cucc" "$CUDA_DIR/bin/nvcc"', ninja)

        # plain-g++ link carrier: proves the lib64 symlink family end-to-end
        gxx_dir = ROOT / "examples/build_compat/make_gxx_link"
        self.assertTrue((gxx_dir / "Makefile").is_file())
        makefile = (gxx_dir / "Makefile").read_text(encoding="utf-8")
        self.assertIn("-L$(CUDA_PATH)/lib64 -lcudart", makefile)
        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("make_gxx_link: plain-g++ link with -lcudart -> bridge runtime", server_cmd)

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
        self.assertIn('echo 1 > "$DST_WCUDA/make.log"', pre_cce)
        self.assertIn('eval "$*"', pre_cce)
        # Fifth parity batch: the fake toolkit comes from the shared builder;
        # pre_cce binds the shim nvcc to the real cucc like make_cce, and its
        # host compiler entries go through hostcc_cce (was: identity links).
        self.assertIn('--nvcc-target "$CUCC_PATH/bin/cucc"', pre_cce)
        self.assertIn('--hostcc-mode wrapper', pre_cce)

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
            self.assertIn("tools/build_shim_tree.py", text)
            self.assertIn("tools/device_arch.sh", text)
        builder_text = (ROOT / "tools/build_shim_tree.py").read_text(encoding="utf-8")
        self.assertIn("hostcc_cce", builder_text)
        layout_text = (ROOT / "config/shim_layout.json").read_text(encoding="utf-8")
        self.assertIn("extras/CUPTI/include", layout_text)
        # the driver already accepts --options-file (cu-bridge conf.json parity)
        self.assertIn('"--options-file"', (ROOT / "tools/cu2cce_driver.py").read_text(encoding="utf-8"))

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
