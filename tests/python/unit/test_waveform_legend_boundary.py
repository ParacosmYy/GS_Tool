"""WaveformLegend update_channels _make_chip 边界扩展测试。

test_waveform_overlays 覆盖基础 update_channels；本文件补 latest_values None/短/长 边界。

覆盖：
1. update_channels None latest_values → chip 无数值后缀。
2. update_channels 短 latest_values（少于通道数）→ 超出部分无数值。
3. update_channels 长 latest_values（多于通道数）→ 多余忽略。
4. update_channels 空 channel_names → 无 chip。
5. update_channels 多次调用清旧 chip。
6. _make_chip objectName 契约。
7. chip 文本含通道名。
8. update_channels 有数值时 chip 文本含数值。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QLabel

from embeddebug.serial_station.ui.waveform_overlays import WaveformLegend


def _chips(legend):
    return legend.findChildren(QLabel, "serialStationWaveformLegendChip")


# ── update_channels None latest_values ───────────────────────────
def test_update_channels_none_latest_no_value_suffix(qtbot):
    """latest_values=None → chip 文本只有通道名（无数值后缀）。"""

    legend = WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels(("ch0", "ch1"), None)
    chips = _chips(legend)
    assert len(chips) == 2
    for chip in chips:
        text = chip.text()
        assert "ch" in text
        # 无 .3f 数值后缀（只有 ⬤ name）。
        assert ".000" not in text


# ── update_channels 短 latest_values ─────────────────────────────
def test_update_channels_short_latest_values(qtbot):
    """latest_values 短于通道数 → 超出部分无数值。"""

    legend = WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels(("a", "b", "c"), (1.0,))  # 只给 1 个值
    chips = _chips(legend)
    assert len(chips) == 3
    # 第一个有数值。
    assert "1.000" in chips[0].text()
    # 第二、三个无数值。
    assert ".000" not in chips[1].text()
    assert ".000" not in chips[2].text()


# ── update_channels 长 latest_values ─────────────────────────────
def test_update_channels_long_latest_values(qtbot):
    """latest_values 长于通道数 → 多余忽略。"""

    legend = WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels(("a",), (1.0, 2.0, 3.0))  # 给 3 个值但只 1 通道
    chips = _chips(legend)
    assert len(chips) == 1
    assert "1.000" in chips[0].text()


# ── update_channels 空 channel_names ─────────────────────────────
def test_update_channels_empty_names_no_chips(qtbot):
    """空 channel_names → 无 chip。"""

    legend = WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels((), None)
    assert _chips(legend) == []


# ── update_channels 多次调用清旧 chip ────────────────────────────
def test_update_channels_multiple_calls_clears_old(qtbot):
    """多次 update_channels 清旧 chip（_clear_chips deleteLater）。"""

    legend = WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels(("a", "b"), (1.0, 2.0))
    assert len(_chips(legend)) == 2
    legend.update_channels(("x",), (9.0,))
    assert len(_chips(legend)) == 1


# ── chip 文本契约 ─────────────────────────────────────────────────
def test_chip_text_contains_channel_name(qtbot):
    """chip 文本含通道名。"""

    legend = WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels(("temp", "volt"), (24.5, 3.3))
    chips = _chips(legend)
    texts = [c.text() for c in chips]
    assert any("temp" in t for t in texts)
    assert any("volt" in t for t in texts)


def test_chip_text_contains_value_when_provided(qtbot):
    """有 latest_values 时 chip 文本含数值。"""

    legend = WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels(("ch0",), (42.5,))
    chips = _chips(legend)
    assert "42.500" in chips[0].text()


# ── chip objectName 契约 ──────────────────────────────────────────
def test_chip_objectname_contract(qtbot):
    """所有 chip objectName == serialStationWaveformLegendChip。"""

    legend = WaveformLegend()
    qtbot.addWidget(legend)
    legend.update_channels(("a", "b", "c"), (1.0, 2.0, 3.0))
    chips = _chips(legend)
    for chip in chips:
        assert chip.objectName() == "serialStationWaveformLegendChip"
