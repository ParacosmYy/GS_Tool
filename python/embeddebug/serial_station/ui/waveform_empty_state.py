"""Batch 49-3: 波形区空态 + 连接加载态覆盖层 helper。

独立模块，保持 ``waveform_preview.py`` 体积 ≤ 300 行（铁律 20）。

提供两类覆盖层：
- ``WaveformEmptyOverlay``：无 batch 时的空态（图标 + 标题 + 描述）。
- ``WaveformLoadingOverlay``：连接中加载态（ProgressRing + 文案）。

两者都作为 SerialWaveformPreview 的子控件覆盖层，由 resizeEvent 居中跟随。

约束：只依赖 PyQt6 + controls + widgets + animations + theme，
不访问 controller/transport/protocol。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QHBoxLayout, QLabel, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.controls import ProgressRing
from embeddebug.serial_station.ui.widgets import EmptyStateWidget


class WaveformEmptyOverlay(EmptyStateWidget):
    """波形区空态：activity 图标 + 「等待波形数据」+ CTA 无。"""

    def __init__(self, parent: QWidget) -> None:
        super().__init__(
            icon_name="activity",
            title=parent.tr("等待波形数据"),
            description=parent.tr("连接设备后，收到的测量批次将在此绘制"),
            parent=parent,
        )
        # 复用 EmptyStateWidget 的 objectName 契约（governance test 兼容）。


class WaveformLoadingOverlay(QWidget):
    """波形区连接加载态：ProgressRing + 「正在建立连接…」文案。

    用 ProgressRing 旋转动画替代裸文字「…」，对齐 spec B49-3 加载态要求。
    调用方通过 ``show_with_fade``/``hide_with_fade`` 或 ``show``/``hide`` 切换。
    """

    def __init__(self, parent: QWidget) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationWaveformLoadingOverlay")
        outer = QVBoxLayout(self)
        outer.setContentsMargins(32, 32, 32, 32)
        outer.setSpacing(12)
        outer.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # ProgressRing（indeterminate 旋转）。sizeHint=64×64，符合覆盖层尺寸。
        self._ring = ProgressRing(parent=self)
        self._ring.setObjectName("serialStationWaveformLoadingRing")
        self._ring.setIndeterminate(True)
        # 横向居中：用一个 HBoxLayout 让 ring 居中放置。
        ring_row = QHBoxLayout()
        ring_row.addStretch(1)
        ring_row.addWidget(self._ring)
        ring_row.addStretch(1)
        outer.addLayout(ring_row)

        self._label = QLabel(self.tr("正在建立连接…"), self)
        self._label.setObjectName("serialStationWaveformLoadingLabel")
        self._label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        outer.addWidget(self._label)
        self.hide()


def build_waveform_overlays(parent: QWidget) -> tuple[WaveformEmptyOverlay, WaveformLoadingOverlay]:
    """构建波形区空态 + 加载态两个覆盖层。

    Args:
        parent: 波形面板根 widget（SerialWaveformPreview）。

    Returns:
        ``(empty_overlay, loading_overlay)``。调用方负责在 resizeEvent
        里 setGeometry(parent.rect()) 居中两者，并按状态切换显隐。
    """

    return WaveformEmptyOverlay(parent), WaveformLoadingOverlay(parent)
