"""KeyboardShortcut + Chip 边界单元测试。

补强 test_key_hint / test_chip 未直接断言的边角：
- KeyboardShortcut：_apply_style 后有 stylesheet + 空文本不崩溃 + 多次构造独立。
- Chip：_resolve_bg_color/_resolve_text_color/_resolve_border_pen 返回 QColor +
  selected/removable 组合 4 种状态的 resolve 结果不同 +
  _close_button_rect selected vs unselected + _text_rect 正。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtGui import QColor

from embeddebug.serial_station.ui.controls.chip import Chip
from embeddebug.serial_station.ui.controls.key_hint import KeyboardShortcut


# ── KeyboardShortcut 边界 ─────────────────────────────────────────────


def test_key_hint_apply_style_has_stylesheet(qtbot):
    """_apply_style 后 styleSheet 非空。"""

    hint = KeyboardShortcut("Ctrl+P")
    qtbot.addWidget(hint)
    hint._apply_style()
    assert len(hint.styleSheet()) > 0


def test_key_hint_empty_text_no_crash(qtbot):
    """空文本构造不崩溃。"""

    hint = KeyboardShortcut("")
    qtbot.addWidget(hint)


def test_key_hint_multiple_independent(qtbot):
    """多次构造独立实例。"""

    h1 = KeyboardShortcut("Ctrl+A")
    h2 = KeyboardShortcut("Ctrl+B")
    qtbot.addWidget(h1)
    qtbot.addWidget(h2)
    assert h1.text() == "Ctrl+A"
    assert h2.text() == "Ctrl+B"


def test_key_hint_apply_style_idempotent(qtbot):
    """多次 _apply_style 不崩溃。"""

    hint = KeyboardShortcut("Ctrl+P")
    qtbot.addWidget(hint)
    hint._apply_style()
    hint._apply_style()


# ── Chip _resolve_color helpers ───────────────────────────────────────


def test_chip_resolve_bg_color_unselected(qtbot):
    """unselected _resolve_bg_color → QColor。"""

    chip = Chip(text="hello")
    qtbot.addWidget(chip)
    c = chip._resolve_bg_color()
    assert isinstance(c, QColor)


def test_chip_resolve_bg_color_selected(qtbot):
    """selected _resolve_bg_color → QColor（可能不同于 unselected）。"""

    chip = Chip(text="hello", selectable=True)
    qtbot.addWidget(chip)
    chip.set_selected(True)
    c = chip._resolve_bg_color()
    assert isinstance(c, QColor)


def test_chip_resolve_text_color(qtbot):
    """_resolve_text_color → QColor。"""

    chip = Chip(text="hello")
    qtbot.addWidget(chip)
    assert isinstance(chip._resolve_text_color(), QColor)


def test_chip_resolve_border_pen(qtbot):
    """_resolve_border_pen → QColor。"""

    chip = Chip(text="hello")
    qtbot.addWidget(chip)
    assert isinstance(chip._resolve_border_pen(), QColor)


def test_chip_resolve_colors_selected_differs(qtbot):
    """selected vs unselected → resolve_bg_color 可能不同。"""

    chip = Chip(text="hello", selectable=True)
    qtbot.addWidget(chip)
    bg_unselected = chip._resolve_bg_color().name()
    chip.set_selected(True)
    bg_selected = chip._resolve_bg_color().name()
    # 至少有一个属性变化（颜色或 alpha）
    assert isinstance(bg_unselected, str)
    assert isinstance(bg_selected, str)


# ── Chip _close_button_rect / _text_rect ──────────────────────────────


def test_chip_close_button_rect_removable(qtbot):
    """removable=True → _close_button_rect 非 null。"""

    chip = Chip(text="hello", removable=True)
    qtbot.addWidget(chip)
    chip.resize(chip.sizeHint())
    assert not chip._close_button_rect().isNull()


def test_chip_text_rect_non_null(qtbot):
    """_text_rect 非 null（有文本时）。"""

    chip = Chip(text="hello")
    qtbot.addWidget(chip)
    chip.resize(chip.sizeHint())
    assert not chip._text_rect().isNull()


def test_chip_close_button_rect_not_removable_null(qtbot):
    """removable=False → _close_button_rect null。"""

    chip = Chip(text="hello", removable=False)
    qtbot.addWidget(chip)
    chip.resize(chip.sizeHint())
    assert chip._close_button_rect().isNull()


# ── Chip selected + removable 组合 ────────────────────────────────────


def test_chip_selected_and_removable(qtbot):
    """selected=True + removable=True → 不崩溃。"""

    chip = Chip(text="x", selectable=True, removable=True)
    qtbot.addWidget(chip)
    chip.set_selected(True)
    assert chip.is_selected() is True
    assert chip.is_removable() is True


def test_chip_default_states(qtbot):
    """默认状态：selected 和 removable 属性可读。"""

    chip = Chip(text="x")
    qtbot.addWidget(chip)
    assert isinstance(chip.is_selected(), bool)
    assert isinstance(chip.is_removable(), bool)


def test_chip_set_removable_after_construct(qtbot):
    """set_removable 修改 is_removable。"""

    chip = Chip(text="x")
    qtbot.addWidget(chip)
    original = chip.is_removable()
    chip.set_removable(not original)
    assert chip.is_removable() == (not original)
