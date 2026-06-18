"""waveform_overlays 遗留死代码清理守护测试。

Batch 7 用 CursorManager + waveform_measure 取代旧版固定双游标 API 后，
attach_cursors/cursor_readout/_make_cursor/_sample_y 仍残留为死代码（仅注释引用）。
Batch 20 删除它们，本测试固化「模块只保留在用 API」事实，防止回归。

覆盖：
1. attach_cursors/cursor_readout/_make_cursor/_sample_y 已从 waveform_overlays 移除。
2. WaveformLegend/build_cursor_hud（在用 API）仍存在。
3. waveform_measure 全部公开函数仍在用（无回归死代码）。
4. waveform_preview 不再实际调用旧 API（只在注释提及）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


# ── 旧 API 已移除 ──────────────────────────────────────────────────
def test_attach_cursors_removed():
    """attach_cursors（旧固定双游标）应已从 waveform_overlays 移除。"""

    from embeddebug.serial_station.ui import waveform_overlays

    assert not hasattr(waveform_overlays, "attach_cursors"), (
        "attach_cursors is legacy dead code (replaced by CursorManager); remove it"
    )


def test_cursor_readout_removed():
    """cursor_readout（旧游标读数）应已移除。"""

    from embeddebug.serial_station.ui import waveform_overlays

    assert not hasattr(waveform_overlays, "cursor_readout"), (
        "cursor_readout is legacy dead code (replaced by waveform_measure); remove it"
    )


def test_make_cursor_removed():
    """_make_cursor（旧游标构造，仅 cursor_readout 用）应已移除。"""

    from embeddebug.serial_station.ui import waveform_overlays

    assert not hasattr(waveform_overlays, "_make_cursor")


def test_sample_y_removed():
    """_sample_y（旧插值，仅 cursor_readout 用）应已移除。"""

    from embeddebug.serial_station.ui import waveform_overlays

    assert not hasattr(waveform_overlays, "_sample_y")


# ── 在用 API 仍在 ──────────────────────────────────────────────────
def test_waveform_legend_still_present():
    """WaveformLegend（多通道图例，在用）应保留。"""

    from embeddebug.serial_station.ui.waveform_overlays import WaveformLegend

    assert WaveformLegend is not None


def test_build_cursor_hud_still_present():
    """build_cursor_hud（游标读数 HUD 容器，在用）应保留。"""

    from embeddebug.serial_station.ui import waveform_overlays

    assert callable(waveform_overlays.build_cursor_hud)


# ── waveform_measure 全函数在用（无回归死代码） ────────────────────
def test_waveform_measure_all_functions_used():
    """waveform_measure 的 4 个公开函数应都在 waveform_preview 实际调用。"""

    from embeddebug.serial_station.ui import waveform_preview

    src = inspect.getsource(waveform_preview)
    for func in (
        "compute_channel_stats",
        "compute_cursor_measurement",
        "format_stats",
        "format_cursor_measurement",
    ):
        assert f"waveform_measure.{func}" in src, (
            f"waveform_measure.{func} should be called in waveform_preview"
        )


# ── waveform_preview 不再调用旧 overlays API ───────────────────────
def test_waveform_preview_uses_cursor_manager_not_attach_cursors():
    """waveform_preview 应使用 CursorManager（非旧 attach_cursors）。"""

    from embeddebug.serial_station.ui import waveform_preview

    src = inspect.getsource(waveform_preview)
    assert "CursorManager" in src
    # 不应有 attach_cursors(...) 实际调用（注释提及 OK，但不应是调用语句）。
    import re

    calls = re.findall(r"\battach_cursors\s*\(", src)
    assert calls == [], "waveform_preview should not call attach_cursors (legacy)"


# ── 导入完整性 ─────────────────────────────────────────────────────
def test_waveform_overlays_imports_clean(qtbot):
    """waveform_overlays 删除死代码后应仍能正常导入。"""

    from embeddebug.serial_station.ui.waveform_overlays import (
        WaveformLegend,
        build_cursor_hud,
    )

    # 构建一个 HUD + 图例验证实例化不崩溃。
    legend = WaveformLegend()
    qtbot.addWidget(legend)
    hud = build_cursor_hud(legend)
    assert hud.objectName() == "serialStationWaveformCursorHud"
