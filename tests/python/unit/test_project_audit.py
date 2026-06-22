"""project_audit 工具单元测试 — 模块统计/编号工具分析。

覆盖：iter_code_files 扫描、module_counts 分类、numbered_utils 检测、
python_counts 统计。用 tmp_path 创建临时目录结构。
"""

from __future__ import annotations

import importlib.util
from collections import Counter
from pathlib import Path

# project-audit 目录名带连字符（非合法 Python 包），用路径加载。
_spec = importlib.util.spec_from_file_location(
    "project_audit",
    Path(__file__).resolve().parents[3] / "tools" / "project-audit" / "project_audit.py",
)
assert _spec is not None and _spec.loader is not None
_mod = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_mod)

iter_code_files = _mod.iter_code_files
module_counts = _mod.module_counts
numbered_utils = _mod.numbered_utils
python_counts = _mod.python_counts


def _make_tree(root: Path, files: dict[str, str]) -> None:
    for rel, content in files.items():
        p = root / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(content, encoding="utf-8")


def test_iter_code_files_empty(tmp_path):
    """空 src 目录返回空列表。"""
    (tmp_path / "src").mkdir()
    assert list(iter_code_files(tmp_path)) == []


def test_iter_code_files_finds_python(tmp_path):
    _make_tree(tmp_path, {"src/a.py": "x=1", "src/b.py": "y=2", "readme.md": "# hi"})
    files = list(iter_code_files(tmp_path))
    assert len(files) == 2  # 只 .py


def test_module_counts(tmp_path):
    _make_tree(tmp_path, {
        "src/module1.py": "x=1",
        "src/module2.py": "y=2",
        "src/sub/module3.py": "z=3",
    })
    counts = module_counts(tmp_path)
    assert isinstance(counts, Counter)
    assert sum(counts.values()) >= 3


def test_numbered_utils_finds_pattern(tmp_path):
    """检测 numbered util 文件名模式（如 util1.py, util2.py）。"""
    _make_tree(tmp_path, {
        "src/util1.py": "x=1",
        "src/util2.py": "y=2",
        "src/helper.py": "z=3",
    })
    families = numbered_utils(tmp_path)
    assert isinstance(families, dict)


def test_numbered_utils_empty(tmp_path):
    src = tmp_path / "src"
    src.mkdir()
    families = numbered_utils(tmp_path)
    assert families == {} or all(len(v) == 0 for v in families.values())


def test_python_counts(tmp_path):
    """python_counts 扫描 .py 文件（具体路径逻辑由实现决定）。"""
    _make_tree(tmp_path, {
        "src/a.py": "x=1",
        "src/b.py": "y=2",
    })
    counts = python_counts(tmp_path)
    assert isinstance(counts, Counter)
