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
from embeddebug.serial_station.ui.animations.tokens import AnimationTokens
from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.waveform_cursors import CursorManager
from embeddebug.serial_station.ui.waveform_perf import BatchAccumulator, RefreshThrottle


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
        # Batch 7: 用 CursorManager 替换固定双游标（激活 waveform_cursors 死代码）。
        # 支持运行时增删 X/Y 游标，配合 compute_cursor_measurement 计算测量值。
        self._cursor_manager: CursorManager | None = None
        self._sample_rate = 1.0  # 默认采样率 1Hz（ΔT 即 Δindex），供 cursor 测量计算

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

        # Batch 9-3: 游标交互（双击添加 X 游标，右键游标删除）。
        self._install_cursor_interactions()

        # 游标 + 读数 HUD（对齐 VOFA+ 波形游标能力）。
        self._cursor_hud = waveform_overlays.build_cursor_hud(self)
        layout.addWidget(self._cursor_hud)

        # 多通道图例。
        self._legend = waveform_overlays.WaveformLegend(self)
        layout.addWidget(self._legend)

        self._status_label = QLabel(self.tr("Waveform: no samples"), self)
        self._status_label.setObjectName("serialStationWaveformStatusLabel")
        layout.addWidget(self._status_label)

        # 首通道 Vpp/Mean/Max/Min/Std/RMS，对齐 VOFA+ 测量面板。
        self._stats_label = QLabel(self.tr("Stats: —"), self)
        self._stats_label.setObjectName("serialStationWaveformStatsLabel")
        layout.addWidget(self._stats_label)

        # 热路径：批次累积 + 60Hz 节流刷新，避免高频逐批 setData 卡 UI。
        self._accumulator = BatchAccumulator(flush_interval_ms=50, parent=self)
        self._accumulator.flush_signal.connect(self._on_accumulator_flush)
        self._accumulator.start()
        self._throttle = RefreshThrottle(
            callback=self._flush_latest_to_plot,
            target_hz=60,
            parent=self,
        )
        self._latest_batch: ChannelBatch | None = None
        self._is_shutting_down = False
        # Batch 49-3: 空态 + 加载态覆盖层（helper 在 waveform_empty_state 模块）。
        from embeddebug.serial_station.ui.waveform_empty_state import build_waveform_overlays

        self._empty_overlay, self._loading_overlay = build_waveform_overlays(self)
        self._empty_overlay.show_with_fade()

    def submit_batch(self, batch: ChannelBatch) -> None:
        """热路径入口：累积批次并节流刷新。"""

        if self._is_shutting_down:
            return
        self._accumulator.push(batch.values, batch.channel_names, batch.dt_ns)

    def resizeEvent(self, event: object) -> None:
        super().resizeEvent(event)
        rect = self.rect()
        for overlay in (self._empty_overlay, self._loading_overlay):
            if overlay is not None:
                overlay.setGeometry(rect)

    def set_connecting(self, connecting: bool) -> None:
        """Batch 49-3: 切换连接加载态（连接中显示 ProgressRing 覆盖层）。"""

        if connecting:
            self._empty_overlay.hide()
            self._loading_overlay.show()
            self._loading_overlay.raise_()
        else:
            self._loading_overlay.hide()
            if self._latest_batch is None:
                self._empty_overlay.show_with_fade()

    def _on_accumulator_flush(self, merged: ChannelBatch) -> None:
        """BatchAccumulator flush 回调：缓存最新合并批次，请求节流刷新。"""

        if self._is_shutting_down:
            return
        self._latest_batch = merged
        self._throttle.maybe_refresh()

    def _flush_latest_to_plot(self) -> None:
        """RefreshThrottle 触发的真实刷新：把最新合并批次推到 plot。"""

        if self._latest_batch is not None:
            self.update_batch(self._latest_batch)
            self._latest_batch = None

    def update_batch(self, batch: ChannelBatch) -> None:
        # Batch 49-3: 首个真实刷新批次到达，淡出空态（仅一次，覆盖 submit_batch
        # 节流路径与直接 update_batch 两条入口）。
        if not self._curves and not self._empty_overlay.isHidden():
            self._empty_overlay.hide_with_fade()
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
        """首次绘制后初始化 CursorManager（替换旧固定双游标，Batch 7）。"""

        if self._cursor_manager is not None:
            return
        self._cursor_manager = CursorManager(self._plot)
        # 默认两条 X 游标（对齐旧 attach_cursors 的 25%/75% 位置）。
        self._cursor_manager.add_x_cursor(0.25)
        self._cursor_manager.add_x_cursor(0.75)

    def _install_cursor_interactions(self) -> None:
        """装双击添加 X 游标 + 右键删除菜单（委托独立模块，Batch 9-3）。"""

        from embeddebug.serial_station.ui.waveform_cursor_interactions import (
            install_cursor_interactions,
        )

        install_cursor_interactions(self._plot, lambda: self._cursor_manager)

    def _update_legend(self, batch: ChannelBatch) -> None:
        """刷新多通道图例的当前值。"""

        latest: tuple[float, ...] = ()
        if batch.values.size > 0:
            latest = tuple(float(v) for v in batch.values[-1])
        self._legend.update_channels(batch.channel_names, latest)

    def _update_cursor_hud(self, values: np.ndarray) -> None:
        """刷新游标读数 HUD（compute_cursor_measurement + format，Batch 7）。"""

        if self._cursor_manager is None:
            return
        x_values, y_values = self._cursor_manager.cursor_values()
        measurement = waveform_measure.compute_cursor_measurement(
            x_values, y_values, self._sample_rate
        )
        self._cursor_hud.setText(waveform_measure.format_cursor_measurement(measurement))

    def cursor_manager(self) -> CursorManager | None:
        """返回当前 CursorManager（供外部增删游标，None 表示尚未初始化）。"""

        return self._cursor_manager

    def set_sample_rate(self, rate: float) -> None:
        """设置采样率（Hz），供游标 ΔT/频率测量计算。

        默认 1.0（ΔT 即 Δindex）；真实采样率已知时设置可获得正确的时间/频率读数。
        """

        self._sample_rate = max(0.0, float(rate))

    def _ensure_curves(self, channel_names: tuple[str, ...]) -> None:
        """确保曲线数与通道数一致，配渐变填充 + 发光（Batch 6 美化）。"""

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
            glow.setBlurRadius(AnimationTokens.SHADOW_BLUR_CURVE_GLOW)
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
        if self._is_shutting_down:
            return
        self._is_shutting_down = True
        self.setUpdatesEnabled(False)
        self._throttle.stop()
        try:
            self._accumulator.flush_signal.disconnect(self._on_accumulator_flush)
        except TypeError:
            pass
        self._accumulator.stop()
        self._plot.setUpdatesEnabled(False)
        self._plot.hide()
        self._plot.clear()
        self._curves.clear()

    def closeEvent(self, event: object) -> None:
        self.shutdown()
        super().closeEvent(event)
