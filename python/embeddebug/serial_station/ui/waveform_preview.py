"""PyQtGraph waveform preview for measurement batches."""

from __future__ import annotations

import os

import numpy as np
import pyqtgraph as pg
from PyQt6.QtWidgets import QLabel, QVBoxLayout, QWidget

from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui import waveform_overlays


class SafePlotWidget(pg.PlotWidget):
    """PlotWidget guard for delayed paint events during Qt teardown."""

    def resizeEvent(self, event: object) -> None:
        if event is not None and os.environ.get("QT_QPA_PLATFORM") == "offscreen":
            event.accept()
            return
        try:
            super().resizeEvent(event)
        except RuntimeError:
            event.accept()

    def paintEvent(self, event: object) -> None:
        if os.environ.get("QT_QPA_PLATFORM") == "offscreen":
            event.accept()
            return
        try:
            super().paintEvent(event)
        except RuntimeError:
            event.accept()


class SerialWaveformPreview(QWidget):
    """Display the latest measurement batch without leaking UI into core."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationWaveformPanel")
        self._curves: list[pg.PlotDataItem] = []
        self._cursor_x1 = None
        self._cursor_x2 = None

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(6)

        self._plot = SafePlotWidget(self)
        self._plot.setObjectName("serialStationWaveformPlot")
        self._plot.setMinimumHeight(180)
        self._plot.showGrid(x=True, y=True, alpha=0.25)
        self._plot.setLabel("bottom", self.tr("Sample"))
        self._plot.setLabel("left", self.tr("Value"))
        layout.addWidget(self._plot, 1)

        # 游标 + 读数 HUD（对齐 VOFA+ 波形游标能力）。
        self._cursor_hud = waveform_overlays.build_cursor_hud(self)
        layout.addWidget(self._cursor_hud)

        # 多通道图例。
        self._legend = waveform_overlays.WaveformLegend(self)
        layout.addWidget(self._legend)

        self._status_label = QLabel(self.tr("Waveform: no samples"), self)
        self._status_label.setObjectName("serialStationWaveformStatusLabel")
        layout.addWidget(self._status_label)

    def update_batch(self, batch: ChannelBatch) -> None:
        self._ensure_curves(batch.channel_names)
        self._ensure_cursors()
        x_values = np.arange(batch.values.shape[0], dtype=np.float32)
        for channel_index, curve in enumerate(self._curves):
            if channel_index < batch.values.shape[1]:
                curve.setData(x_values, batch.values[:, channel_index])
            else:
                curve.setData([], [])
        self._update_legend(batch)
        self._update_cursor_hud(batch.values)
        self._status_label.setText(
            self.tr("{channels} channels / {samples} samples").format(
                channels=len(batch.channel_names),
                samples=batch.values.shape[0],
            )
        )

    def _ensure_cursors(self) -> None:
        """首次绘制后附加两条游标（只附加一次）。"""

        if self._cursor_x1 is not None:
            return
        self._cursor_x1, self._cursor_x2 = waveform_overlays.attach_cursors(self._plot)

    def _update_legend(self, batch: ChannelBatch) -> None:
        """刷新多通道图例的当前值。"""

        latest: tuple[float, ...] = ()
        if batch.values.size > 0:
            latest = tuple(float(v) for v in batch.values[-1])
        self._legend.update_channels(batch.channel_names, latest)

    def _update_cursor_hud(self, values: np.ndarray) -> None:
        """刷新游标读数 HUD。"""

        if self._cursor_x1 is None or self._cursor_x2 is None:
            return
        readout = waveform_overlays.cursor_readout(
            self._cursor_x1, self._cursor_x2, values
        )
        self._cursor_hud.setText(readout)

    def _ensure_curves(self, channel_names: tuple[str, ...]) -> None:
        while len(self._curves) < len(channel_names):
            index = len(self._curves)
            curve = self._plot.plot(
                pen=pg.intColor(index, hues=max(3, len(channel_names))),
                name=channel_names[index],
            )
            self._curves.append(curve)
        for index, curve in enumerate(self._curves):
            if index < len(channel_names):
                curve.setVisible(True)
            else:
                curve.setVisible(False)

    def shutdown(self) -> None:
        self.setUpdatesEnabled(False)
        self._plot.setUpdatesEnabled(False)
        self._plot.hide()
        self._plot.clear()
        self._curves.clear()

    def closeEvent(self, event: object) -> None:
        self.shutdown()
        super().closeEvent(event)
