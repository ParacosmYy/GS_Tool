"""波形游标与多通道图例（对齐 VOFA+ 波形引擎能力）。

为 SerialWaveformPreview 提供两条可拖拽游标（X1/X2）和通道图例：
- 游标：点击波形区放置 X1，按住拖拽移动；显示 ΔX 与两游标处 Y 值。
- 图例：按通道名列出颜色块 + 名称 + 当前值，点击切换可见性。

设计要点：
- 游标用 pyqtgraph ``InfiniteLine``，可拖拽，颜色取 palette 强调色。
- 图例是 QWidget（QHBoxLayout of QLabel），objectName 带 ``serialStationWaveformLegend`` 前缀。
- 读数 HUD 显示游标间距与 Y 值，objectName ``serialStationWaveformCursorHud``。

约束：本模块只依赖 PyQt6 + pyqtgraph + theme.palette，不访问 controller/transport。
"""

from __future__ import annotations

import numpy as np
import pyqtgraph as pg
from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QFrame, QHBoxLayout, QLabel, QWidget

from embeddebug.serial_station.ui.theme import palette as P


def attach_cursors(plot: pg.PlotWidget) -> tuple[pg.InfiniteLine, pg.InfiniteLine]:
    """给绘图区附加两条可拖拽 X 游标，返回 (cursor_x1, cursor_x2)。"""

    x1 = _make_cursor(P.ACCENT, 0.25)
    x2 = _make_cursor(P.WARNING, 0.75)
    plot.addItem(x1)
    plot.addItem(x2)
    return x1, x2


def _make_cursor(color: str, start_ratio: float) -> pg.InfiniteLine:
    line = pg.InfiniteLine(
        pos=start_ratio,
        angle=90,
        pen=pg.mkPen(color=color, width=1.4, style=Qt.PenStyle.DashLine),
        movable=True,
        label="{value:.3f}",
        labelOpts={"position": 0.96, "color": color, "fill": P.BG_PANEL},
    )
    return line


def cursor_readout(
    x1: pg.InfiniteLine,
    x2: pg.InfiniteLine,
    values: np.ndarray | None,
) -> str:
    """计算两游标的 ΔX 与各自处 Y 值（取首通道插值），返回读数字符串。"""

    xa = float(x1.value())
    xb = float(x2.value())
    delta = abs(xb - xa)
    y1 = y2 = float("nan")
    if values is not None and values.size > 0:
        y1 = _sample_y(values, xa)
        y2 = _sample_y(values, xb)
    return f"ΔX {delta:.3f}  ·  Y1 {y1:.3f}  ·  Y2 {y2:.3f}"


def _sample_y(values: np.ndarray, x: float) -> float:
    """对首通道在 x 处做线性插值取 Y。"""

    n = values.shape[0]
    if n == 0:
        return float("nan")
    col = values[:, 0] if values.ndim == 2 else values
    idx = int(round(x))
    idx = max(0, min(n - 1, idx))
    return float(col[idx])


class WaveformLegend(QWidget):
    """多通道图例：颜色块 + 名称 + 当前值，点击切换可见性。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationWaveformLegend")
        self._layout = QHBoxLayout(self)
        self._layout.setContentsMargins(8, 4, 8, 4)
        self._layout.setSpacing(12)
        self._chips: dict[str, QLabel] = {}

    def update_channels(
        self,
        channel_names: tuple[str, ...],
        latest_values: tuple[float, ...] | None = None,
    ) -> None:
        """刷新图例通道色块与当前值。"""

        self._clear_chips()
        for index, name in enumerate(channel_names):
            color_hex = P.WAVE_CURVES[index % len(P.WAVE_CURVES)]
            color = pg.QtGui.QColor(color_hex)
            chip = self._make_chip(name, color, latest_values, index)
            self._layout.addWidget(chip)
            self._chips[name] = chip
        self._layout.addStretch(1)

    def _clear_chips(self) -> None:
        for chip in self._chips.values():
            chip.setParent(None)
            chip.deleteLater()
        self._chips.clear()

    def _make_chip(
        self,
        name: str,
        color: pg.QtGui.QColor,
        latest_values: tuple[float, ...] | None,
        index: int,
    ) -> QLabel:
        hex_color = color.name()
        value_text = ""
        if latest_values is not None and index < len(latest_values):
            value_text = f"  {latest_values[index]:.3f}"
        chip = QLabel(self)
        chip.setObjectName("serialStationWaveformLegendChip")
        chip.setText(f"⬤ {name}{value_text}")
        chip.setStyleSheet(
            f"color: {P.TEXT_SECONDARY}; font-family: {P.BG_PANEL};"
            f"QLabel {{ color: {hex_color}; }}"
        )
        chip.setStyleSheet(f"color: {hex_color}; font-size: 12px;")
        return chip


def build_cursor_hud(parent: QWidget) -> QLabel:
    """构建游标读数 HUD 标签。"""

    hud = QLabel(parent)
    hud.setObjectName("serialStationWaveformCursorHud")
    hud.setText("ΔX —  ·  Y1 —  ·  Y2 —")
    hud.setStyleSheet(f"color: {P.TEXT_MUTED}; font-size: 12px; padding: 4px 8px;")
    return hud
