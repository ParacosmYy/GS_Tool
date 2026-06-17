#!/usr/bin/env python3
"""
EmbedDebug project audit helper.

Generates a lightweight, read-only summary for iteration planning:
- source/header counts by top-level module
- numbered utility directory families
- Python/PyQt runtime and test file counts
"""

from __future__ import annotations

import argparse
import re
from collections import Counter, defaultdict
from pathlib import Path


SOURCE_EXTENSIONS = {".cpp", ".h", ".hpp", ".ipp"}
PYTHON_EXTENSIONS = {".py"}
NUMBERED_UTIL_RE = re.compile(r"^([A-Za-z_]+)(\d+)$")


def iter_code_files(root: Path):
    src_root = root / "src"
    for path in src_root.rglob("*"):
        if path.is_file() and path.suffix in SOURCE_EXTENSIONS:
            yield path


def module_counts(root: Path) -> Counter[str]:
    counts: Counter[str] = Counter()
    src_root = root / "src"
    for path in iter_code_files(root):
        try:
            module = path.relative_to(src_root).parts[0]
        except (IndexError, ValueError):
            continue
        counts[module] += 1
    return counts


def numbered_utils(root: Path):
    families: dict[str, list[str]] = defaultdict(list)
    utils_root = root / "src" / "utils"
    if not utils_root.exists():
        return families

    for path in utils_root.iterdir():
        if not path.is_dir():
            continue
        match = NUMBERED_UTIL_RE.match(path.name)
        if match:
            families[match.group(1)].append(path.name)

    for names in families.values():
        names.sort(key=lambda value: int(NUMBERED_UTIL_RE.match(value).group(2)))
    return dict(sorted(families.items(), key=lambda item: (-len(item[1]), item[0])))


def python_counts(root: Path) -> Counter[str]:
    counts: Counter[str] = Counter()
    runtime_root = root / "python" / "embeddebug"
    tests_root = root / "tests" / "python"

    if runtime_root.exists():
        counts["runtime"] = sum(
            1 for path in runtime_root.rglob("*") if path.is_file() and path.suffix in PYTHON_EXTENSIONS
        )
    if tests_root.exists():
        counts["tests"] = sum(
            1 for path in tests_root.rglob("*") if path.is_file() and path.suffix in PYTHON_EXTENSIONS
        )
    return counts


def print_module_counts(counts: Counter[str], limit: int) -> None:
    print("## Module File Counts")
    for module, count in counts.most_common(limit):
        print(f"{module}: {count}")
    print()


def print_numbered_utils(families: dict[str, list[str]], limit: int) -> None:
    print("## Numbered utils directory families")
    for family, names in list(families.items())[:limit]:
        first = names[0]
        last = names[-1]
        print(f"{family}: {len(names)} dirs ({first} .. {last})")
    print()


def print_python_counts(counts: Counter[str]) -> None:
    print("## Python/PyQt File Counts")
    print(f"runtime: {counts.get('runtime', 0)}")
    print(f"tests: {counts.get('tests', 0)}")
    print()


def main() -> int:
    parser = argparse.ArgumentParser(description="Audit EmbedDebug project structure.")
    parser.add_argument("--root", type=Path, default=Path.cwd(), help="repository root")
    parser.add_argument("--limit", type=int, default=20, help="max rows per section")
    args = parser.parse_args()

    root = args.root.resolve()
    print(f"# EmbedDebug Project Audit: {root}")
    print()
    print_module_counts(module_counts(root), args.limit)
    print_numbered_utils(numbered_utils(root), args.limit)
    print_python_counts(python_counts(root))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
