"""Badge 状态徽章控件单元测试。

覆盖：
- objectName == "serialStationBadge"。
- 默认 kind 为 INFO。
- text getter/setter 往返一致。
- set_kind 切换状态。
- KIND_COLORS 完整性（4 个 enum 值，每个 (bg, text) 2-tuple 非空字符串）。
- sizeHint 非零，高度恒为 24。
- sizeHint 宽度随文本变长而增长。
- paintEvent 四种 kind 不抛异常（offscreen 平台下 repaint 跑代码路径）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.ui.controls.badge import Badge, BadgeKind


def _make_badge(
    qtbot,
    kind: BadgeKind = BadgeKind.INFO,
    text: str = "hello",
) -> Badge:
    """构造一个加入 qtbot 的 Badge。"""

    badge = Badge(text=text, kind=kind)
    qtbot.addWidget(badge)
    return badge


# ── 基础属性 ────────────────────────────────────────────────────────
def test_badge_objectname(qtbot):
    badge = _make_badge(qtbot)
    assert badge.objectName() == "serialStationBadge"


def test_badge_default_kind_info(qtbot):
    badge = _make_badge(qtbot)
    assert badge.kind() == BadgeKind.INFO


def test_badge_set_kind_changes_state(qtbot):
    badge = _make_badge(qtbot)
    badge.set_kind(BadgeKind.ERROR)
    assert badge.kind() == BadgeKind.ERROR
    badge.set_kind(BadgeKind.WARNING)
    assert badge.kind() == BadgeKind.WARNING
    badge.set_kind(BadgeKind.SUCCESS)
    assert badge.kind() == BadgeKind.SUCCESS
    badge.set_kind(BadgeKind.INFO)
    assert badge.kind() == BadgeKind.INFO


def test_badge_text_getset_roundtrip(qtbot):
    badge = _make_badge(qtbot, text="initial")
    assert badge.text() == "initial"
    badge.set_text("updated")
    assert badge.text() == "updated"
    badge.set_text("")
    assert badge.text() == ""


# ── KIND_COLORS 完整性 ─────────────────────────────────────────────
def test_badge_kind_colors_all_defined(qtbot):
    kinds = {BadgeKind.INFO, BadgeKind.WARNING, BadgeKind.ERROR, BadgeKind.SUCCESS}
    assert set(Badge.KIND_COLORS.keys()) == kinds
    for k in kinds:
        entry = Badge.KIND_COLORS[k]
        assert isinstance(entry, tuple) and len(entry) == 2
        bg, text = entry
        assert isinstance(bg, str) and bg, f"bg color for {k} must be non-empty str"
        assert isinstance(text, str) and text, f"text color for {k} must be non-empty str"


# ── sizeHint ───────────────────────────────────────────────────────
def test_badge_size_hint_nonzero_height_24(qtbot):
    badge = _make_badge(qtbot, text="ok")
    hint = badge.sizeHint()
    assert hint.width() > 0
    assert hint.height() == 24


def test_badge_size_hint_empty_text_is_square(qtbot):
    """空文本时宽度回退到基线高度（24×24 正方形/正圆胶囊）。"""

    badge = _make_badge(qtbot, text="")
    hint = badge.sizeHint()
    assert hint.height() == 24
    assert hint.width() == 24


def test_badge_size_hint_grows_with_longer_text(qtbot):
    """更长的文本应产生更宽的 sizeHint。"""

    short = _make_badge(qtbot, text="AB")
    long = _make_badge(qtbot, text="ABCDEFGHIJKLMNOPQRSTUVWXYZ")
    w_short = short.sizeHint().width()
    w_long = long.sizeHint().width()
    assert w_long > w_short, (
        f"longer text must yield wider sizeHint: long={w_long} short={w_short}"
    )


def test_badge_size_hint_monotonic_with_text_length(qtbot):
    """同一字体下，sizeHint 宽度应随字符数单调不减。"""

    widths = [Badge(text=("X" * n)).sizeHint().width() for n in (1, 5, 10, 20, 40)]
    for prev, curr in zip(widths, widths[1:]):
        assert curr >= prev, f"width not monotonic: {widths}"


# ── paintEvent 不抛 ────────────────────────────────────────────────
@pytest.mark.parametrize(
    "kind",
    [BadgeKind.INFO, BadgeKind.WARNING, BadgeKind.ERROR, BadgeKind.SUCCESS],
)
def test_badge_paint_no_raise(qtbot, kind: BadgeKind):
    badge = _make_badge(qtbot, kind=kind, text=f"msg-{kind.value}")
    # offscreen 平台下 repaint 触发 paintEvent 代码路径但不真正绘制到屏幕；
    # 不抛即通过。
    badge.repaint()


def test_badge_paint_no_raise_empty_text(qtbot):
    """空文本也应在所有 kind 下安全绘制。"""

    for kind in (BadgeKind.INFO, BadgeKind.WARNING, BadgeKind.ERROR, BadgeKind.SUCCESS):
        badge = _make_badge(qtbot, kind=kind, text="")
        badge.repaint()


# ── set_text/set_kind 触发重绘不抛 ─────────────────────────────────
def test_badge_set_text_then_repaint_no_raise(qtbot):
    badge = _make_badge(qtbot, text="a")
    badge.set_text("much longer text after update")
    badge.repaint()


def test_badge_set_kind_then_repaint_no_raise(qtbot):
    badge = _make_badge(qtbot, text="status")
    for kind in (BadgeKind.WARNING, BadgeKind.ERROR, BadgeKind.SUCCESS, BadgeKind.INFO):
        badge.set_kind(kind)
        badge.repaint()
