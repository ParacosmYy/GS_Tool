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

    assert "uv run start-embeddebug" in bat_text
    assert "powershell" not in bat_text.lower()
    assert "tools\\launch_embeddebug.ps1" not in bat_text
    assert "build\\EmbedDebug.exe" not in bat_text
    assert "cmake" not in bat_text.lower()


def test_active_tools_do_not_reintroduce_cpp_packaging():
    active_tool_paths = [
        Path("tools/start_embeddebug.py"),
        Path("tools/package_embeddebug.py"),
        Path("tools/verify_package_embeddebug.py"),
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


def test_script_surface_is_minimal_python_product_lane():
    root_bat_files = sorted(path.name for path in Path(".").glob("*.bat"))
    assert root_bat_files == ["EmbedDebug.bat"]
    powershell_files = sorted(str(path) for path in Path("tools").rglob("*.ps1"))
    assert powershell_files == []
    vscode_text = Path(".vscode/settings.json").read_text(
        encoding="utf-8",
        errors="ignore",
    ).lower()
    assert "cmake" not in vscode_text
    assert "mingw" not in vscode_text
    assert "cpp" not in vscode_text

    removed_tool_paths = [
        Path("Beta.bat"),
        Path("tools/bootstrap_env.bat"),
        Path("tools/doctor.ps1"),
        Path("tools/debug-trace.ps1"),
        Path("tools/launch_embeddebug.ps1"),
        Path("tools/source-tree-audit.ps1"),
        Path("tools/simplify-scan.ps1"),
        Path("tools/verify_embeddebug_launch.ps1"),
        Path("tools/agent-loop"),
        Path(".vscode/c_cpp_properties.json"),
        Path("docs/superpowers/plans/BATCH_001_Active_Utils_CMake_Split.md"),
        Path("docs/superpowers/plans/BATCH_077_SerialStation_Minimal_UART_Skeleton.md"),
    ]

    for path in removed_tool_paths:
        assert not path.exists(), f"{path} should not be part of the product lane"


def test_python_runtime_files_stay_within_maintainability_line_budget():
    offenders = []
    for path in Path("python/embeddebug").rglob("*.py"):
        line_count = len(path.read_text(encoding="utf-8").splitlines())
        if line_count > 300:
            offenders.append(f"{path} has {line_count} lines")

    assert not offenders, "oversized Python runtime files: " + "; ".join(offenders)


def test_serial_station_ui_sections_use_explicit_owner_contract():
    sections = Path("python/embeddebug/serial_station/ui/sections.py").read_text(
        encoding="utf-8",
    )

    assert "class SerialStationSectionsHost(Protocol):" in sections
    assert "from typing import Any" not in sections
    assert "owner: Any" not in sections


def test_python_tests_are_split_by_behavior_domain():
    required_files = (
        Path("tests/python/unit/test_python_governance_docs.py"),
        Path("tests/python/unit/test_workbench_controller_profiles.py"),
    )
    files = Path("tests/python").rglob("test_*.py")
    oversized = [f"{p} has {len(p.read_text(encoding='utf-8').splitlines())} lines" for p in files if len(p.read_text(encoding="utf-8").splitlines()) > 250]

    assert all(path.exists() for path in required_files)
    assert oversized == []
