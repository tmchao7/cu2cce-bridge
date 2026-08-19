#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
kernel_convert.py —— CUDA kernel → 昇腾 SIMT VF 自动转换（v0）

cu2cce-bridge 的核心组件（对应 cu-bridge 的 tools_src/ptx2cpp 职责，方向相反）：
让用户**保持纯 CUDA kernel 源码零改写**，cucc 编译 .cu 前自动调用本工具，
把 __global__ kernel 转换为昇腾 ccec 可编译的 VF 模板：

    用户源码（不变）                        转换产物（喂给 ccec）
    ──────────────────────────            ──────────────────────────────
    __global__ void k(A, B, n) { body } → __global__ void k(A, B, n) {
                                              cce::async_invoke<k_VF>(
                                                  cce::dim3{256,1,1}, a, b, n);
                                          }
                                          __simt_vf__ [aicore] inline
                                          void k_VF(__gm__ A, __gm__ B, n) { body }

转换依据：真机验证的 ccec SIMT VF 模板（2026-08-19，cann-9.2.0/dav-c310）——
kernel 入口 __global__[aicore]（由 simt_adaptor.h 的宏表统一加限定符）、
计算体放 __simt_vf__（warp 原语只能在此上下文）、指针加 __gm__ 地址空间。

v0 限制（后续迭代）：
  - 只处理简单 __global__ void k(...) { ... }（无嵌套括号参数、无函数指针、
    无模板 kernel）
  - VF 线程数固定 cce::dim3{256,1,1}（TODO：从 <<<>>> 启动处提取 blockDim）
  - 指针参数自动加 __gm__（const __gm__ 位置按真机反馈调整）
  - 无 __global__ 的输入原样输出（host-only 文件不受影响）

用法：
  python3 tools/kernel_convert.py in.cu -o out.cce
"""

import argparse
import re
import sys

VF_THREADS = "cce::dim3{256, 1, 1}"


def find_kernels(src):
    """定位 __global__ void name(params) { body }，返回 (name, params, body, start, end)。"""
    kernels = []
    # 参数内不允许出现括号（v0 限制）
    pat = re.compile(r"__global__\s+void\s+(\w+)\s*\(([^()]*)\)\s*\{", re.S)
    for m in pat.finditer(src):
        name, params = m.group(1), m.group(2).strip()
        i = m.end()
        depth = 1
        while i < len(src) and depth > 0:
            if src[i] == "{":
                depth += 1
            elif src[i] == "}":
                depth -= 1
            i += 1
        if depth != 0:
            continue  # 括号不闭合，跳过（源码有问题）
        body = src[m.end():i - 1]
        kernels.append((name, params, body, m.start(), i))
    return kernels


def arg_names(params):
    """从参数列表提取形参名：'float *c, const float *a, int n' -> ['c', 'a', 'n']。"""
    names = []
    for p in params.split(","):
        p = p.strip()
        if not p:
            continue
        m = re.search(r"(\w+)\s*$", p)
        if m:
            names.append(m.group(1))
    return names


def add_gm(params):
    """指针参数加 __gm__ 地址空间（昇腾要求，真机验证）。
    'float *c' -> '__gm__ float *c'；'const float *a' -> 'const __gm__ float *a'"""
    def repl(m):
        const = m.group(1) or ""
        return const + "__gm__ " + m.group(2)
    return re.sub(r"(\bconst\s+)?((?:unsigned\s+|signed\s+)?(?:long\s+|short\s+)?(?:char|short|int|long|float|double|half|bfloat16_t|void|\w+)\s*(\*+))\s*(\w+)",
                  lambda m: (m.group(1) or "") + "__gm__ " + m.group(2) + " " + m.group(4), params)


def convert_launch_config(src):
    """CUDA <<<grid, block, sharedMem, stream>>> -> 昇腾 <<<blocks, threads, stream>>>
    （昇腾 SIMT 语法糖只支持 3 参，真机实测报 'too many arguments, expected 7,
    have 8'；sharedMem 由昇腾 UB 动态内存机制替代，=0 场景直接丢弃）"""
    pat = re.compile(r"<<<\s*([^,>]+?)\s*,\s*([^,>]+?)\s*,\s*([^,>]+?)\s*,\s*([^>]+?)\s*>>>")

    def repl(m):
        shared = m.group(3).strip()
        if shared not in ("0", "0u", "0U", "0UL", "0L", "0LL"):
            print(f"kernel_convert: WARN 启动配置 sharedMem={shared} 非 0，昇腾语义不同，已丢弃（TODO）",
                  file=sys.stderr)
        return f"<<<{m.group(1).strip()}, {m.group(2).strip()}, {m.group(4).strip()}>>>"

    return pat.sub(repl, src)


def convert(src):
    """返回 (转换后源码, 转换的 kernel 数)。无 kernel 时原样返回。"""
    src = convert_launch_config(src)
    kernels = find_kernels(src)
    if not kernels:
        return src, 0
    out = []
    pos = 0
    for name, params, body, start, end in kernels:
        out.append(src[pos:start])
        vf_name = name + "_VF"
        gm_params = add_gm(params)
        args = ", ".join(arg_names(params))
        # 注意顺序：VF 定义必须在 kernel 之前（kernel 里 async_invoke<VF> 引用它）；
        # __global__[aicore] 显式写出（ccec 内置 __global__ 宏，重定义无效——
        # simt_adaptor.h 不再提供 __global__ 宏，真机实测）；
        # kernel 包装参数与 VF 一样带 __gm__（async_invoke 参数类型需匹配，
        # 真机实测报 'expecting parameter to have memory qualifier __gm__'）
        out.append(
            f"""/* kernel 自动转换（tools/kernel_convert.py v0）：CUDA kernel {name} 零改写，
 * 此处生成昇腾 SIMT VF 等价实现（模板经真机 2026-08-19 验证） */
__simt_vf__ [aicore] __attribute__((always_inline)) inline
void {vf_name}({gm_params}) {{
{body}
}}

__global__[aicore] void {name}({gm_params}) {{
    cce::async_invoke<{vf_name}>({VF_THREADS}, {args});
}}
"""
        )
        pos = end
    out.append(src[pos:])
    return "".join(out), len(kernels)


def main():
    ap = argparse.ArgumentParser(description="CUDA kernel -> Ascend SIMT VF converter (v0)")
    ap.add_argument("input", help="输入 .cu 文件")
    ap.add_argument("-o", "--output", required=True, help="输出文件（转换后喂给 ccec）")
    args = ap.parse_args()

    try:
        with open(args.input, "r", encoding="utf-8", errors="replace") as f:
            src = f.read()
    except OSError as e:
        print(f"kernel_convert: cannot read {args.input}: {e}", file=sys.stderr)
        return 1

    converted, n = convert(src)
    try:
        with open(args.output, "w", encoding="utf-8") as f:
            f.write(converted)
    except OSError as e:
        print(f"kernel_convert: cannot write {args.output}: {e}", file=sys.stderr)
        return 1

    if n:
        print(f"kernel_convert: converted {n} kernel(s) in {args.input} -> {args.output}")
    else:
        print(f"kernel_convert: no __global__ kernel found, passthrough {args.input} -> {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
