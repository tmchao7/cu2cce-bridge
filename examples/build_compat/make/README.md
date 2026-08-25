# Makefile build compatibility smoke

This project intentionally uses a conventional Makefile with `nvcc`, multiple
CUDA translation units, dependency files, a host C++ object, and a final link.
Run it through `make_cce` so `nvcc` is intercepted by `cucc`:

```bash
export CUCC_PATH=/path/to/cu2cce-bridge/install
export PATH=$CUCC_PATH/bin:$PATH
make_cce -C examples/build_compat/make -j2
examples/build_compat/make/build/build_compat_app
```
