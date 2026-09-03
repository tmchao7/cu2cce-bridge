from __future__ import annotations

import os
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class CannApiProbeTest(unittest.TestCase):
    def test_probe_scripts_are_present(self) -> None:
        for name in ("cann_api_surface_probe.sh", "cann_runtime_link_probe.sh"):
            path = ROOT / "tools/real_test/diag" / name
            self.assertTrue(path.is_file(), name)
            self.assertTrue(os.access(path, os.X_OK), name)

    def test_api_surface_probe_reports_required_headers(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "include/acl").mkdir(parents=True)
            (root / "include/acl/acl_rt.h").write_text(
                "#ifndef ACL_RT_H\n#define ACL_RT_H\nint aclrtGetDeviceCount(int *count);\n#endif\n",
                encoding="utf-8",
            )
            (root / "include/acl/acl_rt_api.h").write_text(
                "#ifndef ACL_RT_API_H\n#define ACL_RT_API_H\n#endif\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                [str(ROOT / "tools/real_test/diag/cann_api_surface_probe.sh"),
                 "--cann-base", str(root)],
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("acl_rt.h", result.stdout)

    def test_api_surface_probe_fails_when_runtime_header_is_missing(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            result = subprocess.run(
                [str(ROOT / "tools/real_test/diag/cann_api_surface_probe.sh"),
                 "--cann-base", tmp],
                capture_output=True, text=True, check=False,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("acl_rt.h", result.stdout + result.stderr)

    def test_surface_probe_accepts_device_compile_inputs(self) -> None:
        script = (ROOT / "tools/real_test/diag/cann_api_surface_probe.sh").read_text(encoding="utf-8")
        self.assertIn("--bridge-include", script)
        self.assertIn("--ccec", script)
        self.assertIn("--npu-arch", script)
        self.assertIn("__shfl_down_sync", script)
        self.assertIn("atomicCAS", script)

    def test_surface_probe_checks_installed_simt_api_layout(self) -> None:
        script = (ROOT / "tools/real_test/diag/cann_api_surface_probe.sh").read_text(encoding="utf-8")
        self.assertIn('"simt_api/asc_simt.h"', script)
        self.assertIn('"simt_api/device_warp_functions.h"', script)
        self.assertIn('"simt_api/device_atomic_functions.h"', script)
        self.assertIn("acl_rt_api.h", script)
        self.assertIn("include compile acl/acl_rt.h + acl/acl_rt_api.h", script)
