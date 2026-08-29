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


if __name__ == "__main__":
    unittest.main()
