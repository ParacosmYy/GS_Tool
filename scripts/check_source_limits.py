"""Enforce the repository's per-source-file maintainability limit."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

DEFAULT_LIMIT = 1_000


def source_files(root: Path) -> tuple[Path, ...]:
    """Return Python source files in deterministic order."""

    return tuple(sorted(path for path in root.rglob("*.py") if path.is_file()))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", nargs="?", type=Path, default=Path("src"))
    parser.add_argument("--limit", type=int, default=DEFAULT_LIMIT)
    args = parser.parse_args()
    if args.limit <= 0:
        parser.error("--limit must be positive")

    root = args.root.resolve()
    if not root.is_dir():
        print(f"source root not found: {root}", file=sys.stderr)
        return 2

    violations = []
    for path in source_files(root):
        line_count = len(path.read_text(encoding="utf-8").splitlines())
        if line_count > args.limit:
            violations.append((path, line_count))

    if violations:
        for path, line_count in violations:
            print(f"{path}: {line_count} lines > {args.limit}", file=sys.stderr)
        return 1

    print(f"source line limit: pass ({len(source_files(root))} files <= {args.limit})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
