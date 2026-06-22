"""mode_panel register/registered/reset + waveform_engine 配置函数边界测试。

补强 test_dashboard_core_b / test_waveform_overlays 未直接断言的边角：
- register_panel：去重（同 mode_id 不重复）+ 注册顺序保留。
- registered_panels：空/注册后返回 tuple。
- reset_registry：清空后 registered_panels=()。
- PanelRegistration：frozen + 4 字段。
- waveform_engine.try_enable_opengl：返回 bool（不崩溃）。
- waveform_engine.configure_high_performance_plot：不崩溃。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QWidget

from embeddebug.app.mode_panel import (
    PanelRegistration,
    register_panel,
    registered_panels,
    reset_registry,
)


# ── PanelRegistration frozen ──────────────────────────────────────────


def test_panel_registration_is_frozen():
    """PanelRegistration 是 frozen dataclass。"""

    import pytest

    reg = PanelRegistration(
        mode_id="test", icon="cable", label="Test",
        factory=lambda app: QWidget(),
    )
    with pytest.raises((AttributeError, Exception)):
        reg.mode_id = "changed"  # type: ignore[misc]


def test_panel_registration_has_four_fields():
    """PanelRegistration 含 mode_id/icon/label/factory 4 字段。"""

    reg = PanelRegistration(
        mode_id="x", icon="cpu", label="X",
        factory=lambda app: QWidget(),
    )
    assert reg.mode_id == "x"
    assert reg.icon == "cpu"
    assert reg.label == "X"
    assert callable(reg.factory)


# ── register_panel / registered_panels / reset_registry ───────────────


def _dummy_factory(app):
    return QWidget()


def test_reset_registry_clears_all():
    """reset_registry → registered_panels=()。"""

    reset_registry()
    assert registered_panels() == ()


def test_register_then_registered_returns_it():
    """注册后 registered_panels 含该 panel。"""

    reset_registry()
    register_panel("test1", "cable", "Test1", _dummy_factory)
    regs = registered_panels()
    assert len(regs) >= 1
    assert any(r.mode_id == "test1" for r in regs)
    reset_registry()


def test_register_duplicate_id_ignored():
    """同 mode_id 二次注册 → 不重复（去重）。"""

    reset_registry()
    register_panel("dup", "cable", "First", _dummy_factory)
    register_panel("dup", "cpu", "Second", _dummy_factory)
    regs = registered_panels()
    dup_count = sum(1 for r in regs if r.mode_id == "dup")
    assert dup_count == 1
    reset_registry()


def test_register_preserves_order():
    """注册顺序保留。"""

    reset_registry()
    register_panel("alpha", "cable", "A", _dummy_factory)
    register_panel("beta", "cpu", "B", _dummy_factory)
    register_panel("gamma", "zap", "C", _dummy_factory)
    ids = [r.mode_id for r in registered_panels()]
    assert ids.index("alpha") < ids.index("beta") < ids.index("gamma")
    reset_registry()


def test_registered_panels_returns_tuple():
    """registered_panels 返回 tuple。"""

    reset_registry()
    result = registered_panels()
    assert isinstance(result, tuple)
    reset_registry()


# ── waveform_engine 配置函数 ──────────────────────────────────────────


def test_try_enable_opengl_returns_bool(qtbot):
    """try_enable_opengl 返回 bool（不崩溃）。"""

    import pyqtgraph as pg

    from embeddebug.serial_station.ui.waveform_engine import try_enable_opengl

    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    result = try_enable_opengl(plot)
    assert isinstance(result, bool)


def test_configure_high_performance_plot_no_crash(qtbot):
    """configure_high_performance_plot 不崩溃。"""

    import pyqtgraph as pg

    from embeddebug.serial_station.ui.waveform_engine import (
        configure_high_performance_plot,
    )

    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    configure_high_performance_plot(plot)


def test_apply_curve_perf_no_crash(qtbot):
    """apply_curve_perf 不崩溃。"""

    import pyqtgraph as pg

    from embeddebug.serial_station.ui.waveform_engine import apply_curve_perf

    plot = pg.PlotWidget()
    qtbot.addWidget(plot)
    curve = plot.plot([1, 2, 3])
    apply_curve_perf(curve)
