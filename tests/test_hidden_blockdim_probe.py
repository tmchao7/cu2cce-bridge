import importlib.util
import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
PROBE = ROOT / "examples" / "experimental" / "hiddenBlockDim" / "hidden_blockdim_probe.cu"
CONVERTER = ROOT / "tools" / "kernel_convert.py"
SPEC = importlib.util.spec_from_file_location("kernel_convert", CONVERTER)
kernel_convert = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(kernel_convert)


class HiddenBlockDimProbeTests(unittest.TestCase):
    def test_probe_covers_dynamic_scalar_and_2d_hidden_arguments(self):
        source = PROBE.read_text()

        self.assertIn("unsigned bx, unsigned by, unsigned bz", source)
        self.assertIn("cce::dim3{bx, by, bz}", source)
        self.assertIn("hiddenAdd<<<1, threads256, 0, stream>>>", source)
        self.assertIn("hiddenAdd<<<1, threads512, 0, stream>>>", source)
        self.assertIn("hidden2D<<<1, block2d, 0, stream>>>", source)
        self.assertNotIn(
            "cce::dim3{blockDim.x, blockDim.y, blockDim.z}",
            source,
        )

    def test_probe_passes_through_kernel_converter_and_launches_become_three_arg(self):
        source = PROBE.read_text()

        converted, count = kernel_convert.convert(source)

        self.assertEqual(count, 0)
        self.assertIn("hiddenAdd<<<1, threads256, stream>>>", converted)
        self.assertIn("hiddenAdd<<<1, threads512, stream>>>", converted)
        self.assertIn("hidden2D<<<1, block2d, stream>>>", converted)
        self.assertIn("cce::dim3{bx, by, bz}", converted)


if __name__ == "__main__":
    unittest.main()
