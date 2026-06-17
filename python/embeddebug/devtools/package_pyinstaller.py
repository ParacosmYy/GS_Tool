"""Package the Python/PyQt EmbedDebug lane with PyInstaller."""

from __future__ import annotations

import argparse
import importlib.metadata
import re
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path


APP_NAME = "EmbedDebugPy"
DIST_ROOT = "dist"
PACKAGE_SUFFIX = "-windows-x64"
NOTICE_SOURCE = Path("docs") / "legal" / "PYTHON_THIRD_PARTY_NOTICES.md"
RUNTIME_DEPENDENCY_NAMES = [
    "PyQt6",
    "PyQt6-Qt6",
    "PyQt6-sip",
    "numpy",
    "pyqtgraph",
]


@dataclass(frozen=True)
class PackageConfig:
    """PyInstaller package command configuration."""

    root: Path
    version: str
    temp_root: Path
    clean: bool = False
    windowed: bool = True
    log_level: str = "WARN"


def repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


def package_name(version: str) -> str:
    safe = re.sub(r"[^A-Za-z0-9_.-]+", "-", version).strip("-")
    return f"{APP_NAME}-{safe}{PACKAGE_SUFFIX}"


def default_temp_root() -> Path:
    return Path(tempfile.gettempdir()) / "embeddebug-pyinstaller"


def pyinstaller_command(config: PackageConfig) -> list[str]:
    entry_script = config.root / "python" / "embeddebug" / "app" / "main.py"
    python_path = config.root / "python"
    workpath = config.temp_root / "work"
    distpath = config.temp_root / "dist"
    specpath = config.temp_root / "spec"

    command = [
        sys.executable,
        "-m",
        "PyInstaller",
        "--noconfirm",
        "--onedir",
        "--name",
        APP_NAME,
        "--distpath",
        str(distpath),
        "--workpath",
        str(workpath),
        "--specpath",
        str(specpath),
        "--paths",
        str(python_path),
        "--hidden-import",
        "PyQt6.QtSerialPort",
        "--hidden-import",
        "PyQt6.QtOpenGLWidgets",
        "--hidden-import",
        "numpy",
        "--hidden-import",
        "pyqtgraph",
        "--log-level",
        config.log_level,
    ]
    if config.clean:
        command.append("--clean")
    if config.windowed:
        command.append("--windowed")
    command.append(str(entry_script))
    return command


def package_dir(config: PackageConfig) -> Path:
    return config.root / DIST_ROOT / package_name(config.version)


def dependency_snapshot_lines(names: list[str] | None = None) -> list[str]:
    lines: list[str] = []
    for name in names or RUNTIME_DEPENDENCY_NAMES:
        try:
            version = importlib.metadata.version(name)
        except importlib.metadata.PackageNotFoundError:
            continue
        lines.append(f"{name}=={version}")
    return sorted(lines, key=str.casefold)


def write_package_notice_files(
    root: Path,
    package_dir: Path,
    dependency_lines: list[str] | None = None,
) -> None:
    notice_source = root / NOTICE_SOURCE
    if not notice_source.is_file():
        raise RuntimeError(f"third-party notice source missing: {notice_source}")

    shutil.copy2(notice_source, package_dir / "THIRD_PARTY_NOTICES.md")
    lines = dependency_lines if dependency_lines is not None else dependency_snapshot_lines()
    (package_dir / "PYTHON_DEPENDENCIES.txt").write_text(
        "\n".join(lines) + "\n",
        encoding="utf-8",
    )


def run_pyinstaller(config: PackageConfig) -> Path:
    final_dir = package_dir(config)
    staging_dir = config.temp_root / "dist" / APP_NAME

    if final_dir.exists():
        if not config.clean:
            raise RuntimeError(f"package directory already exists: {final_dir}")
        shutil.rmtree(final_dir)
    if staging_dir.exists():
        shutil.rmtree(staging_dir)

    command = pyinstaller_command(config)
    print("+", " ".join(command))
    completed = subprocess.run(command, cwd=config.root, check=False)
    if completed.returncode != 0:
        raise RuntimeError(f"PyInstaller failed with exit code {completed.returncode}")
    if not staging_dir.is_dir():
        raise RuntimeError(f"PyInstaller output missing: {staging_dir}")

    final_dir.parent.mkdir(parents=True, exist_ok=True)
    shutil.move(str(staging_dir), str(final_dir))
    write_package_notice_files(config.root, final_dir)
    return final_dir


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="package-embeddebug-py",
        description=__doc__,
    )
    parser.add_argument("--version", help="override package version suffix")
    parser.add_argument("--clean", action="store_true", help="remove existing output first")
    parser.add_argument("--dry-run", action="store_true", help="print the PyInstaller command only")
    parser.add_argument("--no-windowed", action="store_true", help="keep a console window attached")
    parser.add_argument("--log-level", default="WARN", help="PyInstaller log level")
    parser.add_argument(
        "--temp-root",
        type=Path,
        default=default_temp_root(),
        help="temporary PyInstaller work/dist/spec root; must not be repository build/",
    )
    return parser.parse_args(argv)


def build_config(args: argparse.Namespace) -> PackageConfig:
    version = args.version or datetime.now().strftime("%Y%m%d-%H%M%S")
    return PackageConfig(
        root=repo_root(),
        version=version,
        temp_root=args.temp_root,
        clean=args.clean,
        windowed=not args.no_windowed,
        log_level=args.log_level,
    )


def main(argv: list[str] | None = None) -> int:
    try:
        args = parse_args(argv or sys.argv[1:])
    except SystemExit as exc:
        return int(exc.code)

    try:
        config = build_config(args)
        if args.dry_run:
            print(f"package_dir={package_dir(config)}")
            print("+", " ".join(pyinstaller_command(config)))
            return 0

        final_dir = run_pyinstaller(config)
        print(f"package_dir={final_dir}")
        return 0
    except RuntimeError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
