"""约束文档漂移守护脚本.

由 ``docs/constraints/09-closed-loop.md`` 门禁 5 调用，机械检测以下漂移：

1. 评分一致性：CLAUDE.md / README.md / 01-project-overview.md / 06-git-commit.md
   里出现的硬编码分数必须与 docs/tracking/SCORE_TRACKING.md 首行一致。
2. SSOT 引用合规：CLAUDE.md / README.md 禁止出现硬编码 "NNNN passed" /
   "NNN 个测试文件" 字样（应由命令实时采集）。
3. 冻结目录守护：仓库根不得出现 src/、build2/、build-debug/、build-release/、
   native-build-*。
4. 评分加分门禁：若本次改动把 canonical 分数上调，diff 修改行数必须 ≥1000。
5. 文件行数门禁：python/embeddebug/**/*.py 单文件 ≤300 行；
   tests/python/**/*.py 单文件 ≤250 行。

退出码 0 = 全绿；非 0 = 有漂移（stdout 打印每条违规，最后汇总）。

设计原则：无外部依赖、纯标准库、单文件、可被 ``uv run check-constraints`` 直接调用。
"""

from __future__ import annotations

import re
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]

# canonical 评分文件
SCORE_FILE = REPO_ROOT / "docs" / "tracking" / "SCORE_TRACKING.md"

# 受 SSOT 约束的核心文档（出现分数/测试数硬编码即违规）
SSOT_GUARDED_DOCS = [
    REPO_ROOT / "CLAUDE.md",
    REPO_ROOT / "README.md",
    REPO_ROOT / "docs" / "constraints" / "01-project-overview.md",
    REPO_ROOT / "docs" / "constraints" / "06-git-commit.md",
]

# 禁止出现的冻结/平行目录（仓库根直接子目录）
FORBIDDEN_DIRS = {
    "src",
    "build2",
    "build-debug",
    "build-release",
}

# 文件行数门禁
RUNTIME_LINE_LIMIT = 300
TEST_LINE_LIMIT = 250

# 评分加分门槛：只有本次 diff 修改行数达到该值，才允许 canonical 分数 +1。
SCORE_INCREMENT_LINE_THRESHOLD = 1000

# 测试组织门禁：孤儿文件阈值（<此数测试函数的新 test 文件即孤儿）
TEST_ORPHAN_THRESHOLD = 3
# 测试三层目录（unit 必须纯、ui_smoke 必须 QApplication）
TEST_UNIT_DIR = REPO_ROOT / "tests" / "python" / "unit"
TEST_INTEGRATION_DIR = REPO_ROOT / "tests" / "python" / "integration"
TEST_UI_SMOKE_DIR = REPO_ROOT / "tests" / "python" / "ui_smoke"

# 评分抽取正则：匹配 "当前: 787分" / "当前 787" / "787 / 1000"
SCORE_LINE_RE = re.compile(r"当前[：:]\s*(\d+)\s*分")
SCORE_GENERIC_RE = re.compile(r"\b(\d{2,4})\s*/\s*1000\b")
# 硬编码测试数字："NNNN passed" / "NNN passed, M skipped"
TESTS_PASSED_RE = re.compile(r"\b\d{3,5}\s+passed\b", re.IGNORECASE)
# 硬编码测试文件数："测试文件数 172" / "172 个测试文件" / "测试文件 172"
TEST_FILES_RE = re.compile(r"测试文件[数]?\s*\d{2,4}|(\d{2,4})\s*个测试文件")
# 测试函数定义行（用于孤儿检测与计数）
TEST_FUNC_RE = re.compile(r"^\s*def\s+test_", re.MULTILINE)
# unit 层禁止 import PyQt（layer-misplacement 检测）
UNIT_PYQT_IMPORT_RE = re.compile(
    r"^\s*(?:from|import)\s+(?:PyQt\d?|pyqtgraph)", re.MULTILINE
)


# 历史快照围栏：标记存档/历史段，其内的硬编码数字不算漂移
HISTORICAL_OPEN = "<!-- check-constraints: historical -->"
HISTORICAL_CLOSE = "<!-- check-constraints: /historical -->"


def _strip_historical_fences(text: str) -> str:
    """删除被历史围栏标记的段落（行级），返回剩余文本。

    用途：CLAUDE.md 的 "会话成果存档" / README 的 "历史变更" 等段落记录的是
    过去快照（如 "Batch 49: 1695 passed"），不是当前事实漂移。用围栏显式标记，
    脚本即跳过，既保留历史又防止当前事实漂移。
    """
    in_historical = False
    kept: list[str] = []
    for line in text.splitlines():
        if HISTORICAL_OPEN in line:
            in_historical = True
            continue
        if HISTORICAL_CLOSE in line:
            in_historical = False
            continue
        if in_historical:
            continue
        kept.append(line)
    return "\n".join(kept)


