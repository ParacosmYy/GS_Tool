"""PyQtGraph waveform preview for measurement batches."""

from __future__ import annotations

import os

import numpy as np
import pyqtgraph as pg
from PyQt6.QtWidgets import QLabel, QVBoxLayout, QWidget

from embeddebug.serial_station.core import ChannelBatch


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

        self._status_label = QLabel(self.tr("Waveform: no samples"), self)
        self._status_label.setObjectName("serialStationWaveformStatusLabel")
        layout.addWidget(self._status_label)

    def update_batch(self, batch: ChannelBatch) -> None:
        self._ensure_curves(batch.channel_names)
        x_values = np.arange(batch.values.shape[0], dtype=np.float32)
        for channel_index, curve in enumerate(self._curves):
            if channel_index < batch.values.shape[1]:
                curve.setData(x_values, batch.values[:, channel_index])
            else:
                curve.setData([], [])
        self._status_label.setText(
            self.tr("{channels} channels / {samples} samples").format(
                channels=len(batch.channel_names),
                samples=batch.values.shape[0],
            )
        )

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
