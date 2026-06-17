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


def test_active_architecture_docs_are_python_pyqt_only():
    expected_docs = {
        "README.md",
        "frozen-dirs.md",
        "migration-roadmap.md",
        "module-boundaries.md",
        "target-structure.md",
    }
    actual_docs = {path.name for path in Path("docs/architecture").glob("*.md")}
    forbidden = [
        "cmake",
        "c++",
        "cpp",
        ".h/",
        ".h ",
        ".cpp",
        "src/",
        "windeployqt",
        "mingw",
    ]

    assert actual_docs == expected_docs
    for path in Path("docs/architecture").glob("*.md"):
        text = path.read_text(encoding="utf-8", errors="ignore").lower()
        for token in forbidden:
            assert token not in text, f"{path} still references {token}"


def test_active_prd_specs_do_not_keep_retired_native_batches():
    retired_name_tokens = [
        "cmake",
        "source_tree",
        "source-tree",
        "go_agent_loop",
        "go-agent-loop",
        "uv_package_tool",
        "uv_start_tool",
        "uv_tools_self_test",
        "uv_package_verify_tool",
    ]
    active_docs = [
        path
        for root in (Path("docs/prd"), Path("docs/superpowers/specs"))
        for path in root.glob("*.md")
    ]
    offenders = [
        str(path)
        for path in active_docs
        if any(token in path.name.lower() for token in retired_name_tokens)
    ]

    assert not offenders, "retired native/tool batches remain active: " + ", ".join(
        offenders
    )


def test_active_prd_specs_text_is_python_pyqt_only():
    active_docs = [
        path
        for root in (Path("docs/prd"), Path("docs/superpowers/specs"))
        for path in root.glob("*.md")
    ]
    forbidden = [
        "c++",
        "cmake",
        "windeployqt",
        "mingw",
        "build/embeddebug.exe",
        "build\\embeddebug.exe",
        "src/",
        "cpp",
        ".h/cpp",
        ".h/.cpp",
        "```cpp",
    ]
    offenders = []

    for path in active_docs:
        text = path.read_text(encoding="utf-8", errors="ignore").lower()
        for token in forbidden:
            if token in text:
                offenders.append(f"{path} contains {token}")
                break

    assert not offenders, "legacy native wording remains active: " + "; ".join(
        offenders
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
        "c++",
        "cmake",
        "cmake --build",
        "windeployqt",
        "mingw",
        "qmake",
        "msvc",
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


def test_active_governance_docs_do_not_reference_removed_shell_scripts():
    docs = [
        Path("AGENTS.md"),
        Path("CLAUDE.md"),
        Path("README.md"),
        Path("docs/constraints/01-project-overview.md"),
        Path("docs/constraints/02-workflow.md"),
        Path("docs/constraints/06-git-commit.md"),
        Path("docs/constraints/07-directory-structure.md"),
        Path("docs/serial_station_architecture.md"),
        Path("docs/superpowers/LOOP_PROTOCOL.md"),
    ]
    forbidden = [
        "tools\\doctor.ps1",
        "tools/doctor.ps1",
        ".\\tools\\doctor.ps1",
        "launch_embeddebug.ps1",
        "verify_embeddebug_launch.ps1",
        "bootstrap_env.bat",
        "Beta.bat",
    ]

    for path in docs:
        text = path.read_text(encoding="utf-8", errors="ignore")
        for token in forbidden:
            assert token.lower() not in text.lower(), f"{path} still references {token}"
