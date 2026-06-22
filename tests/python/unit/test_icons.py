"""IconManager 单元测试 — SVG 加载、着色、缓存与按钮装饰。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtGui import QIcon
from PyQt6.QtWidgets import QPushButton, QWidget

from embeddebug.serial_station.ui import button_icons
from embeddebug.serial_station.ui.icons import IconManager, button_icon
from embeddebug.serial_station.ui.theme import palette as P


def test_icon_manager_is_singleton():
    a = IconManager()
    b = IconManager()
    assert a is b


def test_icon_manager_renders_existing_lucide_icon():
    manager = IconManager()
    manager.reset()
    icon = manager.icon("send")
    assert isinstance(icon, QIcon)
    assert not icon.isNull()


def test_icon_manager_caches_by_name_and_color():
    manager = IconManager()
    manager.reset()
    first = manager.icon("send", color=P.ACCENT)
    second = manager.icon("send", color=P.ACCENT)
    assert first is second
    different = manager.icon("send", color=P.ERROR)
    assert different is not first


def test_icon_manager_cache_key_includes_pixels():
    """缓存键必须含 pixels 维度，否则不同尺寸互相命中返回错误尺寸。

    Batch 3 修复：旧版缓存键只有 (name, color)，18px 和 24px 的 send 图标
    会互相命中缓存，返回错误尺寸的 QIcon。
    """

    manager = IconManager()
    manager.reset()
    small = manager.icon("send", color=P.ACCENT, pixels=18)
    big = manager.icon("send", color=P.ACCENT, pixels=24)
    # 不同 pixels 应是不同缓存条目（不同 QIcon 对象）。
    assert small is not big


def test_tint_replaces_all_strokes_for_multi_path_icons():
    """多 path 图标的所有 stroke 属性都应被着色（旧版 count=1 只换第一个）。

    Batch 3 修复：lucide 多笔画图标（如 'activity' 有多个 path），
    旧版 re.sub count=1 只替换第一个 stroke，后续 path 仍是原色。
    """

    svg = (
        '<svg stroke="#000">'
        '<path d="M1"/><path d="M2" stroke="#fff"/>'
        '<circle stroke="#aaa"/>'
        '</svg>'
    )
    tinted = IconManager._tint(svg, "#abcdef")
    # 所有 stroke 属性都应变成目标色。
    assert tinted.count('stroke="#abcdef"') == 3
    assert "#000" not in tinted
    assert "#fff" not in tinted
    assert "#aaa" not in tinted


def test_icon_manager_returns_empty_icon_for_missing_name():
    manager = IconManager()
    manager.reset()
    icon = manager.icon("does-not-exist-xyz")
    assert isinstance(icon, QIcon)
    assert icon.isNull()


def test_button_icon_convenience_returns_qicon():
    icon = button_icon("plug")
    assert isinstance(icon, QIcon)


def test_tint_replaces_current_color():
    svg = '<svg stroke="currentColor"><path/></svg>'
    tinted = IconManager._tint(svg, "#abcdef")
    assert "currentColor" not in tinted
    assert "#abcdef" in tinted


def test_apply_button_icons_decorates_known_buttons(qtbot):
    button = QPushButton("Connect")
    button.setObjectName("serialStationConnectButton")
    parent = QWidget()
    parent.setObjectName("embeddebugPySerialStationWindow")
    button.setParent(parent)
    qtbot.addWidget(parent)

    count = button_icons.apply_button_icons(parent)
    assert count >= 1
    assert not button.icon().isNull()


def test_apply_button_icons_skips_missing_buttons(qtbot):
    parent = QWidget()
    parent.setObjectName("embeddebugPySerialStationWindow")
    qtbot.addWidget(parent)

    count = button_icons.apply_button_icons(parent)
    assert count == 0


# ── Batch 7-3: apply_focus_rings 全局输入框 focus_ring 接线 ─────────
def test_apply_focus_rings_decorates_line_edits(qtbot):
    """apply_focus_rings 应为所有 QLineEdit 装 focus_ring（激活死代码）。"""

    from PyQt6.QtWidgets import QLineEdit

    parent = QWidget()
    qtbot.addWidget(parent)
    le1 = QLineEdit(parent)
    le1.setObjectName("serialStationLogSearchEdit")
    le2 = QLineEdit(parent)
    le2.setObjectName("serialStationLogPathEdit")
    count = button_icons.apply_focus_rings(parent)
    assert count >= 2
    # 装 focus_ring 后控件应有 graphicsEffect。
    assert le1.graphicsEffect() is not None
    assert le2.graphicsEffect() is not None


def test_apply_focus_rings_skips_readonly(qtbot):
    """只读控件不应装 focus_ring（对只读无意义）。"""

    from PyQt6.QtWidgets import QLineEdit

    parent = QWidget()
    qtbot.addWidget(parent)
    readonly_le = QLineEdit(parent)
    readonly_le.setReadOnly(True)
    button_icons.apply_focus_rings(parent)
    # 只读控件应被跳过。
    assert readonly_le.graphicsEffect() is None


def test_apply_focus_rings_handles_combos(qtbot):
    """QComboBox 也应被装 focus_ring。"""

    from PyQt6.QtWidgets import QComboBox

    parent = QWidget()
    qtbot.addWidget(parent)
    QComboBox(parent)
    count = button_icons.apply_focus_rings(parent)
    assert count >= 1


def test_apply_focus_rings_returns_zero_on_no_inputs(qtbot):
    """无输入控件时返回 0。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    count = button_icons.apply_focus_rings(parent)
    assert count == 0
