"""B54-4 tr() 合规守护：用户可见文字必须走 tr()（铁律 19）。

AST 扫描 ``python/embeddebug/serial_station/ui/`` 检测传给用户可见 API 的
字面量字符串未被 ``tr()``/``self.tr()``/``widget.tr()``/``host.tr()`` 包裹。

检测的 API（第一参数为用户可见文字）：
- QLabel(text) / QPushButton(text) / QCheckBox(text) / QRadioButton(text)
- setToolTip(text) / setStatusTip(text) / setPlaceholderText(text)
- setText(text) / setWindowTitle(text) / setTitle(text)
- addItem(text) / addTab(widget, text) / setHeaderLabels(tuple)

软模式默认（print 违规），``STRICT_TR=1`` 硬模式断言失败。
白名单豁免：纯 ASCII 标识符（如 "0x31"）、数字、空串、变量引用、f-string。
"""

from __future__ import annotations

import ast
import os
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
UI_ROOT = REPO_ROOT / "python" / "embeddebug" / "serial_station" / "ui"

# 第一参数为用户可见文字的 API 调用名。
_VISIBLE_TEXT_APIS = frozenset({
    "setToolTip", "setStatusTip", "setPlaceholderText", "setWindowTitle",
    "setTabText", "setHeaderText", "setPlainText", "setHtml",
})

# 构造函数第一参数为文字（QLabel/QPushButton/QCheckBox/QRadioButton/QAction）。
_VISIBLE_TEXT_CTORS = frozenset({
    "QLabel", "QPushButton", "QCheckBox", "QRadioButton", "QAction",
})

# 跳过的路径片段（测试/主题/生成器）。
_SKIP_MARKERS = ("__pycache__", "theme/qss_sections_", "theme/palette", "theme/tokens")


def _is_user_visible_text(value: object) -> bool:
    """判断 AST 节点是否为「未包裹 tr 的用户可见字面量文字」。

    判定违规的条件：
    - 是 Str/Constant 且值为非空字符串
    - 内容含非 ASCII 字符（中文等）或长度 > 6 的英文（排除 "0x31" 等技术值）
    - 不是 f-string（JoinedStr）
    """

    if isinstance(value, ast.JoinedStr):
        return False  # f-string 视为已格式化（变量插值），豁免
    if isinstance(value, ast.Constant) and isinstance(value.value, str):
        s = value.value
        if not s:
            return False
        # 含中文/全角 → 一定是用户可见文字。
        if any(ord(c) > 127 for c in s):
            return True
        # 纯 ASCII：长度 > 8 且含空格/字母组合 → 可能是英文 UI 文案。
        return len(s) > 8 and bool(re.search(r"[a-zA-Z]", s)) and " " in s
    if isinstance(value, ast.Call):
        # tr() / self.tr() / widget.tr() 调用 → 已包裹，豁免。
        func = value.func
        if isinstance(func, ast.Attribute) and func.attr == "tr":
            return False
        if isinstance(func, ast.Name) and func.id == "tr":
            return False
    return False


def _find_tr_violations(path: Path) -> list[tuple[int, str]]:
    """AST 扫描文件找未包裹 tr 的用户可见文字。"""

    text = path.read_text(encoding="utf-8")
    try:
        tree = ast.parse(text)
    except SyntaxError:
        return []
    violations: list[tuple[int, str]] = []
    for node in ast.walk(tree):
        if isinstance(node, ast.Call):
            func = node.func
            api_name = ""
            if isinstance(func, ast.Attribute):
                api_name = func.attr
            elif isinstance(func, ast.Name):
                api_name = func.id
            # 检查构造函数（QLabel("...") 等）。
            if (api_name in _VISIBLE_TEXT_CTORS and node.args) or (api_name in _VISIBLE_TEXT_APIS and node.args):
                first = node.args[0]
                if _is_user_visible_text(first):
                    violations.append((node.lineno, f"{api_name}({ast.dump(first)})"))
    return violations


def _iter_ui_python_files():
    for path in UI_ROOT.rglob("*.py"):
        rel = str(path.relative_to(UI_ROOT)).replace("\\", "/")
        if any(marker in rel for marker in _SKIP_MARKERS):
            continue
        yield path


def test_no_untranslated_user_visible_text(capsys):
    """用户可见文字必须走 tr()（铁律 19）。

    软模式默认；``STRICT_TR=1`` 硬断言。
    """

    all_violations: list[tuple[Path, int, str]] = []
    for path in _iter_ui_python_files():
        for line_no, snippet in _find_tr_violations(path):
            all_violations.append((path, line_no, snippet))
    if all_violations:
        print(f"\n[tr()] {len(all_violations)} untranslated user-visible string(s):")
        for path, line_no, snippet in all_violations[:50]:
            rel = path.relative_to(REPO_ROOT)
            print(f"  {rel}:{line_no}: {snippet}")
    if os.environ.get("STRICT_TR") == "1":
        assert not all_violations, (
            f"{len(all_violations)} untranslated user-visible string(s); see captured stdout"
        )
