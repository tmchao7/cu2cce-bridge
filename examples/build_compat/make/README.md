# Makefile build compatibility smoke

This project intentionally uses a conventional Makefile with `nvcc`, multiple
CUDA translation units, dependency files, a host C++ object, a static archive,
and a shared CUDA library with position-independent code. Each linked library contains one `.cu` translation
unit because the current ccec puresimt backend emits one eager registration
helper per CUDA translation unit; a single static archive containing multiple
`.cu` files is currently diagnosed as an unsupported backend shape.
Run it through `make_cce` so `nvcc` is intercepted by `cucc`:

```bash
export CUCC_PATH=/path/to/cu2cce-bridge/install
export PATH=$CUCC_PATH/bin:$PATH
make_cce -C examples/build_compat/make -j2
examples/build_compat/make/build/build_compat_app
```