def read_text(path: Path) -> str:
    """统一 UTF-8 读取，文件不存在返回空串。"""
    if not path.is_file():
        return ""
    return path.read_text(encoding="utf-8")


def extract_canonical_score() -> int | None:
    """从 SCORE_TRACKING.md 首行抽取 canonical 分数。

    首行格式形如：``> 当前: 787分 | 目标: 1000分 | ...``
    """
    text = read_text(SCORE_FILE)
    if not text:
        return None
    for line in text.splitlines()[:5]:
        match = SCORE_LINE_RE.search(line)
        if match:
            return int(match.group(1))
    return None


def _extract_score_from_text(text: str) -> int | None:
    """从给定文本前几行抽取 canonical 分数。"""
    for line in text.splitlines()[:5]:
        match = SCORE_LINE_RE.search(line)
        if match:
            return int(match.group(1))
    return None


def _head_score() -> int | None:
    """读取 HEAD 版本的 SCORE_TRACKING 分数，用于判断本次是否加分。"""
    try:
        result = subprocess.run(
            ["git", "show", f"HEAD:{SCORE_FILE.relative_to(REPO_ROOT).as_posix()}"],
            cwd=REPO_ROOT,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=10,
            check=False,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    if result.returncode != 0:
        return None
    return _extract_score_from_text(result.stdout)


def _changed_line_count() -> int | None:
    """统计本次相对 HEAD 的新增+删除行数。"""
    try:
        result = subprocess.run(
            ["git", "diff", "--numstat", "HEAD", "--"],
            cwd=REPO_ROOT,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=10,
            check=False,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    if result.returncode != 0:
        return None
    total = 0
    for line in result.stdout.splitlines():
        parts = line.split("\t")
        if len(parts) < 3:
            continue
        added, deleted = parts[0], parts[1]
        if added == "-" or deleted == "-":
            continue
        total += int(added) + int(deleted)
    return total


def check_score_increment_line_gate(canonical: int | None) -> list[str]:
    """若本次分数上调，强制检查 diff 修改行数 ≥1000。"""
    violations: list[str] = []
    if canonical is None:
        return violations
    previous = _head_score()
    if previous is None or canonical <= previous:
        return violations
    changed_lines = _changed_line_count()
    if changed_lines is None:
        violations.append(
            "[score-line-gate] 无法统计本次 diff 行数，分数上调被阻断"
        )
        return violations
    if changed_lines < SCORE_INCREMENT_LINE_THRESHOLD:
        violations.append(
            f"[score-line-gate] 本次分数 {previous}→{canonical}，但 diff 修改行数 "
            f"{changed_lines} < {SCORE_INCREMENT_LINE_THRESHOLD}；不得 +1"
        )
    return violations


def check_score_consistency(canonical: int | None) -> list[str]:
    """检测受约束文档里的硬编码分数是否与 canonical 一致。

    canonical 为 None（SCORE_TRACKING 读不到）时跳过本检查，但单独报告。
    """
    violations: list[str] = []
    if canonical is None:
        violations.append(
            f"[check_constraints] 无法读取 canonical 分数：{SCORE_FILE} 缺失或首行格式不符"
        )
        return violations

    for doc in SSOT_GUARDED_DOCS:
        if not doc.is_file():
            continue
        text = _strip_historical_fences(read_text(doc))
        for lineno, line in enumerate(text.splitlines(), start=1):
            # 跳过 canonical 自身的引用行（明确指向 SCORE_TRACKING 的不算违规）
            if "SCORE_TRACKING" in line:
                continue
            for match in SCORE_GENERIC_RE.finditer(line):
                value = int(match.group(1))
                # 只对 1~999 的"像评分"的数字报错（避免误伤 1000 目标分本身）
                if 1 <= value < 1000 and value != canonical:
                    rel = doc.relative_to(REPO_ROOT)
                    violations.append(
                        f"[score-drift] {rel}:{lineno} 出现分数 {value}，"
                        f"与 canonical {canonical}（SCORE_TRACKING.md）不一致：{line.strip()}"
                    )
    return violations


def check_hardcoded_test_counts() -> list[str]:
    """检测 CLAUDE.md / README.md 是否硬编码测试通过数或测试文件数。"""
    violations: list[str] = []
    for doc in [REPO_ROOT / "CLAUDE.md", REPO_ROOT / "README.md"]:
        if not doc.is_file():
            continue
        text = _strip_historical_fences(read_text(doc))
        for lineno, line in enumerate(text.splitlines(), start=1):
            for match in TESTS_PASSED_RE.finditer(line):
                rel = doc.relative_to(REPO_ROOT)
                violations.append(
                    f"[hardcoded-tests] {rel}:{lineno} 硬编码测试通过数 "
                    f"'{match.group(0)}'，应改为命令实时采集：{line.strip()}"
                )
            for match in TEST_FILES_RE.finditer(line):
                rel = doc.relative_to(REPO_ROOT)
                violations.append(
                    f"[hardcoded-test-files] {rel}:{lineno} 硬编码测试文件数 "
                    f"'{match.group(0)}'，应改为 'git ls-files \"tests/python/*.py\" | wc -l'：{line.strip()}"
                )
    return violations


def check_forbidden_dirs() -> list[str]:
    """检测仓库根是否出现冻结/平行/禁止目录。"""
    violations: list[str] = []
    if not REPO_ROOT.is_dir():
        return violations
    for child in REPO_ROOT.iterdir():
        if not child.is_dir():
            continue
        name = child.name
        if name in FORBIDDEN_DIRS:
            violations.append(
                f"[forbidden-dir] 仓库根出现禁止目录 '{name}'（冻结/平行 native 构建），"
                f"路径：{child.relative_to(REPO_ROOT)}"
            )
        if name.startswith("native-build"):
            violations.append(
                f"[forbidden-dir] 仓库根出现禁止目录 '{name}'（native-build-* 前缀），"
                f"路径：{child.relative_to(REPO_ROOT)}"
            )
    return violations


def _count_meaningful_lines(path: Path) -> int:
    """统计有效行数（非空、非纯注释）。

    Python 文件的行数门禁关注"实质代码体量"，所以排除空行和纯注释行。
    shebang / encoding / docstring 仍计入（它们是结构的一部分）。
    """
    count = 0
    in_docstring = False
    for line in path.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if not stripped:
            continue
        # 粗略处理三引号 docstring：进入/退出状态切换
        triple = stripped.count('"""') + stripped.count("'''")
        if in_docstring:
            if triple % 2 == 1:
                in_docstring = False
            continue
        if triple == 1 and not stripped.startswith('"""') and not stripped.startswith("'''"):
            count += 1
            continue
        if triple % 2 == 1:
            in_docstring = True
            # docstring 起始行本身计入（代表结构）
            count += 1
            continue
        if stripped.startswith("#"):
            continue
        count += 1
    return count


def check_line_limits() -> list[str]:
    """检测 python/embeddebug/**/*.py 与 tests/python/**/*.py 单文件行数。"""
    violations: list[str] = []
    runtime_root = REPO_ROOT / "python" / "embeddebug"
    test_root = REPO_ROOT / "tests" / "python"

    for py in runtime_root.rglob("*.py"):
        if "__pycache__" in py.parts:
            continue
        lines = _count_meaningful_lines(py)
        if lines > RUNTIME_LINE_LIMIT:
            rel = py.relative_to(REPO_ROOT)
            violations.append(
                f"[line-limit] {rel} 实质代码 {lines} 行 > {RUNTIME_LINE_LIMIT}（runtime），需拆分"
            )

    for py in test_root.rglob("*.py"):
        if "__pycache__" in py.parts:
            continue
        lines = _count_meaningful_lines(py)
        if lines > TEST_LINE_LIMIT:
            rel = py.relative_to(REPO_ROOT)
            violations.append(
                f"[line-limit] {rel} 实质代码 {lines} 行 > {TEST_LINE_LIMIT}（test），需拆分"
            )
    return violations


def _count_test_functions(path: Path) -> int:
    """统计一个测试文件的 def test_ 函数数量。"""
    text = path.read_text(encoding="utf-8")
    return len(TEST_FUNC_RE.findall(text))


def check_test_orphans() -> list[str]:
    """检测测试孤儿文件：<TEST_ORPHAN_THRESHOLD 个测试函数的 test_*.py。

    规则来源：CLAUDE.md §测试文件组织规则 23 / 04-coding-standard §6.3。
    孤儿文件（<3 个测试）应并入同域既有文件，不是新建。
    本检查扫描工作区所有 test_*.py，但不阻断既有孤儿（只报告），目的是防止新增孤儿。

    设计权衡：既有仓库已有不少历史孤儿（如 test_tr_compliance 1 个测试），
    这些是技术债，应在后续 Simplify 批次清理，而非本次 commit 阻断。
    因此本检查只对 **git 暂存区/未跟踪的新增文件** 报违规；既有跟踪文件只警告。
    """
    violations: list[str] = []
    if not TEST_UNIT_DIR.is_dir():
        return violations

    # 获取 git 跟踪的文件集合，用于区分"新增 vs 既有"
    tracked = _git_tracked_files("tests/python/")

    for layer_dir in [TEST_UNIT_DIR, TEST_INTEGRATION_DIR, TEST_UI_SMOKE_DIR]:
        if not layer_dir.is_dir():
            continue
        for py in layer_dir.glob("test_*.py"):
            if "__pycache__" in py.parts:
                continue
            count = _count_test_functions(py)
            if count >= TEST_ORPHAN_THRESHOLD:
                continue
            rel = py.relative_to(REPO_ROOT)
            rel_str = str(rel).replace("\\", "/")
            is_tracked = rel_str in tracked
            severity = "既有技术债（不阻断，建议 Simplify 清理）" if is_tracked else "新增孤儿（阻断，必须并入同域既有文件）"
            tag = "[test-orphan]" if not is_tracked else "[test-orphan-warning]"
            violations.append(
                f"{tag} {rel} 仅 {count} 个测试函数（< {TEST_ORPHAN_THRESHOLD}），"
                f"{severity}"
            )
    # 既有技术债只警告不 fail：过滤掉 warning，只保留真阻断项
    return [v for v in violations if not v.startswith("[test-orphan-warning]")]


def check_unit_layer_purity() -> list[str]:
    """检测 unit 层是否混入 PyQt/pyqtgraph import（layer-misplacement）。

    规则来源：CLAUDE.md §测试文件组织规则 24 / 04-coding-standard §6.2。
    unit 层必须纯函数/类、无 Qt 无 IO 无网络、毫秒级。
    import PyQt 或 pyqtgraph 即说明该测试属于 ui_smoke，放错了层。

    与 check_test_orphans 同策略：既有跟踪文件只警告，新增文件阻断。
    """
    violations: list[str] = []
    if not TEST_UNIT_DIR.is_dir():
        return violations

    tracked = _git_tracked_files("tests/python/unit/")
    for py in TEST_UNIT_DIR.rglob("test_*.py"):
        if "__pycache__" in py.parts:
            continue
        text = py.read_text(encoding="utf-8")
        if not UNIT_PYQT_IMPORT_RE.search(text):
            continue
        rel = py.relative_to(REPO_ROOT)
        rel_str = str(rel).replace("\\", "/")
        is_tracked = rel_str in tracked
        severity = (
            "既有技术债（不阻断，建议迁移到 ui_smoke/）"
            if is_tracked
            else "新增 unit 误用 PyQt（阻断，必须放 ui_smoke/ 或改用 fake）"
        )
        tag = "[layer-misplace]" if not is_tracked else "[layer-misplace-warning]"
        violations.append(
            f"{tag} {rel} unit 层混入 PyQt/pyqtgraph import，{severity}"
        )
    return [v for v in violations if not v.startswith("[layer-misplace-warning]")]


def _git_tracked_files(prefix: str) -> set[str]:
    """返回已提交跟踪的指定前缀文件集合（排除 staged 新增文件）。"""
    import subprocess

    try:
        tracked = subprocess.run(
            ["git", "ls-files", prefix],
            cwd=REPO_ROOT,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=10,
            check=False,
        )
        added = subprocess.run(
            ["git", "diff", "--cached", "--name-only", "--diff-filter=A", "--", prefix],
            cwd=REPO_ROOT,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=10,
            check=False,
        )
        tracked_files = {line.strip() for line in tracked.stdout.splitlines() if line.strip()}
        staged_added = {line.strip() for line in added.stdout.splitlines() if line.strip()}
        return tracked_files - staged_added
    except (OSError, subprocess.SubprocessError):
        # git 不可用时退化为"全部视为既有"，避免误阻断
        return {"*"}


def main(argv: list[str] | None = None) -> int:
    """主入口：跑全部检查，汇总违规，返回退出码。"""
    del argv  # 无参数
    all_violations: list[str] = []

    canonical = extract_canonical_score()
    all_violations += check_score_consistency(canonical)
    all_violations += check_score_increment_line_gate(canonical)
    all_violations += check_hardcoded_test_counts()
    all_violations += check_forbidden_dirs()
    all_violations += check_line_limits()
    all_violations += check_test_orphans()
    all_violations += check_unit_layer_purity()

    if not all_violations:
        score_str = f"{canonical}" if canonical is not None else "未知"
        print(f"[check_constraints] PASS — 全部门禁绿（canonical 分数：{score_str}）")
        return 0

    print(f"[check_constraints] FAIL — 发现 {len(all_violations)} 条违规：")
    for v in all_violations:
        print(f"  - {v}")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
