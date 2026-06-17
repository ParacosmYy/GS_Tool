"""Verify a Python/PyQt PyInstaller package directory."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path


APP_NAME = "EmbedDebugPy"
DIST_ROOT = "dist"
PACKAGE_SUFFIX = "-windows-x64"

REQUIRED_RELATIVE_PATHS = [
    "EmbedDebugPy.exe",
    "THIRD_PARTY_NOTICES.md",
    "PYTHON_DEPENDENCIES.txt",
    "_internal/PyQt6/Qt6/plugins/platforms/qwindows.dll",
]


def repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


def is_package_dir(path: Path) -> bool:
    return path.is_dir() and path.name.startswith(f"{APP_NAME}-") and path.name.endswith(PACKAGE_SUFFIX)


def latest_package_dir(root: Path) -> Path:
    dist_root = root / DIST_ROOT
    if not dist_root.is_dir():
        raise RuntimeError(f"dist directory not found: {dist_root}")

    candidates = [path for path in dist_root.iterdir() if is_package_dir(path)]
    if not candidates:
        raise RuntimeError(f"no {APP_NAME}-*{PACKAGE_SUFFIX} package directory found under {dist_root}")
    return max(candidates, key=lambda path: path.stat().st_mtime)


def required_paths(package_dir: Path) -> list[Path]:
    return [package_dir / relative_path for relative_path in REQUIRED_RELATIVE_PATHS]


def verify_package_dir(package_dir: Path) -> tuple[bool, list[str]]:
    if not package_dir.is_dir():
        return False, [f"missing: package directory {package_dir}"]

    messages: list[str] = []
    missing: list[Path] = []
    for path in required_paths(package_dir):
        if not path.is_file():
            missing.append(path)
        relative = path.relative_to(package_dir).as_posix()
        prefix = "missing" if path in missing else "ok"
        messages.append(f"{prefix}: {relative}")
    return not missing, messages


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="verify-package-embeddebug-py",
        description=__doc__,
    )
    parser.add_argument(
        "--package-dir",
        type=Path,
        help="package directory to verify; defaults to the latest dist/EmbedDebugPy-*-windows-x64",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    try:
        args = parse_args(argv or sys.argv[1:])
    except SystemExit as exc:
        return int(exc.code)

    try:
        root = repo_root()
        package_dir = args.package_dir if args.package_dir else latest_package_dir(root)
        if not package_dir.is_absolute():
            package_dir = root / package_dir

        ok, messages = verify_package_dir(package_dir)
        print(f"package_dir={package_dir}")
        for message in messages:
            print(message)
        return 0 if ok else 2
    except RuntimeError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
