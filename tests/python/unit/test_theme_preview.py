"""settings_panel 主题实时预览测试。

覆盖：
1. settings_panel 构建后 combo 接了 currentIndexChanged → _preview_theme。
2. _preview_theme 应用主题 + 更新状态标签为「预览：深色/浅色」。
3. _preview_theme 静默（不弹 toast），_apply_theme 弹 toast（区分）。
4. _preview_theme 失败静默（apply_theme_by_name 抛异常不崩溃）。
5. combo 切换实际触发预览（端到端）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock


def _make_settings_panel(qtbot):
    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    panel = SettingsPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    return panel


# ── 接线断言 ───────────────────────────────────────────────────────
def test_settings_panel_wires_preview_on_combo_change():
    """settings_panel 源码应把 combo currentIndexChanged 接到 _preview_theme。"""

    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    src = inspect.getsource(SettingsPanel._build_theme_tab)
    assert "currentIndexChanged" in src
    assert "_preview_theme" in src


def test_settings_panel_has_preview_method():
    """SettingsPanel 应有 _preview_theme 方法。"""

    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    assert hasattr(SettingsPanel, "_preview_theme")
    assert callable(SettingsPanel._preview_theme)


# ── 预览行为 ───────────────────────────────────────────────────────
def test_preview_applies_theme_and_updates_status(qtbot, monkeypatch):
    """_preview_theme 应应用主题 + 状态标签更新为「预览：...」（不弹 toast）。"""

    apply_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.panels.settings_panel.apply_theme_by_name",
        lambda app, name: apply_calls.append(name),
    )
    panel = _make_settings_panel(qtbot)
    panel._theme_combo.setCurrentIndex(1)  # 切到浅色
    assert len(apply_calls) >= 1
    assert "预览" in panel._theme_status.text()


def test_preview_failure_silent(qtbot, monkeypatch):
    """_preview_theme apply 异常应静默（不崩溃，状态标签不变）。"""

    def _boom(*a, **k):
        raise RuntimeError("boom")

    monkeypatch.setattr(
        "embeddebug.serial_station.ui.panels.settings_panel.apply_theme_by_name",
        _boom,
    )
    panel = _make_settings_panel(qtbot)
    panel._theme_status.setText("before")
    panel._preview_theme(1)  # 不应抛异常
    # 预览失败时状态标签不应被改成「预览：...」。
    assert panel._theme_status.text() == "before"


# ── 预览 vs 应用区分 ───────────────────────────────────────────────
def test_preview_does_not_notify_but_apply_does(qtbot, monkeypatch):
    """预览不弹 toast，应用弹 toast（panel_notify 调用次数区分）。"""

    notify_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.panels._notify.panel_notify",
        lambda *a, **k: notify_calls.append(a),
    )
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.panels.settings_panel.apply_theme_by_name",
        lambda app, name: None,
    )
    panel = _make_settings_panel(qtbot)
    before = len(notify_calls)
    panel._preview_theme(1)
    assert len(notify_calls) == before  # 预览不弹
    panel._apply_theme()
    assert len(notify_calls) > before  # 应用弹


# ── combo 端到端 ───────────────────────────────────────────────────
def test_combo_change_triggers_preview(qtbot, monkeypatch):
    """切换 combo 选择应触发 _preview_theme（currentIndexChanged 信号端到端）。"""

    apply_calls: list = []
    monkeypatch.setattr(
        "embeddebug.serial_station.ui.panels.settings_panel.apply_theme_by_name",
        lambda app, name: apply_calls.append(name),
    )
    panel = _make_settings_panel(qtbot)
    panel._theme_combo.setCurrentIndex(1)
    # currentIndexChanged 应触发 _preview_theme → apply_theme_by_name 被调。
    assert any(call is not None for call in apply_calls)
