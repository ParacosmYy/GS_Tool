"""QSS 分区生成器 — 控件组件库（LED/滑块/按钮/仪表盘/数值显示）。

对齐 VOFA+ 控件目录的样式。颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def controls_section() -> str:
    return f"""/* === Control Widgets (LED / Slider / Button / Gauge / Value) === */
/* 状态 LED：自绘圆形，QSS 仅控制容器背景。 */
QWidget#serialStationStatusLed {{
    background-color: transparent;
}}
/* 命令滑块容器与子控件 */
QWidget#serialStationCommandSlider {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
}}
QLabel#serialStationSliderLabel {{
    color: {P.TEXT_SECONDARY};
    font-size: {T.FONT_SM};
}}
QLabel#serialStationSliderValue {{
    color: {P.ACCENT};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}
QSlider#serialStationSliderTrack::groove:horizontal {{
    background: {P.BG_INPUT};
    height: 6px;
    border-radius: 3px;
}}
QSlider#serialStationSliderTrack::sub-page:horizontal {{
    background: {P.ACCENT};
    border-radius: 3px;
}}
QSlider#serialStationSliderTrack::handle:horizontal {{
    background: {P.TEXT_PRIMARY};
    width: 14px;
    height: 14px;
    margin: -5px 0;
    border-radius: 7px;
}}
/* 可配置按钮 */
QPushButton#serialStationConfigurableButton {{
    background-color: {P.BG_PANEL_RAISED};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.ACCENT_BORDER};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_MD};
}}
QPushButton#serialStationConfigurableButton:hover {{
    background-color: {P.ACCENT_SOFT};
    border-color: {P.ACCENT};
}}
QPushButton#serialStationConfigurableButton:pressed {{
    background-color: {P.BG_SELECTION};
}}
/* 仪表盘：自绘，QSS 仅控制背景。 */
QWidget#serialStationGauge {{
    background-color: transparent;
}}
/* 数值显示 */
QWidget#serialStationValueDisplay {{
    background-color: transparent;
}}
QLabel#serialStationValueLabel {{
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_XS};
}}
QLabel#serialStationValueNumber {{
    color: {P.TEXT_PRIMARY};
    font-weight: 600;
}}
QLabel#serialStationValueTrend {{
    background-color: transparent;
}}"""
