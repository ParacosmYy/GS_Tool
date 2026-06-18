"""Batch 23 测试：AppShell 页面切换离场淡出（激活 panel_animations.fade_out 死代码）。

覆盖：
1. AppShell 切换页面时对老页面调 fade_out（_animate_page_leave）。
2. _animate_page_leave 创建 fade_out 动画并存 _leave_anims。
3. _stop_page_anims 同时停止离场动画。
4. 源码接入断言（_switch_to 调 _animate_page_leave）。
5. panel_animations.fade_out 现有外部消费者（AppShell，激活死代码）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


def _make_shell(qtbot):
    from embeddebug.app.app_shell import AppShell
    from embeddebug.serial_station.ui.panels import register_default_panels

    register_default_panels()  # 确保面板注册（AppShell 按注册装配导航）。
    shell = AppShell()
    qtbot.addWidget(shell)
    return shell


# ── AppShell 离场淡出接线 ──────────────────────────────────────────
def test_appshell_has_animate_page_leave():
    """AppShell 应有 _animate_page_leave 方法。"""

    from embeddebug.app.app_shell import AppShell

    assert hasattr(AppShell, "_animate_page_leave")


def test_appshell_switch_calls_fade_out(qtbot, monkeypatch):
    """切换页面应触发对老页面的 fade_out（激活 panel_animations.fade_out）。"""

    fade_calls: list = []
    import embeddebug.serial_station.ui.panel_animations as pa

    original = pa.fade_out
    monkeypatch.setattr(
        pa, "fade_out",
        lambda widget, duration_ms=160: (fade_calls.append(widget) or original(widget, duration_ms)),
    )
    shell = _make_shell(qtbot)
    before = len(fade_calls)
    # 切到第二个页面（index 1，如 OTA），老页面（serial）应 fade_out。
    if shell._stack.count() > 1:
        shell._switch_to(1)
        assert len(fade_calls) > before, "leaving page should fade_out"


def test_animate_page_leave_stores_anim(qtbot):
    """_animate_page_leave 应把动画存 _leave_anims 防 GC。"""

    shell = _make_shell(qtbot)
    page = shell._stack.widget(0)
    shell._animate_page_leave(page)
    assert hasattr(shell, "_leave_anims")
    assert len(shell._leave_anims) >= 1


def test_stop_page_anims_clears_leave_anims(qtbot):
    """_stop_page_anims 应停止并清空 _leave_anims。"""

    shell = _make_shell(qtbot)
    page = shell._stack.widget(0)
    shell._animate_page_leave(page)
    assert len(shell._leave_anims) >= 1
    shell._stop_page_anims()
    assert shell._leave_anims == []


# ── 源码接入断言 ───────────────────────────────────────────────────
def test_switch_to_wires_animate_page_leave():
    """_switch_to 源码应调 _animate_page_leave（离场淡出接线）。"""

    from embeddebug.app.app_shell import AppShell

    src = inspect.getsource(AppShell._switch_to)
    assert "_animate_page_leave" in src


def test_animate_page_leave_uses_fade_out():
    """_animate_page_leave 源码应调 panel_animations.fade_out（死代码激活）。"""

    from embeddebug.app.app_shell import AppShell

    src = inspect.getsource(AppShell._animate_page_leave)
    assert "fade_out" in src


# ── fade_out 死代码激活验证 ────────────────────────────────────────
def test_fade_out_now_has_production_consumer():
    """panel_animations.fade_out 现应有生产消费者（AppShell），不再是死代码。

    Batch 23 前仅 test 引用；Batch 23 AppShell._animate_page_leave 激活。
    """

    from pathlib import Path

    src = Path("python/embeddebug/app/app_shell.py").read_text(encoding="utf-8")
    assert "fade_out" in src
    assert "_animate_page_leave" in src
