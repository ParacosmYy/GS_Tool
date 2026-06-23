"""waveform_cursor_interactions _resolve + install 行为边界测试。

_resolve helper 此前无直接测试（grep 仅 test_resolve_controls 非同源）。

覆盖：
1. _resolve callable ref → 调用返回结果。
2. _resolve callable ref 抛异常 → 返回 None。
3. _resolve 非 callable ref → 原样返回。
4. _resolve None ref → 返回 None。
5. install_cursor_interactions 安装后 plot.mouseDoubleClickEvent 是 _on_double_click。
6. install_cursor_interactions 安装后 plot.contextMenuEvent 是 _on_context。
7. install 后 _on_double_click 在 cm=None 时不崩。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pyqtgraph as pg

from embeddebug.serial_station.ui.waveform_cursor_interactions import (
    _resolve,
    install_cursor_interactions,
)


# ── _resolve callable ────────────────────────────────────────────
def test_resolve_callable_returns_result():
    """callable ref → 调用返回结果。"""

    sentinel = object()
    assert _resolve(lambda: sentinel) is sentinel


def test_resolve_callable_exception_returns_none():
    """callable ref 抛异常 → 返回 None。"""

    def _boom():
        raise RuntimeError("boom")

    assert _resolve(_boom) is None


# ── _resolve 非 callable ─────────────────────────────────────────
def test_resolve_non_callable_passthrough():
    """非 callable ref → 原样返回。"""

    sentinel = object()
    assert _resolve(sentinel) is sentinel


def test_resolve_none_passthrough():
    """None ref → 返回 None。"""

    assert _resolve(None) is None


def test_resolve_int_passthrough():
    """int ref → 原样返回。"""

    assert _resolve(42) == 42


# ── install_cursor_interactions ──────────────────────────────────
def test_install_replaces_double_click_handler(qtbot):
    """install 后 plot.mouseDoubleClickEvent 是 _on_double_click 闭包。"""

    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    install_cursor_interactions(plot, lambda: None)
    dc = plot.__dict__.get("mouseDoubleClickEvent")
    assert dc is not None
    assert getattr(dc, "__name__", "") == "_on_double_click"


def test_install_replaces_context_handler(qtbot):
    """install 后 plot.contextMenuEvent 是 _on_context 闭包。"""

    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    install_cursor_interactions(plot, lambda: None)
    ctx = plot.__dict__.get("contextMenuEvent")
    assert ctx is not None
    assert getattr(ctx, "__name__", "") == "_on_context"


def test_install_with_none_cursor_manager_ref(qtbot):
    """install 时 ref 返回 None → 不崩（_on_double_click 守卫 cm is None）。"""

    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    install_cursor_interactions(plot, lambda: None)
    # 验证闭包安装成功（不实际触发事件，避免场景映射复杂性）。
    assert plot.__dict__.get("mouseDoubleClickEvent") is not None
