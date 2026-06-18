"""李萨如（X-Y）模式（对齐 VOFA+ X/Y 模式）。

取两个通道做 X/Y 投影绘制（李萨如图），用于判断相位关系、频率比。
独立于时域波形，单独一张 PlotWidget。

约束：本模块只依赖 PyQt6 + pyqtgraph + numpy，不访问 controller/transport。
"""

from __future__ import annotations

import numpy as np
import pyqtgraph as pg
from PyQt6.QtWidgets import QLabel, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.waveform_preview import SafePlotWidget


class LissajousPanel(QWidget):
    """李萨如（X-Y）面板：两通道投影绘制。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationLissajousPanel")
        self._curve: pg.PlotDataItem | None = None
        self._x_channel = 0
        self._y_channel = 1

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(4)

        self._plot = SafePlotWidget(self)
        self._plot.setObjectName("serialStationLissajousPlot")
        self._plot.setMinimumHeight(160)
        self._plot.setAspectLocked(lock=True, ratio=1.0)
        self._plot.showGrid(x=True, y=True, alpha=0.2)
        self._plot.setLabel("bottom", self.tr("Channel X"))
        self._plot.setLabel("left", self.tr("Channel Y"))
        layout.addWidget(self._plot, 1)

        self._status = QLabel(self.tr("X-Y: select two channels"), self)
        self._status.setObjectName("serialStationLissajousStatus")
        layout.addWidget(self._status)

    def set_channels(self, x_channel: int, y_channel: int) -> None:
        """设置 X/Y 通道索引。"""

        self._x_channel = max(0, x_channel)
        self._y_channel = max(0, y_channel)

    def update_batch(self, values: np.ndarray, channel_names: tuple[str, ...]) -> None:
        """用最新批次的两通道做 X/Y 投影。"""

        if values.ndim != 2 or values.shape[0] == 0:
            return
        col_count = values.shape[1]
        xi = min(self._x_channel, col_count - 1)
        yi = min(self._y_channel, col_count - 1)
        x = values[:, xi]
        y = values[:, yi]
        if self._curve is None:
            self._curve = self._plot.plot(
                pen=pg.mkPen(color=P.ACCENT, width=1.2),
                symbol=None,
            )
        self._curve.setData(x, y)
        x_name = channel_names[xi] if xi < len(channel_names) else f"ch{xi}"
        y_name = channel_names[yi] if yi < len(channel_names) else f"ch{yi}"
        self._status.setText(self.tr("X-Y: {x} ↔ {y}  ·  {n} pts").format(
            x=x_name, y=y_name, n=values.shape[0]
        ))

    def shutdown(self) -> None:
        self.setUpdatesEnabled(False)
        self._plot.hide()
        self._plot.clear()
        self._curve = None
