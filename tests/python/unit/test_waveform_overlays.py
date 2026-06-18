"""波形游标/图例、面板动画与响应式断点测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
import pyqtgraph as pg
from PyQt6.QtWidgets import QLabel, QSplitter, QWidget

from embeddebug.serial_station.ui import panel_animations
from embeddebug.serial_station.ui import responsive_layout
from embeddebug.serial_station.ui import waveform_overlays


# ── 波形游标读数 HUD ───────────────────────────────────────────────
# Batch 20：旧 attach_cursors/cursor_readout 已删除（被 CursorManager + waveform_measure
# 取代）。游标能力由 waveform_cursors / waveform_measure 测试覆盖。
def test_build_cursor_hud_has_expected_objectname(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    hud = waveform_overlays.build_cursor_hud(parent)
    assert hud.objectName() == "serialStationWaveformCursorHud"
    assert "ΔX" in hud.text()


# ── 多通道图例 ────────────────────────────────────────────────────
def test_waveform_legend_has_objectname(qtbot):
    legend = waveform_overlays.WaveformLegend()
    qtbot.addWidget(legend)
    assert legend.objectName() == "serialStationWaveformLegend"


def test_waveform_legend_updates_channels(qtbot):
    legend = waveform_overlays.WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels(("ch0", "ch1"), (1.5, 2.5))
    chips = legend.findChildren(QLabel, "serialStationWaveformLegendChip")
    assert len(chips) == 2


def test_waveform_legend_clears_old_chips(qtbot):
    legend = waveform_overlays.WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels(("a", "b", "c"), (1.0, 2.0, 3.0))
    legend.update_channels(("x",), (9.0,))
    chips = legend.findChildren(QLabel, "serialStationWaveformLegendChip")
    assert len(chips) == 1


# ── 面板动画 ──────────────────────────────────────────────────────
def test_fade_in_returns_animation_with_opacity_target(qtbot):
    widget = QWidget()
    qtbot.addWidget(widget)
    anim = panel_animations.fade_in(widget)
    assert anim.duration() == panel_animations.DURATION_NORMAL
    assert anim.startValue() == 0.0
    assert anim.endValue() == 1.0


def test_fade_out_hides_widget_on_finish(qtbot):
    widget = QWidget()
    qtbot.addWidget(widget)
    widget.show()
    anim = panel_animations.fade_out(widget)
    # 模拟 finished 信号触发 hide。
    anim.finished.emit()
    assert not widget.isVisible()


def test_slide_in_sets_start_below_target(qtbot):
    widget = QWidget()
    qtbot.addWidget(widget)
    target = widget.pos()
    anim = panel_animations.slide_in(widget)
    start = anim.startValue()
    assert start.y() == target.y() + panel_animations.SLIDE_PIXELS
    assert anim.endValue() == target


def test_card_enter_returns_two_animations(qtbot):
    widget = QWidget()
    qtbot.addWidget(widget)
    anims = panel_animations.card_enter(widget)
    assert len(anims) == 2


def test_stagger_returns_animation_per_card(qtbot):
    cards = []
    for _ in range(3):
        card = QWidget()
        cards.append(card)
    anims = panel_animations.stagger(cards)
    assert len(anims) == len(cards) * 2


# ── 响应式断点 ────────────────────────────────────────────────────
def _make_three_zone_splitter(qtbot) -> QSplitter:
    splitter = QSplitter()
    qtbot.addWidget(splitter)
    for name in ("serialStationLeftZone", "serialStationCenterZone", "serialStationRightZone"):
        zone = QWidget(splitter)
        zone.setObjectName(name)
    splitter.setSizes([220, 560, 220])
    return splitter


def test_responsive_layout_starts_expanded(qtbot):
    splitter = _make_three_zone_splitter(qtbot)
    rl = responsive_layout.ResponsiveLayout(splitter)
    assert rl.is_collapsed is False


def test_responsive_layout_collapses_below_breakpoint(qtbot):
    splitter = _make_three_zone_splitter(qtbot)
    rl = responsive_layout.ResponsiveLayout(splitter)
    collapsed_signals: list[bool] = []
    rl.sidebar_collapsed.connect(lambda: collapsed_signals.append(True))
    rl.on_window_resized(800)
    assert rl.is_collapsed is True
    assert collapsed_signals == [True]
    assert splitter.sizes()[0] == responsive_layout.COLLAPSED_WIDTH


def test_responsive_layout_expands_above_breakpoint(qtbot):
    splitter = _make_three_zone_splitter(qtbot)
    rl = responsive_layout.ResponsiveLayout(splitter)
    rl.on_window_resized(800)
    assert rl.is_collapsed is True
    expanded_signals: list[bool] = []
    rl.sidebar_expanded.connect(lambda: expanded_signals.append(True))
    rl.on_window_resized(1000)
    assert rl.is_collapsed is False
    assert expanded_signals == [True]


def test_responsive_layout_hysteresis_avoids_jitter(qtbot):
    """在 900~980 滞后区间内不应反复折叠/展开。"""

    splitter = _make_three_zone_splitter(qtbot)
    rl = responsive_layout.ResponsiveLayout(splitter)
    rl.on_window_resized(850)
    assert rl.is_collapsed is True
    # 920 在折叠与展开阈值之间，应保持折叠。
    rl.on_window_resized(920)
    assert rl.is_collapsed is True
    rl.on_window_resized(990)
    assert rl.is_collapsed is False
