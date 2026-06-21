"""波形多通道图例 + 游标读数 HUD（对齐 VOFA+ 波形引擎能力）。

为 SerialWaveformPreview 提供多通道图例（颜色块 + 名称 + 当前值，点击切换可见性）
和游标读数 HUD 标签容器。

历史：旧版固定双游标 API（``attach_cursors`` / ``cursor_readout`` / ``_make_cursor``）
已被 ``waveform_cursors.CursorManager``（可增删任意数量 X/Y 游标）+ ``waveform_measure``
（ΔT/频率/ΔY 测量）取代，Batch 20 删除这些遗留死代码。

设计要点：
- 图例是 QWidget（QHBoxLayout of QLabel），objectName 带 ``serialStationWaveformLegend`` 前缀。
- 读数 HUD 是 QLabel 容器，objectName ``serialStationWaveformCursorHud``，
  实际读数由 waveform_measure.format_cursor_measurement 填充。

约束：本模块只依赖 PyQt6 + pyqtgraph + theme.palette，不访问 controller/transport。
"""

from __future__ import annotations

import pyqtgraph as pg
from PyQt6.QtWidgets import QHBoxLayout, QLabel, QWidget

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


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
        chip.setStyleSheet(f"color: {hex_color}; font-size: {T.FONT_SM};")
        return chip


def build_cursor_hud(parent: QWidget) -> QLabel:
    """构建游标读数 HUD 标签。"""

    hud = QLabel(parent)
    hud.setObjectName("serialStationWaveformCursorHud")
    hud.setText("ΔX —  ·  Y1 —  ·  Y2 —")
    hud.setStyleSheet(f"color: {P.TEXT_MUTED}; font-size: {T.FONT_SM}; padding: 4px 8px;")
    return hud
