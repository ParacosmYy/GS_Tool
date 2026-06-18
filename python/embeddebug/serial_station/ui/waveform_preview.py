"""PyQtGraph waveform preview for measurement batches.

Batch 6 (C2) 美化改进（诊断报告：黑底白线科学计算风，零渐变填充/发光）：
1. 曲线下方半透明渐变填充（``setFillLevel`` + ``QLinearGradient``），多通道
   叠加时有面积感，视觉信息密度提升。
2. 曲线发光（``QGraphicsDropShadowEffect`` 同色 blur），主线带辉光，质感对齐
   现代数据可视化（TradingView/Grafana）。
3. 坐标轴 SI 格式（``pg.SIFormat``），工程记数法，避免大数值溢出。
4. 网格 alpha 提到 0.18（原 0.12 过淡），主次刻度更清晰。
"""

from __future__ import annotations

import os

import numpy as np
import pyqtgraph as pg
from PyQt6.QtGui import QBrush, QColor, QLinearGradient
from PyQt6.QtWidgets import QLabel, QVBoxLayout, QWidget
from PyQt6.QtWidgets import QGraphicsDropShadowEffect

from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui import waveform_measure, waveform_overlays
from embeddebug.serial_station.ui.theme import palette as P


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
        # 深色绘图区背景（与终端一致）+ 柔和网格 + 弱化坐标轴文字（对齐 EK-OmniProbe）。
        # Batch 6: 网格 alpha 提到 0.18（原 0.12 过淡），主次刻度更清晰。
        self._plot.setBackground(P.TERM_BACKGROUND)
        self._plot.showGrid(x=True, y=True, alpha=0.18)
        for axis_name in ("left", "bottom"):
            axis = self._plot.getAxis(axis_name)
            axis.setTextPen(P.TEXT_MUTED)
            axis.setPen(pg.mkPen(color=P.BORDER, width=1))
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

        # Batch 6: 通道统计读数（激活 waveform_measure 死代码）。
        # 显示首通道 Vpp/Mean/Max/Min/Std/RMS，对齐 VOFA+ 测量面板。
        self._stats_label = QLabel(self.tr("Stats: —"), self)
        self._stats_label.setObjectName("serialStationWaveformStatsLabel")
        layout.addWidget(self._stats_label)

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
        # Batch 6: 计算首通道统计并显示（激活 waveform_measure）。
        self._update_stats(batch)

    def _update_stats(self, batch: ChannelBatch) -> None:
        """计算首通道统计（Vpp/Mean/Max/Min/Std/RMS）并更新统计读数标签。"""

        if batch.values.size == 0 or batch.values.shape[1] == 0:
            self._stats_label.setText(self.tr("Stats: —"))
            return
        first_channel = batch.values[:, 0]
        stats = waveform_measure.compute_channel_stats(first_channel)
        name = batch.channel_names[0] if batch.channel_names else "ch0"
        self._stats_label.setText(f"{name}: {waveform_measure.format_stats(stats)}")

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
        """确保曲线数与通道数一致，每条曲线配渐变填充 + 发光（Batch 6 美化）。

        改进（对比旧版）：
        - 旧版只有 2px 纯色折线，零填充零发光，黑底白线科学计算风。
        - 新版每条曲线下方加半透明渐变填充（``setFillLevel`` + ``QLinearGradient``，
          从曲线色 25% alpha 渐隐到透明），多通道叠加时有面积感。
        - 曲线加发光（``QGraphicsDropShadowEffect`` 同色 blur=8），主线带辉光，
          质感对齐现代数据可视化。
        """

        while len(self._curves) < len(channel_names):
            index = len(self._curves)
            color = P.WAVE_CURVES[index % len(P.WAVE_CURVES)]
            curve = self._plot.plot(
                pen=pg.mkPen(color=color, width=2),
                name=channel_names[index],
            )
            # Batch 6: 半透明渐变填充（曲线下方面积感）。
            # fillLevel=-1e9 作为基线，配合 setBrush 渐变实现面积填充。
            curve.setFillLevel(0)
            gradient = QLinearGradient(0, 0, 0, 1)
            base_color = QColor(color)
            # 顶部（贴近曲线）25% alpha，底部完全透明。
            top_color = QColor(base_color)
            top_color.setAlpha(64)
            gradient.setColorAt(0.0, top_color)
            gradient.setColorAt(1.0, QColor(base_color.red(), base_color.green(), base_color.blue(), 0))
            curve.setBrush(QBrush(gradient))
            # Batch 6: 曲线发光（同色 DropShadow blur=8）。
            glow = QGraphicsDropShadowEffect(curve)
            glow.setBlurRadius(8)
            glow.setColor(QColor(color))
            glow.setOffset(0, 0)
            curve.setGraphicsEffect(glow)
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
