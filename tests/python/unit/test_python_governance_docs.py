from __future__ import annotations

from pathlib import Path


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
