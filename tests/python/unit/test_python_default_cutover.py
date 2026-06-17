from __future__ import annotations

import tomllib
from pathlib import Path


def test_default_uv_commands_point_to_python_lane():
    pyproject = tomllib.loads(Path("pyproject.toml").read_text(encoding="utf-8"))

    scripts = pyproject["project"]["scripts"]

    assert scripts["start-embeddebug"] == "embeddebug.app.main:main"
    assert scripts["package-embeddebug"] == "embeddebug.devtools.package_pyinstaller:main"
    assert scripts["verify-package-embeddebug"] == (
        "embeddebug.devtools.verify_pyinstaller_package:main"
    )


def test_batch_launcher_uses_uv_python_startup():
    bat_text = Path("EmbedDebug.bat").read_text(encoding="utf-8", errors="ignore")
    launcher_text = Path("tools/launch_embeddebug.ps1").read_text(
        encoding="utf-8",
        errors="ignore",
    )

    assert "tools\\launch_embeddebug.ps1" in bat_text
    assert "uv run start-embeddebug" in launcher_text
    assert "build\\EmbedDebug.exe" not in launcher_text
    assert "cmake" not in launcher_text.lower()


def test_active_tools_do_not_reintroduce_cpp_packaging():
    active_tool_paths = [
        Path("tools/launch_embeddebug.ps1"),
        Path("tools/start_embeddebug.py"),
        Path("tools/package_embeddebug.py"),
        Path("tools/verify_package_embeddebug.py"),
        Path("tools/doctor.ps1"),
        Path("tools/verify_embeddebug_launch.ps1"),
        Path("tools/bootstrap_env.bat"),
        Path("tools/source-tree-audit.ps1"),
        Path("tools/simplify-scan.ps1"),
    ]
    forbidden = [
        "cmake",
        "windeployqt",
        "mingw",
        "build/embeddebug.exe",
        "build\\embeddebug.exe",
    ]

    for path in active_tool_paths:
        text = path.read_text(encoding="utf-8", errors="ignore").lower()
        for token in forbidden:
            assert token not in text, f"{path} still references {token}"


def test_repository_no_longer_contains_legacy_cpp_cmake_project():
    forbidden_paths = [
        Path("CMakeLists.txt"),
        Path("cmake"),
        Path("src"),
        Path("tests/CMakeLists.txt"),
        Path("tests/serial_station"),
    ]

    for path in forbidden_paths:
        assert not path.exists(), f"{path} should be removed from the Python/PyQt-only repo"


def test_tests_tree_contains_only_python_tests():
    forbidden_suffixes = {
        ".c",
        ".cc",
        ".cpp",
        ".cxx",
        ".h",
        ".hh",
        ".hpp",
        ".qrc",
        ".ui",
    }
    offenders = [
        path
        for path in Path("tests").rglob("*")
        if path.is_file() and path.suffix.lower() in forbidden_suffixes
    ]

    assert not offenders, "legacy native tests remain: " + ", ".join(
        str(path) for path in offenders
    )


def test_active_governance_docs_do_not_reference_legacy_native_workflow():
    docs = [
        Path("AGENTS.md"),
        Path("CLAUDE.md"),
        Path("README.md"),
        Path("docs/constraints/01-project-overview.md"),
        Path("docs/constraints/02-workflow.md"),
        Path("docs/constraints/03-architecture.md"),
        Path("docs/constraints/04-coding-standard.md"),
        Path("docs/constraints/05-ui-standard.md"),
        Path("docs/constraints/06-git-commit.md"),
        Path("docs/constraints/07-directory-structure.md"),
        Path("docs/constraints/08-icon-standard.md"),
        Path("docs/serial_station_architecture.md"),
        Path("docs/architecture/module-boundaries.md"),
        Path("docs/architecture/target-structure.md"),
        Path("docs/superpowers/specs/SPECS_TEMPLATE.md"),
    ]
    forbidden = [
        "cmake --build",
        "windeployqt",
        "build/embeddebug.exe",
        "build\\embeddebug.exe",
        "src/apps/serial_station",
        "tests/serial_station",
        "新增 `.h/.cpp`",
    ]

    for path in docs:
        text = path.read_text(encoding="utf-8", errors="ignore").lower()
        for token in forbidden:
            assert token.lower() not in text, f"{path} still references {token}"
