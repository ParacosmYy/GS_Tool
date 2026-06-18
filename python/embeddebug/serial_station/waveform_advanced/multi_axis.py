"""多 Y 轴绘图面板。"""
from __future__ import annotations
import numpy as np
import pyqtgraph as pg
from PyQt6.QtWidgets import QLabel, QVBoxLayout, QWidget
from embeddebug.serial_station.ui.theme import palette as P

class MultiAxisPlot(QWidget):
    """多 Y 轴波形面板（每通道独立 ViewBox）。"""
    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationMultiAxisPlot")
        self._layout = pg.GraphicsLayoutWidget(self)
        self._layout.setObjectName("serialStationMultiAxisLayout")
        self._axes: list[dict] = []
        self._primary = None
        l = QVBoxLayout(self)
        l.setContentsMargins(0, 0, 0, 0)
        l.addWidget(self._layout)

    def add_axis(self, name: str, color: str = P.ACCENT) -> int:
        if self._primary is None:
            vb = self._layout.addPlot(row=0, col=len(self._axes) * 2)
            self._primary = vb
        else:
            vb = pg.ViewBox()
            self._layout.scene().addItem(vb)
            self._layout.addItem(vb, row=0, col=len(self._axes) * 2)
            axis = pg.AxisItem("right")
            axis.setLabel(name)
            axis.linkToView(vb)
            self._layout.addItem(axis, row=0, col=len(self._axes) * 2 + 1)
            self._primary.setXLink(vb)
        idx = len(self._axes)
        self._axes.append({"viewbox": vb, "name": name, "color": color, "curve": None})
        return idx

    def plot_channel(self, axis_index: int, x: np.ndarray, y: np.ndarray) -> None:
        if axis_index < 0 or axis_index >= len(self._axes):
            return
        entry = self._axes[axis_index]
        vb = entry["viewbox"]
        if entry["curve"] is None:
            curve = pg.PlotCurveItem(pen=pg.mkPen(color=entry["color"], width=1.2))
            vb.addItem(curve)
            entry["curve"] = curve
        entry["curve"].setData(x, y)

    def shutdown(self) -> None:
        self._layout.clear()
        self._axes.clear()
        self._primary = None
