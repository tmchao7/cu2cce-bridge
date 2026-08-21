#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
kernel_convert.py —— CUDA kernel → 昇腾 SIMT VF 自动转换（v1）

将简单 CUDA ``__global__`` kernel 拆成：
  1. ``__simt_vf__`` 计算体；
  2. ``__global__[aicore]`` wrapper。

B 方案（2026-08-20 真机验证）：wrapper 不读取内置 ``blockDim``，而由每个
host launch 把 block 形状作为三个隐藏参数传入。这样同一 kernel 可按 256、
512 或 2D/3D block 多次启动，同时避开 Bisheng 在 wrapper 读取 blockDim 时的
前端崩溃。

已知限制：仅处理简单的 ``__global__ void name(...)``；模板 kernel、函数指针
launch、宏生成的 launch、声明中带数组形参、返回非 void 以及复杂参数声明仍
需要后续 AST 版本。v2（2026-08-21，SIMT 原语方向）：块注释内的伪 launch 文本
从「注释整体保留」改为「连同被注释掉的代码一起去掉」，以便像 reduction 这种
文件头块注释包含 ``<<<...>>>`` 字样的示例也能正确转换（不影响已通过的无注释
转换用例）。
"""

import argparse
import re
import sys


HIDDEN_PARAMS = (
    "unsigned __cu2cce_bx, unsigned __cu2cce_by, unsigned __cu2cce_bz"
)
HIDDEN_DIM = "cce::dim3{__cu2cce_bx, __cu2cce_by, __cu2cce_bz}"
ZERO_SHARED = {"0", "0u", "0U", "0UL", "0L", "0LL"}


class ConversionError(ValueError):
    """输入超出当前安全转换范围。"""


def mask_comments_and_strings(src):
    """用空格遮蔽注释/字符串，保留长度与换行，便于按原文位置改写。"""
    out = list(src)
    i = 0
    quote = None
    while i < len(src):
        ch = src[i]
        if quote:
            if ch != "\n":
                out[i] = " "
            if ch == "\\" and i + 1 < len(src):
                if src[i + 1] != "\n":
                    out[i + 1] = " "
                i += 2
                continue
            if ch == quote:
                quote = None
            i += 1
            continue
        if ch in ("'", '"'):
            quote = ch
            out[i] = " "
            i += 1
            continue
        if src.startswith("//", i):
            out[i] = out[i + 1] = " "
            i += 2
            while i < len(src) and src[i] != "\n":
                out[i] = " "
                i += 1
            continue
        if src.startswith("/*", i):
            out[i] = out[i + 1] = " "
            i += 2
            while i < len(src) and not src.startswith("*/", i):
                # v2（2026-08-21，SIMT 原语方向）：连注释内容一起去掉，避免块注释
                # 里的伪 <<<>>>（如 reduction 文件头注释的字面 <<<grid,block,...>>>）
                # 被后续正则命中；换行保留，其余抹成空格。
                if src[i] != "\n":
                    out[i] = " "
                i += 1
            if i < len(src):
                out[i] = out[i + 1] = " "
                i += 2
            continue
        i += 1
    return "".join(out)


def split_top_level(text):
    """按顶层逗号拆分，保留括号/花括号内逗号。"""
    parts = []
    start = 0
    depth = 0
    for i, ch in enumerate(text):
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        elif ch == "," and depth == 0:
            parts.append(text[start:i].strip())
            start = i + 1
    parts.append(text[start:].strip())
    return parts


def find_matching(masked, start, open_char, close_char):
    """返回从 start 的开符号对应的闭符号位置。"""
    if start >= len(masked) or masked[start] != open_char:
        return None
    depth = 1
    i = start + 1
    while i < len(masked):
        if masked[i] == open_char:
            depth += 1
        elif masked[i] == close_char:
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return None


def find_kernels(src):
    """返回简单 CUDA kernel 的 (name, params, body, start, end)。"""
    kernels = []
    masked = mask_comments_and_strings(src)
    pattern = re.compile(r"__global__\s+void\s+(\w+)\s*\(([^()]*)\)\s*\{", re.S)
    for match in pattern.finditer(masked):
        name = match.group(1)
        params = src[match.start(2):match.end(2)].strip()
        body_end = find_matching(masked, match.end() - 1, "{", "}")
        if body_end is None:
            continue
        body = src[match.end():body_end]
        kernels.append((name, params, body, match.start(), body_end + 1))
    return kernels


def arg_names(params):
    """从简单参数列表提取形参名。"""
    names = []
    for param in split_top_level(params):
        if not param:
            continue
        match = re.search(r"(\w+)\s*$", param)
        if match:
            names.append(match.group(1))
    return names


def add_gm(params):
    """为简单指针参数添加 ``__gm__`` 地址空间。"""
    return re.sub(
        r"(\bconst\s+)?((?:unsigned\s+|signed\s+)?(?:long\s+|short\s+)?"
        r"(?:char|short|int|long|float|double|half|bfloat16_t|void|\w+)\s*(\*+))\s*(\w+)",
        lambda match: (match.group(1) or "")
        + "__gm__ "
        + match.group(2)
        + " "
        + match.group(4),
        params,
    )


def scope_at(masked, pos):
    """返回 pos 所在花括号作用域栈。"""
    stack = []
    for i, ch in enumerate(masked[:pos]):
        if ch == "{":
            stack.append(i)
        elif ch == "}" and stack:
            stack.pop()
    return tuple(stack)


def collect_dim3_defs(src):
    """收集可见的简单 dim3 变量声明，用于识别 block 表达式类型。"""
    masked = mask_comments_and_strings(src)
    pattern = re.compile(
        r"\bdim3\s+(?P<name>\w+)\s*(?:\([^;]*\)|\{[^;]*\})\s*;"
    )
    return [
        (match.start(), match.group("name"), scope_at(masked, match.start()))
        for match in pattern.finditer(masked)
    ]


def is_visible_dim3(name, before, scope, definitions):
    """判断简单标识符是否是当前 launch 可见的 dim3 变量。"""
    candidates = []
    for pos, item_name, item_scope in definitions:
        if item_name != name or pos >= before:
            continue
        if len(item_scope) <= len(scope) and scope[: len(item_scope)] == item_scope:
            candidates.append((len(item_scope), pos))
    return bool(candidates)


def block_lowering(block_expr, before, scope, dim3_defs):
    """返回 (CCE 外层标量线程数, 隐藏 bx, by, bz)。"""
    block = block_expr.strip()
    if re.fullmatch(r"[A-Za-z_]\w*", block) and is_visible_dim3(
        block, before, scope, dim3_defs
    ):
        x, y, z = f"{block}.x", f"{block}.y", f"{block}.z"
        return f"({x} * {y} * {z})", x, y, z

    direct_dim3 = re.fullmatch(r"dim3\s*[({](.*)[)}]", block, re.S)
    if direct_dim3:
        parts = split_top_level(direct_dim3.group(1))
        if len(parts) != 3:
            raise ConversionError(
                f"dim3 block expression '{block}' must have 3 components"
            )
        x, y, z = parts
        return f"(({x}) * ({y}) * ({z}))", x, y, z

    # CCE 外层 launch 的 threads 参数是 unsigned 标量；普通 CUDA 标量 block
    # 视作 1D，并把同一运行时表达式继续传给隐藏 bx。
    return block, block, "1", "1"


def find_launches(src):
    """解析 CUDA launch 及紧随其后的实参列表。"""
    masked = mask_comments_and_strings(src)
    starts = re.finditer(r"\b(?P<name>\w+)\s*<<<", masked)
    launches = []
    for match in starts:
        config_start = masked.find("<<<", match.start())
        config_end = masked.find(">>>", config_start + 3)
        if config_end < 0:
            continue
        config_text = src[config_start + 3:config_end]
        config = split_top_level(config_text)
        if len(config) != 4:
            raise ConversionError(
                f"kernel {match.group('name')} launch config must have 4 arguments"
            )
        call_start = config_end + 3
        while call_start < len(masked) and masked[call_start].isspace():
            call_start += 1
        if call_start >= len(masked) or masked[call_start] != "(":
            continue
        call_end = find_matching(masked, call_start, "(", ")")
        if call_end is None:
            raise ConversionError(
                f"kernel {match.group('name')} launch arguments are unbalanced"
            )
        launches.append(
            {
                "name": match.group("name"),
                "start": match.start(),
                "end": call_end + 1,
                "config": config,
                "args": src[call_start + 1:call_end].strip(),
                "scope": scope_at(masked, match.start()),
            }
        )
    return launches


def rewrite_launches(src, converted_kernel_names):
    """四参→三参，并为本次转换的 kernel 附加隐藏 block 形状参数。"""
    launches = find_launches(src)
    if not launches:
        return src
    dim3_defs = collect_dim3_defs(src)
    out = []
    pos = 0
    for launch in launches:
        out.append(src[pos:launch["start"]])
        grid, block, shared, stream = launch["config"]
        if shared.strip() not in ZERO_SHARED:
            print(
                f"kernel_convert: WARN 启动配置 sharedMem={shared.strip()} 非 0，"
                "昇腾语义不同，已丢弃（TODO）",
                file=sys.stderr,
            )

        args = launch["args"]
        outer_block = block.strip()
        if launch["name"] in converted_kernel_names:
            outer_block, bx, by, bz = block_lowering(
                block, launch["start"], launch["scope"], dim3_defs
            )
            hidden = f"{bx}, {by}, {bz}"
            args = f"{args}, {hidden}" if args else hidden

        out.append(
            f"{launch['name']}<<<{grid.strip()}, {outer_block}, {stream.strip()}>>>"
            f"({args})"
        )
        pos = launch["end"]
    out.append(src[pos:])
    return "".join(out)


def append_params(params, extra):
    return f"{params}, {extra}" if params.strip() else extra


def append_args(args, extra):
    return f"{args}, {extra}" if args.strip() else extra


def convert(src):
    """返回 (转换后源码, 转换 kernel 数)。"""
    kernels = find_kernels(src)
    if not kernels:
        return rewrite_launches(src, set()), 0

    out = []
    pos = 0
    names = set()
    for name, params, body, start, end in kernels:
        names.add(name)
        out.append(src[pos:start])
        vf_name = name + "_VF"
        gm_params = add_gm(params)
        wrapper_params = append_params(gm_params, HIDDEN_PARAMS)
        args = ", ".join(arg_names(params))
        invoke_tail = f", {args}" if args else ""
        out.append(
            f"""/* kernel 自动转换（tools/kernel_convert.py v1）：CUDA kernel {name} 零改写，
 * host launch 通过隐藏参数传递 block 形状（真机 2026-08-20 验证）。 */
__simt_vf__ [aicore] __attribute__((always_inline)) inline
void {vf_name}({gm_params}) {{
{body}
}}

__global__[aicore] void {name}({wrapper_params}) {{
    cce::async_invoke<{vf_name}>({HIDDEN_DIM}{invoke_tail});
}}
"""
        )
        pos = end
    out.append(src[pos:])
    return rewrite_launches("".join(out), names), len(kernels)


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="CUDA kernel -> Ascend SIMT VF converter (v1)"
    )
    parser.add_argument("input", help="输入 .cu 文件")
    parser.add_argument("-o", "--output", required=True, help="转换后输出文件")
    args = parser.parse_args(argv)
    try:
        with open(args.input, "r", encoding="utf-8", errors="replace") as handle:
            source = handle.read()
        converted, count = convert(source)
        with open(args.output, "w", encoding="utf-8") as handle:
            handle.write(converted)
    except (OSError, ConversionError) as error:
        print(f"kernel_convert: {error}", file=sys.stderr)
        return 1

    if count:
        print(
            f"kernel_convert: converted {count} kernel(s) in {args.input} -> {args.output}"
        )
    else:
        print(
            f"kernel_convert: no __global__ kernel found, passthrough "
            f"{args.input} -> {args.output}"
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())
