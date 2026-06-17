from __future__ import annotations

import sys
import tempfile
import tomllib
from pathlib import Path

from embeddebug.devtools import package_pyinstaller, verify_pyinstaller_package


def test_pyproject_exposes_python_packaging_scripts():
    pyproject = tomllib.loads(Path("pyproject.toml").read_text(encoding="utf-8"))

    scripts = pyproject["project"]["scripts"]

    assert scripts["package-embeddebug-py"] == "embeddebug.devtools.package_pyinstaller:main"
    assert scripts["verify-package-embeddebug-py"] == (
        "embeddebug.devtools.verify_pyinstaller_package:main"
    )


def test_package_name_sanitizes_version_text():
    assert (
        package_pyinstaller.package_name("dev build/001")
        == "EmbedDebugPy-dev-build-001-windows-x64"
    )


def test_pyinstaller_command_keeps_workpath_outside_repo_build():
    with tempfile.TemporaryDirectory() as temp_dir:
        root = Path(temp_dir)
        (root / "python" / "embeddebug" / "app").mkdir(parents=True)
        entry = root / "python" / "embeddebug" / "app" / "main.py"
        entry.write_text("print('placeholder')\n", encoding="utf-8")

        config = package_pyinstaller.PackageConfig(
            root=root,
            version="test",
            temp_root=root / ".tmp-package",
            clean=True,
            windowed=True,
        )

        command = package_pyinstaller.pyinstaller_command(config)

    command_text = "\n".join(command)
    assert "--onedir" in command
    assert "--name" in command
    assert "EmbedDebugPy" in command
    assert "--paths" in command
    assert "--hidden-import" in command
    assert "PyQt6.QtSerialPort" in command
    assert str(Path("build")) not in command_text
    assert str(entry) in command


def test_verify_package_dir_reports_python_package_requirements():
    with tempfile.TemporaryDirectory() as temp_dir:
        package_dir = Path(temp_dir) / "EmbedDebugPy-dev-windows-x64"
        for relative_path in verify_pyinstaller_package.REQUIRED_RELATIVE_PATHS:
            target = package_dir / relative_path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text("placeholder", encoding="utf-8")

        ok, messages = verify_pyinstaller_package.verify_package_dir(package_dir)

    assert ok
    assert "ok: EmbedDebugPy.exe" in messages
    assert "ok: _internal/PyQt6/Qt6/plugins/platforms/qwindows.dll" in messages


def test_verify_package_dir_requires_python_notice_files():
    with tempfile.TemporaryDirectory() as temp_dir:
        package_dir = Path(temp_dir) / "EmbedDebugPy-dev-windows-x64"
        for relative_path in [
            "EmbedDebugPy.exe",
            "_internal/PyQt6/Qt6/plugins/platforms/qwindows.dll",
        ]:
            target = package_dir / relative_path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text("placeholder", encoding="utf-8")

        ok, messages = verify_pyinstaller_package.verify_package_dir(package_dir)

    assert not ok
    assert "missing: THIRD_PARTY_NOTICES.md" in messages
    assert "missing: PYTHON_DEPENDENCIES.txt" in messages


def test_package_writes_python_notice_files():
    with tempfile.TemporaryDirectory() as temp_dir:
        root = Path(temp_dir)
        notice_source = root / "docs" / "legal" / "PYTHON_THIRD_PARTY_NOTICES.md"
        notice_source.parent.mkdir(parents=True)
        notice_source.write_text("# notices\n\nPyQt6: GPLv3 route\n", encoding="utf-8")
        package_dir = root / "dist" / "EmbedDebugPy-dev-windows-x64"
        package_dir.mkdir(parents=True)

        package_pyinstaller.write_package_notice_files(
            root=root,
            package_dir=package_dir,
            dependency_lines=["PyQt6==6.11.0", "numpy==2.4.6"],
        )

        assert (package_dir / "THIRD_PARTY_NOTICES.md").read_text(encoding="utf-8") == (
            "# notices\n\nPyQt6: GPLv3 route\n"
        )
        assert (package_dir / "PYTHON_DEPENDENCIES.txt").read_text(encoding="utf-8") == (
            "PyQt6==6.11.0\nnumpy==2.4.6\n"
        )


def test_main_help_returns_zero_for_python_packaging_tools(capsys):
    package_exit = package_pyinstaller.main(["--help"])
    verify_exit = verify_pyinstaller_package.main(["--help"])

    captured = capsys.readouterr()

    assert package_exit == 0
    assert verify_exit == 0
    assert "package-embeddebug-py" in captured.out
    assert "verify-package-embeddebug-py" in captured.out
