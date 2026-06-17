"""Compatibility wrapper for the Python/PyQt PyInstaller package command."""

from __future__ import annotations

from embeddebug.devtools.package_pyinstaller import (
    PackageConfig,
    build_config,
    default_temp_root,
    dependency_snapshot_lines,
    main,
    package_dir,
    package_name,
    parse_args,
    pyinstaller_command,
    repo_root,
    run_pyinstaller,
    write_package_notice_files,
)

__all__ = [
    "PackageConfig",
    "build_config",
    "default_temp_root",
    "dependency_snapshot_lines",
    "main",
    "package_dir",
    "package_name",
    "parse_args",
    "pyinstaller_command",
    "repo_root",
    "run_pyinstaller",
    "write_package_notice_files",
]


if __name__ == "__main__":
    raise SystemExit(main())
