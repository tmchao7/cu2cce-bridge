import importlib.util
import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
EXAMPLE_DIR = ROOT / "examples" / "experimental" / "hiddenBlockDim"
PROBE_1D = EXAMPLE_DIR / "hidden_blockdim_1d_probe.cu"
PROBE_2D = EXAMPLE_DIR / "hidden_blockdim_2d_probe.cu"
CONVERTER = ROOT / "tools" / "kernel_convert.py"
SPEC = importlib.util.spec_from_file_location("kernel_convert", CONVERTER)
kernel_convert = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(kernel_convert)


class HiddenBlockDimProbeTests(unittest.TestCase):
    def test_1d_probe_uses_one_wrapper_with_256_and_512_hidden_dimensions(self):
        source = PROBE_1D.read_text()

        self.assertIn("unsigned bx, unsigned by, unsigned bz", source)
        self.assertIn("cce::dim3{bx, by, bz}", source)
        self.assertIn("hiddenAdd<<<1, threads256, 0, stream>>>", source)
        self.assertIn("hiddenAdd<<<1, threads512, 0, stream>>>", source)
        self.assertNotIn("cce::dim3{blockDim.x, blockDim.y, blockDim.z}", source)

    def test_1d_probe_passes_through_launch_converter(self):
        converted, count = kernel_convert.convert(PROBE_1D.read_text())

        self.assertEqual(count, 0)
        self.assertIn("hiddenAdd<<<1, threads256, stream>>>", converted)
        self.assertIn("hiddenAdd<<<1, threads512, stream>>>", converted)
        self.assertIn("cce::dim3{bx, by, bz}", converted)

    def test_2d_probe_uses_scalar_outer_launch_and_hidden_32x8x1_shape(self):
        source = PROBE_2D.read_text()

        self.assertIn("cce::dim3{bx, by, bz}", source)
        self.assertIn("unsigned launch_threads = 256;", source)
        self.assertIn("hidden2D<<<1, launch_threads, 0, stream>>>", source)
        self.assertIn("block2d.x, block2d.y, block2d.z", source)
        self.assertNotIn("hidden2D<<<1, block2d", source)

    def test_2d_probe_passes_through_launch_converter(self):
        converted, count = kernel_convert.convert(PROBE_2D.read_text())

        self.assertEqual(count, 0)
        self.assertIn("hidden2D<<<1, launch_threads, stream>>>", converted)
        self.assertIn("cce::dim3{bx, by, bz}", converted)


if __name__ == "__main__":
    unittest.main()
