"""Batch 50-5: ruff 静态检查包装器（对齐 test.py 的模式）。

``lint-embeddebug-py`` console script 入口。运行 ruff 检查 ``python/embeddebug/``
+ ``tools/`` + ``tests/python/``，聚焦死代码检测（F 系列：未使用 import/变量）。

退出码：0 = 无错误；非 0 = 有 lint 错误（对齐 ruff 退出码语义）。

用法::

    uv run lint-embeddebug-py              # 检查全部
    uv run lint-embeddebug-py --fix        # ruff 自动修复（F401 删未用 import 等）
"""

from __future__ import annotations

import sys
from collections.abc import Sequence


def main(argv: Sequence[str] | None = None) -> int:
    """运行 ruff lint，返回退出码。"""

    try:
        from ruff.__main__ import find_ruff_bin
        import subprocess
    except ModuleNotFoundError:
        print(
            "ruff is required. Run through uv so the dev dependency group is active.",
            file=sys.stderr,
        )
        return 2

    args = list(argv) if argv is not None else []
    # 默认检查范围：产品代码 + 工具 + 测试。
    if not any(a.endswith(".py") or a in ("check", "format") for a in args):
        args = ["check", "python/embeddebug", "tools", "tests/python", *args]
    elif args and args[0] not in ("check", "format", "version", "linter", "rule", "clean", "server"):
        args = ["check", *args]

    ruff_bin = find_ruff_bin()
    return subprocess.call([str(ruff_bin), *args])


if __name__ == "__main__":
    sys.exit(main())
