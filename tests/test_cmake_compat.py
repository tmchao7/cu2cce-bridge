#!/usr/bin/env python3
"""Static contract checks for the CMake/build compatibility surface."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class CMakeCompatibilityContractTest(unittest.TestCase):
    def test_reference_layout_uses_real_files_and_ascend_backend_names(self) -> None:
        import json
        canonical = {
            ROOT / "bin/conf.json": False,
            ROOT / "bin/gnu": False,
            ROOT / "bin/gfortran": False,
            ROOT / "tools/link.json": False,
            ROOT / "tools/mock/stub_link.json": False,
            ROOT / "tools/mock/virtualDir.py": False,
            ROOT / "tools/mock/nvcc_entry.sh": False,
            ROOT / "tools/virtual_toolkit_env.sh": False,
        }
        for path in canonical:
            self.assertTrue(path.is_file(), path)
            self.assertFalse(path.is_symlink(), path)
        conf = json.loads((ROOT / "bin/conf.json").read_text(encoding="utf-8"))
        for key in ("data", "skipstring", "condition", "not_found"):
            self.assertIn(key, conf)
        for phase in ("all", "compile", "link", "compile_and_link"):
            for key in ("filter", "replacer", "adder"):
                self.assertIn(key, conf[phase])
        self.assertEqual(conf["cu2cce"]["backend"], "ascend-cann")
        self.assertEqual(conf["cu2cce"]["compiler"], "bisheng-ccec")
        self.assertEqual(conf["cu2cce"]["frontend"]["compiler"], "ccec")
        self.assertNotEqual(conf["cu2cce"]["runtime"], "runtime_cu.so")
        self.assertNotEqual(conf["cu2cce"]["runtime"], "symbol_cu.so")
        gnu = (ROOT / "bin/gnu").read_text(encoding="utf-8")
        for compiler_name in ("c++", "g++", "gcc", "cc"):
            self.assertIn(compiler_name, gnu)
        self.assertIn("os.path.realpath", gnu)

    def test_cucc_conf_is_the_single_canonical_rule_document(self) -> None:
        import json
        conf = json.loads((ROOT / "bin/conf.json").read_text(encoding="utf-8"))
        for key in ("schema_version", "data", "skipstring", "condition", "not_found", "cu2cce"):
            self.assertIn(key, conf)
        for phase in ("all", "compile", "link", "compile_and_link"):
            for action in ("filter", "replacer", "adder"):
                self.assertIn(action, conf[phase])
        self.assertEqual(conf["cu2cce"]["backend"], "ascend-cann")
        self.assertEqual(conf["cu2cce"]["compiler"], "bisheng-ccec")
        driver = (ROOT / "tools/cu2cce_driver.py").read_text(encoding="utf-8")
        self.assertIn('self.cucc_path / "bin" / "conf.json"', driver)
        self.assertNotIn('self.cucc_path / "config" / "cu2cce_rules.json"', driver)

    def test_migration_only_builder_and_helper_files_are_removed(self) -> None:
        for path in (
            ROOT / "config/shim_layout.json",
            ROOT / "tools/build_shim_tree.py",
            ROOT / "tools/fake_toolkit_env.sh",
            ROOT / "bin/process_args.py",
        ):
            self.assertFalse(path.exists(), path)

    def test_cubridge_parity_audit_defaults_to_muxi_reference_checkout(self) -> None:
        audit = (ROOT / "tools/compare_cubridge_parity.py").read_text(encoding="utf-8")
        self.assertIn("cu-bridge-muxi", audit)
        result = subprocess.run(
            ["python3", str(ROOT / "tools/compare_cubridge_parity.py"), "--format", "json"],
            capture_output=True, text=True, check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        import json
        report = json.loads(result.stdout)
        self.assertTrue(report["reference"]["path"].endswith("/cu-bridge-muxi"))
        self.assertEqual(report["reference"]["commit"], "6ab1076")

    def test_cubridge_parity_audit_reports_backend_aware_sections(self) -> None:
        # Stage-0 baseline: compare the checked-in cu-bridge reference assets
        # with the NPU implementation without treating MACA-only mappings as
        # missing Ascend features.
        audit = ROOT / "tools/compare_cubridge_parity.py"
        self.assertTrue(audit.is_file())
        result = subprocess.run(
            [
                "python3", str(audit),
                "--reference", str(ROOT.parent / "cu-bridge-muxi"),
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
        self.assertGreater(statuses["DIAGNOSTIC_ONLY"], 0)
        self.assertGreater(report["reference_assets"]["cuda_library_aliases"], 0)
        self.assertFalse(report["reference_assets"]["missing_findcuda_macros"])
        self.assertTrue(report["reference_assets"]["missing_link_names"])

    def test_cmake_cce_accepts_explicit_ccec_path_without_cann_base_bin(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            fake_cmake = fake_bin / "cmake"
            fake_cmake.write_text("#!/bin/sh\nprintf '%s\n' \"CCEC_PATH=$CCEC_PATH\" > \"$CMAKE_LOG\"\n", encoding="utf-8")
            fake_cmake.chmod(0o755)
            explicit_ccec = tmp / "versioned-cann/bin/ccec"
            explicit_ccec.parent.mkdir(parents=True)
            explicit_ccec.write_text("#!/bin/sh\nexit 0\n", encoding="utf-8")
            explicit_ccec.chmod(0o755)
            env = dict(os.environ)
            env.update({
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
                "CUCC_PATH": str(ROOT),
                "CCEC_PATH": str(explicit_ccec),
                "CANN_BASE": str(tmp / "missing-cann"),
                "ASCEND_TOOLKIT_HOME": str(tmp / "ascend"),
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "CU2CCE_CMAKE_SHIM_DIR": str(tmp / "shim"),
                "CMAKE_LOG": str(tmp / "cmake.log"),
            })
            result = subprocess.run(
                [str(ROOT / "tools/cmake_cce"), "-S", str(tmp), "-B", str(tmp / "build")],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn(str(explicit_ccec), (tmp / "cmake.log").read_text(encoding="utf-8"))

    def test_cmake_default_shim_is_scoped_by_build_directory(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            fake_cmake = fake_bin / "cmake"
            fake_cmake.write_text(
                "#!/bin/sh\nprintf '%s\n' \"$CU2CCE_CMAKE_SHIM_DIR\" >> \"$SHIM_LOG\"\n",
                encoding="utf-8",
            )
            fake_cmake.chmod(0o755)
            ccec = tmp / "cann/bin/ccec"
            ccec.parent.mkdir(parents=True)
            ccec.write_text("#!/bin/sh\nexit 0\n", encoding="utf-8")
            ccec.chmod(0o755)
            env = dict(os.environ)
            env.update({
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
                "CUCC_PATH": str(ROOT),
                "CCEC_PATH": str(ccec),
                "CANN_BASE": str(tmp / "cann"),
                "ASCEND_TOOLKIT_HOME": str(tmp / "ascend"),
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "SHIM_LOG": str(tmp / "shim.log"),
                "HOME": str(tmp / "home"),
            })
            for name in ("build-a", "build-b"):
                result = subprocess.run(
                    [str(ROOT / "tools/cmake_cce"), "-S", str(tmp), "-B", str(tmp / name)],
                    env=env, capture_output=True, text=True, check=False,
                )
                self.assertEqual(result.returncode, 0, result.stderr)
            roots = (tmp / "shim.log").read_text(encoding="utf-8").splitlines()
            self.assertEqual(len(roots), 2, roots)
            self.assertNotEqual(roots[0], roots[1])

    def test_cmake_cce_query_modes_bypass_cann_environment_checks(self) -> None:
        # The reference cmake_maca forwards --version/-E/--help directly to
        # cmake. cmake_cce must do the same before requiring an Ascend install,
        # so tooling can inspect the wrapper on a development host.
        env = dict(os.environ)
        for key in ("CUCC_PATH", "CANN_BASE", "ASCEND_TOOLKIT_HOME"):
            env.pop(key, None)
        for args in (("--version",), ("-E", "capabilities"), ("--help",)):
            with self.subTest(args=args):
                result = subprocess.run(
                    [str(ROOT / "tools/cmake_cce"), *args],
                    env=env, capture_output=True, text=True, check=False,
                )
                self.assertEqual(result.returncode, 0, result.stderr)

    def test_cmake_entrypoints_are_installed(self) -> None:
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        for tool in ("tools/cmake_cce", "tools/make_cce", "tools/ninja_cce", "tools/cu2cce_driver.py",
                     "tools/compare_cubridge_parity.py", "tools/validate_cuda_api_parity.py",
                     "tools/generate_cuda_api_parity_inventory.py"):
            self.assertIn(tool, cmake)
        self.assertIn("tools/real_test/verify_artifacts.py", cmake)
        self.assertIn("config/artifact_contract.json", cmake)
        self.assertIn("config/cuda_api_parity_inventory.json", cmake)

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

    def test_real_build_installs_artifact_contract_verifier(self) -> None:
        build = (ROOT / "tools/real_test/build_bridge.sh").read_text(encoding="utf-8")
        self.assertIn("tools/real_test/verify_artifacts.py", build)
        self.assertIn("config/artifact_contract.json", build)

    def test_quickstart_documents_output_artifact_contract(self) -> None:
        quickstart = (ROOT / "QUICKSTART.md").read_text(encoding="utf-8")
        self.assertIn("config/artifact_contract.json", quickstart)
        self.assertIn("verify_artifacts.py", quickstart)
        self.assertIn("NEEDED", quickstart)
        self.assertIn("RPATH/RUNPATH", quickstart)

    def test_package_exports_native_runtime(self) -> None:
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        config = (ROOT / "cmake/cu2cce-bridgeConfig.cmake").read_text(encoding="utf-8")
        self.assertIn("EXPORT cu2cceTargets", cmake)
        self.assertIn("$<INSTALL_INTERFACE:include>", cmake)
        self.assertIn("cu2cceTargets.cmake", config)

    def test_cmake_example_readmes_use_real_cmake_cce_build_entry(self) -> None:
        for relative in (
            "examples/cmake_demo/README.md",
            "examples/cmake_compat/README.md",
            "examples/large_project/README.md",
            "examples/build_compat/cmake/README.md",
        ):
            text = (ROOT / relative).read_text(encoding="utf-8")
            self.assertIn("cmake_cce --build", text, relative)
            self.assertNotIn("cmake --build", text, relative)

    def test_top_level_install_destinations_are_prefix_relative(self) -> None:
        # CMake applies --prefix only to relative DESTINATION values. Absolute
        # destinations embedding CMAKE_INSTALL_PREFIX make custom install roots
        # ineffective and can write into /usr/local on a server.
        top = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertNotRegex(top, r"DESTINATION\s+\$\{CMAKE_INSTALL_PREFIX\}")
        self.assertIn("LIB_DEST_DIR \"lib\"", top)

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
        rules = (ROOT / "bin/conf.json").read_text(encoding="utf-8")
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

    def test_server_cmake_builds_use_real_cmake_cce_build_phase(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        # Configure uses mock_nvcc, but every CUDA build must go through
        # cmake_cce --build so the persistent shim is repointed to real cucc.
        self.assertIn('if "$CMAKE_CCE" --build "$build_dir"', server)
        self.assertNotIn('if cmake --build "$build_dir"', server)
        self.assertIn('"$CMAKE_CCE" --build "$EXTERNAL_BUILD"', server)
        self.assertNotIn('cmake --build "$EXTERNAL_BUILD" --parallel "$EXTERNAL_JOBS" --target vectorAdd matrixMul', server)
        self.assertNotIn('cmake --build "$MULTICU_BUILD"', server)
        self.assertIn("cmake_cce: build phase: virtual toolkit nvcc ->", server)
        self.assertIn('readlink "$CU2CCE_CMAKE_SHIM_DIR/CUDA_DIR/bin/nvcc"', server)
        self.assertIn("cmake_large_project_install", server)
        self.assertIn('cmake --install "$OUT/cmake-large-project-build"', server)

    def test_external_cuda_samples_profile_is_pinned(self) -> None:
        profile = (ROOT / "examples/external/cuda_samples_project/profile_core.json").read_text(encoding="utf-8")
        fetch = (ROOT / "tools/real_test/fetch_external_cuda_samples.sh").read_text(encoding="utf-8")
        self.assertIn("db3eea23946bca2e90a75eca2b5b3e07158a9e11", profile)
        self.assertIn("CUDA_SAMPLES_SOURCE_DIR", fetch)
        self.assertIn("checkout --detach", fetch)
        self.assertTrue((ROOT / "docs/99_归档/CUDA_LARGE_PROJECT_COMPATIBILITY_PLAN.md").is_file())
        # Diagnostic samples must be compiled by the harness before any run
        # attempt: earlier server runs printed empty [DIAG-ONLY] tails because
        # only vectorAdd/matrixMul were in the required "--target" list.
        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("for sample in simpleStreams simpleMultiCopy asyncAPI; do", server_cmd)
        self.assertIn('"$CMAKE_CCE" --build "$EXTERNAL_BUILD" --parallel "$EXTERNAL_JOBS" --target "$sample"', server_cmd)
        # Regression guard: prefixing a shell FUNCTION with env yields
        # "env: 'run_with_timeout': No such file or directory" (exit 127)
        # and silently swallows every diagnostic run. Diagnostics force
        # WCUDA_DEBUG=1 so failing samples leave per-call bridge traces.
        self.assertIn("WCUDA_DEBUG=1 run_with_timeout", server_cmd)
        self.assertNotIn('env WCUDA_DEBUG=', server_cmd)
        self.assertIn('tail -n 12 "$diag_log"', server_cmd)
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
        self.assertIn('"$CMAKE_CCE" --build "$EXTERNAL_BUILD" --parallel "$EXTERNAL_JOBS"', server_cmd)
        self.assertIn("EXTERNAL_JOBS=", server_cmd)
        self.assertNotIn('rm -rf "$EXTERNAL_BUILD"', server_cmd)
        # fast keeps only the core puresimt subset; every legacy case stays in
        # the file guarded by profile checks so full remains complete.
        fast_filter = "vectorAdd|matrixMul|event_minimal|multiKernel_minimal|atomicAdd_minimal|atomicIndex_minimal|math_minimal"
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
        self.assertIn("vectorAdd|matrixMul|event_minimal|multiKernel_minimal|atomicAdd_minimal|atomicIndex_minimal|math_minimal|device_math_minimal",
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
        matrix = ROOT / "docs/02_能力清单/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md"
        text = matrix.read_text(encoding="utf-8")
        self.assertIn("multiple definition of", text)
        self.assertIn("BishengLaunchKernelWithHostArgs", text)
        self.assertIn("CU2CCE_CMAKE_NO_OVERRIDE", text)

    def test_build_matrix_exists(self) -> None:
        matrix = ROOT / "docs/02_能力清单/BUILD_SYSTEM_COMPATIBILITY_MATRIX.md"
        text = matrix.read_text(encoding="utf-8")
        for item in ("CMake", "Makefiles", "Ninja", "device-link", "CUDA::cudart"):
            self.assertIn(item, text)

    def test_large_project_carrier_covers_multi_directory_and_export(self) -> None:
        top = (ROOT / "examples/large_project/CMakeLists.txt").read_text(encoding="utf-8")
        for pattern in ("add_subdirectory(comp)", "add_subdirectory(util)",
                        "install(TARGETS", "install(EXPORT largeProjectTargets"):
            self.assertIn(pattern, top)
        self.assertIn(
            "install(TARGETS large_project_app\n  EXPORT largeProjectTargets",
            top,
        )
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

    def test_legacy_cuda_add_library_preserves_exclude_from_all(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            (tmp / "CMakeLists.txt").write_text(
                "cmake_minimum_required(VERSION 3.16)\n"
                "project(legacy_library_options CXX)\n"
                f"set(CUCC_PATH \"{ROOT}\")\n"
                f"include(\"{ROOT / 'cmake_modules/FindCUDA.cmake'}\")\n"
                "function(_cu2cce_enable_cuda)\nendfunction()\n"
                "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/host.cpp\" \"int f() { return 0; }\n\")\n"
                "cuda_add_library(legacy STATIC EXCLUDE_FROM_ALL host.cpp)\n"
                "get_target_property(excluded legacy EXCLUDE_FROM_ALL)\n"
                "if(NOT excluded)\n"
                "  message(FATAL_ERROR \"EXCLUDE_FROM_ALL was dropped\")\n"
                "endif()\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                ["cmake", "-S", str(tmp), "-B", str(tmp / "build")],
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_legacy_cuda_compile_does_not_leak_options_as_sources(self) -> None:
        # FindCUDA's CUDA_COMPILE accepts `OPTIONS ...` after its source list.
        # The compatibility macro must consume those options rather than
        # handing them back as CMake source names.
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            (tmp / "CMakeLists.txt").write_text(
                "cmake_minimum_required(VERSION 3.16)\n"
                "project(legacy_compile_probe CXX)\n"
                f"set(CUCC_PATH \"{ROOT}\")\n"
                f"include(\"{ROOT / 'cmake_modules/FindCUDA.cmake'}\")\n"
                "function(_cu2cce_enable_cuda)\nendfunction()\n"
                "cuda_compile(out foo.cu OPTIONS -arch=sm_80)\n"
                "message(STATUS \"OUT=[${out}]\")\n"
                "if(\"OPTIONS\" IN_LIST out OR \"-arch=sm_80\" IN_LIST out)\n"
                "  message(FATAL_ERROR \"legacy CUDA options leaked into source list\")\n"
                "endif()\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                ["cmake", "-S", str(tmp), "-B", str(tmp / "build")],
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_legacy_cuda_wrap_srcs_rejects_nvidia_device_artifact_formats(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            (tmp / "CMakeLists.txt").write_text(
                "cmake_minimum_required(VERSION 3.16)\n"
                "project(legacy_artifact_probe CXX)\n"
                f"include(\"{ROOT / 'cmake_modules/FindCUDA.cmake'}\")\n"
                "function(_cu2cce_enable_cuda)\nendfunction()\n"
                "cuda_wrap_srcs(probe PTX generated foo.cu)\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                ["cmake", "-S", str(tmp), "-B", str(tmp / "build")],
                capture_output=True, text=True, check=False,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("PTX generation is unsupported", result.stderr)

    def test_legacy_cuda_wrap_srcs_drops_phony_marker_from_compile_options(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            (tmp / "CMakeLists.txt").write_text(
                "cmake_minimum_required(VERSION 3.16)\n"
                "project(legacy_wrap_probe CXX)\n"
                f"include(\"{ROOT / 'cmake_modules/FindCUDA.cmake'}\")\n"
                "function(_cu2cce_enable_cuda)\nendfunction()\n"
                "cuda_wrap_srcs(probe OBJ generated foo.cu OPTIONS -arch=sm_80 PHONY)\n"
                "get_source_file_property(opts foo.cu COMPILE_OPTIONS)\n"
                "message(STATUS \"OPTS=[${opts}]\")\n"
                "if(\"PHONY\" IN_LIST opts)\n"
                "  message(FATAL_ERROR \"PHONY leaked into CUDA compile options\")\n"
                "endif()\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                ["cmake", "-S", str(tmp), "-B", str(tmp / "build")],
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_cmake_build_phase_and_lib_symlinks_align_with_cubridge(self) -> None:
        # cu-bridge alignment (2026-08-28): two-phase protocol and the CUDA-
        # named lib64 symlink family. Their cmake_maca answers configure
        # probes via mock_nvcc and repoints CUDA_DIR/bin/nvcc -> cucc for the
        # build phase; their link.json pre_make section creates lib64/libcudart
        # -> runtime symlinks so plain-g++ linked Make projects resolve
        # -lcudart. Our cmake_cce/make_cce replicate both; since the fifth
        # parity batch the shim TREE itself is built from the shared manifest
        # tools/link.json by tools/mock/virtualDir.py (their
        # link.json/stub_link.json single-manifest parity).
        cmake_cce = (ROOT / "tools/cmake_cce").read_text(encoding="utf-8")
        self.assertIn('cmake_cce: build phase: virtual toolkit nvcc ->', cmake_cce)
        self.assertIn('cu2cce_prepare_virtual_toolkit', cmake_cce)
        self.assertIn('"$VIRTUAL_TOOLKIT_ROOT" "$CUCC_PATH/bin/cucc"', cmake_cce)
        self.assertNotIn('ln -sfn "$CUCC_PATH/bin/cucc" "$CUDA_DIR/bin/nvcc"', cmake_cce)
        # The build-phase re-point remains persistent, but the default root is
        # keyed by the canonical build directory so separate projects do not
        # overwrite one another. Explicit CU2CCE_CMAKE_SHIM_DIR still wins.
        self.assertIn('cu2cce_virtual_toolkit_root_for_build', cmake_cce)
        self.assertIn('cmake-virtual-toolkit-', (ROOT / "tools/virtual_toolkit_env.sh").read_text(encoding="utf-8"))
        # Configure phase binds the virtual toolkit nvcc to the mock through the shared
        # helper (re-pointing here makes repeated configure/build cycles on
        # one persistent shim deterministic). Since the seventh batch the
        # sentinel/builder logic lives in tools/virtual_toolkit_env.sh.
        self.assertIn('cu2cce_prepare_virtual_toolkit "$VIRTUAL_TOOLKIT_ROOT" "$CUCC_PATH/tools/mock/nvcc_entry.sh"',
                      cmake_cce)
        helper = (ROOT / "tools/virtual_toolkit_env.sh").read_text(encoding="utf-8")
        self.assertIn('--hostcc-mode "$hostcc_mode"', helper)
        self.assertIn('echo 1 > "$dst/make.log"', helper)
        # The CUDA-named runtime alias family lives in the shared manifest.
        layout = (ROOT / "tools/link.json").read_text(encoding="utf-8")
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
        # make.log sentinel is created by the helper (persistent virtual CUDA toolkit);
        # make_cce sources it and binds the virtual toolkit nvcc to the real cucc.
        self.assertIn('. "${CUCC_PATH}/tools/virtual_toolkit_env.sh"', make_cce)
        self.assertIn('cu2cce_prepare_virtual_toolkit "$VIRTUAL_TOOLKIT_ROOT" "$CUCC_PATH/bin/cucc"', make_cce)
        self.assertIn('PATH="$VIRTUAL_TOOLKIT_ROOT/bin:$CUDA_DIR/bin:$PATH"', make_cce)

    def test_virtual_toolkit_manifests_drive_all_entries(self) -> None:
        # Fifth parity batch: cmake_cce/make_cce/pre_cce used to carry three
        # hand-written copies of the mkdir/ln sequences (cu-bridge keeps ONE
        # manifest: tools/link.json + tools/mock/stub_link.json). All three
        # entries must consume tools/link.json — since the seventh
        # batch indirectly, through the sourced helper
        # tools/virtual_toolkit_env.sh (sentinel + builder invocation).
        builder = ROOT / "tools/mock/virtualDir.py"
        helper = ROOT / "tools/virtual_toolkit_env.sh"
        self.assertTrue(builder.is_file())
        self.assertTrue(helper.is_file())
        helper_text = helper.read_text(encoding="utf-8")
        self.assertIn("tools/mock/virtualDir.py", helper_text)
        for entry in ("tools/cmake_cce", "tools/make_cce", "tools/pre_cce"):
            text = (ROOT / entry).read_text(encoding="utf-8")
            self.assertIn("tools/virtual_toolkit_env.sh", text, entry)
            self.assertIn("cu2cce_prepare_virtual_toolkit", text, entry)
        import json
        link = json.loads((ROOT / "tools/link.json").read_text(encoding="utf-8"))
        self.assertEqual(link["backend"], "ascend-cann")
        self.assertEqual(link["compiler"], "bisheng-ccec")
        self.assertEqual(link["runtime"], "libcce_bridge_runtime.so")
        layout = link["pre_make"]
        for section in ("dirs", "dir_links", "file_links", "hostcc_names",
                        "cuda_lib_aliases", "alias_dirs", "alias_target", "files"):
            self.assertIn(section, layout)
        stub = json.loads((ROOT / "tools/mock/stub_link.json").read_text(encoding="utf-8"))
        self.assertIn("virtual_cmake", stub)
        self.assertEqual(stub["virtual_cmake"]["base_manifest"], "../link.json")

    def test_cubridge_environment_aliases_are_supported_across_entries(self) -> None:
        device_arch = (ROOT / "tools/device_arch.sh").read_text(encoding="utf-8")
        driver = (ROOT / "tools/cu2cce_driver.py").read_text(encoding="utf-8")
        self.assertIn("CUCC_TARGETS_FROM_DEVICE", device_arch)
        self.assertIn("CUCC_TARGETS_FROM_DEVICE", driver)
        for entry in ("tools/cmake_cce", "tools/make_cce", "tools/ninja_cce", "tools/pre_cce"):
            text = (ROOT / entry).read_text(encoding="utf-8")
            self.assertIn("WCUDA_HOME", text, entry)

    def test_device_arch_ignores_malformed_cached_dav_value(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            cache_dir = tmp / "home" / "cu2cce-bridge"
            cache_dir.mkdir(parents=True)
            (cache_dir / "device_arch.cache").write_text("dav-c310-vec\n--bad\n", encoding="utf-8")
            env = dict(os.environ)
            for key in ("CU2CCE_NPU_ARCH", "CUCC_NPU_ARCH", "CUCC_TARGETS", "CUCC_TARGETS_FROM_DEVICE"):
                env.pop(key, None)
            env["CU2CCE_HOME"] = str(tmp / "home")
            env["PATH"] = "/usr/bin:/bin"
            result = subprocess.run(
                ["/bin/bash", str(ROOT / "tools/device_arch.sh")],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), "dav-c310-vec")
            self.assertNotIn("--bad", result.stdout)

    def test_device_arch_cache_honors_wcuda_home_alias(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            cache_dir = tmp / "wcuda" / "cu2cce-bridge"
            cache_dir.mkdir(parents=True)
            (cache_dir / "device_arch.cache").write_text("dav-c100-vec\n", encoding="utf-8")
            env = dict(os.environ)
            for key in ("CU2CCE_NPU_ARCH", "CUCC_NPU_ARCH", "CUCC_TARGETS", "CUCC_TARGETS_FROM_DEVICE"):
                env.pop(key, None)
            env.pop("CU2CCE_HOME", None)
            env.pop("CUBRIDGE_HOME", None)
            env["WCUDA_HOME"] = str(tmp / "wcuda")
            env["PATH"] = "/usr/bin:/bin"
            result = subprocess.run(
                ["/bin/bash", str(ROOT / "tools/device_arch.sh")],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), "dav-c100-vec")

    def test_device_arch_centralizes_dav_architecture_validation(self) -> None:
        script = (ROOT / "tools/device_arch.sh").read_text(encoding="utf-8")
        self.assertIn("is_valid_dav_arch()", script)
        self.assertGreaterEqual(script.count("is_valid_dav_arch"), 5)

    def test_device_arch_ignores_invalid_cached_cuda_architecture(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            cache_dir = tmp / "home" / "cu2cce-bridge"
            cache_dir.mkdir(parents=True)
            (cache_dir / "device_arch.cache").write_text("sm_80\n", encoding="utf-8")
            env = dict(os.environ)
            for key in ("CU2CCE_NPU_ARCH", "CUCC_NPU_ARCH", "CUCC_TARGETS", "CUCC_TARGETS_FROM_DEVICE"):
                env.pop(key, None)
            env["CU2CCE_HOME"] = str(tmp / "home")
            env["PATH"] = "/usr/bin:/bin"
            result = subprocess.run(
                ["/bin/bash", str(ROOT / "tools/device_arch.sh")],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), "dav-c310-vec")

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
        helper = (ROOT / "tools/virtual_toolkit_env.sh").read_text(encoding="utf-8")
        self.assertIn("cu2cce_resolve_npu_arch()", helper)
        for name in ("tools/cmake_cce", "tools/make_cce", "tools/ninja_cce", "tools/pre_cce"):
            text = (ROOT / name).read_text(encoding="utf-8")
            self.assertIn("cu2cce_resolve_npu_arch", text, name)
            self.assertNotIn("|| echo dav-c310-vec", text, name)

    def test_direct_ninja_preserves_existing_cmake_shim_from_cache(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            fake_ninja = fake_bin / "ninja"
            fake_ninja.write_text(
                "#!/bin/sh\nprintf '%s\n' \"$CU2CCE_CMAKE_SHIM_DIR\" > \"$NINJA_LOG\"\n",
                encoding="utf-8",
            )
            fake_ninja.chmod(0o755)
            build = tmp / "old-build"
            build.mkdir()
            old_shim = tmp / "old-shim"
            old_shim.mkdir()
            (old_shim / "CUDA_DIR/bin").mkdir(parents=True)
            (build / "CMakeCache.txt").write_text(
                f"CMAKE_CUDA_COMPILER:FILEPATH={old_shim}/CUDA_DIR/bin/nvcc\n",
                encoding="utf-8",
            )
            (build / "build.ninja").write_text("# fake\n", encoding="utf-8")
            log = tmp / "ninja.log"
            env = dict(os.environ)
            env.update({
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
                "CUCC_PATH": str(ROOT),
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "CU2CCE_HOME": str(tmp / "home"),
                "NINJA_LOG": str(log),
            })
            result = subprocess.run(
                [str(ROOT / "tools/ninja_cce"), "-C", str(build)],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(log.read_text(encoding="utf-8").strip(), str(old_shim))

    def test_ninja_cce_rebuilds_the_same_persistent_npu_shim(self) -> None:
        # The shared builder materializes <shim-root>/CUDA_DIR. Direct ninja
        # invocation must use that same path as cmake_cce --build; using a
        # different lowercase /cuda path silently leaves nvcc bound to mock.
        ninja = (ROOT / "tools/ninja_cce").read_text(encoding="utf-8")
        cmake = (ROOT / "tools/cmake_cce").read_text(encoding="utf-8")
        self.assertIn('$VIRTUAL_TOOLKIT_ROOT/CUDA_DIR', ninja)
        self.assertIn('$VIRTUAL_TOOLKIT_ROOT/CUDA_DIR', cmake)
        self.assertNotIn('CUDA_DIR="$VIRTUAL_TOOLKIT_ROOT/cuda"', ninja)
        self.assertNotIn('CUDA_DIR="$VIRTUAL_TOOLKIT_ROOT/cuda"', cmake)
        for text in (ninja,):
            self.assertIn("tools/virtual_toolkit_env.sh", text)
            self.assertIn("cu2cce_prepare_virtual_toolkit", text)
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

    def test_build_wrapper_rejects_invalid_targets_from_device_without_defaulting(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            fake_ninja = fake_bin / "ninja"
            fake_ninja.write_text("#!/bin/sh\nexit 0\n", encoding="utf-8")
            fake_ninja.chmod(0o755)
            env = dict(os.environ)
            env.update({
                "CUCC_PATH": str(ROOT),
                "CU2CCE_CMAKE_SHIM_DIR": str(tmp / "shim"),
                "CUCC_TARGETS_FROM_DEVICE": "sm_80",
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
            })
            result = subprocess.run(
                [str(ROOT / "tools/ninja_cce"), "-n"],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("CUCC_TARGETS_FROM_DEVICE", result.stderr)

    def test_make_cce_honors_cmake_shim_directory_for_build_dispatch(self) -> None:
        # cmake_cce and make_cce must share one shim when the caller selects a
        # custom persistent root; otherwise CMake's recorded host/compiler
        # paths and Make's runtime environment diverge.
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            log = tmp / "make.log"
            fake_make = fake_bin / "make"
            fake_make.write_text(
                "#!/bin/sh\n"
                "printf '%s\n' \"CUDA_PATH=$CUDA_PATH\" > \"$MAKE_LOG\"\n",
                encoding="utf-8",
            )
            fake_make.chmod(0o755)
            shim_root = tmp / "custom-shim"
            env = dict(os.environ)
            env.update({
                "CUCC_PATH": ".",
                "CU2CCE_CMAKE_SHIM_DIR": str(shim_root),
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "MAKE_LOG": str(log),
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
            })
            result = subprocess.run(
                [str(ROOT / "tools/make_cce"), "all"],
                env=env, cwd=ROOT, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(log.read_text(encoding="utf-8").strip(),
                             f"CUDA_PATH={shim_root / 'CUDA_DIR'}")

    def test_make_cce_resolves_relative_shim_before_make_changes_directory(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            log = tmp / "make.log"
            fake_make = fake_bin / "make"
            fake_make.write_text(
                "#!/bin/sh\n"
                "printf '%s\n' \"CUDA_PATH=$CUDA_PATH\" > \"$MAKE_LOG\"\n",
                encoding="utf-8",
            )
            fake_make.chmod(0o755)
            (tmp / "build").mkdir()
            env = dict(os.environ)
            env.update({
                "CUCC_PATH": str(ROOT),
                "CU2CCE_CMAKE_SHIM_DIR": "relative-shim",
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "MAKE_LOG": str(log),
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
            })
            result = subprocess.run(
                [str(ROOT / "tools/make_cce"), "-C", "build", "all"],
                env=env, cwd=tmp, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(log.read_text(encoding="utf-8").strip(),
                             f"CUDA_PATH={(tmp / 'relative-shim' / 'CUDA_DIR').resolve()}")

    def test_ninja_cce_resolves_relative_shim_before_ninja_changes_directory(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            log = tmp / "ninja.log"
            fake_ninja = fake_bin / "ninja"
            fake_ninja.write_text(
                "#!/bin/sh\n"
                "printf '%s\n' \"CUDA_PATH=$CUDA_PATH\" > \"$NINJA_LOG\"\n",
                encoding="utf-8",
            )
            fake_ninja.chmod(0o755)
            (tmp / "build").mkdir()
            env = dict(os.environ)
            env.update({
                "CUCC_PATH": str(ROOT),
                "CU2CCE_CMAKE_SHIM_DIR": "relative-shim",
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "NINJA_LOG": str(log),
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
            })
            result = subprocess.run(
                [str(ROOT / "tools/ninja_cce"), "-C", "build"],
                env=env, cwd=tmp, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(log.read_text(encoding="utf-8").strip(),
                             f"CUDA_PATH={(tmp / 'relative-shim' / 'CUDA_DIR').resolve()}")

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
            (install / "tools/virtual_toolkit_env.sh").write_text(
                (ROOT / "tools/virtual_toolkit_env.sh").read_text(encoding="utf-8"),
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

            log.unlink(missing_ok=True)
            verbose = subprocess.run(
                [str(ROOT / "tools/cmake_cce"), "--build", str(build_dir), "--verbose"],
                env={**env, "CUCC_PATH": "install"}, cwd=tmp,
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(verbose.returncode, 0, verbose.stderr)
            self.assertEqual(log.read_text(encoding="utf-8").splitlines(), ["ARG=VERBOSE=1"])

    def test_cmake_cce_supports_attached_build_target_and_job_forms(self) -> None:
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
            (install / "tools/virtual_toolkit_env.sh").write_text(
                (ROOT / "tools/virtual_toolkit_env.sh").read_text(encoding="utf-8"),
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
            build_dir = tmp / "build"
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
            result = subprocess.run(
                [str(ROOT / "tools/cmake_cce"), f"--build={build_dir}",
                 "--target=app", "-j=4", "--config=Release"],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(log.read_text(encoding="utf-8").splitlines(),
                             ["ARG=-j", "ARG=4", "ARG=app"])

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

            (ninja_build / "build.ninja").touch()
            log.unlink()
            verbose = subprocess.run(
                [str(ROOT / "tools/cmake_cce"), "--build", str(ninja_build), "--verbose"],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(verbose.returncode, 0, verbose.stderr)
            self.assertEqual(log.read_text(encoding="utf-8").splitlines()[1:], ["ARG=-v"])

            log.unlink()
            unbounded = subprocess.run(
                [str(ROOT / "tools/cmake_cce"), "--build", str(ninja_build), "--parallel"],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(unbounded.returncode, 0, unbounded.stderr)
            self.assertEqual(log.read_text(encoding="utf-8").splitlines()[1:], ["ARG=-j", "ARG=0"])

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

    def test_cmake_cce_forwards_config_and_clean_first_for_ninja(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            fake_bin = tmp / "bin"
            fake_bin.mkdir()
            fake_ninja = fake_bin / "ninja"
            fake_ninja.write_text(
                "#!/bin/sh\nprintf 'RUN\n' >> \"$GENERATOR_LOG\"\n"
                "for arg in \"$@\"; do printf 'ARG=%s\n' \"$arg\" >> \"$GENERATOR_LOG\"; done\n",
                encoding="utf-8",
            )
            fake_ninja.chmod(0o755)
            cann_bin = tmp / "cann/bin"
            cann_bin.mkdir(parents=True)
            ccec = cann_bin / "ccec"
            ccec.write_text("#!/bin/sh\nexit 0\n", encoding="utf-8")
            ccec.chmod(0o755)
            build = tmp / "build"
            build.mkdir()
            (build / "build.ninja").write_text("# fake\n", encoding="utf-8")
            (build / "build-Release.ninja").write_text("# fake config\n", encoding="utf-8")
            log = tmp / "generator.log"
            env = dict(os.environ)
            env.update({
                "PATH": f"{fake_bin}{os.pathsep}{env.get('PATH', '')}",
                "CUCC_PATH": str(ROOT),
                "CANN_BASE": str(tmp / "cann"),
                "ASCEND_TOOLKIT_HOME": str(tmp / "ascend"),
                "CCEC_PATH": str(ccec),
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "CU2CCE_CMAKE_SHIM_DIR": str(tmp / "shim"),
                "GENERATOR_LOG": str(log),
            })
            result = subprocess.run(
                [str(ROOT / "tools/cmake_cce"), "--build", str(build),
                 "--config", "Release", "--clean-first", "--target", "app"],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            lines = log.read_text(encoding="utf-8").splitlines()
            self.assertEqual(lines[:2], ["RUN", "ARG=-t"], lines)
            self.assertIn("ARG=clean", lines)
            self.assertIn("ARG=-f", lines)
            self.assertIn("ARG=build-Release.ninja", lines)
            self.assertIn("ARG=app", lines)

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
        # `cleanpack` would delete the virtual CUDA toolkit. Both make_cce and
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
        layout = (ROOT / "tools/link.json").read_text(encoding="utf-8")
        builder = (ROOT / "tools/mock/virtualDir.py").read_text(encoding="utf-8")
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

    def test_virtual_toolkit_builder_version_file_matches_cucc_version_environment(self) -> None:
        import subprocess as sp
        import sys

        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            cucc = tmp / "install"
            (cucc / "bin").mkdir(parents=True)
            (cucc / "include").mkdir()
            (cucc / "include/cuda_runtime.h").touch()
            (cucc / "lib").mkdir()
            (cucc / "lib/libcce_bridge_runtime.so").touch()
            (cucc / "tools").mkdir()
            (cucc / "tools/hostcc_cce").write_text("#!/bin/sh\n", encoding="utf-8")
            (cucc / "tools/hostcc_cce").chmod(0o755)
            (cucc / "bin/cucc").write_text("#!/bin/sh\n", encoding="utf-8")
            (cucc / "bin/cucc").chmod(0o755)
            env = dict(os.environ)
            env["CUCC_CUDA_VERSION"] = "12020"
            result = sp.run([
                sys.executable, str(ROOT / "tools/mock/virtualDir.py"),
                "--cuda-dir", str(tmp / "shim"),
                "--cucc-path", str(cucc),
                "--nvcc-target", str(cucc / "bin/cucc"),
                "--quiet",
            ], env=env, capture_output=True, text=True, check=False)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(
                (tmp / "shim/version.txt").read_text(encoding="utf-8"),
                "CUDA Version 12.2.0\n",
            )

    def test_virtual_toolkit_builder_resolves_relative_cli_paths(self) -> None:
        import os
        import subprocess as sp
        import sys

        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            cucc = tmp / "install"
            (cucc / "bin").mkdir(parents=True)
            (cucc / "include").mkdir()
            (cucc / "include/cuda_runtime.h").write_text("/* shim */\n", encoding="utf-8")
            (cucc / "lib").mkdir()
            (cucc / "lib/libcce_bridge_runtime.so").write_text("", encoding="utf-8")
            (cucc / "tools").mkdir()
            (cucc / "tools/hostcc_cce").write_text("#!/bin/sh\n", encoding="utf-8")
            (cucc / "bin/cucc").write_text("#!/bin/sh\n", encoding="utf-8")
            (cucc / "bin/cucc").chmod(0o755)
            (cucc / "tools/hostcc_cce").chmod(0o755)
            result = sp.run([
                sys.executable, str(ROOT / "tools/mock/virtualDir.py"),
                "--cuda-dir", "shim/CUDA_DIR",
                "--cucc-path", "install",
                "--nvcc-target", "install/bin/cucc",
                "--hostcc-bin-dir", "shim/bin",
                "--quiet",
            ], cwd=tmp, capture_output=True, text=True, check=False)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertTrue((tmp / "shim/CUDA_DIR/include/cuda_runtime.h").is_file())
            self.assertEqual(
                os.readlink(tmp / "shim/CUDA_DIR/bin/nvcc"),
                str((cucc / "bin/cucc").resolve()),
            )

    def test_virtual_toolkit_builder_is_idempotent_and_produces_the_manifest_layout(self) -> None:
        # Functional check of the shared builder (no CANN/ccec needed): the
        # produced tree must carry the toolkit shape, the CUDA-named alias
        # family pointing at the bridge runtime, hostcc entries, and must be
        # idempotent (re-running with a different nvcc target re-points).
        import json
        import os
        import subprocess as sp
        import sys
        import tempfile

        sys.path.insert(0, str(ROOT / "tools/mock"))
        import virtualDir  # noqa: PLC0415

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
            (cucc / "bin/gnu").write_text("#!/bin/sh\n", encoding="utf-8")
            (cucc / "bin/gnu").chmod(0o755)
            (cucc / "tools/hostcc_cce").write_text("#!/bin/sh\n", encoding="utf-8")
            (cucc / "tools/gfortran_cce").write_text("#!/bin/sh\n", encoding="utf-8")
            (cucc / "tools/mpi_cce").write_text("#!/bin/sh\n", encoding="utf-8")
            (cucc / "bin/cucc").write_text("#!/bin/sh\n", encoding="utf-8")

            cuda_dir = tmp / "cuda"
            hostbin = tmp / "hostbin"
            result = sp.run([
                sys.executable,
                str(ROOT / "tools/mock/virtualDir.py"),
                "--cuda-dir", str(cuda_dir),
                "--cucc-path", str(cucc),
                "--nvcc-target", str((cucc / "bin/cucc").resolve()),
                "--hostcc-bin-dir", str(hostbin),
                "--hostcc-mode", "wrapper",
            ], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)

            self.assertTrue((cuda_dir / "include/cuda_runtime.h").is_file())
            self.assertEqual(os.readlink(cuda_dir / "bin/nvcc"), str((cucc / "bin/cucc").resolve()))
            self.assertEqual(os.readlink(cuda_dir / "bin/cucc"), str((cucc / "bin/cucc").resolve()))
            self.assertEqual(os.readlink(hostbin / "g++"), str((cucc / "bin/gnu").resolve()))
            self.assertEqual(os.readlink(hostbin / "gfortran"), str((cucc / "tools/gfortran_cce").resolve()))
            self.assertEqual(os.readlink(hostbin / "mpicc"), str((cucc / "tools/mpi_cce").resolve()))
            self.assertEqual(os.readlink(hostbin / "mpicxx"), str((cucc / "tools/mpi_cce").resolve()))
            self.assertEqual(os.readlink(cuda_dir / "lib64/libcudart.so"),
                             str((cucc / "lib/libcce_bridge_runtime.so").resolve()))
            self.assertEqual(os.readlink(cuda_dir / "lib64/libcublas.so"),
                             str((cucc / "lib/libcce_bridge_runtime.so").resolve()))
            self.assertEqual(os.readlink(cuda_dir / "lib64/stubs/libcuda.so.1"),
                             str((cucc / "lib/libcce_bridge_runtime.so").resolve()))
            self.assertTrue((cuda_dir / "version.txt").read_text(encoding="utf-8").startswith("CUDA Version 11.6"))
            self.assertTrue((cuda_dir / "lib64/cmake/CUDAToolkit/CUDAToolkitConfig.cmake").is_file())
            self.assertTrue((cuda_dir / "extras/CUPTI/include").is_dir())
            triple_dir = cuda_dir / "targets" / virtualDir.host_triple()
            self.assertTrue(triple_dir.is_dir(), triple_dir)

            # A damaged real directory at a manifest-owned link path must be
            # repaired instead of reaching Path.unlink(directory) and failing.
            broken_include = cuda_dir / "targets" / virtualDir.host_triple() / "include"
            broken_include.unlink()
            broken_include.mkdir(parents=True)
            repaired = sp.run([
                sys.executable,
                str(ROOT / "tools/mock/virtualDir.py"),
                "--cuda-dir", str(cuda_dir),
                "--cucc-path", str(cucc),
                "--nvcc-target", str((cucc / "bin/cucc").resolve()),
                "--quiet",
            ], capture_output=True, text=True)
            self.assertEqual(repaired.returncode, 0, repaired.stderr)
            self.assertTrue(broken_include.is_symlink())

            # Idempotent re-run re-points the virtual toolkit nvcc without disturbing
            # the rest of the tree.
            rerun = sp.run([
                sys.executable,
                str(ROOT / "tools/mock/virtualDir.py"),
                "--cuda-dir", str(cuda_dir),
                "--cucc-path", str(cucc),
                "--nvcc-target", str((cucc / "tools/mock/nvcc_entry.sh").resolve()),
                "--quiet",
            ], capture_output=True, text=True)
            self.assertEqual(rerun.returncode, 0, rerun.stderr)
            self.assertEqual(os.readlink(cuda_dir / "bin/nvcc"), str((cucc / "tools/mock/nvcc_entry.sh").resolve()))
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

    def test_mock_entrypoints_are_installed_as_executable_programs(self) -> None:
        top = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("install(PROGRAMS tools/mock/virtualDir.py", top)
        self.assertIn("tools/mock/mock_nvcc.py", top)
        self.assertIn("tools/mock/nvcc_entry.sh", top)
        for path in (ROOT / "bin/gnu", ROOT / "bin/nvcc", ROOT / "tools/mock/nvcc_entry.sh"):
            self.assertTrue(path.is_file())
            self.assertTrue(path.stat().st_mode & 0o111, path)

    def test_install_tree_provides_nvcc_wrapper(self) -> None:
        # The install tree carries a regular bin/nvcc wrapper so Makefiles
        # hard-coding `nvcc` work without installation-time symlink compatibility.
        top = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        wrapper = ROOT / "bin/nvcc"
        self.assertTrue(wrapper.is_file())
        self.assertFalse(wrapper.is_symlink())
        self.assertIn("install(PROGRAMS bin/nvcc DESTINATION bin)", top)
        self.assertIn("exec \"$SCRIPT_DIR/cucc\"", wrapper.read_text(encoding="utf-8"))

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
        audit = (ROOT / "docs/04_对比与验证/PARITY_AUDIT_2026-08-28.md").read_text(encoding="utf-8")
        self.assertIn("-dc", audit)
        self.assertIn("libsymbol_cu", audit)
        self.assertIn("thrust", audit)

    def test_alignment_doc_records_cubridge_mechanism_and_reading_map(self) -> None:
        doc = (ROOT / "docs/04_对比与验证/CMAKE_MAKE_ALIGNMENT_WITH_CUBRIDGE.md").read_text(encoding="utf-8")
        for section in ("cmake_maca", "cmake_mock", "cmake_assist", "link.json",
                        "USE_CMAKE_MACA", "预编译真 ELF", "双阶段协议",
                        "逐项对齐表", "代码导读", "剩余差距"):
            self.assertIn(section, doc)
        self.assertIn("docs/01_架构设计/06_cmake_principle_flow.md", doc)

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

    def test_findcuda_uses_cmake_compatibility_conditions(self) -> None:
        findcuda = (ROOT / "cmake_modules/FindCUDA.cmake").read_text(encoding="utf-8")
        # IS_EXECUTABLE was added in CMake 3.29; this project supports older
        # CMake releases, so discovery must use portable file conditions.
        self.assertNotIn("IS_EXECUTABLE", findcuda)
        self.assertIn("IS_DIRECTORY", findcuda)

    def test_findcuda_rejects_missing_bridge_install_when_required(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            script = tmp / "probe.cmake"
            script.write_text(
                f"set(ENV{{CUCC_PATH}} \"{tmp / 'missing'}\")\n"
                f"set(CUDA_FIND_REQUIRED TRUE)\ninclude(\"{ROOT / 'cmake_modules/FindCUDA.cmake'}\")\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                ["cmake", "-P", str(script)], capture_output=True, text=True, check=False
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("bridge", result.stderr.lower())

    def test_findcuda_exports_common_legacy_compatibility_variables(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            install = tmp / "install"
            (install / "bin").mkdir(parents=True)
            (install / "include").mkdir()
            (install / "lib").mkdir()
            (install / "bin/cucc").write_text("#!/bin/sh\n", encoding="utf-8")
            (install / "bin/cucc").chmod(0o755)
            (install / "lib/libcce_bridge_runtime.so").write_bytes(b"bridge")
            script = tmp / "probe.cmake"
            script.write_text(
                f"set(ENV{{CUCC_PATH}} \"{install}\")\n"
                f"include(\"{ROOT / 'cmake_modules/FindCUDA.cmake'}\")\n"
                "if(NOT DEFINED CUDA_VERSION_STRING OR NOT DEFINED CUDA_HAS_FP16 OR "
                "NOT DEFINED CUDA_CUDART_LIBRARY_VAR OR NOT DEFINED CUDA_SDK_ROOT_DIR OR "
                "NOT DEFINED CUDA_cudart_static_LIBRARY OR NOT DEFINED CUDA_cudadevrt_LIBRARY)\n"
                "message(FATAL_ERROR \"missing legacy CUDA compatibility variable\")\n"
                "endif()\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                ["cmake", "-P", str(script)], capture_output=True, text=True, check=False
            )
            self.assertEqual(result.returncode, 0, result.stderr)

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

    def test_findcuda_public_adders_enable_cuda_before_target_creation(self) -> None:
        findcuda = (ROOT / "cmake_modules/FindCUDA.cmake").read_text(encoding="utf-8")
        for function_name in ("cuda_add_executable", "cuda_add_library"):
            start = findcuda.index(f"function({function_name}")
            end = findcuda.index("endfunction()", start)
            self.assertIn("_cu2cce_enable_cuda()", findcuda[start:end], function_name)

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
        self.assertIn('CU2CCE_CMAKE_SHIM_DIR', ninja)
        self.assertIn('cu2cce_virtual_toolkit_root_for_build', ninja)
        self.assertIn('cmake-virtual-toolkit-', (ROOT / "tools/virtual_toolkit_env.sh").read_text(encoding="utf-8"))
        self.assertIn("ninja_cce: build phase: virtual toolkit nvcc ->", ninja)
        self.assertIn('cu2cce_prepare_virtual_toolkit', ninja)
        self.assertIn('"$CUCC_PATH/bin/cucc"', ninja)

        # plain-g++ link carrier: proves the lib64 symlink family end-to-end
        gxx_dir = ROOT / "examples/build_compat/make_gxx_link"
        self.assertTrue((gxx_dir / "Makefile").is_file())
        makefile = (gxx_dir / "Makefile").read_text(encoding="utf-8")
        self.assertIn("-L$(CUDA_PATH)/lib64 -lcudart", makefile)
        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("make_gxx_link: plain-g++ link with -lcudart -> bridge runtime", server_cmd)

    def test_wrappers_export_consistent_nonempty_library_environment(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_name:
            tmp = Path(tmp_name)
            recorder = tmp / "record-env.sh"
            recorder.write_text(
                "#!/bin/sh\nprintf 'LD_LIBRARY_PATH=%s\nCUDA_HOME=%s\nCUDA_PATH=%s\n' \"$LD_LIBRARY_PATH\" \"$CUDA_HOME\" \"$CUDA_PATH\" > \"$ENV_LOG\"\n",
                encoding="utf-8",
            )
            recorder.chmod(0o755)
            env = dict(os.environ)
            env.update({
                "CUCC_PATH": str(ROOT),
                "CU2CCE_HOME": str(tmp / "shim-home"),
                "CANN_BASE": str(tmp / "cann"),
                "ASCEND_TOOLKIT_HOME": str(tmp / "ascend"),
                "CU2CCE_NPU_ARCH": "dav-c310-vec",
                "ENV_LOG": str(tmp / "env.log"),
            })
            env.pop("LD_LIBRARY_PATH", None)
            result = subprocess.run(
                [str(ROOT / "tools/pre_cce"), str(recorder)],
                env=env, capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            values = dict(line.split("=", 1) for line in (tmp / "env.log").read_text().splitlines())
            self.assertTrue(values["LD_LIBRARY_PATH"])
            self.assertFalse(values["LD_LIBRARY_PATH"].endswith(":"))
            self.assertEqual(values["CUDA_HOME"], values["CUDA_PATH"])

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
        # Fifth parity batch: the virtual CUDA toolkit comes from the shared builder;
        # seventh batch: sentinel/builder calls moved into the sourced helper
        # tools/virtual_toolkit_env.sh (pre_cce binds the virtual toolkit nvcc to the real
        # cucc like make_cce, and its host compiler entries go through
        # hostcc_cce).
        self.assertIn('. "${CUCC_PATH}/tools/virtual_toolkit_env.sh"', pre_cce)
        self.assertIn('cu2cce_prepare_virtual_toolkit "$VIRTUAL_TOOLKIT_ROOT" "$CUCC_PATH/bin/cucc"', pre_cce)
        self.assertIn('exec "$@"', pre_cce)
        self.assertNotIn('eval "$*"', pre_cce)

        hostcc = (ROOT / "tools/hostcc_cce").read_text(encoding="utf-8")
        # bin/gnu parity, NPU-adapted: real compiler for compiles, and the
        # link injection is rule-driven (hostcc.link in cu2cce_rules.json,
        # consumed by the driver's --hostcc-mode). The script keeps its
        # no-bridge degradation to the plain system compiler.
        self.assertIn('exec "/usr/bin/$name" "$@"', hostcc)
        self.assertIn("--hostcc-mode", hostcc)
        rules_text = (ROOT / "bin/conf.json").read_text(encoding="utf-8")
        self.assertIn('"hostcc"', rules_text)
        self.assertIn('"-lcce_bridge_runtime"', rules_text)
        self.assertIn("is a CUDA source",
                      (ROOT / "tools/cu2cce_driver.py").read_text(encoding="utf-8"))

        for wrapper in ("tools/cmake_cce", "tools/make_cce"):
            text = (ROOT / wrapper).read_text(encoding="utf-8")
            self.assertIn("tools/virtual_toolkit_env.sh", text)
            self.assertIn("cu2cce_resolve_npu_arch", text)
        helper_text = (ROOT / "tools/virtual_toolkit_env.sh").read_text(encoding="utf-8")
        self.assertIn("tools/device_arch.sh", helper_text)
        self.assertIn("tools/mock/virtualDir.py", helper_text)
        self.assertIn("hostcc_cce",
                      (ROOT / "tools/mock/virtualDir.py").read_text(encoding="utf-8"))
        layout_text = (ROOT / "tools/link.json").read_text(encoding="utf-8")
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

    def test_multi_cu_diagnostic_is_explicitly_non_blocking_npu_limit(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("multi_cu_diagnostic (diagnostic-only, non-blocking)", server)
        self.assertIn("expected Ascend backend limitation", server)
        self.assertIn("duplicate eager-registration", (ROOT / "examples/multi_cu_diagnostic/CMakeLists.txt").read_text(encoding="utf-8"))

    def test_round9_query_arch_and_options_probes_are_wired_into_the_harness(self) -> None:
        server_cmd = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("-E query passthrough", server_cmd)
        self.assertIn("--options-file", server_cmd)
        self.assertIn("CUCC_TARGETS=dav-c310-vec", server_cmd)
        self.assertIn("direct ninja_cce", server_cmd)
        self.assertIn("CUCC_USE_GOPENMP=1", server_cmd)
        self.assertIn("WCUDA_BAZEL_TF_BUILD=1", server_cmd)
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

class CanonicalAscendRuntimeCMakeTest(unittest.TestCase):
    def test_cmake_modules_select_acl_rt_before_legacy_ascendcl(self) -> None:
        root = Path(__file__).resolve().parents[1]
        for name in ("FindCUDAToolkit.cmake", "FindCUDA.cmake"):
            text = (root / "cmake_modules" / name).read_text(encoding="utf-8")
            self.assertIn("libacl_rt.so", text, name)
            self.assertIn("libascendcl.so", text, name)
            self.assertLess(
                text.index("libacl_rt.so"),
                text.index("libascendcl.so"),
                name,
            )

    def test_top_level_runtime_link_does_not_hardcode_legacy_library(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("libacl_rt.so", text)
        self.assertIn("libascendcl.so", text)
        self.assertIn("ASCEND_RUNTIME_LIB", text)

    def test_server_harness_runs_ascend_api_probes(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("cann_api_surface_probe.sh", text)
        self.assertIn("cann_runtime_link_probe.sh", text)
        self.assertIn("ASCEND_API_MAPPING_REGRESSION_PASS", text)


class CudaDeviceApiMappingContractTest(unittest.TestCase):
    def test_cuda_device_api_header_is_included_by_runtime_surface(self) -> None:
        root = Path(__file__).resolve().parents[1]
        mapping = root / "include/bridge/kernel/cuda_device_api.h"
        runtime = (root / "include/__cuda_runtime.h").read_text(encoding="utf-8")
        self.assertTrue(mapping.is_file())
        self.assertIn('"bridge/kernel/cuda_device_api.h"', runtime)

    def test_mapping_covers_extended_simt_and_atomic_surface(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "include/bridge/kernel/cuda_device_api.h").read_text(encoding="utf-8")
        for cuda_name, ascend_name in (
            ("__activemask", "asc_activemask"),
            ("__reduce_max_sync", "asc_reduce_max"),
            ("__reduce_min_sync", "asc_reduce_min"),
            ("atomicInc", "asc_atomic_inc"),
            ("atomicDec", "asc_atomic_dec"),
            ("atomicCAS", "asc_atomic_cas"),
            ("atomicAnd", "asc_atomic_and"),
            ("atomicOr", "asc_atomic_or"),
            ("atomicXor", "asc_atomic_xor"),
        ):
            self.assertIn(cuda_name, text)
            self.assertIn(ascend_name, text)

    def test_mapping_preserves_cuda_width_argument_when_present(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "include/bridge/kernel/cuda_device_api.h").read_text(encoding="utf-8")
        self.assertIn("CU2CCE_SHFL_SYNC4", text)
        self.assertIn("asc_shfl((var), (srcLane), (width))", text)
        self.assertIn("CU2CCE_SHFL_UP_SYNC4", text)
        self.assertIn("CU2CCE_SHFL_DOWN_SYNC4", text)
        self.assertIn("CU2CCE_SHFL_XOR_SYNC4", text)

    def test_unsupported_system_fence_is_not_silently_aliased(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "include/bridge/kernel/cuda_device_api.h").read_text(encoding="utf-8")
        self.assertIn("__threadfence_system", text)
        self.assertIn("UNSUPPORTED", text)
        self.assertNotRegex(text, r"#define\s+__threadfence_system\s+asc_threadfence")

class CudaRuntimeHeaderSurfaceContractTest(unittest.TestCase):
    def test_cuda_runtime_exposes_cuda_crt_device_surface(self) -> None:
        root = Path(__file__).resolve().parents[1]
        runtime = (root / "include/cuda_runtime.h").read_text(encoding="utf-8")
        for header in (
            "crt/builtin_types.h",
            "crt/device_functions.h",
            "device_launch_parameters.h",
            "math_constants.h",
        ):
            self.assertIsNotNone(
                re.search(
                    r"^\s*#\s*include\s+[<\"]" + re.escape(header) + r"[>\"]",
                    runtime, re.MULTILINE,
                ),
                header,
            )

class CanonicalRuntimeProbeContractTest(unittest.TestCase):
    def test_environment_probe_prefers_acl_rt_over_legacy_ascendcl(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "tools/real_test/env_probe.sh").read_text(encoding="utf-8")
        self.assertLess(text.index("libacl_rt.so"), text.index("libascendcl.so"))

    def test_bridge_build_precheck_accepts_either_runtime_with_acl_rt_first(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "tools/real_test/build_bridge.sh").read_text(encoding="utf-8")
        self.assertIn("libacl_rt.so", text)
        self.assertIn("libascendcl.so", text)
        self.assertIn("ACL_RUNTIME_LIB", text)
        self.assertLess(text.index("libacl_rt.so"), text.index("libascendcl.so"))

class CudaRuntimeMappingCompletenessTest(unittest.TestCase):
    def test_every_public_wcuda_function_has_a_cuda_runtime_alias(self) -> None:
        root = Path(__file__).resolve().parents[1]
        adaptor = (root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        wrapper = (root / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        aliases = set(re.findall(r"^#define\s+(cuda[A-Za-z0-9_]+)\s+(wcuda[A-Za-z0-9_]+)$", adaptor, re.MULTILINE))
        public_decls = set(re.findall(r"\b(wcuda[A-Za-z0-9_]+)\s*\(", wrapper))
        intentionally_internal = {"wcudaMallocHostWithFlags"}
        missing = sorted(
            name for name in public_decls - intentionally_internal
            if not any(target == name for _, target in aliases)
        )
        self.assertEqual(missing, [])

class LegacyCudaWarpMappingContractTest(unittest.TestCase):
    def test_legacy_warp_intrinsics_are_mapped_with_optional_width(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "include/bridge/kernel/cuda_device_api.h").read_text(encoding="utf-8")
        for name, target in (
            ("__shfl", "asc_shfl"),
            ("__shfl_up", "asc_shfl_up"),
            ("__shfl_down", "asc_shfl_down"),
            ("__shfl_xor", "asc_shfl_xor"),
            ("__all", "asc_all"),
            ("__any", "asc_any"),
            ("__ballot", "asc_ballot"),
        ):
            self.assertIn(name, text)
            self.assertIn(target, text)
        self.assertIn("CU2CCE_LEGACY_SHFL2", text)
        self.assertIn("CU2CCE_LEGACY_SHFL3", text)

class DeviceApiProbeHarnessContractTest(unittest.TestCase):
    def test_server_harness_passes_real_ccec_and_bridge_include_to_probe(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("--bridge-include \"$REPO_INC\"", text)
        self.assertIn("--ccec \"$API_CCEC\"", text)
        self.assertIn("--npu-arch \"${API_NPU_ARCH:-dav-c310-vec}\"", text)
        probe = (root / "tools/real_test/diag/cann_api_surface_probe.sh").read_text(encoding="utf-8")
        self.assertIn("device API compile: core SIMT/warp/vote/reduce", probe)
        self.assertIn("device API compile: extended atomics/active-mask", probe)

class CudaCrtDeviceFunctionEntryContractTest(unittest.TestCase):
    def test_direct_crt_device_functions_include_exposes_cuda_device_map(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "include/crt/device_functions.h").read_text(encoding="utf-8")
        self.assertIn('"bridge/kernel/cuda_device_api.h"', text)

class CudaDefaultMemcpyContractTest(unittest.TestCase):
    def test_default_memcpy_does_not_silently_become_device_to_device(self) -> None:
        root = Path(__file__).resolve().parents[1]
        adaptor = (root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        wrapper_h = (root / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        wrapper = (root / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        self.assertIn("#define cudaMemcpyDefault wcudaMemcpyDefault", adaptor)
        self.assertIn("wcudaMemcpyDefault", wrapper_h)
        self.assertIn("ACL_ERROR_API_NOT_SUPPORT", wrapper)
        self.assertIn("pointer attributes", wrapper)

class NpuAwareExtendedDeviceApiContractTest(unittest.TestCase):
    def test_extended_atomics_are_explicitly_opt_in(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "include/bridge/kernel/cuda_device_api.h").read_text(encoding="utf-8")
        self.assertIn("CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS", text)
        self.assertIn("defined(CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS)", text)
        self.assertLess(text.index("CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS"), text.index("atomicCAS"))

    def test_server_probe_separates_core_and_extended_device_api_checks(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "tools/real_test/diag/cann_api_surface_probe.sh").read_text(encoding="utf-8")
        self.assertIn("[PASS] device API compile: core SIMT", text)
        self.assertIn("[DIAG-ONLY] device API compile: extended", text)
        self.assertIn("CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS", text)

class CudaStreamRuntimeSurfaceContractTest(unittest.TestCase):
    def test_common_stream_runtime_entries_have_ascend_aware_wrappers(self) -> None:
        root = Path(__file__).resolve().parents[1]
        adaptor = (root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        wrapper_h = (root / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        wrapper = (root / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        for cuda_name, wcuda_name in (
            ("cudaStreamCreateWithPriority", "wcudaStreamCreateWithPriority"),
            ("cudaStreamGetFlags", "wcudaStreamGetFlags"),
            ("cudaStreamGetPriority", "wcudaStreamGetPriority"),
            ("cudaStreamGetDevice", "wcudaStreamGetDevice"),
            ("cudaStreamGetId", "wcudaStreamGetId"),
            ("cudaStreamAttachMemAsync", "wcudaStreamAttachMemAsync"),
        ):
            self.assertIn(f"#define {cuda_name} {wcuda_name}", adaptor)
            self.assertIn(wcuda_name, wrapper_h)
            self.assertIn(wcuda_name, wrapper)
        self.assertIn("priority", wrapper)
        self.assertIn("streamId", wrapper)

class HostWrapperBackendIndependenceContractTest(unittest.TestCase):
    def test_runtime_source_does_not_depend_on_cuda_flag_macros(self) -> None:
        root = Path(__file__).resolve().parents[1]
        source = (root / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        self.assertNotIn("cudaStreamDefault", source)
        self.assertIn("backend wrapper independent", source)

class CudaRuntimeTypeSurfaceContractTest(unittest.TestCase):
    def test_common_cuda_runtime_struct_types_have_npu_neutral_storage(self) -> None:
        root = Path(__file__).resolve().parents[1]
        types = (root / "include/bridge/runtime/cuda_runtime_types.h").read_text(encoding="utf-8")
        wrapper_types = (root / "include/bridge/runtime/cuda_wrapper_types.h").read_text(encoding="utf-8")
        adaptor = (root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        for name in ("wcceExtent", "wccePitchedPtr", "wccePos", "wccePointerAttributes", "wcceMemcpy3DParms", "wcceChannelFormatDesc"):
            self.assertIn(name, types)
        self.assertIn('"cuda_runtime_types.h"', wrapper_types)
        for cuda_name, bridge_name in (
            ("cudaExtent", "wcceExtent"),
            ("cudaPitchedPtr", "wccePitchedPtr"),
            ("cudaPos", "wccePos"),
            ("cudaPointerAttributes", "wccePointerAttributes"),
            ("cudaMemcpy3DParms", "wcceMemcpy3DParms"),
            ("cudaChannelFormatDesc", "wcceChannelFormatDesc"),
        ):
            self.assertIn(f"#define {cuda_name} {bridge_name}", adaptor)

    def test_cuda_runtime_factories_are_declared(self) -> None:
        root = Path(__file__).resolve().parents[1]
        wrapper_h = (root / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        adaptor = (root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        for cuda_name, wcuda_name in (
            ("make_cudaExtent", "make_wcudaExtent"),
            ("make_cudaPitchedPtr", "make_wcudaPitchedPtr"),
            ("make_cudaPos", "make_wcudaPos"),
        ):
            self.assertIn(f"#define {cuda_name} {wcuda_name}", adaptor)
            self.assertIn(wcuda_name, wrapper_h)

class CudaRuntimeTypedLongTailContractTest(unittest.TestCase):
    def test_long_tail_runtime_signatures_use_compatibility_types(self) -> None:
        root = Path(__file__).resolve().parents[1]
        header = (root / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        source = (root / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        expected = (
            "wcudaMemcpy3D(const wcceMemcpy3DParms *p)",
            "wcudaMemcpy3DAsync(const wcceMemcpy3DParms *p",
            "wcudaPointerGetAttributes(wccePointerAttributes *attributes",
            "wcudaFuncGetAttributes(wcceFuncAttributes *attr",
            "wcudaMemPoolCreate(wcceMemPool_t *memPool",
            "wcudaDeviceGetMemPool(wcceMemPool_t *memPool",
            "wcudaDeviceSetMemPool(int device, wcceMemPool_t memPool",
        )
        for signature in expected:
            self.assertIn(signature, header)
            self.assertIn(signature, source)

class Cuda3dRuntimeSurfaceContractTest(unittest.TestCase):
    def test_common_3d_runtime_entries_have_typed_compatibility_signatures(self) -> None:
        root = Path(__file__).resolve().parents[1]
        adaptor = (root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        header = (root / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        source = (root / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        for cuda_name, wcuda_name in (
            ("cudaMalloc3D", "wcudaMalloc3D"),
            ("cudaMemset3D", "wcudaMemset3D"),
            ("cudaMemset3DAsync", "wcudaMemset3DAsync"),
        ):
            self.assertIn(f"#define {cuda_name} {wcuda_name}", adaptor)
            self.assertIn(wcuda_name, header)
            self.assertIn(wcuda_name, source)
        self.assertIn("wcudaMalloc3D(wccePitchedPtr *pitchedDevPtr", header)
        self.assertIn("wcudaMemset3D(wccePitchedPtr pitchedDevPtr", header)
        self.assertIn("wcudaMemset3DAsync(wccePitchedPtr pitchedDevPtr", header)

class Cuda3dAllocationSafetyContractTest(unittest.TestCase):
    def test_malloc3d_checks_extent_multiplication_overflow(self) -> None:
        root = Path(__file__).resolve().parents[1]
        source = (root / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        start = source.index("aclError wcudaMalloc3D")
        end = source.index("aclError wcudaMallocPitch", start)
        body = source[start:end]
        self.assertIn("SIZE_MAX", body)
        self.assertIn("ACL_ERROR_INVALID_PARAM", body)
if __name__ == "__main__":
    unittest.main()

class NpuAwareReductionContractTest(unittest.TestCase):
    def test_warp_reduce_add_is_part_of_the_core_verified_surface(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "include/bridge/kernel/cuda_device_api.h").read_text(encoding="utf-8")
        self.assertIn("__reduce_add_sync", text)
        opt_in = text.index("defined(CU2CCE_ENABLE_EXPERIMENTAL_DEVICE_APIS)")
        self.assertLess(text.index("__reduce_add_sync"), opt_in)

class CudaTextureDescriptorSurfaceContractTest(unittest.TestCase):
    def test_texture_descriptor_types_are_visible_without_claiming_execution_support(self) -> None:
        root = Path(__file__).resolve().parents[1]
        types = (root / "include/bridge/runtime/cuda_runtime_types.h").read_text(encoding="utf-8")
        adaptor = (root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        header = (root / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        for name in ("wcceResourceType", "wcceResourceDesc", "wcceTextureDesc", "wcceResourceViewDesc"):
            self.assertIn(name, types)
        for cuda_name, bridge_name in (
            ("cudaResourceType", "wcceResourceType"),
            ("cudaResourceDesc", "wcceResourceDesc"),
            ("cudaTextureDesc", "wcceTextureDesc"),
            ("cudaResourceViewDesc", "wcceResourceViewDesc"),
        ):
            self.assertIn(f"#define {cuda_name} {bridge_name}", adaptor)
        self.assertIn("const wcceResourceDesc *pResDesc", header)
        self.assertIn("const wcceTextureDesc *pTexDesc", header)
        self.assertIn("const wcceResourceViewDesc *pResViewDesc", header)
        for name in (
            "cudaResViewFormatUnsignedChar4",
            "cudaResViewFormatSignedInt4",
            "cudaResViewFormatHalf2",
            "cudaResViewFormatFloat4",
        ):
            self.assertIn(name, adaptor)

class CudaArrayAndPeerTypeSurfaceContractTest(unittest.TestCase):
    def test_3d_peer_and_array_info_entries_are_typed(self) -> None:
        root = Path(__file__).resolve().parents[1]
        types = (root / "include/bridge/runtime/cuda_runtime_types.h").read_text(encoding="utf-8")
        header = (root / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        adaptor = (root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        source = (root / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        self.assertIn("wcceMemcpy3DPeerParms", types)
        for cuda_name, wcuda_name in (
            ("cudaMemcpy3DPeer", "wcudaMemcpy3DPeer"),
            ("cudaMemcpy3DPeerAsync", "wcudaMemcpy3DPeerAsync"),
            ("cudaArrayGetInfo", "wcudaArrayGetInfo"),
            ("cudaGetChannelDesc", "wcudaGetChannelDesc"),
        ):
            self.assertIn(f"#define {cuda_name} {wcuda_name}", adaptor)
            self.assertIn(wcuda_name, header)
            self.assertIn(wcuda_name, source)
        self.assertIn("wcudaMemcpy3DPeer(const wcceMemcpy3DPeerParms *p)", header)

class CudaChannelDescriptorOverloadContractTest(unittest.TestCase):
    def test_channel_descriptor_preserves_explicit_format_kind_overload(self) -> None:
        root = Path(__file__).resolve().parents[1]
        header = (root / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        source = (root / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        self.assertIn("wcudaCreateChannelDesc(int x, int y, int z, int w)", header)
        self.assertIn("wcceChannelFormatKind f", header)
        self.assertIn("wcceChannelFormatKindNone", source)

class CudaRuntimeCompatibilityHeaderEntryTest(unittest.TestCase):
    def test_common_cuda_type_headers_are_real_forwarding_entries(self) -> None:
        root = Path(__file__).resolve().parents[1]
        expected = {
            "vector_types.h": "crt/vector_types.h",
            "device_types.h": "bridge/runtime/cuda_runtime_types.h",
            "surface_types.h": "bridge/runtime/cuda_runtime_types.h",
            "texture_types.h": "bridge/runtime/cuda_runtime_types.h",
            "cuda_surface_types.h": "surface_types.h",
            "cuda_texture_types.h": "texture_types.h",
        }
        for name, target in expected.items():
            path = root / "include" / name
            self.assertTrue(path.is_file(), name)
            text = path.read_text(encoding="utf-8")
            self.assertIn(target, text)
            self.assertNotIn("__simt_vf__", text)

class CudaStandaloneTypeHeaderCompileTest(unittest.TestCase):
    def test_type_headers_compile_without_cann_or_cuda_installation(self) -> None:
        root = Path(__file__).resolve().parents[1]
        compiler = shutil.which("c++") or shutil.which("g++")
        if not compiler:
            self.skipTest("no C++ compiler")
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "types.cpp"
            source.write_text(
                "#include <bridge/runtime/cuda_runtime_types.h>\n"
                "#include <device_types.h>\n"
                "#include <surface_types.h>\n"
                "#include <texture_types.h>\n"
                "int main() { wcceExtent e{1, 2, 3}; return (int)e.width; }\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                [compiler, "-std=c++11", "-fsyntax-only", "-I", str(root / "include"), str(source)],
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)

class CudaRuntimeTopLevelHeaderParityTest(unittest.TestCase):
    def test_driver_and_library_type_headers_are_present(self) -> None:
        root = Path(__file__).resolve().parents[1]
        for name, target in (
            ("driver_types.h", "bridge/runtime/cuda_runtime_types.h"),
            ("library_types.h", "bridge/runtime/cuda_runtime_types.h"),
        ):
            path = root / "include" / name
            self.assertTrue(path.is_file(), name)
            self.assertIn(target, path.read_text(encoding="utf-8"))

class CudaTopLevelCrtHeaderEntryTest(unittest.TestCase):
    def test_top_level_crt_headers_forward_to_bridge_entries(self) -> None:
        root = Path(__file__).resolve().parents[1]
        expected = {
            "device_functions.h": "crt/device_functions.h",
            "common_functions.h": "crt/common_functions.h",
            "math_functions.h": "crt/math_functions.h",
            "builtin_types.h": "crt/builtin_types.h",
            "host_defines.h": "crt/host_defines.h",
        }
        for name, target in expected.items():
            path = root / "include" / name
            self.assertTrue(path.is_file(), name)
            self.assertIn(target, path.read_text(encoding="utf-8"))

class CudaExecutionAndLimitSurfaceContractTest(unittest.TestCase):
    def test_common_execution_and_limit_entries_have_typed_compatibility_stubs(self) -> None:
        root = Path(__file__).resolve().parents[1]
        types = (root / "include/bridge/runtime/cuda_runtime_types.h").read_text(encoding="utf-8")
        adaptor = (root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        header = (root / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        source = (root / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        for name in ("wcceFuncCache", "wcceSharedMemConfig", "wcceStreamCallback_t"):
            self.assertIn(name, types)
        for cuda_name, wcuda_name in (
            ("cudaFuncSetCacheConfig", "wcudaFuncSetCacheConfig"),
            ("cudaFuncSetSharedMemConfig", "wcudaFuncSetSharedMemConfig"),
            ("cudaStreamAddCallback", "wcudaStreamAddCallback"),
            ("cudaThreadGetLimit", "wcudaThreadGetLimit"),
            ("cudaThreadSetLimit", "wcudaThreadSetLimit"),
            ("cudaLaunchCooperativeKernel", "wcudaLaunchCooperativeKernel"),
        ):
            self.assertIn(f"#define {cuda_name} {wcuda_name}", adaptor)
            self.assertIn(wcuda_name, header)
            self.assertIn(wcuda_name, source)

class CudaAsyncArraySymbolSurfaceContractTest(unittest.TestCase):
    def test_async_array_and_symbol_entries_are_declared_as_unsupported(self) -> None:
        root = Path(__file__).resolve().parents[1]
        adaptor = (root / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        header = (root / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        source = (root / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        for cuda_name, wcuda_name in (
            ("cudaMemcpyToArrayAsync", "wcudaMemcpyToArrayAsync"),
            ("cudaMemcpyFromArrayAsync", "wcudaMemcpyFromArrayAsync"),
            ("cudaMemcpy2DToArrayAsync", "wcudaMemcpy2DToArrayAsync"),
            ("cudaMemcpy2DFromArrayAsync", "wcudaMemcpy2DFromArrayAsync"),
            ("cudaMemcpyArrayToArray", "wcudaMemcpyArrayToArray"),
            ("cudaMemcpy2DArrayToArray", "wcudaMemcpy2DArrayToArray"),
            ("cudaMemcpyToSymbolAsync", "wcudaMemcpyToSymbolAsync"),
            ("cudaMemcpyFromSymbolAsync", "wcudaMemcpyFromSymbolAsync"),
        ):
            self.assertIn(f"#define {cuda_name} {wcuda_name}", adaptor)
            self.assertIn(wcuda_name, header)
            self.assertIn(wcuda_name, source)
        self.assertIn("array_not_supported", source)

class ServerDeviceProbeRegressionContractTest(unittest.TestCase):
    def test_probe_places_language_selector_before_cuda_input(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "tools/real_test/diag/cann_api_surface_probe.sh").read_text(encoding="utf-8")
        self.assertIn('"$CCEC" -x asc --enable-simt --cce-aicore-arch="$NPU_ARCH" \\', text)
        self.assertNotIn('"$CCEC" -c "$device_root/device_api_probe.cu" \\', text)
        self.assertNotIn('"$CCEC" -c "$device_root/device_api_extended_probe.cu" \\', text)

    def test_legacy_shfl_macros_are_opt_in_to_avoid_cann_internal_collisions(self) -> None:
        root = Path(__file__).resolve().parents[1]
        text = (root / "include/bridge/kernel/cuda_device_api.h").read_text(encoding="utf-8")
        marker = "defined(CU2CCE_ENABLE_LEGACY_DEVICE_APIS)"
        self.assertIn(marker, text)
        self.assertLess(text.index(marker), text.index("define __shfl(...)"))
