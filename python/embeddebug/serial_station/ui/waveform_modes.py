"""波形显示模式：条形图 + 独立 X/Y 缩放控制（对齐 VOFA+）。

- ``BarChartPanel``：通道值条形展示，适合离散状态/多通道瞬时值对比。
- ``ZoomMode`` + ``apply_zoom_mode``：X-only / Y-only / 同时缩放三档切换，
  通过设置 PlotWidget 的 mouseEnabled 实现。

约束：本模块只依赖 PyQt6 + pyqtgraph + numpy，不访问 controller/transport。
"""

from __future__ import annotations

from enum import Enum

import numpy as np
import pyqtgraph as pg
from PyQt6.QtWidgets import QLabel, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.waveform_preview import SafePlotWidget


class ZoomMode(Enum):
    """缩放模式枚举。"""

    BOTH = "both"      # X/Y 同时缩放（默认）
    X_ONLY = "x_only"  # 仅 X 轴缩放
    Y_ONLY = "y_only"  # 仅 Y 轴缩放


def apply_zoom_mode(plot: pg.PlotWidget, mode: ZoomMode) -> None:
    """按缩放模式启用/禁用鼠标滚轮在 X/Y 轴的缩放。"""

    x_enabled = mode in (ZoomMode.BOTH, ZoomMode.X_ONLY)
    y_enabled = mode in (ZoomMode.BOTH, ZoomMode.Y_ONLY)
    plot.setMouseEnabled(x=x_enabled, y=y_enabled)


class BarChartPanel(QWidget):
    """通道值条形图面板：多通道瞬时值对比。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationBarChartPanel")
        self._bar: pg.BarGraphItem | None = None
        self._names: tuple[str, ...] = ()

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(4)

        self._plot = SafePlotWidget(self)
        self._plot.setObjectName("serialStationBarChartPlot")
        self._plot.setMinimumHeight(140)
        self._plot.setLabel("bottom", self.tr("Channel"))
        self._plot.setLabel("left", self.tr("Value"))
        layout.addWidget(self._plot, 1)

        self._status = QLabel(self.tr("Bar: no samples"), self)
        self._status.setObjectName("serialStationBarChartStatus")
        layout.addWidget(self._status)

    def update_batch(self, values: np.ndarray, channel_names: tuple[str, ...]) -> None:
        """用最新样本行的各通道值绘制条形图。"""

        if values.ndim != 2 or values.shape[0] == 0:
            return
        self._names = channel_names
        latest = values[-1]
        count = latest.shape[0]
        x = np.arange(count, dtype=np.float32)
        # 用通道索引生成稳定色相。
        colors = [pg.intColor(i, hues=max(3, count)) for i in range(count)]
        if self._bar is not None:
            self._plot.removeItem(self._bar)
        self._bar = pg.BarGraphItem(
            x=x,
            height=latest.astype(np.float64),
            width=0.6,
            brushes=colors,
        )
        self._plot.addItem(self._bar)
        # X 轴刻度改为通道名。
        ticks = [[(i, channel_names[i] if i < len(channel_names) else f"ch{i}")
                  for i in range(count)]]
        self._plot.getAxis("bottom").setTicks(ticks)
        self._status.setText(self.tr("Bar: {n} channels").format(n=count))

    def shutdown(self) -> None:
        self.setUpdatesEnabled(False)
        self._plot.hide()
        self._plot.clear()
        self._bar = None


def latest_per_channel(values: np.ndarray) -> np.ndarray:
    """返回最新样本行的各通道值（1D 数组）。"""

    if values.ndim != 2 or values.shape[0] == 0:
        return np.array([], dtype=np.float32)
    return values[-1].astype(np.float32)
