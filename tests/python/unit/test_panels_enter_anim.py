"""域面板 stagger 入场动画接线测试。

覆盖诊断报告剩余项『动画死代码全没接』（7 个域面板未接 stagger）。
验证：
1. _enter_anim helper 的 play/stop 正确驱动 card_enter。
2. 6 个域面板 on_enter 调用了 play_panel_enter（源码级断言，避免实例化复杂面板）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import inspect

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.panels._enter_anim import play_panel_enter, stop_panel_enter


class _FakePanel:
    """模拟域面板（有 _widget + _enter_anims）。"""

    def __init__(self) -> None:
        self._widget = QWidget()
        self._enter_anims: list = []


def test_play_panel_enter_starts_animation(qtbot):
    """play_panel_enter 应对 _widget 启动 card_enter 动画。"""

    panel = _FakePanel()
    qtbot.addWidget(panel._widget)
    play_panel_enter(panel)
    assert len(panel._enter_anims) > 0


def test_stop_panel_enter_stops_animation(qtbot):
    """stop_panel_enter 应停止并清空 _enter_anims。"""

    panel = _FakePanel()
    qtbot.addWidget(panel._widget)
    play_panel_enter(panel)
    assert len(panel._enter_anims) > 0
    stop_panel_enter(panel)
    assert panel._enter_anims == []


def test_play_panel_enter_no_widget_does_not_crash():
    """_widget 为 None 时不崩溃（helper 健壮性）。"""

    class _NoWidget:
        _widget = None
        _enter_anims = []

    panel = _NoWidget()
    play_panel_enter(panel)  # 不应抛异常
    assert panel._enter_anims == []


# ── 各域面板 on_enter/on_leave 源码级接入断言 ──────────────────────
def _panel_on_enter_calls_play(panel_cls) -> bool:
    """检查面板类的 on_enter 源码是否调用了 play_panel_enter。"""

    src = inspect.getsource(panel_cls.on_enter)
    return "play_panel_enter" in src


def _panel_on_leave_calls_stop(panel_cls) -> bool:
    """检查面板类的 on_leave 源码是否调用了 stop_panel_enter。"""

    src = inspect.getsource(panel_cls.on_leave)
    return "stop_panel_enter" in src


def test_ble_panel_on_enter_wired():
    from embeddebug.serial_station.ui.panels.ble_panel import BlePanel

    assert _panel_on_enter_calls_play(BlePanel)
    assert _panel_on_leave_calls_stop(BlePanel)


def test_can_panel_on_enter_wired():
    from embeddebug.serial_station.ui.panels.can_panel import CanPanel

    assert _panel_on_enter_calls_play(CanPanel)
    assert _panel_on_leave_calls_stop(CanPanel)


def test_rtt_panel_on_enter_wired():
    from embeddebug.serial_station.ui.panels.rtt_panel import RttPanel

    assert _panel_on_enter_calls_play(RttPanel)
    assert _panel_on_leave_calls_stop(RttPanel)


def test_ota_panel_on_enter_wired():
    from embeddebug.serial_station.ui.panels.ota_panel import OtaPanel

    assert _panel_on_enter_calls_play(OtaPanel)
    assert _panel_on_leave_calls_stop(OtaPanel)


def test_automation_panel_on_enter_wired():
    from embeddebug.serial_station.ui.panels.automation_panel import AutomationPanel

    assert _panel_on_enter_calls_play(AutomationPanel)
    assert _panel_on_leave_calls_stop(AutomationPanel)


def test_settings_panel_on_enter_wired():
    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    assert _panel_on_enter_calls_play(SettingsPanel)
    assert _panel_on_leave_calls_stop(SettingsPanel)
