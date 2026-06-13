#!/usr/bin/env python3
"""
EmbedDebug project audit helper.

Generates a lightweight, read-only summary for iteration planning:
- source/header counts by top-level module
- numbered utility directory families
- CMake source/header list duplicate and missing-file checks
"""

from __future__ import annotations

import argparse
import re
from collections import Counter, defaultdict
from pathlib import Path


SOURCE_EXTENSIONS = {".cpp", ".h", ".hpp", ".ipp"}
CMAKE_LIST_RE = re.compile(r"^\s*(src/[^\s#]+\.(?:cpp|h|hpp|ipp))\s*(?:#.*)?$")
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


def cmake_audit_files(root: Path) -> list[Path]:
    """返回需要参与源码清单审计的 CMake 文件。"""
    paths = [path for path in root.rglob("CMakeLists.txt") if "build" not in path.parts]
    cmake_dir = root / "cmake"
    if cmake_dir.exists():
        paths.extend(path for path in cmake_dir.rglob("*.cmake") if "build" not in path.parts)
    return sorted(set(paths))


def cmake_entries(root: Path):
    entries: list[str] = []

    for cmake_path in cmake_audit_files(root):
        for line in cmake_path.read_text(encoding="utf-8", errors="replace").splitlines():
            match = CMAKE_LIST_RE.match(line.replace("\\", "/"))
            if match:
                entries.append(match.group(1))
    return entries


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


def print_cmake_audit(root: Path, entries: list[str], limit: int) -> None:
    print("## CMake list audit")
    duplicates = [path for path, count in Counter(entries).items() if count > 1]
    missing = [path for path in entries if not (root / path).exists()]

    print(f"listed code entries: {len(entries)}")
    print(f"duplicate entries: {len(duplicates)}")
    for path in duplicates[:limit]:
        print(f"  duplicate: {path}")

    print(f"missing listed files: {len(missing)}")
    for path in missing[:limit]:
        print(f"  missing: {path}")
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
    print_cmake_audit(root, cmake_entries(root), args.limit)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
