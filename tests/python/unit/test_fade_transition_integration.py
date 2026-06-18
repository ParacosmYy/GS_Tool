"""Batch 11 测试：EmptyStateWidget.show_with_fade 激活 FadeTransition 死代码。

覆盖：
1. show_with_fade 创建并启动 FadeTransition.fade_in 动画。
2. 连续调用 show_with_fade 先停旧动画（不叠加）。
3. 域面板（CAN/RTT/BLE）空状态恢复路径调用 show_with_fade（源码级断言）。
4. FadeTransition 模块被 empty_state 引用（死代码接入断言）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.widgets import EmptyStateWidget


# ── show_with_fade 行为 ────────────────────────────────────────────
def test_show_with_fade_starts_animation(qtbot):
    """show_with_fade 应创建并启动 fade_in 动画。"""

    w = EmptyStateWidget(title="空", description="无数据")
    qtbot.addWidget(w)
    w.show_with_fade()
    assert getattr(w, "_fade_anim", None) is not None
    # 动画应处于运行态。
    assert w._fade_anim.state() == w._fade_anim.State.Running


def test_show_with_fade_stops_previous(qtbot):
    """连续 show_with_fade 应先停旧动画再启新动画（不叠加）。"""

    w = EmptyStateWidget(title="空", description="无数据")
    qtbot.addWidget(w)
    w.show_with_fade()
    first = w._fade_anim
    w.show_with_fade()
    second = w._fade_anim
    # 引用应被替换（新动画），旧的已停。
    assert second is not first


def test_show_with_fade_custom_duration(qtbot):
    """自定义 duration 应透传到 fade_in。"""

    w = EmptyStateWidget(title="空", description="无数据")
    qtbot.addWidget(w)
    w.show_with_fade(duration=500)
    assert w._fade_anim.duration() == 500


def test_show_with_fade_uses_fade_transition(qtbot):
    """show_with_fade 应实际调用 FadeTransition.fade_in（激活死代码）。"""

    from unittest.mock import MagicMock

    import embeddebug.serial_station.ui.animations.fade as fade_mod

    original = fade_mod.FadeTransition.fade_in
    calls: list = []
    fake_anim = MagicMock()
    fake_anim.start = MagicMock()
    fake_anim.stop = MagicMock()
    try:
        fade_mod.FadeTransition.fade_in = staticmethod(lambda *a, **k: (calls.append(k) or fake_anim))
        w = EmptyStateWidget(title="空", description="无数据")
        qtbot.addWidget(w)
        w.show_with_fade()
        assert len(calls) == 1  # fade_in 被调用一次
    finally:
        fade_mod.FadeTransition.fade_in = original


# ── 死代码接入断言 ─────────────────────────────────────────────────
def test_empty_state_imports_fade_transition():
    """empty_state 源码应引用 FadeTransition（死代码接入路径）。"""

    from embeddebug.serial_station.ui.widgets import empty_state

    src = inspect.getsource(empty_state)
    assert "FadeTransition" in src
    assert "fade_in" in src
    assert "show_with_fade" in src


# ── 域面板恢复路径接入（源码级断言） ───────────────────────────────
def test_can_panel_clear_uses_show_with_fade():
    """CAN _clear 应调 show_with_fade（而非裸 show）。"""

    from embeddebug.serial_station.ui.panels.can_panel import CanPanel

    src = inspect.getsource(CanPanel._clear)
    assert "show_with_fade" in src


def test_rtt_panel_clear_uses_show_with_fade():
    """RTT _clear 应调 show_with_fade。"""

    from embeddebug.serial_station.ui.panels.rtt_panel import RttPanel

    src = inspect.getsource(RttPanel._clear)
    assert "show_with_fade" in src


def test_ble_panel_disconnect_uses_show_with_fade():
    """BLE 断开后空状态恢复应调 show_with_fade。"""

    from embeddebug.serial_station.ui.panels.ble_panel import BlePanel

    src = inspect.getsource(BlePanel._connect)
    assert "show_with_fade" in src
