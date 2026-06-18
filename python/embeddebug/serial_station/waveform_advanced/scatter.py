"""散点图（z 值彩色映射）。"""
from __future__ import annotations
import numpy as np
import pyqtgraph as pg
from PyQt6.QtWidgets import QVBoxLayout, QWidget
from embeddebug.serial_station.ui.waveform_preview import SafePlotWidget
from embeddebug.serial_station.ui.theme import palette as P

class ScatterPlot(QWidget):
    """2D 散点图，颜色由 z 值映射。"""
    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationScatterPlot")
        self._plot = SafePlotWidget(self)
        self._plot.setObjectName("serialStationScatterPlotWidget")
        l = QVBoxLayout(self)
        l.setContentsMargins(0, 0, 0, 0)
        l.addWidget(self._plot)
        self._scatter: pg.ScatterPlotItem | None = None
        self._lut = None
        try:
            cmap = pg.ColorMap([0.0, 1.0], [pg.mkColor(P.BG_PANEL), pg.mkColor(P.ACCENT)])
            self._lut = cmap.getLookupTable(0.0, 1.0, 256)
        except Exception:
            pass

    def set_points(self, x: np.ndarray, y: np.ndarray, z: np.ndarray | None = None, size: int = 8) -> None:
        if len(x) == 0:
            return
        if z is None:
            z = np.zeros(len(x))
        z_norm = (z - z.min()) / max(z.max() - z.min(), 1e-9)
        if self._lut is not None:
            indices = (z_norm * 255).clip(0, 255).astype(int)
            brushes = [pg.mkBrush(self._lut[i]) for i in indices]
        else:
            brushes = [pg.mkBrush(P.ACCENT)] * len(x)
        if self._scatter is not None:
            self._plot.removeItem(self._scatter)
        self._scatter = pg.ScatterPlotItem(x=x, y=y, brush=brushes, size=size)
        self._plot.addItem(self._scatter)

    def shutdown(self) -> None:
        self._plot.clear()
        self._scatter = None
