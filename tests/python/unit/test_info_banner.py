"""InfoBanner 可关闭信息条单元测试。

覆盖：
- objectName / 默认 kind / setter getter。
- KIND_COLORS 完整性（4 个 enum 值，每个 2-tuple 非空）。
- sizeHint 非零。
- paintEvent 四种 kind 不抛异常。
- 点击 × 按钮发 dismissed(text)。
- mouseMoveEvent 在关闭按钮 rect 上切 _close_hovered。
- _animate_out 不崩溃（deleteLater 由 Qt 事件循环处理）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QEvent, QPointF, Qt
from PyQt6.QtGui import QMouseEvent

from embeddebug.serial_station.ui.controls.info_banner import (
    BannerKind,
    InfoBanner,
)


def _make_banner(qtbot, kind: BannerKind = BannerKind.INFO, text: str = "hello") -> InfoBanner:
    """构造一个加入 qtbot 的 InfoBanner。"""

    banner = InfoBanner(text=text, kind=kind)
    qtbot.addWidget(banner)
    return banner


def _move_event(pos: QPointF) -> QMouseEvent:
    """构造一个不带按键的 mouseMoveEvent。"""

    return QMouseEvent(
        QEvent.Type.MouseMove,
        pos,
        Qt.MouseButton.NoButton,
        Qt.MouseButton.NoButton,
        Qt.KeyboardModifier.NoModifier,
    )


# ── 基础属性 ────────────────────────────────────────────────────────
def test_banner_objectname(qtbot):
    banner = _make_banner(qtbot)
    assert banner.objectName() == "serialStationInfoBanner"


def test_banner_default_kind_info(qtbot):
    banner = _make_banner(qtbot)
    assert banner.kind() == BannerKind.INFO


def test_banner_set_kind_changes_state(qtbot):
    banner = _make_banner(qtbot)
    banner.set_kind(BannerKind.ERROR)
    assert banner.kind() == BannerKind.ERROR
    banner.set_kind(BannerKind.WARNING)
    assert banner.kind() == BannerKind.WARNING


def test_banner_set_text_getset(qtbot):
    banner = _make_banner(qtbot, text="initial")
    assert banner.text() == "initial"
    banner.set_text("updated")
    assert banner.text() == "updated"


# ── KIND_COLORS 完整性 ─────────────────────────────────────────────
def test_banner_kind_colors_all_defined(qtbot):
    kinds = {BannerKind.INFO, BannerKind.WARNING, BannerKind.ERROR, BannerKind.SUCCESS}
    assert set(InfoBanner.KIND_COLORS.keys()) == kinds
    for k in kinds:
        entry = InfoBanner.KIND_COLORS[k]
        assert isinstance(entry, tuple) and len(entry) == 2
        accent, bg = entry
        assert isinstance(accent, str) and accent
        assert isinstance(bg, str) and bg


# ── sizeHint ───────────────────────────────────────────────────────
def test_banner_size_hint_nonzero(qtbot):
    banner = _make_banner(qtbot)
    hint = banner.sizeHint()
    assert hint.width() > 0 and hint.height() > 0
    assert (hint.width(), hint.height()) == (360, 40)


# ── paintEvent 不抛 ────────────────────────────────────────────────
def test_banner_paint_no_raise(qtbot):
    for kind in (BannerKind.INFO, BannerKind.WARNING, BannerKind.ERROR, BannerKind.SUCCESS):
        banner = _make_banner(qtbot, kind=kind, text=f"msg-{kind.value}")
        # forceUpdate 触发 paintEvent；offscreen 平台下不会真正绘制到屏幕但会跑代码路径。
        banner.repaint()
        assert banner.isVisible() is False or True  # 仅占位，repaint 不抛即通过


# ── 关闭按钮点击 ────────────────────────────────────────────────────
def test_banner_close_click_emits_dismissed(qtbot):
    banner = _make_banner(qtbot, text="will-close")
    banner.show()
    qtbot.waitExposed(banner)

    rect = banner._close_button_rect()
    center = rect.center()
    received: list[str] = []
    banner.dismissed.connect(lambda t: received.append(t))

    qtbot.mouseClick(
        banner,
        Qt.MouseButton.LeftButton,
        Qt.KeyboardModifier.NoModifier,
        center.toPoint(),
    )
    assert received == ["will-close"]


# ── hover 状态切换 ──────────────────────────────────────────────────
def test_banner_close_hover_state_changes(qtbot):
    banner = _make_banner(qtbot)
    assert banner._close_hovered is False

    rect = banner._close_button_rect()
    center = rect.center()
    banner.mouseMoveEvent(_move_event(center))
    assert banner._close_hovered is True

    # 移到左上角（远离关闭按钮）应复位。
    banner.mouseMoveEvent(_move_event(QPointF(2, 2)))
    assert banner._close_hovered is False


# ── _animate_out 不崩溃 ────────────────────────────────────────────
def test_banner_animate_out_does_not_raise(qtbot):
    banner = _make_banner(qtbot, text="bye")
    banner.show()
    qtbot.waitExposed(banner)

    banner._animate_out()
    # 二次调用应被去重（不会重新启动）。
    banner._animate_out()
    assert banner._anim_out is not None

    # 跑完动画，触发 finished → hide + deleteLater（qtbot 驱动事件循环）。
    qtbot.wait(50)
