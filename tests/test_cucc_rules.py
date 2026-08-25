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
from cu2cce_driver import DriverError, RuleDriver  # noqa: E402


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

    def test_object_link_drops_cudart_but_does_not_map_cublas(self) -> None:
        prepared = self.driver().prepare([
            "build/main.o", "-lcudart", "-lcublas", "-o", "build/app",
        ])
        self.assertEqual(prepared.phase, "link")
        self.assertNotIn("-lcudart", prepared.args)
        self.assertIn("-lcublas", prepared.args)
        self.assertIn("-lcce_bridge_runtime", prepared.args)
        self.assertNotIn("--enable-simt", prepared.args)

    def test_source_without_c_is_compile_and_link(self) -> None:
        prepared = self.driver().prepare(["main.cu", "-o", "app"])
        self.assertEqual(prepared.phase, "compile_and_link")
        self.assertIn("--enable-simt", prepared.args)
        self.assertIn("-lcce_bridge_runtime", prepared.args)

    def test_ptx_and_device_link_are_rejected(self) -> None:
        for flag in ("--ptx", "--device-link", "-rdc=true"):
            with self.subTest(flag=flag):
                with self.assertRaises(DriverError):
                    self.driver().prepare([flag, "main.cu"])

    def test_response_file_uses_shell_quoting(self) -> None:
        response = Path(self.tmp.name) / "options.rsp"
        response.write_text('-I "path with spaces" -DVALUE=7\n', encoding="utf-8")
        prepared = self.driver().prepare([f"@{response}", "-c", "main.cu", "-o", "main.o"])
        self.assertIn("path with spaces", prepared.args)
        self.assertIn("-DVALUE=7", prepared.args)

    def test_strict_mode_rejects_unvalidated_library(self) -> None:
        with self.assertRaises(DriverError):
            self.driver(CU2CCE_STRICT_FLAGS="1").prepare(["main.o", "-lcufft", "-o", "app"])

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

    def test_cmake_identity_macros_are_added_by_shared_driver(self) -> None:
        prepared = RuleDriver(env=self.env, cmake_identity=True).prepare(["-c", "main.cu", "-o", "main.o"])
        self.assertIn("-D__CUDACC__", prepared.args)
        self.assertIn("-D__CUDACC_VER__=110600", prepared.args)


if __name__ == "__main__":
    unittest.main()
