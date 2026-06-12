"""Build and package the EmbedDebug Qt application for Windows."""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile


APP_NAME = "EmbedDebug"
DIST_ROOT = "dist"
BUILD_DIR = "build"


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def read_local_env(root: Path) -> dict[str, str]:
    env_file = root / "local_env.bat"
    result: dict[str, str] = {}
    if not env_file.exists():
        return result

    pattern = re.compile(r'^\s*set\s+"([^=]+)=(.*)"\s*$')
    for line in env_file.read_text(encoding="utf-8", errors="ignore").splitlines():
        match = pattern.match(line)
        if match:
            result[match.group(1)] = match.group(2)
    return result


def command_path(name: str) -> str | None:
    return shutil.which(name)


def require_file(path: Path, label: str) -> Path:
    if path.is_file():
        return path
    raise RuntimeError(f"{label} not found: {path}")


def require_dir(path: Path, label: str) -> Path:
    if path.is_dir():
        return path
    raise RuntimeError(f"{label} not found: {path}")


def resolve_toolchain(root: Path) -> dict[str, Path]:
    env = read_local_env(root)

    cmake = Path(env["CMAKE_BIN"]) if env.get("CMAKE_BIN") else None
    if not cmake or not cmake.is_file():
        cmake_found = command_path("cmake.exe") or command_path("cmake")
        cmake = Path(cmake_found) if cmake_found else None

    ninja = Path(env["NINJA_BIN"]) if env.get("NINJA_BIN") else None
    if not ninja or not ninja.is_file():
        ninja_found = command_path("ninja.exe") or command_path("ninja")
        ninja = Path(ninja_found) if ninja_found else None

    qt_prefix = Path(env["QT_PREFIX"]) if env.get("QT_PREFIX") else None
    if not qt_prefix or not (qt_prefix / "bin" / "windeployqt.exe").is_file():
        for candidate in [Path("C:/msys64/mingw64")]:
            if (candidate / "bin" / "windeployqt.exe").is_file():
                qt_prefix = candidate
                break

    mingw_bin = Path(env["MINGW_BIN"]) if env.get("MINGW_BIN") else None
    if not mingw_bin or not (mingw_bin / "g++.exe").is_file():
        gxx = command_path("g++.exe") or command_path("g++")
        mingw_bin = Path(gxx).parent if gxx else None
    if not mingw_bin or not (mingw_bin / "g++.exe").is_file():
        candidate = Path("C:/msys64/mingw64/bin")
        if (candidate / "g++.exe").is_file():
            mingw_bin = candidate

    return {
        "cmake": require_file(cmake or Path(), "CMake"),
        "ninja": require_file(ninja or Path(), "Ninja"),
        "qt_prefix": require_dir(qt_prefix or Path(), "Qt prefix"),
        "mingw_bin": require_dir(mingw_bin or Path(), "MinGW bin"),
    }


def run(args: list[str], root: Path, env: dict[str, str]) -> None:
    print("+", " ".join(args))
    completed = subprocess.run(args, cwd=root, env=env, check=False)
    if completed.returncode != 0:
        raise RuntimeError(f"command failed with exit code {completed.returncode}: {' '.join(args)}")


def build_app(root: Path, tools: dict[str, Path]) -> None:
    build_dir = root / BUILD_DIR
    build_dir.mkdir(exist_ok=True)

    env = os.environ.copy()
    env["PATH"] = (
        f"{tools['qt_prefix'] / 'bin'};"
        f"{tools['qt_prefix'] / 'share' / 'qt6' / 'bin'};"
        f"{tools['mingw_bin']};"
        f"{env.get('PATH', '')}"
    )

    run(
        [
            str(tools["cmake"]),
            "-S",
            str(root),
            "-B",
            str(build_dir),
            "-G",
            "Ninja",
            "-DCMAKE_BUILD_TYPE=Release",
            f"-DCMAKE_PREFIX_PATH={tools['qt_prefix'].as_posix()}",
            f"-DCMAKE_MAKE_PROGRAM={tools['ninja']}",
            f"-DCMAKE_C_COMPILER={tools['mingw_bin'] / 'gcc.exe'}",
            f"-DCMAKE_CXX_COMPILER={tools['mingw_bin'] / 'g++.exe'}",
        ],
        root,
        env,
    )
    run([str(tools["cmake"]), "--build", str(build_dir), "--target", APP_NAME, "--parallel", "4"], root, env)


def package_name(version: str) -> str:
    safe = re.sub(r"[^A-Za-z0-9_.-]+", "-", version).strip("-")
    return f"{APP_NAME}-{safe}-windows-x64"


def copy_optional(root: Path, package_dir: Path, relative_path: str) -> None:
    source = root / relative_path
    if source.is_file():
        target = package_dir / relative_path
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)


def deploy_qt(root: Path, tools: dict[str, Path], package_exe: Path) -> None:
    env = os.environ.copy()
    env["PATH"] = (
        f"{tools['qt_prefix'] / 'bin'};"
        f"{tools['qt_prefix'] / 'share' / 'qt6' / 'bin'};"
        f"{tools['mingw_bin']};"
        f"{env.get('PATH', '')}"
    )
    windeploy = tools["qt_prefix"] / "bin" / "windeployqt.exe"
    run([str(windeploy), "--release", "--compiler-runtime", str(package_exe)], root, env)


def create_zip(package_dir: Path) -> Path:
    zip_path = package_dir.with_suffix(".zip")
    if zip_path.exists():
        zip_path.unlink()

    with ZipFile(zip_path, "w", ZIP_DEFLATED) as archive:
        for path in package_dir.rglob("*"):
            if path.is_file():
                archive.write(path, path.relative_to(package_dir.parent))
    return zip_path


def package_app(args: argparse.Namespace) -> None:
    root = repo_root()
    tools = resolve_toolchain(root)

    if not args.skip_build:
        build_app(root, tools)

    exe_path = root / BUILD_DIR / f"{APP_NAME}.exe"
    require_file(exe_path, "built application")

    version = args.version or datetime.now().strftime("%Y%m%d-%H%M%S")
    dist_root = root / DIST_ROOT
    package_dir = dist_root / package_name(version)

    if args.clean and package_dir.exists():
        shutil.rmtree(package_dir)

    package_dir.mkdir(parents=True, exist_ok=True)
    package_exe = package_dir / exe_path.name
    shutil.copy2(exe_path, package_exe)

    deploy_qt(root, tools, package_exe)

    for relative_path in [
        "README.md",
        "EmbedDebug.bat",
        "docs/constraints/01-project-overview.md",
    ]:
        copy_optional(root, package_dir, relative_path)

    print(f"package_dir={package_dir}")
    if args.zip:
        print(f"zip_path={create_zip(package_dir)}")


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--skip-build", action="store_true", help="reuse build/EmbedDebug.exe")
    parser.add_argument("--zip", action="store_true", help="also create a zip archive")
    parser.add_argument("--clean", action="store_true", help="remove existing package directory first")
    parser.add_argument("--version", help="override package version suffix")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    try:
        package_app(parse_args(argv or sys.argv[1:]))
        return 0
    except RuntimeError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
