#!/usr/bin/env python3
"""把 assets/ 下的资源文件嵌成 generated/assets.h。

用法（在 homework 目录下）：
    python tools/embed.py

生成的 assets.h 里每个资源是一个 raw string 字面量，
分隔符由本脚本自动挑选（保证内容里不出现），所以不会再踩
"内容里恰好出现 )XXX\" 导致字面量提前结束" 这个坑。
"""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASSETS = ROOT / "assets"
OUT = ROOT / "generated" / "assets.h"

# 资源文件 -> (C++ 变量名, 是否去掉末尾那个换行)
#
# 为什么要区分：
#   模板里的占位符各自单独占一行，那一行的换行符会留在替换结果后面；
#   所以被塞进去的内容要先去掉自己末尾的换行，两边才刚好对上，不会多出空行。
#   而 anim.html 整体就是模板本身，文件末尾的换行属于内容，原样保留。
FILES = [
    ("anim.html", "kAnimTemplate", False),
    ("anim.js", "kAnimScript", True),
]


def read_asset(path: Path) -> str:
    """按 UTF-8 读（顺带吃掉 BOM），并把换行统一成 \\n。"""
    text = path.read_text(encoding="utf-8-sig")
    return text.replace("\r\n", "\n").replace("\r", "\n")


def raw_literal(text: str) -> str:
    """生成一个内容绝对安全的 C++ raw string 字面量。

    依次试 ASSET、ASSET0、ASSET1 …… 直到分隔符的闭合序列 ")分隔符\"" 不在内容里出现。
    这样即使哪天 CSS/JS 里写了 )ASSET" 这种字符串，也不会把字面量提前截断。
    """
    delim = "ASSET"
    n = 0
    while f'){delim}"' in text:
        delim = f"ASSET{n}"
        n += 1
    return f'R"{delim}({text}){delim}"'


def main() -> int:
    missing = [name for name, _, _ in FILES if not (ASSETS / name).is_file()]
    if missing:
        for name in missing:
            print(f"找不到资源文件：{ASSETS / name}", file=sys.stderr)
        return 1

    lines = [
        "// 本文件由 tools/embed.py 自动生成，请不要手改。",
        "// 内容来源：" + "、".join(f"assets/{name}" for name, _, _ in FILES),
        "// 改过资源后重新运行：  python tools/embed.py",
        "#pragma once",
        "",
    ]
    for name, var, strip_newline in FILES:
        text = read_asset(ASSETS / name)
        if strip_newline and text.endswith("\n"):
            text = text[:-1]
        lines.append(f"// assets/{name}")
        lines.append(f"inline constexpr char {var}[] = {raw_literal(text)};")
        lines.append("")

    OUT.parent.mkdir(parents=True, exist_ok=True)
    with open(OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))

    print(f"已生成 {OUT.relative_to(ROOT)}  ({OUT.stat().st_size} 字节)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
