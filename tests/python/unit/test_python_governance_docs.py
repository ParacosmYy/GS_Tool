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


def test_ui_smoke_tests_are_split_by_focused_behavior_domain():
    offenders = [
        f"{path}:{len(path.read_text(encoding='utf-8').splitlines())}"
        for path in Path("tests/python/ui_smoke").glob("test_*.py")
        if len(path.read_text(encoding="utf-8").splitlines()) > 225
    ]

    assert not offenders, "ui_smoke test files exceed 225 lines: " + ", ".join(offenders)


def test_workbench_controller_unit_tests_are_split_by_behavior_domain():
    path = Path("tests/python/unit/test_workbench_controller.py")
    line_count = len(path.read_text(encoding="utf-8").splitlines())

    assert line_count <= 180, f"{path} has {line_count} lines; split connection/profile cases"


def test_workbench_controller_runtime_facade_stays_below_250_lines():
    path = Path("python/embeddebug/serial_station/controllers/workbench_controller.py")
    line_count = len(path.read_text(encoding="utf-8").splitlines())

    assert line_count <= 250, f"{path} has {line_count} lines; split runtime state helpers"


def test_score_tracking_head_not_behind_max_record():
    """SCORE_TRACKING 不得滞后：头部>=最大记录，且文档 batch 号不落后 git 太多。

    1. 内部一致性：head_score >= max_record。
    2. 外部滞后：文档最大 Batch 号 >= git 最新 Batch 号 - 3（容差 3）。
    """
    import re
    import subprocess

    text = Path("docs/tracking/SCORE_TRACKING.md").read_text(encoding="utf-8")
    head_match = re.search(r"当前:\s*(\d+)分", text)
    assert head_match, "SCORE_TRACKING.md 头部缺少「当前: NNN分」声明"
    head_score = int(head_match.group(1))

    record_nums = [int(n) for n in re.findall(r"^\|\s*(\d+)\s*\|", text, re.MULTILINE)]
    assert record_nums, "SCORE_TRACKING.md 无任何评分记录行（| NNN | ...）"
    max_record = max(record_nums)

    assert head_score >= max_record, (
        f"SCORE_TRACKING 头部分数 {head_score} 落后于最大记录编号 {max_record}；"
        f"请回填头部「当前」分到至少 {max_record}"
    )

    # 外部滞后检查：文档 Batch 号 vs git 最新 Batch 号（容差 3）。
    doc_batches = [int(m) for m in re.findall(r"Batch (\d+)", text)]
    try:
        log = subprocess.check_output(
            ["git", "log", "--oneline", "-50"], text=True, stderr=subprocess.DEVNULL
        )
        git_batches = [int(m) for m in re.findall(r"Batch (\d+)", log)]
    except (FileNotFoundError, subprocess.CalledProcessError):
        return  # git 不可用，跳过外部检查。

    if doc_batches and git_batches:
        doc_max_batch = max(doc_batches)
        git_max_batch = max(git_batches)
        assert doc_max_batch >= git_max_batch - 3, (
            f"SCORE_TRACKING 文档最新 Batch {doc_max_batch} 严重落后于 git "
            f"最新 Batch {git_max_batch}（容差 3）；请回填 SCORE_TRACKING"
        )


def test_no_new_batch_suffixed_test_files():
    """禁止新增 *_batch*.py 测试文件（白名单覆盖已知在途项）。

    UI 美化迭代曾持续产出 test_*_batchNN.py 碎片文件，文件名携带无意义迭代
    编号，降低可搜索性。存量已全部 rename 清零（3c912fff3/f1ab1a995）。
    本守护防止复发：新增的 batch 文件必须先 rename 为语义命名（去 _batchNN
    后缀）再提交。

    白名单 _PENDING_RENAME 列出已知的、尚未迁移的在途文件；迁移后从此移除。
    """
    _PENDING_RENAME = frozenset()  # 无待迁移项；并发进程 Batch 26 已自行 rename 为 test_fullscreen_esc.py

    offenders = sorted(
        p.name
        for root in (Path("tests/python/unit"), Path("tests/python/ui_smoke"))
        for p in root.glob("test_*_batch*.py")
        if p.name not in _PENDING_RENAME
    )

    assert not offenders, (
        "发现新的 batch 后缀测试文件，请改为语义命名（去 _batchNN 后缀）再提交: "
        + ", ".join(offenders)
    )
