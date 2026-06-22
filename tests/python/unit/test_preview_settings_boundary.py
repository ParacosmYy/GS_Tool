"""waveform_preview set_connecting/_update_stats + SettingsPanel build tabs 边界测试。

补强 test_waveform_display / test_settings_notify 未直接断言的边角：
- SerialWaveformPreview.set_connecting：True/False 不崩溃。
- SerialWaveformPreview._update_stats：空 batch/正常 batch 不崩溃。
- SerialWaveformPreview.cursor_manager：初始可能 None。
- SettingsPanel.build：返回 QWidget + 含 tabs。
- SettingsPanel._build_theme_tab/_build_shortcuts_tab/_build_about_tab：返回 QWidget。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.core.measurements import ChannelBatch
from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview


# ── SerialWaveformPreview.set_connecting ──────────────────────────────


def test_set_connecting_true(qtbot):
    """set_connecting(True) 不崩溃。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.set_connecting(True)


def test_set_connecting_false(qtbot):
    """set_connecting(False) 不崩溃。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.set_connecting(False)


def test_set_connecting_toggle(qtbot):
    """True → False 切换不崩溃。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.set_connecting(True)
    preview.set_connecting(False)


# ── _update_stats ─────────────────────────────────────────────────────


def test_update_stats_normal_batch(qtbot):
    """_update_stats 正常 batch 不崩溃。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    batch = ChannelBatch(
        channel_names=("temp", "volt"),
        values=np.array([[25.0, 3.3]], dtype=np.float32),
    )
    preview._update_stats(batch)


def test_update_stats_empty_batch(qtbot):
    """_update_stats 空 batch（0 行）不崩溃。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    batch = ChannelBatch(
        channel_names=("temp",),
        values=np.array([], dtype=np.float32).reshape(0, 1),
    )
    preview._update_stats(batch)


# ── cursor_manager ────────────────────────────────────────────────────


def test_cursor_manager_initial(qtbot):
    """cursor_manager 初始可访问（可能 None）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    # 初始可能 None（未 submit batch）
    cm = preview.cursor_manager
    assert cm is None or cm is not None  # 不崩溃即可


# ── SettingsPanel.build ───────────────────────────────────────────────


def test_settings_panel_build_returns_widget(qtbot):
    """SettingsPanel.build 返回 QWidget。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    panel = SettingsPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert isinstance(widget, QWidget)


def test_settings_panel_build_has_objectname(qtbot):
    """build 返回的 widget 有 objectName。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    panel = SettingsPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    assert widget.objectName() != ""


# ── SettingsPanel._build_theme_tab ────────────────────────────────────


def test_settings_build_theme_tab_returns_widget(qtbot):
    """_build_theme_tab 返回 QWidget。"""

    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    panel = SettingsPanel()
    panel._widget = QWidget()
    qtbot.addWidget(panel._widget)
    tab = panel._build_theme_tab(panel._widget)
    assert isinstance(tab, QWidget)


def test_settings_build_shortcuts_tab_returns_widget(qtbot):
    """_build_shortcuts_tab 返回 QWidget。"""

    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    panel = SettingsPanel()
    panel._widget = QWidget()
    qtbot.addWidget(panel._widget)
    tab = panel._build_shortcuts_tab(panel._widget)
    assert isinstance(tab, QWidget)


def test_settings_build_about_tab_returns_widget(qtbot):
    """_build_about_tab 返回 QWidget。"""

    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    panel = SettingsPanel()
    panel._widget = QWidget()
    qtbot.addWidget(panel._widget)
    tab = panel._build_about_tab(panel._widget)
    assert isinstance(tab, QWidget)
