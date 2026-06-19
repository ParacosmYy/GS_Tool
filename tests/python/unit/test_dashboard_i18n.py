"""Batch 39 测试：dashboard 用户可见文字国际化（tr() 合规，铁律 19）。

覆盖：
1. WidgetPaletteButton tooltip 走 tr()（原硬编码 "Drag to canvas to add"）。
2. WidgetPalette 标题 "Widgets" 走 tr()。
3. DashboardTabs 默认标签页名走 tr()（原 f"Dashboard {count}"）。
4. 源码无残留硬编码英文 f-string 用户可见文字。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


def test_palette_button_tooltip_uses_tr(qtbot):
    """WidgetPaletteButton tooltip 应经 tr()（i18n 合规）。"""

    from embeddebug.serial_station.ui.dashboard.palette import WidgetPaletteButton

    src = inspect.getsource(WidgetPaletteButton.__init__)
    assert "self.tr(" in src
    assert "Drag to canvas to add {name}" in src
    # 不应再有硬编码 f"...Drag..."。
    assert 'f"Drag to canvas' not in src
    assert "f'Drag to canvas" not in src


def test_palette_button_tooltip_rendered(qtbot):
    """tooltip 实际渲染应含控件名。"""

    from embeddebug.serial_station.ui.dashboard.palette import WidgetPaletteButton

    btn = WidgetPaletteButton("led", "LED", "circle")
    qtbot.addWidget(btn)
    tip = btn.toolTip()
    assert "LED" in tip


def test_palette_title_uses_tr(qtbot):
    """WidgetPalette 标题 "Widgets" 应经 tr()。"""

    from embeddebug.serial_station.ui.dashboard import WidgetPalette

    src = inspect.getsource(WidgetPalette.__init__)
    assert 'self.tr("Widgets")' in src
    assert 'QLabel("Widgets"' not in src  # 不应硬编码


def test_palette_title_rendered(qtbot):
    """WidgetPalette 应渲染标题。"""

    from embeddebug.serial_station.ui.dashboard import WidgetPalette

    palette = WidgetPalette()
    qtbot.addWidget(palette)
    from PyQt6.QtWidgets import QLabel

    title = palette.findChild(QLabel, "serialStationPaletteTitle")
    assert title is not None
    assert title.text() == "Widgets"  # 默认未翻译仍是 Widgets


def test_tabs_default_name_uses_tr(qtbot):
    """DashboardTabs._default_tab_name 应经 tr()（原 f"Dashboard {count}"）。"""

    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    src = inspect.getsource(DashboardTabs._default_tab_name)
    assert "self.tr(" in src
    # 不应再有硬编码 f-string。
    assert 'f"Dashboard' not in src


def test_tabs_default_name_rendered(qtbot):
    """默认标签页名应含 "Dashboard"。"""

    from embeddebug.serial_station.ui.dashboard import DashboardTabs

    tabs = DashboardTabs()
    qtbot.addWidget(tabs)
    name = tabs.tabText(0)
    assert "Dashboard" in name


# ── 无残留硬编码英文用户可见文字 ───────────────────────────────────
def test_no_hardcoded_drag_tooltip():
    """palette.py 源码不应再有硬编码 'Drag to canvas' f-string。"""

    from pathlib import Path

    src = Path("python/embeddebug/serial_station/ui/dashboard/palette.py").read_text(encoding="utf-8")
    # tr() 包裹的 template 允许；裸 f"Drag..." 不允许。
    assert 'f"Drag to canvas' not in src
    assert "f'Drag to canvas" not in src
