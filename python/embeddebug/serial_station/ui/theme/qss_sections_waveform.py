"""QSS 波形/图表分区 — 波形预览 + 李萨如 + 条形图样式。

从 ``qss_sections_widgets`` 拆出（守 ≤300 行运行时门禁），集中波形引擎相关
控件样式：波形预览面板、多通道图例、游标 HUD、李萨如（X-Y）、条形图。

颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def waveform_section() -> str:
    return f"""/* === Waveform Preview === */
QWidget#serialStationWaveformPanel {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_LG};
}}
/* pyqtgraph PlotWidget：QSS 仅控制背景，绘图区背景在运行时通过
   setBackground 设置，此处保证控件框与面板一致。 */
QWidget#serialStationWaveformPlot,
QGraphicsView#serialStationWaveformPlot {{
    background-color: {P.TERM_BACKGROUND};
    border: none;
}}
QLabel#serialStationWaveformStatusLabel {{
    color: {P.TEXT_MUTED};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
    padding: {T.SPACING_SM} {T.SPACING_MD};
}}
QLabel#serialStationWaveformStatsLabel {{
    color: {P.TEXT_SECONDARY};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
    padding: {T.SPACING_XS} {T.SPACING_MD};
}}
/* 多通道图例（对齐 VOFA+ 通道色块） */
QWidget#serialStationWaveformLegend {{
    background-color: transparent;
    border-top: {T.BORDER_THIN} solid {P.BORDER};
}}
/* Batch 49-3: 连接加载态覆盖层（ProgressRing 居中 + 文案）。
   复用 BG_OVERLAY 模态遮罩色，覆盖在 plot 上方，对齐铁律 18 过渡动画语义。 */
QWidget#serialStationWaveformLoadingOverlay {{
    background-color: {P.BG_OVERLAY};
    border-radius: {T.RADIUS_LG};
}}
QWidget#serialStationWaveformLoadingRing {{
    background-color: transparent;
}}
QLabel#serialStationWaveformLoadingLabel {{
    color: {P.TEXT_PRIMARY};
    font-size: {T.FONT_MD};
    padding: {T.SPACING_SM} {T.SPACING_MD};
}}
QLabel#serialStationWaveformLegendChip {{
    background-color: transparent;
    font-size: {T.FONT_SM};
}}
/* 游标读数 HUD */
QLabel#serialStationWaveformCursorHud {{
    background-color: transparent;
    color: {P.TEXT_MUTED};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
    padding: {T.SPACING_XS} {T.SPACING_MD};
}}
/* 富游标 objectName 契约：pyqtgraph InfiniteLine 不可由 QSS 设置样式，
   样式通过运行时 pen 参数设置（强调青/警告黄），此处保留契约占位。 */
QObject#serialStationWaveformCursorX,
QObject#serialStationWaveformCursorY {{
    /* no-op */
}}
/* === 李萨如（X-Y）面板 === */
QWidget#serialStationLissajousPanel {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_LG};
}}
QWidget#serialStationLissajousPlot,
QGraphicsView#serialStationLissajousPlot {{
    background-color: {P.TERM_BACKGROUND};
    border: none;
}}
QLabel#serialStationLissajousStatus {{
    color: {P.TEXT_MUTED};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
    padding: {T.SPACING_XS} {T.SPACING_MD};
}}
/* === 条形图面板 === */
QWidget#serialStationBarChartPanel {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_LG};
}}
QWidget#serialStationBarChartPlot,
QGraphicsView#serialStationBarChartPlot {{
    background-color: {P.TERM_BACKGROUND};
    border: none;
}}
QLabel#serialStationBarChartStatus {{
    color: {P.TEXT_MUTED};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
    padding: {T.SPACING_XS} {T.SPACING_MD};
}}"""
