"""Compatibility wrapper for the Python/PyQt package verifier."""

from __future__ import annotations

from embeddebug.devtools.verify_pyinstaller_package import (
    APP_NAME,
    DIST_ROOT,
    PACKAGE_SUFFIX,
    REQUIRED_RELATIVE_PATHS,
    is_package_dir,
    latest_package_dir,
    main,
    parse_args,
    repo_root,
    required_paths,
    verify_package_dir,
)

__all__ = [
    "APP_NAME",
    "DIST_ROOT",
    "PACKAGE_SUFFIX",
    "REQUIRED_RELATIVE_PATHS",
    "is_package_dir",
    "latest_package_dir",
    "main",
    "parse_args",
    "repo_root",
    "required_paths",
    "verify_package_dir",
]


if __name__ == "__main__":
    raise SystemExit(main())
