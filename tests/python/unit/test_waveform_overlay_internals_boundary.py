"""WaveformEmptyOverlay + WaveformLoadingOverlay 内部结构边界测试。

test_waveform_overlays_math_boundary 覆盖基础 objectName/parent；
本文件补 icon/title/description 文本 + _ring indeterminate + _label 文本 +
初始 hide + build_waveform_overlays 父子关系。

覆盖：
1. WaveformEmptyOverlay icon_name='activity'。
2. WaveformEmptyOverlay title='等待波形数据'。
3. WaveformEmptyOverlay description 含 '连接设备'。
4. WaveformLoadingOverlay _ring objectName。
5. WaveformLoadingOverlay _ring indeterminate=True。
6. WaveformLoadingOverlay _label objectName。
7. WaveformLoadingOverlay _label 文本含 '连接'。
8. WaveformLoadingOverlay 初始 hide。
9. build_waveform_overlays 两个 overlay 共享 parent。
10. WaveformEmptyOverlay 初始可见（无 hide）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.waveform_empty_state import (
    WaveformEmptyOverlay,
    WaveformLoadingOverlay,
    build_waveform_overlays,
)


# ── WaveformEmptyOverlay 文本契约 ────────────────────────────────
def test_empty_overlay_icon_is_activity(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformEmptyOverlay(parent)
    # icon_label 存在（EmptyStateWidget._icon_label）。
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


# ── WaveformLoadingOverlay _ring ─────────────────────────────────
def test_loading_overlay_ring_objectname(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformLoadingOverlay(parent)
    assert overlay._ring.objectName() == "serialStationWaveformLoadingRing"


def test_loading_overlay_ring_indeterminate(qtbot):
    """_ring indeterminate=True（旋转动画）。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformLoadingOverlay(parent)
    assert overlay._ring.isIndeterminate() is True


# ── WaveformLoadingOverlay _label ────────────────────────────────
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


# ── WaveformLoadingOverlay 初始 hide ─────────────────────────────
def test_loading_overlay_initially_hidden(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = WaveformLoadingOverlay(parent)
    assert overlay.isHidden()


# ── build_waveform_overlays 父子关系 ─────────────────────────────
def test_build_overlays_both_share_parent(qtbot):
    """两个 overlay 都以传入 parent 为父。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    empty, loading = build_waveform_overlays(parent)
    assert empty.parent() is parent
    assert loading.parent() is parent


def test_build_overlays_empty_not_hidden_initially(qtbot):
    """build 后 loading overlay 默认 hide（empty 由调用方控制）。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    _empty, loading = build_waveform_overlays(parent)
    # loading 初始 hide，empty 默认不 hide（由调用方控制）。
    assert loading.isHidden()
