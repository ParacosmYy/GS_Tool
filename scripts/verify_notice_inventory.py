"""Verify that the release notice inventory covers the resolved lockfile packages."""

from __future__ import annotations

import argparse
import re
import sys
import tomllib
from pathlib import Path


def canonical_name(name: str) -> str:
    """Return the PEP 503-style spelling used for inventory comparisons."""

    return re.sub(r"[-_.]+", "-", name.strip().lower()).strip("-")


def resolved_packages(lock_path: Path) -> dict[str, str]:
    with lock_path.open("rb") as lock_file:
        lock_data = tomllib.load(lock_file)

    packages: dict[str, str] = {}
    for package in lock_data.get("package", []):
        source = package.get("source") or {}
        if isinstance(source, dict) and "editable" in source:
            continue
        name = package.get("name")
        version = package.get("version")
        if not isinstance(name, str) or not isinstance(version, str):
            raise ValueError("uv.lock contains a package without a name or exact version")
        normalized = canonical_name(name)
        previous = packages.get(normalized)
        if previous is not None and previous != version:
            raise ValueError(
                f"uv.lock resolves {name!r} to conflicting versions: {previous} and {version}"
            )
        packages[normalized] = version
    return packages


def notice_rows(notice_path: Path) -> dict[str, str]:
    rows: dict[str, str] = {}
    in_component_table = False
    for line in notice_path.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if not stripped.startswith("|"):
            in_component_table = False
            continue
        cells = [cell.strip() for cell in stripped.strip("|").split("|")]
        if len(cells) < 2:
            in_component_table = False
            continue
        if cells[0].casefold() == "component" and cells[1].casefold() == "resolved version":
            in_component_table = True
            continue
        if not in_component_table:
            continue
        component = cells[0]
        version = cells[1]
        if not component or set(component) <= {"-", ":", " "}:
            continue
        normalized = canonical_name(component)
        if normalized in rows and rows[normalized] != version:
            raise ValueError(
                f"NOTICE.md contains conflicting versions for {component!r}: "
                f"{rows[normalized]} and {version}"
            )
        rows[normalized] = version
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verify uv.lock package/version coverage in a third-party NOTICE inventory."
    )
    parser.add_argument("--lock", type=Path, default=Path("uv.lock"))
    parser.add_argument("--notice", type=Path, default=Path("docs/third_party/NOTICE.md"))
    args = parser.parse_args()

    try:
        packages = resolved_packages(args.lock)
        rows = notice_rows(args.notice)
    except (OSError, tomllib.TOMLDecodeError, ValueError) as error:
        print(f"NOTICE inventory check failed: {error}", file=sys.stderr)
        return 1

    failures: list[str] = []
    for package_name, expected_version in sorted(packages.items()):
        actual_version = rows.get(package_name)
        if actual_version is None:
            failures.append(f"missing row for {package_name} ({expected_version})")
        elif actual_version != expected_version:
            failures.append(
                f"version mismatch for {package_name}: "
                f"lock={expected_version}, notice={actual_version}"
            )

    if failures:
        print("NOTICE inventory check failed:", file=sys.stderr)
        for failure in failures:
            print(f"- {failure}", file=sys.stderr)
        return 1

    print(
        f"NOTICE inventory coverage passed: {len(packages)} lockfile packages "
        "(editable project package excluded)."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
