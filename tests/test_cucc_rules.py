#!/usr/bin/env python3
"""Unit tests for the rule-driven cu2cce argument adapter."""
from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from cu2cce_driver import DriverError, RuleDriver, scan_device_double_usage  # noqa: E402


class CuccRuleDriverTest(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        root = Path(self.tmp.name)
        self.cucc = root / "install"
        (self.cucc / "bin").mkdir(parents=True)
        (self.cucc / "include/shadow").mkdir(parents=True)
        (self.cucc / "include/bridge/kernel").mkdir(parents=True)
        (self.cucc / "third_party/nvidia_cuda_samples/Common").mkdir(parents=True)
        (self.cucc / "include/bridge/kernel/simt_adaptor.h").touch()
        (self.cucc / "lib").mkdir()
        self.ccec = root / "ccec"
        self.ccec.touch()
        self.simt = root / "simt"
        (self.simt / "simt_api").mkdir(parents=True)
        self.env = {
            "CUCC_PATH": str(self.cucc),
            "CCEC_PATH": str(self.ccec),
            "ASCEND_TOOLKIT_HOME": str(root / "ascend"),
            "CANN_BASE": str(root / "cann"),
            "SIMT_API_ROOT": str(self.simt),
            "CU2CCE_NPU_ARCH": "dav-c310-vec",
            "CUCC_CUDA_VERSION": "11060",
        }

    def tearDown(self) -> None:
        self.tmp.cleanup()

    def driver(self, **extra: str) -> RuleDriver:
        env = dict(self.env)
        env.update(extra)
        return RuleDriver(env=env)

    def test_compile_phase_drops_gpu_flags_and_injects_simt(self) -> None:
        prepared = self.driver().prepare([
            "-c", "main.cu", "-gencode", "arch=compute_80,code=sm_80",
            "-Xcompiler", "-fPIC", "-o", "build/main.o",
        ])
        self.assertEqual(prepared.phase, "compile")
        self.assertIn("-x", prepared.args)
        self.assertIn("asc", prepared.args)
        self.assertEqual(prepared.args.count("-x"), 1)
        self.assertEqual(prepared.args.count("asc"), 1)
        self.assertIn("--enable-simt", prepared.args)
        self.assertIn("-fPIC", prepared.args)
        self.assertNotIn("-gencode", prepared.args)
        self.assertIn("-c", prepared.args)
        self.assertIn("--cce-aicore-arch=dav-c310-vec", prepared.args)
        self.assertIn("__CU2CCE_DEVICE_COMPILATION__", prepared.args)

    def test_propagated_host_flags_are_filtered(self) -> None:
        # CMake CUDA_PROPAGATE_HOST_FLAGS sends host flags through
        # -Xcompiler. -m64/-march are machine-specific and must not reach the
        # Ascend device compile; -fno-exceptions/-fPIC are safe and are kept.
        dropped = self.driver().prepare([
            "-c", "main.cu", "-Xcompiler", "-m64",
            "-Xcompiler", "-march=native", "-o", "main.o",
        ])
        self.assertNotIn("-m64", dropped.args)
        self.assertNotIn("-march=native", dropped.args)
        kept = self.driver().prepare([
            "-c", "main.cu", "-Xcompiler", "-fno-exceptions",
            "-Xcompiler", "-fPIC", "-o", "main.o",
        ])
        self.assertIn("-fno-exceptions", kept.args)
        self.assertIn("-fPIC", kept.args)

    def test_object_link_drops_cudart_but_does_not_map_cublas(self) -> None:
        prepared = self.driver().prepare([
            "build/main.o", "-lcudart", "-lcublas", "-o", "build/app",
        ])
        self.assertEqual(prepared.phase, "link")
        self.assertNotIn("-lcudart", prepared.args)
        self.assertIn("-lcublas", prepared.args)
        self.assertIn("-lcce_bridge_runtime", prepared.args)
        self.assertNotIn("--enable-simt", prepared.args)

    def test_fp64_policy_is_validated(self) -> None:
        with self.assertRaises(DriverError):
            self.driver(CU2CCE_DEVICE_FP64_POLICY="bad")

    def test_device_double_scanner_hits_only_device_regions(self) -> None:
        # Mentor guidance (2026-08-27): unsupported device types must be
        # reported proactively so the user downgrades to float manually.
        source = (
            "// double in a comment must be ignored\n"
            "double host_var = 1.0;\n"                      # host scope: OK
            "static const char *s = \"double in string\";\n"
            "\n"
            "__device__ float dev_fn(float v) {\n"
            "    double acc = 0.5;\n"                       # body hit
            "    return v;\n"
            "}\n"
            "__global__ void kern(const double* a, double* o) {\n"  # params hit
            "    o[0] = a[0];\n"
            "}\n"
            "double host_fn(double x) { return x; }\n"      # host-only fn: OK
        )
        hits = scan_device_double_usage(source)
        hit_lines = {ln for ln, _ in hits}
        self.assertIn(6, hit_lines)
        self.assertIn(9, hit_lines)
        for banned in (2, 3, 12):
            self.assertNotIn(banned, hit_lines, f"false positive at line {banned}")

    def test_error_policy_blocks_build_before_ccec(self) -> None:
        src = Path(self.tmp.name) / "with_double.cu"
        src.write_text(
            "__global__ void k(const double* a, double* o) {\n"
            "    o[threadIdx.x] = a[threadIdx.x] * 2.0;\n"
            "}\n",
            encoding="utf-8",
        )
        with self.assertRaises(DriverError) as ctx:
            self.driver(CU2CCE_DEVICE_FP64_POLICY="error").prepare(
                ["-c", str(src), "-o", str(src.with_suffix(".o"))]
            )
        self.assertIn("[type-policy]", str(ctx.exception))
        self.assertIn("float", str(ctx.exception))

    def test_warn_policy_reports_then_continues_to_prepare(self) -> None:
        src = Path(self.tmp.name) / "warn_case.cu"
        src.write_text("__device__ void d() { double x = 0; (void)x; }\n", encoding="utf-8")
        prepared = self.driver(CU2CCE_DEVICE_FP64_POLICY="warn").prepare(
            ["-c", str(src), "-o", str(src.with_suffix(".o"))]
        )
        self.assertEqual(prepared.phase, "compile")  # did not raise

    def test_allow_policy_skips_scan_entirely(self) -> None:
        src = Path(self.tmp.name) / "allow_case.cu"
        src.write_text("__global__ void g(double* o) { *o = 1.0; }\n", encoding="utf-8")
        prepared = self.driver(CU2CCE_DEVICE_FP64_POLICY="allow").prepare(
            ["-c", str(src), "-o", str(src.with_suffix(".o"))]
        )
        self.assertEqual(prepared.phase, "compile")

    def test_system_libc_mode_avoids_shadow_headers(self) -> None:
        prepared = self.driver(CU2CCE_LIBC_MODE="system").prepare(["-c", "main.cu", "-o", "main.o"])
        self.assertNotIn("__CU2CCE_NO_LIBC__", prepared.args)
        self.assertIn("__CU2CCE_SYSTEM_LIBC__", prepared.args)
        self.assertIn("__CU2CCE_DEVICE_COMPILATION__", prepared.args)
        self.assertNotIn(str(self.cucc / "include/shadow"), prepared.args)
        self.assertEqual(self.driver(CU2CCE_LIBC_MODE="system").libc_mode, "system")

    def test_bridge_include_paths_precede_user_cann_paths(self) -> None:
        prepared = self.driver().prepare(["-c", "main.cu", "-I", "/usr/local/Ascend/latest/include", "-o", "main.o"])
        bridge_index = prepared.args.index("-I")
        self.assertEqual(prepared.args[bridge_index + 1], str(self.cucc / "include/shadow"))
        cann_index = prepared.args.index("/usr/local/Ascend/latest/include")
        self.assertLess(bridge_index, cann_index)

    def test_shared_cuda_link_injects_pic(self) -> None:
        prepared = self.driver().prepare(["-shared", "main.cu", "-o", "libmain.so"])
        self.assertEqual(prepared.phase, "compile_and_link")
        self.assertIn("-fPIC", prepared.args)

    def test_source_without_c_is_compile_and_link(self) -> None:
        prepared = self.driver().prepare(["main.cu", "-o", "app"])
        self.assertEqual(prepared.phase, "compile_and_link")
        self.assertIn("--enable-simt", prepared.args)
        self.assertIn("-lcce_bridge_runtime", prepared.args)

    def test_ptx_and_device_link_are_rejected(self) -> None:
        for flag in ("--ptx", "-rdc=true"):
            with self.subTest(flag=flag):
                with self.assertRaises(DriverError):
                    self.driver().prepare([flag, "main.cu"])

    def test_device_link_flag_is_dropped_and_phase_downgraded(self) -> None:
        # The cucc CMake layer forces CUDA_SEPARABLE_COMPILATION OFF, so a
        # stray -dlink/--device-link must not hard-crash configure. The flag
        # is dropped by the rules and the phase degrades to a normal link.
        prepared = self.driver().prepare(["main.cu", "-o", "app", "-dlink"])
        self.assertEqual(prepared.phase, "device_link")
        self.assertEqual(prepared.mode, "link")
        self.assertNotIn("-dlink", prepared.args)
        self.assertNotIn("--device-link", prepared.args)

    def test_new_large_project_flags_are_dropped(self) -> None:
        for flag in ("-maxrregcount", "-maxrregcount=128",
                     "--generate-line-info", "-cubin", "--Werror",
                     "-m64", "--machine", "--machine=64",
                     "-Xcudafe", "-Xcudafe=--no-abi",
                     "--use_fast_math", "-G",
                     "--ftz", "--ftz=true",
                     "-Wl,-Bstatic", "-Wl,--start-group",
                     "-Xnvlink", "--forward-unknown-to-host-compiler=on",
                     "-rdc=off"):
            with self.subTest(flag=flag):
                prepared = self.driver().prepare(["-c", "main.cu", flag, "-o", "main.o"])
                self.assertNotIn(flag, prepared.args, flag)

    def test_rdc_true_variants_are_rejected(self) -> None:
        # -rdc / -rdc=true / --relocatable-device-code / =true are still a
        # hard error (separable compilation is not just dropped); the =false
        # and -rdc=off spellings are benign and dropped by the drop rules.
        for flag in ("-rdc", "-rdc=true", "--relocatable-device-code",
                     "--relocatable-device-code=true"):
            with self.subTest(flag=flag):
                with self.assertRaises(DriverError):
                    self.driver().prepare([flag, "main.cu"])
        for flag in ("-rdc=off", "--relocatable-device-code=false"):
            with self.subTest(flag=flag):
                prepared = self.driver().prepare(["-c", "main.cu", flag, "-o", "main.o"])
                self.assertNotIn(flag, prepared.args, flag)

    def test_response_file_uses_shell_quoting(self) -> None:
        response = Path(self.tmp.name) / "options.rsp"
        response.write_text('-I "path with spaces" -DVALUE=7\n', encoding="utf-8")
        prepared = self.driver().prepare([f"@{response}", "-c", "main.cu", "-o", "main.o"])
        self.assertIn("path with spaces", prepared.args)
        self.assertIn("-DVALUE=7", prepared.args)

    def test_strict_mode_rejects_unvalidated_library(self) -> None:
        with self.assertRaises(DriverError):
            self.driver(CU2CCE_STRICT_FLAGS="1").prepare(["main.o", "-lcufft", "-o", "app"])

    def test_cubridge_parity_flags_round2(self) -> None:
        # Second full-surface audit vs cu-bridge bin/cucc + conf.json:
        # separable-compilation spellings, -link/-l: forms, nvcc-specific
        # warning/diagnostic/opt spellings, stale CUDA include paths.
        src = "t.cu"
        (Path(self.tmp.name) / src).write_text("", encoding="utf-8")

        # -dc/--device-c (emitted by CMake CUDA_SEPARABLE_COMPILATION=ON) must
        # be rejected like -rdc=true, never forwarded to ccec.
        for flag in ("-dc", "--device-c"):
            with self.assertRaises(DriverError, msg=flag):
                self.driver().prepare(["-c", src, flag, "-o", "t.o"])

        # -dw/--device-w/--compile normalize to plain -c.
        for flag in ("-dw", "--device-w", "--compile"):
            prepared = self.driver().prepare(["-c", src, flag, "-o", "t.o"])
            acts = [a.action for a in prepared.actions if a.argument == flag]
            self.assertEqual(acts, ["replace"], flag)

        def dropped(flag: str) -> None:
            prepared = self.driver().prepare(["-c", src, flag, "-o", "t.o"])
            acts = [a.action for a in prepared.actions if a.argument == flag]
            self.assertEqual(acts, ["drop"], flag)

        # -link must be dropped before the -l* forward rule treats it as a
        # library reference.
        dropped("-link")
        for flag in ("-x=cu", "-l:libcudart.so", "-l:libcuda.so",
                     "--Wno-deprecated-gpu-targets", "--offload-arch=sm_80",
                     "-flto=thin", "--diag_suppress=xyz", "--restrict",
                     "--source-in-ptx", "--disable-warnings"):
            dropped(flag)

        # stale CUDA toolkit include paths are dropped like -L shim paths
        prepared = self.driver().prepare(
            ["-c", src, "-I/usr/local/cuda/include", "-o", "t.o"])
        acts = [a.action for a in prepared.actions
                if a.argument == "-I/usr/local/cuda/include"]
        self.assertEqual(acts, ["drop"])

    def test_double_dash_werror_consumes_warning_name(self) -> None:
        # cu-bridge bin/cucc:84-116 pair logic: nvcc's --Werror takes a
        # warning NAME as the next token; that name must not leak into ccec
        # as a stray input.
        src = "t.cu"
        (Path(self.tmp.name) / src).write_text("", encoding="utf-8")
        prepared = self.driver().prepare(
            ["-c", src, "--Werror", "cross-execution-space-call", "-o", "t.o"])
        args = " ".join(prepared.args)
        self.assertNotIn("cross-execution-space-call", args)

    def test_custom_rules_file_overrides_default(self) -> None:
        rules = Path(self.tmp.name) / "custom.json"
        rules.write_text(json.dumps({
            "schema_version": 1,
            "frontend": {"default_arch": "custom-arch"},
            "all": {
                "forward": [], "drop": [], "replace": [],
                "reject": [{"pattern": "--custom-reject", "reason": "test rule"}]
            },
            "compile": {"inject": []},
            "link": {"inject": []},
            "compile_and_link": {"inject": []},
            "capabilities": {}
        }), encoding="utf-8")
        with self.assertRaises(DriverError):
            self.driver(CU2CCE_RULES_FILE=str(rules)).prepare(["--custom-reject", "main.cu"])

    def test_trace_contains_decisions_without_source_content(self) -> None:
        trace = Path(self.tmp.name) / "trace.json"
        prepared = self.driver(CU2CCE_TRACE_FILE=str(trace)).prepare([
            "-c", "secret_kernel.cu", "-gencode=arch=compute_80,code=sm_80", "-o", "kernel.o",
        ])
        payload = json.loads(trace.read_text(encoding="utf-8"))
        self.assertEqual(payload["phase"], "compile")
        self.assertTrue(any(item["action"] == "drop" for item in payload["actions"]))
        self.assertNotIn("secret_kernel source", trace.read_text(encoding="utf-8"))
        self.assertEqual(prepared.phase, "compile")

    def test_mock_nvcc_uses_shared_driver(self) -> None:
        fake = Path(self.tmp.name) / "fake-ccec"
        log = Path(self.tmp.name) / "mock-ccec-args"
        fake.write_text("#!/bin/sh\nprintf '%s\n' \"$@\" > \"$FAKE_LOG\"\nexit 0\n", encoding="utf-8")
        fake.chmod(0o755)
        env = dict(os.environ)
        env.update(self.env)
        env["CUCC_PATH"] = str(Path(__file__).resolve().parents[1])
        env["CCEC_PATH"] = str(fake)
        env["FAKE_LOG"] = str(log)
        result = subprocess.run([
            sys.executable,
            str(Path(__file__).resolve().parents[1] / "tools/mock/mock_nvcc.py"),
            "-c", "main.cu", "-gencode", "arch=compute_80,code=sm_80", "-o", "main.o",
        ], env=env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        args = log.read_text(encoding="utf-8").splitlines()
        self.assertIn("-D__CUDACC__", args)
        self.assertIn("--enable-simt", args)
        self.assertNotIn("-gencode", args)

    def test_preprocess_and_dependency_phases_are_distinct(self) -> None:
        preprocess = self.driver().prepare(["-E", "main.cu", "-o", "main.i"])
        dependency = self.driver().prepare(["-M", "main.cu", "-MF", "main.d"])
        self.assertEqual(preprocess.phase, "preprocess")
        self.assertEqual(preprocess.mode, "preprocess")
        self.assertEqual(dependency.phase, "dependency")
        self.assertEqual(dependency.mode, "dependency")
        self.assertNotIn("-lcce_bridge_runtime", preprocess.args)
        self.assertNotIn("-lcce_bridge_runtime", dependency.args)

    def test_assembly_mode_is_rejected(self) -> None:
        with self.assertRaises(DriverError):
            self.driver().prepare(["-S", "main.cu", "-o", "main.s"])

    def test_dryrun_and_query_entrypoints(self) -> None:
        driver = Path(__file__).resolve().parents[1] / "bin/cucc"
        version = subprocess.run([str(driver), "--version"], capture_output=True, text=True)
        help_result = subprocess.run([str(driver), "--help"], capture_output=True, text=True)
        self.assertEqual(version.returncode, 0)
        self.assertEqual(help_result.returncode, 0)

    def test_cmake_identity_macros_are_added_by_shared_driver(self) -> None:
        prepared = RuleDriver(env=self.env, cmake_identity=True).prepare(["-c", "main.cu", "-o", "main.o"])
        self.assertIn("-D__CUDACC__", prepared.args)
        self.assertIn("-D__CUDACC_VER__=110600", prepared.args)

    def test_conda_ldflags_are_appended_to_link_phase(self) -> None:
        env = dict(self.env)
        env["CONDA_EXE"] = "/opt/conda/bin/conda"
        env["LDFLAGS"] = "-L/opt/conda/lib -Wl,-rpath,/opt/conda/lib"
        linked = RuleDriver(env=env).prepare(["main.o", "-o", "app"])
        self.assertEqual(linked.phase, "link")
        self.assertIn("-L/opt/conda/lib", linked.args)
        self.assertIn("-Wl,-rpath,/opt/conda/lib", linked.args)
        # Compile phase must not gain conda link flags.
        compiled = RuleDriver(env=env).prepare(["-c", "main.cu", "-o", "main.o"])
        self.assertNotIn("-L/opt/conda/lib", compiled.args)

    def test_mock_nvcc_splices_cudaflags_env(self) -> None:
        # mock_nvcc reads CUDAFLAGS (nvcc style) and feeds it to the shared
        # rule driver. The driver trace captures the final argument list
        # without executing any compiler, so this stays a pure-contract test.
        import importlib.util  # noqa: PLC0415
        mock_path = Path(__file__).resolve().parents[1] / "tools/mock/mock_nvcc.py"
        spec = importlib.util.spec_from_file_location("mock_nvcc_mod", mock_path)
        assert spec is not None and spec.loader is not None
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        env = {"CUDAFLAGS": "-lineinfo --cuda-gpu-arch=sm_80"}
        spliced = module._splice_cudaflags(["-c", "main.cu", "-o", "main.o"], env)
        self.assertIn("-lineinfo", spliced)
        self.assertIn("--cuda-gpu-arch=sm_80", spliced)
        self.assertEqual(spliced[2:], ["-c", "main.cu", "-o", "main.o"])
        # Empty CUDAFLAGS is a no-op.
        self.assertEqual(
            module._splice_cudaflags(["-c", "main.cu"], {"CUDAFLAGS": ""}),
            ["-c", "main.cu"],
        )

    def test_mock_nvcc_probe_short_circuits(self) -> None:
        # CMake probes that must never reach the rule driver/ccec are answered
        # by the shim as pure functions (no subprocess, no compiler). Verify
        # the --simple-preprocess response and the ABI-neuter handler.
        import importlib.util  # noqa: PLC0415
        mock_path = Path(__file__).resolve().parents[1] / "tools/mock/mock_nvcc.py"
        spec = importlib.util.spec_from_file_location("mock_nvcc_probe_mod", mock_path)
        assert spec is not None and spec.loader is not None
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)

        preprocess = module._simple_preprocess_response()
        self.assertIn("__CUDACC_VER_MAJOR__ 11", preprocess)
        self.assertIn("__CUDACC_VER_MINOR__ 6", preprocess)

        cwd = Path(self.tmp.name)
        abi_obj = cwd / "abi.o"
        code = module._handle_abi_probe([
            "-c", "Modules/CMakeCUDACompilerABI.cu", "-o", str(abi_obj),
        ], cwd)
        self.assertEqual(code, 0)
        self.assertTrue(abi_obj.is_file())
        self.assertGreater(abi_obj.stat().st_size, 0)

    def test_mock_nvcc_abi_neuter_pure(self) -> None:
        # The ABI-neuter handler writes a placeholder where CMake expects the
        # ABI object, without invoking any compiler.
        import importlib.util  # noqa: PLC0415
        mock_path = Path(__file__).resolve().parents[1] / "tools/mock/mock_nvcc.py"
        spec = importlib.util.spec_from_file_location("mock_nvcc_abi", mock_path)
        assert spec is not None and spec.loader is not None
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        cwd = Path(self.tmp.name)
        obj = cwd / "sub" / "abi-placeholder.o"
        code = module._handle_abi_probe([
            "-c", "Modules/CMakeCUDACompilerABI.cu", "-o", str(obj),
        ], cwd)
        self.assertEqual(code, 0)
        self.assertTrue(obj.is_file())

    def test_explicit_cuda_arch_injects_arch_macro(self) -> None:
        # A user-pinned CUDA_ARCHITECTURES value (mapped to CU2CCE_CUDA_ARCH
        # by the caller) is injected as __CUDA_ARCH__ so device code behind
        # `#if __CUDA_ARCH__ >= NNN` compiles against the intended fake arch.
        prepared = self.driver(CU2CCE_CUDA_ARCH="800").prepare([
            "-c", "main.cu", "-o", "main.o",
        ])
        self.assertIn("__CUDA_ARCH__=800", prepared.args)
        # Without the env var, nothing is injected.
        plain = self.driver().prepare(["-c", "main.cu", "-o", "main.o"])
        self.assertNotIn("__CUDA_ARCH__", " ".join(plain.args))

    def test_query_probes_bypass_the_rule_pipeline(self) -> None:
        # cu-bridge skipstring parity (conf.json:3-17): toolchain probes
        # (-dumpversion, -print-file-name=...) bypass every injection and are
        # handed to ccec unchanged, so autoconf/libtool-style probes see the
        # backend compiler's own answers.
        for probe in ("-print-file-name=cudart", "-dumpmachine",
                      "-print-search-dirs", "-print-prog-name=cc1"):
            with self.subTest(probe=probe):
                prepared = self.driver().prepare([probe])
                self.assertEqual(prepared.phase, "query")
                self.assertEqual(prepared.mode, "query")
                self.assertEqual(prepared.args, [probe])
                self.assertNotIn("-x", prepared.args)
                self.assertNotIn("--enable-simt", prepared.args)
                self.assertEqual(prepared.compiler, str(self.ccec))

    def test_user_std_flag_suppresses_default_injection(self) -> None:
        # Only one -std may reach ccec: an explicit user value wins over the
        # injected -std=gnu++17 (cu-bridge forwards user flags verbatim).
        explicit = self.driver().prepare(["-c", "main.cu", "-std=c++14", "-o", "main.o"])
        self.assertIn("-std=c++14", explicit.args)
        self.assertNotIn("-std=gnu++17", explicit.args)
        default = self.driver().prepare(["-c", "main.cu", "-o", "main.o"])
        self.assertIn("-std=gnu++17", default.args)

    def test_arch_macro_auto_derives_from_gencode(self) -> None:
        # CU2CCE_ARCH_MACRO=auto borrows cu-bridge's not_found.dev_compile
        # chain: the dropped -gencode/-arch value is recovered and injected
        # as __CUDA_ARCH__ (nvcc encoding: sm_80 -> 800, sm_86 -> 860,
        # sm_120 -> 1200). Off by default — the puresimt path has no separate
        # device pass, so this stays an explicit opt-in (PARITY_AUDIT).
        auto = self.driver(CU2CCE_ARCH_MACRO="auto").prepare([
            "-c", "main.cu", "-gencode", "arch=compute_80,code=sm_86", "-o", "main.o"])
        self.assertIn("__CUDA_ARCH__=860", auto.args)
        off = self.driver().prepare([
            "-c", "main.cu", "-gencode", "arch=compute_80,code=sm_86", "-o", "main.o"])
        self.assertNotIn("__CUDA_ARCH__", " ".join(off.args))
        offload = self.driver(CU2CCE_ARCH_MACRO="auto").prepare([
            "-c", "main.cu", "--offload-arch=sm_90", "-o", "main.o"])
        self.assertIn("__CUDA_ARCH__=900", offload.args)
        # Explicit CU2CCE_CUDA_ARCH still wins over the derived value.
        pinned = self.driver(CU2CCE_ARCH_MACRO="auto", CU2CCE_CUDA_ARCH="800").prepare([
            "-c", "main.cu", "-gencode", "arch=compute_80,code=sm_86", "-o", "main.o"])
        self.assertIn("__CUDA_ARCH__=800", pinned.args)
        self.assertNotIn("__CUDA_ARCH__=860", pinned.args)
        # Invalid knob values are rejected like the other policy knobs.
        with self.assertRaises(DriverError):
            self.driver(CU2CCE_ARCH_MACRO="sometimes")

    def test_cuda_runtime_h_is_preincluded_for_cuda_sources(self) -> None:
        # nvcc auto-preincludes cuda_runtime.h for every .cu TU (cu-bridge
        # condition parity). The bridge runtime header transitively includes
        # simt_adaptor.h behind an include guard, so ordering is safe either
        # way; the injection only fires when the header ships.
        (self.cucc / "include/cuda_runtime.h").touch()
        prepared = self.driver().prepare(["-c", "main.cu", "-o", "main.o"])
        args = prepared.args
        self.assertIn("-include", args)
        runtime_idx = args.index(str(self.cucc / "include/cuda_runtime.h"))
        adaptor_idx = args.index(str(self.cucc / "include/bridge/kernel/simt_adaptor.h"))
        self.assertLess(runtime_idx, adaptor_idx)
        # Non-CUDA TUs are untouched.
        host = self.driver().prepare(["-c", "main.cpp", "-o", "main.o"])
        self.assertNotIn("-include", host.args)

    def test_hostcc_mode_is_rule_driven(self) -> None:
        # bin/gnu parity moved into the rules file: hostcc_cce delegates to
        # the driver's --hostcc-mode; links carry the bridge runtime + CANN
        # acl, compiles/queries pass through, and .cu is refused.
        driver_path = Path(__file__).resolve().parents[1] / "tools/cu2cce_driver.py"

        def hostcc(*cli_args: str) -> subprocess.CompletedProcess:
            env = dict(os.environ)
            env.update(self.env)
            return subprocess.run(
                [sys.executable, str(driver_path), "--hostcc-mode",
                 "--hostcc-name", "g++", "--proxy-mode", "--", *cli_args],
                env=env, capture_output=True, text=True)

        link = hostcc("main.o", "-o", "app")
        self.assertEqual(link.returncode, 0, link.stderr)
        self.assertIn("-lcce_bridge_runtime", link.stdout)
        self.assertIn("-lascendcl", link.stdout)
        self.assertIn("-o app", link.stdout)
        compile_only = hostcc("-c", "helper.cpp", "-o", "helper.o")
        self.assertEqual(compile_only.returncode, 0, compile_only.stderr)
        self.assertNotIn("-lcce_bridge_runtime", compile_only.stdout)
        self.assertIn("-c helper.cpp", compile_only.stdout)
        refused = hostcc("-c", "kern.cu", "-o", "kern.o")
        self.assertEqual(refused.returncode, 2)
        self.assertIn("cucc", refused.stderr)

    def test_capability_prefix_match_stays_advisory(self) -> None:
        # Unknown variants of a capability library keep the advisory warning
        # (never silently forwarded), but the message is honest about being
        # a heuristic prefix match rather than an exact capability hit.
        prepared = self.driver().prepare(["main.o", "-lcufftw", "-o", "app"])
        self.assertIn("-lcufftw", prepared.args)
        actions = [action for action in prepared.actions if action.argument == "-lcufftw"]
        self.assertEqual(actions[0].action, "forward")
        self.assertIn("prefix match", actions[0].reason)

    def test_host_flags_are_filtered_per_flag_in_one_group(self) -> None:
        # cu-bridge cleans per argument; a whole-group decision leaked
        # machine-specific flags sharing the comma list with safe ones.
        # CMake emits -Xcompiler values comma-joined (-fPIC,-march=native).
        prepared = self.driver().prepare([
            "-c", "main.cu", "-Xcompiler", "-fPIC,-march=native,-Wall", "-o", "main.o",
        ])
        self.assertIn("-fPIC", prepared.args)
        self.assertIn("-Wall", prepared.args)
        self.assertNotIn("-march=native", prepared.args)

    def test_xcompiler_attached_form_goes_through_the_host_filter(self) -> None:
        # Regression: the -Xcompiler=... attached spelling used to bypass the
        # host-flag filter entirely (the filter matched the literal pattern
        # instead of the concrete argument).
        prepared = self.driver().prepare([
            "-c", "main.cu", "-Xcompiler=-fPIC,-Werror=all-warnings", "-o", "main.o",
        ])
        self.assertIn("-fPIC", prepared.args)
        self.assertNotIn("-Werror=all-warnings", prepared.args)

    def test_xcompiler_werror_group_is_removed(self) -> None:
        # conf_regex.yaml C500S-11780 parity: only the bare -Werror token is
        # removed from -Xcompiler values; clang-valid spellings such as
        # -Wno-error=cpp stay (the yaml's own example list keeps it).
        prepared = self.driver().prepare([
            "-c", "main.cu", "-Xcompiler", "-Wall,-Werror,-Wno-error=cpp", "-o", "main.o",
        ])
        self.assertIn("-Wall", prepared.args)
        self.assertIn("-Wno-error=cpp", prepared.args)
        self.assertNotIn("-Werror", prepared.args)

    def test_implicit_fallthrough_is_normalized_to_clang_spelling(self) -> None:
        # cu-bridge bin/cucc:250-280: the nvcc/gcc =N level spelling is
        # rewritten to the clang-compatible form instead of being dropped.
        for flag in ("--Werror=implicit-fallthrough=5", "-Werror=implicit-fallthrough=3"):
            with self.subTest(flag=flag):
                prepared = self.driver().prepare(["-c", "main.cu", flag, "-o", "main.o"])
                self.assertIn("-Werror=implicit-fallthrough", prepared.args)
                self.assertNotIn(flag, prepared.args)
        via_xcompiler = self.driver().prepare([
            "-c", "main.cu", "-Xcompiler", "-Werror=implicit-fallthrough=2,-fPIC", "-o", "main.o",
        ])
        self.assertIn("-Werror=implicit-fallthrough", via_xcompiler.args)
        self.assertIn("-fPIC", via_xcompiler.args)

    def test_regex_cleanup_layer_drops_gcc_only_flags(self) -> None:
        # gomxccbin/conf_regex.yaml parity: a data-driven final-command
        # cleanup pass. Seeds remove gcc-only spellings wherever they arrive
        # (direct nvcc argument or via -Xcompiler splitting).
        direct = self.driver().prepare([
            "-c", "main.cu", "-fno-canonical-system-headers", "-o", "main.o"])
        self.assertNotIn("-fno-canonical-system-headers", direct.args)
        via_xcompiler = self.driver().prepare([
            "-c", "main.cu", "-Xcompiler", "-Wno-nonnull-compare", "-o", "main.o"])
        self.assertNotIn("-Wno-nonnull-compare", via_xcompiler.args)
        # The cleanup decision is visible in the trace.
        trace = Path(self.tmp.name) / "cleanup-trace.json"
        traced = self.driver(CU2CCE_TRACE_FILE=str(trace)).prepare([
            "-c", "main.cu", "-fno-canonical-system-headers", "-o", "main.o"])
        payload = json.loads(trace.read_text(encoding="utf-8"))
        self.assertTrue(any(item["action"] == "regex_drop" for item in payload["actions"]))
        self.assertNotIn("-fno-canonical-system-headers", traced.args)

    def test_gcc_toolchain_injection_follows_the_reference_condition(self) -> None:
        # add_gcc_toolchain parity (bin/cucc:1055-1066): only when the real
        # gcc is NOT /usr/bin/gcc; CU2CCE_GCC_TOOLCHAIN=off disables.
        driver = self.driver()
        driver._locate_real_gcc = lambda: "/usr/bin/gcc"  # type: ignore[method-assign]
        self.assertEqual(driver._gcc_toolchain_flags(), [])
        driver2 = self.driver()
        driver2._locate_real_gcc = lambda: "/opt/rh/gcc-toolset-12/root/usr/bin/gcc"  # type: ignore[method-assign]
        self.assertEqual(driver2._gcc_toolchain_flags(),
                         ["--gcc-toolchain=/opt/rh/gcc-toolset-12/root/usr/"])
        off = self.driver(CU2CCE_GCC_TOOLCHAIN="off")
        off._locate_real_gcc = lambda: "/opt/rh/gcc-toolset-12/root/usr/bin/gcc"  # type: ignore[method-assign]
        self.assertEqual(off._gcc_toolchain_flags(), [])
        with self.assertRaises(DriverError):
            self.driver(CU2CCE_GCC_TOOLCHAIN="sometimes")

    def test_cucc_entrypoint_propagates_bash_debug_state(self) -> None:
        # make_maca <-> cucc parity: the shell debug flags exported by
        # make_cce/pre_cce surface as CU2CCE_DEBUG in the python driver.
        driver = Path(__file__).resolve().parents[1] / "bin/cucc"
        env = dict(os.environ)
        env.update(self.env)
        env["BASH_XTRACE"] = "1"
        env.pop("CU2CCE_DEBUG", None)
        env.pop("WCUDA_DEBUG", None)
        traced = subprocess.run(
            [str(driver), "--proxy-mode", "-c", "main.cu", "-o", "main.o"],
            env=env, capture_output=True, text=True)
        self.assertEqual(traced.returncode, 0, traced.stderr)
        self.assertIn("[cu2cce]", traced.stderr)
        # Without the shell debug flags there is no driver debug output.
        env.pop("BASH_XTRACE")
        quiet = subprocess.run(
            [str(driver), "--proxy-mode", "-c", "main.cu", "-o", "main.o"],
            env=env, capture_output=True, text=True)
        self.assertNotIn("[cu2cce]", quiet.stderr)

    def test_validator_rejects_broken_regex_cleanup_patterns(self) -> None:
        import validate_cucc_rules  # noqa: PLC0415
        rules = Path(self.tmp.name) / "bad-regex.json"
        rules.write_text(json.dumps({
            "schema_version": 1,
            "frontend": {},
            "all": {"forward": [], "drop": [], "replace": [], "reject": []},
            "compile": {"inject": []},
            "link": {"inject": []},
            "compile_and_link": {"inject": []},
            "capabilities": {},
            "diagnostics": {},
            "regex_cleanup": [{"pattern": "-Werror(=[^)]", "reason": "broken PCRE"}],
        }), encoding="utf-8")
        errors = validate_cucc_rules.validate(rules)
        self.assertTrue(any("invalid regex" in error for error in errors))

    def test_cudacc_macros_are_injected_for_cuda_translation_units(self) -> None:
        # cu-bridge -imacros __macro_mxcc.h parity: nvcc defines
        # __CUDACC__/__CUDACC_VER_* for every .cu TU. The values derive from
        # CUCC_CUDA_VERSION so user overrides propagate, the macros never
        # leak into host-only TUs, and the mock identity path no longer
        # duplicates them (single definition site).
        prepared = self.driver().prepare(["-c", "main.cu", "-o", "main.o"])
        args = " ".join(prepared.args)
        self.assertIn("-D__CUDACC__", prepared.args)
        self.assertIn("-D__CUDACC_VER_MAJOR__=11", args)
        self.assertIn("-D__CUDACC_VER_MINOR__=6", args)
        self.assertIn("-D__CUDACC_VER_BUILD__=0", args)
        self.assertIn("-D__CUDACC_VER__=110600", args)
        host_tu = self.driver().prepare(["-c", "main.cpp", "-o", "main.o"])
        self.assertNotIn("-D__CUDACC__", host_tu.args)
        overridden = self.driver(CUCC_CUDA_VERSION="12090").prepare(
            ["-c", "main.cu", "-o", "main.o"])
        oargs = " ".join(overridden.args)
        self.assertIn("-D__CUDACC_VER_MAJOR__=12", oargs)
        self.assertIn("-D__CUDACC_VER_MINOR__=9", oargs)
        self.assertIn("-D__CUDACC_VER__=120900", oargs)
        identity = RuleDriver(env=self.env, cmake_identity=True).prepare(
            ["-c", "main.cu", "-o", "main.o"])
        self.assertEqual(identity.args.count("-D__CUDACC__"), 1)

    def test_nvrtc_and_opencl_libraries_are_dropped_not_forwarded(self) -> None:
        # cu-bridge conf.json parity: -lnvrtc/-lOpenCL were silently
        # forwarded by the -l* rule and only exploded at link time; they are
        # dropped with a warning now.
        for flag in ("-lnvrtc", "-l:libnvrtc.so", "-lOpenCL"):
            with self.subTest(flag=flag):
                prepared = self.driver().prepare(["main.o", flag, "-o", "app"])
                self.assertNotIn(flag, prepared.args)
                actions = [action for action in prepared.actions if action.argument == flag]
                self.assertEqual(actions[0].action, "drop")

    def test_share_spelling_is_normalized_to_shared(self) -> None:
        # cu-bridge conf.json:216 --share -> -shared.
        prepared = self.driver().prepare(["--share", "main.cu", "-o", "libmain.so"])
        self.assertIn("-shared", prepared.args)
        self.assertNotIn("--share", prepared.args)

    def test_xlinker_pair_is_forwarded_to_ccec(self) -> None:
        # cu-bridge forwards -Xlinker to the clang-based backend; dropping
        # the pair silently lost user link options (rpath, version scripts).
        prepared = self.driver().prepare([
            "main.o", "-Xlinker", "-rpath=/opt/x", "-o", "app"])
        self.assertIn("-Xlinker", prepared.args)
        self.assertIn("-rpath=/opt/x", prepared.args)
        self.assertLess(prepared.args.index("-Xlinker"),
                        prepared.args.index("-rpath=/opt/x"))

    def test_var_tracking_family_is_cleaned_by_regex_layer(self) -> None:
        # gcc-only debug flags: clang rejects them as unknown arguments;
        # cu-bridge's conf.json filter drops them.
        direct = self.driver().prepare([
            "-c", "main.cu", "-fvar-tracking-assignments", "-o", "main.o"])
        self.assertNotIn("-fvar-tracking-assignments", direct.args)
        via_xcompiler = self.driver().prepare([
            "-c", "main.cu", "-Xcompiler", "-fno-var-tracking", "-o", "main.o"])
        self.assertNotIn("-fno-var-tracking", via_xcompiler.args)


if __name__ == "__main__":
    unittest.main()
