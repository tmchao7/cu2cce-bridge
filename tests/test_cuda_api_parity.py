#!/usr/bin/env python3
"""Contracts for the complete CUDA Runtime/device parity inventory."""
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
CUBRIDGE_REFERENCE = Path(
    os.environ.get("CU2CCE_CUBRIDGE_REFERENCE", str(ROOT.parent / "cu-bridge-muxi"))
).expanduser()
sys.path.insert(0, str(TOOLS))


class CudaApiParityToolTest(unittest.TestCase):
    def _write(self, path: Path, text: str) -> None:
        path.write_text(text, encoding="utf-8")

    def test_validator_reports_active_reserved_and_missing_entries(self) -> None:
        import validate_cuda_api_parity as parity

        with tempfile.TemporaryDirectory() as tmp:
            tmpdir = Path(tmp)
            reference = tmpdir / "cuda_to_maca_mcr_adaptor.h"
            adaptor = tmpdir / "cuda_to_cce_mcr_adaptor.h"
            inventory = tmpdir / "cuda_to_cce_reserved_parity.h"
            self._write(reference, "\n".join([
                "#define cudaMalloc mcMalloc",
                "#define cudaGraphCreate mcGraphCreate",
                "#define make_cudaExtent make_mcudaExtent",
                "#define ignored mcIgnored",
            ]))
            self._write(adaptor, "#define cudaMalloc wcudaMalloc\n")
            self._write(inventory, "#if 0\n#define cudaGraphCreate CU2CCE_RESERVED_NPU_cudaGraphCreate\n#endif\n")
            result = parity.audit(reference, adaptor, inventory)
            self.assertEqual(result["reference_entries"], 3)
            self.assertEqual(result["active_entries"], 1)
            self.assertEqual(result["reserved_entries"], 1)
            self.assertEqual(result["missing_entries"], ["make_cudaExtent"])
            self.assertEqual(result["active_maca_backend_names"], [])

    def test_validator_reports_active_wrapper_missing_from_runtime_header(self) -> None:
        import validate_cuda_api_parity as parity

        with tempfile.TemporaryDirectory() as tmp:
            tmpdir = Path(tmp)
            reference = tmpdir / "reference.h"
            adaptor = tmpdir / "adaptor.h"
            inventory = tmpdir / "inventory.h"
            runtime_header = tmpdir / "cuda_runtime_wrapper.h"
            status = tmpdir / "status.json"
            self._write(reference, "#define cudaFoo mcFoo\n")
            self._write(adaptor, "#define cudaFoo wcudaFoo\n")
            self._write(inventory, "#if 0\n#endif\n")
            self._write(runtime_header, "/* no wcudaFoo declaration */\n")
            status.write_text(json.dumps({
                "schema_version": 1,
                "entries": {
                    "cudaFoo": {
                        "status": "ACTIVE_ADAPTED",
                        "reference_maca": "mcFoo",
                        "cu2cce_target": "wcudaFoo",
                        "category": "Other_Runtime_Device",
                    }
                },
            }), encoding="utf-8")
            result = parity.audit(reference, adaptor, inventory, status, runtime_header)
            self.assertEqual(result["runtime_header_missing_entries"], ["cudaFoo"])

    def test_validator_accepts_runtime_header_contract(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmpdir = Path(tmp)
            reference = tmpdir / "reference.h"
            adaptor = tmpdir / "adaptor.h"
            inventory = tmpdir / "inventory.h"
            runtime_header = tmpdir / "cuda_runtime_wrapper.h"
            status = tmpdir / "status.json"
            self._write(reference, "#define cudaMalloc mcMalloc\n")
            self._write(adaptor, "#define cudaMalloc wcudaMalloc\n")
            self._write(inventory, "#if 0\n#endif\n")
            self._write(runtime_header, "aclError wcudaMalloc(void **ptr, size_t size);\n")
            status.write_text(json.dumps({
                "schema_version": 1,
                "entries": {
                    "cudaMalloc": {
                        "status": "ACTIVE_ADAPTED",
                        "reference_maca": "mcMalloc",
                        "cu2cce_target": "wcudaMalloc",
                        "category": "Other_Runtime_Device",
                    }
                },
            }), encoding="utf-8")
            result = subprocess.run(
                [sys.executable, str(TOOLS / "validate_cuda_api_parity.py"),
                 "--reference", str(reference),
                 "--implementation", str(adaptor),
                 "--inventory", str(inventory),
                 "--runtime-header", str(runtime_header),
                 "--status-inventory", str(status),
                 "--format", "json"],
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            report = json.loads(result.stdout)
            self.assertEqual(report["runtime_header_missing_entries"], [])

    def test_validator_uses_captured_status_inventory_without_reference_checkout(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmpdir = Path(tmp)
            adaptor = tmpdir / "adaptor.h"
            inventory = tmpdir / "inventory.h"
            runtime_header = tmpdir / "cuda_runtime_wrapper.h"
            status = tmpdir / "status.json"
            self._write(adaptor, "#define cudaMalloc wcudaMalloc\n")
            self._write(inventory, "#if 0\n#endif\n")
            self._write(runtime_header, "aclError wcudaMalloc(void **ptr, size_t size);\n")
            status.write_text(json.dumps({
                "schema_version": 1,
                "entries": {
                    "cudaMalloc": {
                        "status": "ACTIVE_ADAPTED",
                        "reference_maca": "mcMalloc",
                        "cu2cce_target": "wcudaMalloc",
                        "category": "Other_Runtime_Device",
                    }
                },
            }), encoding="utf-8")
            result = subprocess.run(
                [sys.executable, str(TOOLS / "validate_cuda_api_parity.py"),
                 "--implementation", str(adaptor),
                 "--inventory", str(inventory),
                 "--runtime-header", str(runtime_header),
                 "--status-inventory", str(status),
                 "--format", "json"],
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(json.loads(result.stdout)["reference_entries"], 1)

    def test_validator_flags_active_maca_rhs(self) -> None:
        import validate_cuda_api_parity as parity

        with tempfile.TemporaryDirectory() as tmp:
            tmpdir = Path(tmp)
            reference = tmpdir / "reference.h"
            adaptor = tmpdir / "adaptor.h"
            inventory = tmpdir / "inventory.h"
            self._write(reference, "#define cudaMalloc mcMalloc\n")
            self._write(adaptor, "#define cudaMalloc mcMalloc\n")
            self._write(inventory, "#if 0\n#endif\n")
            result = parity.audit(reference, adaptor, inventory)
            self.assertEqual(result["active_maca_backend_names"], ["cudaMalloc"])

    def test_generator_creates_reserved_entries_for_every_missing_reference_name(self) -> None:
        import generate_cuda_api_parity_inventory as generator
        import validate_cuda_api_parity as parity

        with tempfile.TemporaryDirectory() as tmp:
            tmpdir = Path(tmp)
            reference = tmpdir / "reference.h"
            adaptor = tmpdir / "adaptor.h"
            inventory = tmpdir / "inventory.h"
            self._write(reference, "\n".join([
                "#define cudaMalloc mcMalloc",
                "#define cudaGraphCreate mcGraphCreate",
                "#define cudaStreamCreate mcStreamCreate",
            ]))
            self._write(adaptor, "#define cudaMalloc wcudaMalloc\n")
            generator.generate(reference, adaptor, inventory, "reference-test")
            text = inventory.read_text(encoding="utf-8")
            self.assertIn("RESERVED_NPU", text)
            self.assertIn("cudaGraphCreate", text)
            self.assertIn("cudaStreamCreate", text)
            self.assertNotIn("#define cudaMalloc CU2CCE_RESERVED", text)
            result = parity.audit(reference, adaptor, inventory)
            self.assertEqual(result["missing_entries"], [])
            self.assertEqual(result["duplicate_entries"], [])


class CudaRuntimeTypedPlaceholderContractTest(unittest.TestCase):
    def test_unsupported_occupancy_extensions_are_typed_and_explicit(self) -> None:
        header = (ROOT / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        source = (ROOT / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        names = (
            "wcudaOccupancyAvailableDynamicSMemPerBlock",
            "wcudaOccupancyMaxActiveBlocksPerMultiprocessorWithFlags",
            "wcudaOccupancyMaxPotentialBlockSizeWithFlags",
        )
        for name in names:
            self.assertIn(f"{name}(", header)
            start = source.find(f"{name}(")
            self.assertNotEqual(start, -1, name)
            body = source[start:source.find("\n}", start) + 2]
            self.assertIn("ACL_ERROR_API_NOT_SUPPORT", body, name)


class CudaRuntimeReservedFamilyPromotionTest(unittest.TestCase):
    def test_high_frequency_reserved_families_use_typed_unsupported_wrappers(self) -> None:
        adaptor = (ROOT / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        types = (ROOT / "include/bridge/runtime/cuda_runtime_types.h").read_text(encoding="utf-8")
        header = (ROOT / "include/bridge/runtime/cuda_runtime_wrapper.h").read_text(encoding="utf-8")
        source = (ROOT / "src/bridge/runtime/src/cuda_runtime_wrapper.cpp").read_text(encoding="utf-8")
        expectations = {
            "cudaIpcGetMemHandle": "wcudaIpcGetMemHandle",
            "cudaMallocFromPoolAsync": "wcudaMallocFromPoolAsync",
            "cudaMemPoolTrimTo": "wcudaMemPoolTrimTo",
            "cudaGetMipmappedArrayLevel": "wcudaGetMipmappedArrayLevel",
            "cudaGetTextureObjectResourceDesc": "wcudaGetTextureObjectResourceDesc",
            "cudaGetSurfaceObjectResourceDesc": "wcudaGetSurfaceObjectResourceDesc",
            "cudaGraphGetNodes": "wcudaGraphGetNodes",
            "cudaGraphGetRootNodes": "wcudaGraphGetRootNodes",
            "cudaGraphClone": "wcudaGraphClone",
            "cudaStreamGetAttribute": "wcudaStreamGetAttribute",
        }
        for cuda_name, wrapper_name in expectations.items():
            self.assertIn(f"#define {cuda_name} {wrapper_name}", adaptor)
            self.assertIn(f"{wrapper_name}(", header)
            start = source.find(f"{wrapper_name}(")
            self.assertNotEqual(start, -1, wrapper_name)
            body = source[start:source.find("\n}", start) + 2]
            self.assertIn("ACL_ERROR_API_NOT_SUPPORT", body, wrapper_name)
        for type_name in (
            "wcceIpcMemHandle_t", "wcceIpcEventHandle_t", "wcceGraph_t",
            "wcceGraphNode_t", "wcceStreamAttrValue", "wcceMemPoolAttr",
        ):
            self.assertIn(type_name, types)


class CudaApiParityStatusClassifierTest(unittest.TestCase):
    def test_forwarded_typed_stub_keeps_unsupported_status(self) -> None:
        import generate_cuda_api_parity_inventory as generator

        source = """
aclError wcudaBase() { return ACL_ERROR_API_NOT_SUPPORT; }
aclError wcudaForward() { return wcudaBase(); }
"""
        self.assertEqual(
            generator._active_status("cudaForward", "wcudaForward", source),
            "UNSUPPORTED_TYPED",
        )


class CudaApiParityStatusInventoryTest(unittest.TestCase):
    def test_runtime_generator_preserves_header_wrapper_inventory_sections(self) -> None:
        import generate_cuda_api_parity_inventory as generator

        with tempfile.TemporaryDirectory() as tmp:
            tmpdir = Path(tmp)
            reference = tmpdir / "reference.h"
            adaptor = tmpdir / "adaptor.h"
            reserved = tmpdir / "reserved.h"
            status = tmpdir / "status.json"
            reference.write_text("#define cudaMalloc mcMalloc\n", encoding="utf-8")
            adaptor.write_text("#define cudaMalloc wcudaMalloc\n", encoding="utf-8")
            status.write_text(json.dumps({"headers": {"cuda.h": {"status": "ACTIVE"}}}), encoding="utf-8")
            generator.generate(reference, adaptor, reserved, "reference-test", status)
            data = json.loads(status.read_text(encoding="utf-8"))
            self.assertIn("headers", data)
            self.assertIn("cuda.h", data["headers"])

    def test_generated_status_inventory_records_every_reference_entry(self) -> None:
        import generate_cuda_api_parity_inventory as generator

        with tempfile.TemporaryDirectory() as tmp:
            tmpdir = Path(tmp)
            reference = tmpdir / "reference.h"
            adaptor = tmpdir / "adaptor.h"
            reserved = tmpdir / "reserved.h"
            status = tmpdir / "status.json"
            reference.write_text("#define cudaMalloc mcMalloc\n#define cudaGraphCreate mcGraphCreate\n", encoding="utf-8")
            adaptor.write_text("#define cudaMalloc wcudaMalloc\n", encoding="utf-8")
            generator.generate(reference, adaptor, reserved, "reference-test", status)
            data = json.loads(status.read_text(encoding="utf-8"))
            self.assertEqual(set(data["entries"]), {"cudaMalloc", "cudaGraphCreate"})
            self.assertEqual(data["entries"]["cudaMalloc"]["status"], "ACTIVE_ADAPTED")
            self.assertEqual(data["entries"]["cudaGraphCreate"]["status"], "RESERVED_NPU")


class CudaApiParityRepositoryContractTest(unittest.TestCase):
    def test_server_regression_validates_committed_parity_snapshot(self) -> None:
        server = (ROOT / "tools/real_test/server_cmd.txt").read_text(encoding="utf-8")
        self.assertIn("validate_cuda_api_parity.py", server)
        self.assertIn("cuda_api_parity.log", server)
        self.assertIn("CUDA API parity inventory", server)

    @unittest.skipUnless(
        CUBRIDGE_REFERENCE.is_dir(),
        "cu-bridge reference checkout not available; set CU2CCE_CUBRIDGE_REFERENCE",
    )
    def test_repository_inventory_covers_reference_runtime_entries(self) -> None:
        result = subprocess.run(
            [sys.executable, str(TOOLS / "validate_cuda_api_parity.py"),
             "--reference", str(CUBRIDGE_REFERENCE),
             "--implementation", str(ROOT / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h"),
             "--inventory", str(ROOT / "include/bridge/runtime/cuda_to_cce_reserved_parity.h"),
             "--format", "json"],
            capture_output=True, text=True, check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        implementation = (ROOT / "include/bridge/runtime/cuda_to_cce_mcr_adaptor.h").read_text(encoding="utf-8")
        self.assertIn('"cuda_to_cce_reserved_parity.h"', implementation)
        report = json.loads(result.stdout)
        self.assertEqual(report["missing_entries"], [])
        self.assertEqual(report["duplicate_entries"], [])
        self.assertEqual(report["active_maca_backend_names"], [])
        self.assertGreater(report["reserved_entries"], 0)


if __name__ == "__main__":
    unittest.main()
