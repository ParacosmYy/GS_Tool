"""StatusDot 控件 + PulseAnimation 死代码激活测试。

覆盖：
1. StatusDot 控件自绘 + objectName 合规 + 状态切换。
2. PulseAnimation.breathing 在活动态（GREEN/BLUE）启动、非活动态停止。
3. set_active 便捷切换、set_breathing 手动开关。
4. QSS 覆盖 serialStationStatusDot。
5. 域面板状态指示符接入（OTA/RTT/Automation）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.ui.animations.pulse import PulseAnimation
from embeddebug.serial_station.ui.controls import DotState, StatusDot


@pytest.fixture(autouse=True)
def _reset_pulse_active():
    """每个测试前后清空 PulseAnimation._active，避免跨测试动画残留污染断言。"""

    saved = list(PulseAnimation._active)
    PulseAnimation._active.clear()
    yield
    PulseAnimation._active.clear()
    PulseAnimation._active.extend(saved)


# ── StatusDot 控件基础 ─────────────────────────────────────────────
def test_status_dot_has_objectname(qtbot):
    dot = StatusDot()
    qtbot.addWidget(dot)
    assert dot.objectName() == "serialStationStatusDot"


def test_status_dot_default_state_off(qtbot):
    dot = StatusDot()
    qtbot.addWidget(dot)
    assert dot.state == DotState.OFF


def test_status_dot_fixed_size_matches_diameter(qtbot):
    dot = StatusDot(diameter=12)
    qtbot.addWidget(dot)
    assert dot.width() == 12
    assert dot.height() == 12


def test_status_dot_set_state_changes(qtbot):
    dot = StatusDot()
    qtbot.addWidget(dot)
    dot.set_state(DotState.GREEN)
    assert dot.state == DotState.GREEN
    dot.set_state(DotState.RED)
    assert dot.state == DotState.RED


def test_status_dot_same_state_no_restart(qtbot):
    """同状态重复 set_state 不应重置（无动画抖动）。"""

    dot = StatusDot()
    qtbot.addWidget(dot)
    dot.set_state(DotState.GREEN)
    anim_before = dot._breathing_anim
    dot.set_state(DotState.GREEN)
    # 同状态应早返回，breathing 引用不变（未重建）。
    assert dot._breathing_anim is anim_before


def test_status_dot_set_active_toggle(qtbot):
    dot = StatusDot()
    qtbot.addWidget(dot)
    dot.set_active(True)
    assert dot.state == DotState.GREEN
    dot.set_active(False)
    assert dot.state == DotState.OFF


# ── PulseAnimation 死代码激活 ──────────────────────────────────────
def test_status_dot_green_starts_breathing(qtbot):
    """GREEN 态应启动 PulseAnimation.breathing（激活死代码）。"""

    dot = StatusDot()
    qtbot.addWidget(dot)
    dot.set_state(DotState.GREEN)
    assert dot._breathing_anim is not None
    assert dot._breathing_anim.loopCount() == -1  # 无限呼吸循环
    # PulseAnimation._active 应含该动画（注册成功）。
    assert dot._breathing_anim in PulseAnimation._active


def test_status_dot_blue_starts_breathing(qtbot):
    """BLUE 态（活动）也应呼吸。"""

    dot = StatusDot()
    qtbot.addWidget(dot)
    dot.set_state(DotState.BLUE)
    assert dot._breathing_anim is not None


def test_status_dot_off_yellow_red_no_breathing(qtbot):
    """非活动态（OFF/YELLOW/RED）不应启动呼吸，避免干扰。"""

    dot = StatusDot()
    qtbot.addWidget(dot)
    for state in (DotState.OFF, DotState.YELLOW, DotState.RED):
        dot.set_state(state)
        assert dot._breathing_anim is None, f"{state} should not breathe"


def test_status_dot_stop_breathing_on_deactivate(qtbot):
    """从 GREEN 切到 OFF 应停止呼吸并清理 PulseAnimation._active。"""

    dot = StatusDot()
    qtbot.addWidget(dot)
    dot.set_state(DotState.GREEN)
    anim = dot._breathing_anim
    assert anim in PulseAnimation._active
    dot.set_state(DotState.OFF)
    assert dot._breathing_anim is None
    # stop_looping 应已从 _active 移除。
    assert anim not in PulseAnimation._active


def test_status_dot_set_breathing_false_stops(qtbot):
    """set_breathing(False) 应停止呼吸即使仍为 GREEN。"""

    dot = StatusDot()
    qtbot.addWidget(dot)
    dot.set_state(DotState.GREEN)
    assert dot._breathing_anim is not None
    dot.set_breathing(False)
    assert dot._breathing_anim is None


def test_status_dot_set_breathing_false_on_init_no_anim(qtbot):
    """构造时 breathing=False，切到 GREEN 也不呼吸。"""

    dot = StatusDot(breathing=False)
    qtbot.addWidget(dot)
    dot.set_state(DotState.GREEN)
    assert dot._breathing_anim is None


# ── QSS 覆盖 ───────────────────────────────────────────────────────
def test_qss_covers_status_dot_objectname():
    """build_qss 应含 serialStationStatusDot 选择器。"""

    from embeddebug.serial_station.ui.theme.qss_builder import build_qss

    assert "#serialStationStatusDot" in build_qss()


# ── PulseAnimation 死代码接入断言 ──────────────────────────────────
def test_pulse_animation_module_used_by_status_dot():
    """status_dot 源码应引用 PulseAnimation.breathing / stop_looping。"""

    from embeddebug.serial_station.ui.controls import status_dot

    src = inspect.getsource(status_dot)
    assert "PulseAnimation.breathing" in src
    assert "PulseAnimation.stop_looping" in src


# ── Batch 10-2: 域面板状态指示符接入（源码级断言） ─────────────────
def test_ota_panel_wired_to_status_dot():
    """OTA 面板应含 _status_dot，且 _refresh_connection_state 驱动它。"""

    from embeddebug.serial_station.ui.panels.ota_panel import OtaPanel

    src = inspect.getsource(OtaPanel)
    assert "_status_dot" in src
    assert "StatusDot" in src
    assert "DotState.GREEN" in src  # 已连接 → GREEN
    assert "DotState.OFF" in src    # 未连接 → OFF


def test_rtt_panel_wired_to_status_dot():
    """RTT 面板应含 _status_dot，_start/_stop 驱动它（GREEN/BLUE/OFF）。"""

    from embeddebug.serial_station.ui.panels.rtt_panel import RttPanel

    src = inspect.getsource(RttPanel)
    assert "_status_dot" in src
    assert "StatusDot" in src
    assert "DotState.GREEN" in src  # 串口运行
    assert "DotState.BLUE" in src   # 演示运行
    assert "DotState.OFF" in src    # 停止


def test_automation_panel_wired_to_status_dot():
    """Automation 面板应含 _status_dot，_set_active 驱动它（GREEN/OFF）。"""

    from embeddebug.serial_station.ui.panels.automation_panel import AutomationPanel

    src = inspect.getsource(AutomationPanel)
    assert "_status_dot" in src
    assert "StatusDot" in src
    assert "DotState.GREEN" in src  # 监听中
    assert "DotState.OFF" in src    # 未启用


def test_ble_panel_wired_to_status_dot():
    """BLE 面板应含 _status_dot，_connect 驱动它（连接 GREEN/断开 OFF）。"""

    from embeddebug.serial_station.ui.panels.ble_panel import BlePanel

    src = inspect.getsource(BlePanel)
    assert "_status_dot" in src
    assert "StatusDot" in src
    assert "DotState.GREEN" in src  # 已连接
    assert "DotState.OFF" in src    # 已断开


def test_panels_no_longer_use_status_glyphs_in_text():
    """Batch 10-2 后，域面板状态文字不应再用 ●/○ 字符（圆点承担视觉）。"""

    from embeddebug.serial_station.ui.panels.automation_panel import AutomationPanel
    from embeddebug.serial_station.ui.panels.ota_panel import OtaPanel
    from embeddebug.serial_station.ui.panels.rtt_panel import RttPanel

    for panel_cls in (OtaPanel, RttPanel, AutomationPanel):
        src = inspect.getsource(panel_cls)
        # 状态文字里不应残留 ●/○（被 StatusDot 取代）。
        assert "● 监听中" not in src, f"{panel_cls.__name__} 仍用 ● 字符"
        assert "○ 监听未启用" not in src
        assert "● 运行中" not in src
        assert "● 已连接" not in src
        assert "○ 未连接" not in src
