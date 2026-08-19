# examples/reference —— 昇腾原生参考示例（用户实测）

| 文件 | 说明 |
|---|---|
| `vector.cce` | 昇腾原生 SIMT VF 写法（用户实测，ccec 编译通过）。**转换目标形态**：cu2cce 把用户纯 CUDA kernel（`examples/vectorAdd/vectorAdd.cu`、`examples/minimal/minimal.cu`）自动转换成这种结构（`__simt_vf__` 计算体 + `__global__[aicore]` 入口 + `cce::async_invoke` 启动 + `__gm__` 指针） |

## 与 cu2cce 示例的关系

```
用户写（纯 CUDA，零改写）          昇腾原生形态（转换目标）
examples/vectorAdd/vectorAdd.cu  →  examples/reference/vector.cce 的结构
examples/minimal/minimal.cu          （由 kernel_convert.py 自动生成，
                                      用户无需手写）
```

编译验证（真机）：

```bash
/usr/local/Ascend/latest/bin/ccec -std=gnu++17 -xcce --cce-aicore-arch=dav-c310 \
  examples/reference/vector.cce -o /tmp/vector.o
```
