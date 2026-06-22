"""约束文档漂移守护脚本.

由 ``docs/constraints/09-closed-loop.md`` 门禁 5 调用，机械检测以下漂移：

1. 评分一致性：CLAUDE.md / README.md / 01-project-overview.md / 06-git-commit.md
   里出现的硬编码分数必须与 docs/tracking/SCORE_TRACKING.md 首行一致。
2. SSOT 引用合规：CLAUDE.md / README.md 禁止出现硬编码 "NNNN passed" /
   "NNN 个测试文件" 字样（应由命令实时采集）。
3. 冻结目录守护：仓库根不得出现 src/、build2/、build-debug/、build-release/、
   native-build-*。
4. 文件行数门禁：python/embeddebug/**/*.py 单文件 ≤300 行；
   tests/python/**/*.py 单文件 ≤250 行。

退出码 0 = 全绿；非 0 = 有漂移（stdout 打印每条违规，最后汇总）。

设计原则：无外部依赖、纯标准库、单文件、可被 ``uv run check-constraints`` 直接调用。
"""

from __future__ import annotations

import re
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

# 评分抽取正则：匹配 "当前: 787分" / "当前 787" / "787 / 1000"
SCORE_LINE_RE = re.compile(r"当前[：:]\s*(\d+)\s*分")
SCORE_GENERIC_RE = re.compile(r"\b(\d{2,4})\s*/\s*1000\b")
# 硬编码测试数字："NNNN passed" / "NNN passed, M skipped"
TESTS_PASSED_RE = re.compile(r"\b\d{3,5}\s+passed\b", re.IGNORECASE)
# 硬编码测试文件数："测试文件数 172" / "172 个测试文件" / "测试文件 172"
TEST_FILES_RE = re.compile(r"测试文件[数]?\s*\d{2,4}|(\d{2,4})\s*个测试文件")


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


def main(argv: list[str] | None = None) -> int:
    """主入口：跑全部检查，汇总违规，返回退出码。"""
    del argv  # 无参数
    all_violations: list[str] = []

    canonical = extract_canonical_score()
    all_violations += check_score_consistency(canonical)
    all_violations += check_hardcoded_test_counts()
    all_violations += check_forbidden_dirs()
    all_violations += check_line_limits()

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
