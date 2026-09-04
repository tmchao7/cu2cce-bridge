#!/usr/bin/env python3
"""Contracts for the CUDA public header and wrapper parity surface."""
from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / "tools"


class CudaHeaderWrapperParityToolTest(unittest.TestCase):
    def test_validator_reports_header_wrapper_and_api_gaps(self) -> None:
        reference = ROOT.parent / "cu-bridge-muxi"
        result = subprocess.run(
            [sys.executable, str(TOOLS / "validate_cuda_header_wrapper_parity.py"),
             "--reference", str(reference),
             "--implementation", str(ROOT),
             "--inventory", str(ROOT / "config/cuda_api_parity_inventory.json"),
             "--format", "json"],
            capture_output=True, text=True, check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        report = json.loads(result.stdout)
        self.assertGreaterEqual(report["reference_header_entries"], 100)
        self.assertGreaterEqual(report["reference_wrapper_entries"], 60)
        self.assertGreaterEqual(report["reference_api_entries"], 300)
        self.assertEqual(report["missing_headers"], [])
        self.assertEqual(report["missing_wrapper_entries"], [])
        self.assertEqual(report["missing_api_entries"], [])
        self.assertEqual(report["duplicate_entries"], [])
        self.assertEqual(report["active_maca_backend_names"], [])

    def test_validator_requires_reserved_header_for_non_active_wrapper(self) -> None:
        sys.path.insert(0, str(TOOLS))
        import validate_cuda_header_wrapper_parity as parity

        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            reference = root / "reference"
            implementation = root / "implementation"
            for base in (reference, implementation):
                (base / "include/bridge").mkdir(parents=True)
            (reference / "include/cuda.h").write_text("#pragma once\n", encoding="utf-8")
            (reference / "include/bridge/wrapper.h").write_text(
                "int wcuFuture(void);\n", encoding="utf-8"
            )
            (implementation / "include/cuda.h").write_text("#pragma once\n", encoding="utf-8")
            (implementation / "include/bridge/cuda_to_cce_reserved_wrapper_parity.h").write_text(
                "#if 0\n#endif\n", encoding="utf-8"
            )
            inventory = root / "inventory.json"
            inventory.write_text(json.dumps({
                "headers": {"cuda.h": {"implementation": "include/cuda.h", "status": "ACTIVE_ADAPTED"}},
                "wrappers": {},
                "api_entries": {
                    "cuFuture": {"wrapper": "wcuFuture", "status": "RESERVED_NPU"}
                },
            }), encoding="utf-8")
            report = parity.audit(reference, implementation, inventory)
            self.assertEqual(report["missing_api_entries"], ["wcuFuture"])

    def test_validator_uses_committed_inventory_without_reference_checkout(self) -> None:
        result = subprocess.run(
            [sys.executable, str(TOOLS / "validate_cuda_header_wrapper_parity.py"),
             "--implementation", str(ROOT),
             "--inventory", str(ROOT / "config/cuda_api_parity_inventory.json"),
             "--format", "json"],
            capture_output=True, text=True, check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(json.loads(result.stdout)["missing_headers"], [])

    def test_public_cuda_headers_are_regular_files_and_key_wrappers_are_present(self) -> None:
        expected_headers = (
            "cuda.h", "cudaEGL.h", "cudaGL.h", "cudaVDPAU.h", "nvrtc.h",
            "cufft.h", "curand.h", "cusparse.h", "cusolverDn.h", "cudnn.h",
            "nccl.h", "npp.h", "nvjpeg.h", "nvml.h",
        )
        for name in expected_headers:
            path = ROOT / "include" / name
            self.assertTrue(path.is_file(), name)
            self.assertFalse(path.is_symlink(), name)
        for path in (
            ROOT / "include/bridge/runtime/cuda_driver_wrapper.h",
            ROOT / "include/bridge/runtime/cuda_runtime_template_wrapper.h",
            ROOT / "include/bridge/runtime/cuda_egl_wrapper.h",
            ROOT / "include/bridge/runtime/cuda_gl_wrapper.h",
            ROOT / "include/bridge/video/cuda_vdpau_wrapper.h",
        ):
            self.assertTrue(path.is_file(), str(path))
            self.assertFalse(path.is_symlink(), str(path))

    def test_library_headers_forward_to_explicit_bridge_or_placeholder_routes(self) -> None:
        expected = {
            "cublasLt.h": "bridge/blas/cublasLt_wrapper.h",
            "cublasXt.h": "bridge/blas/cublasXt_wrapper.h",
            "cufft.h": "bridge/fft/cufft_wrapper.h",
            "curand.h": "bridge/rand/curand_wrapper.h",
            "cusolverDn.h": "bridge/solver/cusolverDn_wrapper.h",
            "cusparse.h": "bridge/sparse/cusparse_wrapper.h",
            "cudnn.h": "bridge/dnn/cudnn_wrapper.h",
            "npp.h": "bridge/mpp/npp_wrapper.h",
            "nvjpeg.h": "bridge/jpeg/nvjpeg_wrapper.h",
            "nvml.h": "bridge/sml/nvml_wrapper.h",
            "nvrtc.h": "bridge/runtime/nvrtc_wrapper.h",
            "nccl.h": "cuda_library_placeholder.h",
        }
        for header, route in expected.items():
            self.assertIn(route, (ROOT / "include" / header).read_text(encoding="utf-8"), header)

    def test_basic_driver_api_is_typed_unsupported_not_a_fake_acl_alias(self) -> None:
        types = (ROOT / "include/bridge/runtime/cuda_driver_types.h").read_text(encoding="utf-8")
        wrapper = (ROOT / "include/bridge/runtime/cuda_driver_wrapper.h").read_text(encoding="utf-8")
        source = (ROOT / "src/bridge/runtime/src/cuda_driver_wrapper.cpp").read_text(encoding="utf-8")
        for item in ("CUresult", "CUdevice", "CUcontext", "CUmodule", "CUfunction", "CUdeviceptr"):
            self.assertIn(item, types)
        for item in ("wcuInit", "wcuDriverGetVersion", "wcuDeviceGetCount", "wcuDeviceGet"):
            self.assertIn(item, wrapper)
            self.assertIn(item, source)
        self.assertIn("ACL_ERROR_API_NOT_SUPPORT", source)
        self.assertNotIn("#define cuInit aclInit", wrapper)

    def test_common_driver_module_context_and_launch_apis_are_typed_unsupported(self) -> None:
        wrapper = (ROOT / "include/bridge/runtime/cuda_driver_wrapper.h").read_text(encoding="utf-8")
        source = (ROOT / "src/bridge/runtime/src/cuda_driver_wrapper.cpp").read_text(encoding="utf-8")
        for name in (
            "wcuCtxGetCurrent", "wcuCtxSetCurrent", "wcuCtxSynchronize",
            "wcuModuleLoadData", "wcuModuleUnload", "wcuModuleGetFunction",
            "wcuLaunchKernel",
        ):
            self.assertIn(name, wrapper)
            self.assertIn(name, source)
        self.assertIn("ACL_ERROR_API_NOT_SUPPORT", source)
        self.assertGreaterEqual(source.count("return kUnsupported"), 8)
        inventory = json.loads((ROOT / "config/cuda_api_parity_inventory.json").read_text(encoding="utf-8"))
        for name in ("cuCtxGetCurrent", "cuModuleLoadData", "cuLaunchKernel"):
            self.assertEqual(inventory["api_entries"][name]["status"], "UNSUPPORTED_TYPED")

    def test_key_cuda_headers_forward_to_cu2cce_surfaces(self) -> None:
        expected = {
            "cuda.h": "cuda_runtime.h",
            "cudaTypedefs.h": "driver_types.h",
            "driver_functions.h": "cuda.h",
            "cuda_device_api.h": "bridge/kernel/cuda_device_api.h",
            "cuda_occupancy.h": "cuda_runtime.h",
            "cublas.h": "cublas_v2.h",
            "cudaEGL.h": "bridge/runtime/cuda_egl_wrapper.h",
            "cudaGL.h": "bridge/runtime/cuda_gl_wrapper.h",
            "cudaVDPAU.h": "bridge/video/cuda_vdpau_wrapper.h",
            "nvrtc.h": "bridge/runtime/nvrtc_wrapper.h",
        }
        for name, include_name in expected.items():
            text = (ROOT / "include" / name).read_text(encoding="utf-8")
            self.assertIn(include_name, text, name)

    def test_cmake_install_tree_covers_new_parity_tools_and_headers(self) -> None:
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("tools/validate_cuda_header_wrapper_parity.py", cmake)
        self.assertIn("tools/generate_cuda_header_wrapper_parity.py", cmake)
        self.assertIn("tools/real_test/diag/cuda_header_surface_probe.sh", cmake)
        self.assertIn("install(DIRECTORY include/ DESTINATION include)", cmake)

    def test_server_regression_wires_header_wrapper_parity_contract(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("validate_cuda_header_wrapper_parity.py", server)
        self.assertIn("cuda_header_surface_probe.sh", server)
        self.assertIn("CUDA_HEADER_WRAPPER_PARITY_REGRESSION_PASS", server)

    def test_header_surface_probe_initializes_work_before_parity_log_redirection(self) -> None:
        script = (ROOT / "tools/real_test/diag/cuda_header_surface_probe.sh").read_text(encoding="utf-8")
        work_pos = script.index('work="${TMPDIR:-/tmp}/cu2cce-header-probe.$$"')
        parity_log_pos = script.index('>"$work/parity.log"')
        self.assertLess(work_pos, parity_log_pos)

    def test_header_surface_probe_has_layered_checks_and_compact_output(self) -> None:
        path = ROOT / "tools/real_test/diag/cuda_header_surface_probe.sh"
        self.assertTrue(path.is_file())
        self.assertTrue(os.access(path, os.X_OK))
        text = path.read_text(encoding="utf-8")
        for item in (
            "public CUDA header inventory", "core runtime header include",
            "BLAS/FFT/RAND/SOLVER/SPARSE/DNN", "Interop", "NVRTC",
            "--bridge-include", "--ccec", "--output",
        ):
            self.assertIn(item, text)

    def test_generated_reserved_wrapper_inventory_keeps_future_unlock_points(self) -> None:
        path = ROOT / "include/bridge/cuda_to_cce_reserved_wrapper_parity.h"
        self.assertTrue(path.is_file())
        text = path.read_text(encoding="utf-8")
        for item in (
            "cudaGraphAddChildGraphNode", "wcudaGraphAddChildGraphNode",
            "cuArray3DCreate", "wcuArray3DCreate",
            "Status: RESERVED_NPU", "Future implementation:",
        ):
            self.assertIn(item, text)
        typed_driver = (ROOT / "include/bridge/runtime/cuda_driver_wrapper.h").read_text(encoding="utf-8")
        self.assertIn("cuInit", typed_driver)
        self.assertIn("wcuInit", typed_driver)
        self.assertIn("cublasCreate", (ROOT / "include/bridge/blas/cublas_v2_wrapper.h").read_text(encoding="utf-8"))
        self.assertIn("wcublasCreate", (ROOT / "include/bridge/blas/cublas_v2_wrapper.h").read_text(encoding="utf-8"))
        self.assertIn("#if 0", text)


if __name__ == "__main__":
    unittest.main()
