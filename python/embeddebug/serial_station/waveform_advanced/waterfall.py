"""瀑布图（时间-频率频谱）。"""
from __future__ import annotations
import numpy as np
import pyqtgraph as pg
from PyQt6.QtWidgets import QVBoxLayout, QWidget
from embeddebug.serial_station.ui.waveform_preview import SafePlotWidget

class WaterfallPlot(QWidget):
    """滚动瀑布图（ImageItem + 环形缓冲）。"""
    def __init__(self, max_rows: int = 100, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationWaterfallPlot")
        self._max_rows = max_rows
        self._plot = SafePlotWidget(self)
        self._plot.setObjectName("serialStationWaterfallPlotWidget")
        self._image = pg.ImageItem()
        self._plot.addItem(self._image)
        self._buffer: list[np.ndarray] = []
        l = QVBoxLayout(self)
        l.setContentsMargins(0, 0, 0, 0)
        l.addWidget(self._plot)

    def append_row(self, row: np.ndarray) -> None:
        self._buffer.append(row.astype(np.float32))
        if len(self._buffer) > self._max_rows:
            self._buffer.pop(0)
        if self._buffer:
            data = np.vstack(self._buffer)
            self._image.setImage(data.T)

    def clear(self) -> None:
        self._buffer.clear()
        self._image.clear()

    @property
    def row_count(self) -> int:
        return len(self._buffer)

    def shutdown(self) -> None:
        self.clear()


class SpectrumWaterfall(WaterfallPlot):
    """频谱瀑布图（append_spectrum 归一化后追加）。"""
    def __init__(self, freq_count: int = 256, max_rows: int = 100, parent: QWidget | None = None) -> None:
        super().__init__(max_rows=max_rows, parent=parent)
        self.setObjectName("serialStationSpectrumWaterfall")
        self._freq_count = freq_count

    def append_spectrum(self, freqs: np.ndarray, magnitudes: np.ndarray) -> None:
        mag = np.asarray(magnitudes, dtype=np.float32)
        if mag.size == 0:
            return
        lo, hi = float(mag.min()), float(mag.max())
        normalized = (mag - lo) / max(hi - lo, 1e-9)
        self.append_row(normalized)
