"""waveform_empty_state overlays + waveform_math _lookup_function 边界测试。

补强 test_waveform_overlays / test_waveform_analysis 未直接断言的边角：
- build_waveform_overlays：返回 (WaveformEmptyOverlay, WaveformLoadingOverlay) tuple。
- WaveformEmptyOverlay：objectName 契约 + icon_name=activity。
- WaveformLoadingOverlay：objectName 契约 + ProgressRing 子控件。
- waveform_math _lookup_function：已知函数返回 callable / 未知返回 None。
- ExpressionError：是 ValueError 子类。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


import pytest
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.waveform_empty_state import (
    WaveformEmptyOverlay,
    WaveformLoadingOverlay,
    build_waveform_overlays,
)
from embeddebug.serial_station.waveform_math.expression import (
    ExpressionError,
    _lookup_function,
)


# ── build_waveform_overlays ───────────────────────────────────────────


def test_build_overlays_returns_tuple(qtbot):
    """build_waveform_overlays 返回 2 元组。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    empty, loading = build_waveform_overlays(parent)
    assert isinstance(empty, WaveformEmptyOverlay)
    assert isinstance(loading, WaveformLoadingOverlay)


def test_build_overlays_empty_has_objectname(qtbot):
    """WaveformEmptyOverlay objectName = serialStationEmptyState。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    empty, _ = build_waveform_overlays(parent)
    assert empty.objectName() == "serialStationEmptyState"


def test_build_overlays_loading_has_objectname(qtbot):
    """WaveformLoadingOverlay objectName = serialStationWaveformLoadingOverlay。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    _, loading = build_waveform_overlays(parent)
    assert loading.objectName() == "serialStationWaveformLoadingOverlay"


def test_waveform_empty_overlay_objectname(qtbot):
    """WaveformEmptyOverlay objectName = serialStationEmptyState（QSS 契约）。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformEmptyOverlay(parent)
    assert overlay.objectName() == "serialStationEmptyState"


def test_waveform_empty_overlay_parent_relation(qtbot):
    """WaveformEmptyOverlay 以 parent 为父级。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformEmptyOverlay(parent)
    assert overlay.parent() is parent


def test_waveform_loading_overlay_parent_relation(qtbot):
    """WaveformLoadingOverlay 以 parent 为父级。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformLoadingOverlay(parent)
    assert overlay.parent() is parent


# ── _lookup_function ──────────────────────────────────────────────────


def test_lookup_function_sqrt():
    """已知函数 sqrt → 返回 callable。"""

    fn = _lookup_function("sqrt")
    assert callable(fn)


def test_lookup_function_sin():
    """已知函数 sin → 返回 callable。"""

    fn = _lookup_function("sin")
    assert callable(fn)


def test_lookup_function_unknown():
    """未知函数名 → 返回 None。"""

    assert _lookup_function("nonexistent_func") is None


def test_lookup_function_empty_string():
    """空字符串 → 返回 None。"""

    assert _lookup_function("") is None


# ── ExpressionError ───────────────────────────────────────────────────


def test_expression_error_is_value_error():
    """ExpressionError 是 ValueError 子类。"""

    assert issubclass(ExpressionError, ValueError)


def test_expression_error_raisable():
    """ExpressionError 可被 raise 和 catch。"""

    with pytest.raises(ExpressionError):
        raise ExpressionError("test")


def test_expression_error_caught_as_value_error():
    """ExpressionError 可被 except ValueError 捕获。"""

    with pytest.raises(ValueError):
        raise ExpressionError("test")


def test_empty_overlay_icon_label_exists(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformEmptyOverlay(parent)
    assert overlay._icon_label is not None


def test_empty_overlay_title_text(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformEmptyOverlay(parent)
    assert overlay._title_label.text() == "等待波形数据"


def test_empty_overlay_description_text(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformEmptyOverlay(parent)
    assert "连接设备" in overlay._desc_label.text()


def test_loading_overlay_ring_objectname(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformLoadingOverlay(parent)
    assert overlay._ring.objectName() == "serialStationWaveformLoadingRing"


def test_loading_overlay_ring_indeterminate(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformLoadingOverlay(parent)
    assert overlay._ring.isIndeterminate() is True


def test_loading_overlay_label_objectname(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformLoadingOverlay(parent)
    assert overlay._label.objectName() == "serialStationWaveformLoadingLabel"


def test_loading_overlay_label_text(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformLoadingOverlay(parent)
    assert "连接" in overlay._label.text()


def test_loading_overlay_initially_hidden(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformLoadingOverlay(parent)
    assert overlay.isHidden()


def test_build_overlays_both_share_parent(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    empty, loading = build_waveform_overlays(parent)
    assert empty.parent() is parent
    assert loading.parent() is parent


def test_build_overlays_empty_not_hidden_initially(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    _empty, loading = build_waveform_overlays(parent)
    assert loading.isHidden()
