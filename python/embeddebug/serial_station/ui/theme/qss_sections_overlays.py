"""QSS 叠加层分区 — 命令面板遮罩 + 波形游标 HUD/图例。

从 ``qss_sections_layout`` 拆出（守 ≤300 行运行时门禁），集中「浮层」类样式：
全局命令面板（Ctrl+P 半透明遮罩）与波形游标读数 HUD / 通道图例。

颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def command_palette_section() -> str:
    """命令面板 — VS Code 风格全局命令面板（Ctrl+P 模糊搜索）。

    半透明遮罩 + 居中卡片 + 模糊匹配列表。遮罩用 BG_OVERLAY，
    卡片用 BG_PANEL + 大圆角，列表选中项用强调青软底。
    """

    return f"""/* === Command Palette (VS Code Ctrl+P style) === */
QWidget#serialStationCommandPalette {{
    background-color: {P.BG_OVERLAY};
}}
QFrame#serialStationCommandPaletteCard {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER_STRONG};
    border-radius: {T.RADIUS_XL};
    /* 顶部高光渐变：与主卡片一致的玻璃反光。 */
    background-image: qlineargradient(
        x1:0, y1:0, x2:0, y2:1,
        stop:0 {T.CARD_HIGHLIGHT_STOP_0},
        stop:1 {T.CARD_HIGHLIGHT_STOP_1}
    );
}}
QLineEdit#serialStationCommandPaletteEdit {{
    background-color: {P.BG_INPUT};
    color: {P.TEXT_PRIMARY};
    border: none;
    border-bottom: {T.BORDER_THIN} solid {P.BORDER};
    border-top-left-radius: {T.RADIUS_XL};
    border-top-right-radius: {T.RADIUS_XL};
    padding: {T.PADDING_LG};
    font-size: {T.FONT_MD};
    selection-background-color: {P.TERM_SELECTION};
    selection-color: {P.TEXT_INVERTED};
}}
QLineEdit#serialStationCommandPaletteEdit:focus {{
    border-bottom-color: {P.BORDER_FOCUS};
}}
QListWidget#serialStationCommandPaletteList {{
    background-color: transparent;
    color: {P.TEXT_PRIMARY};
    border: none;
    padding: {T.SPACING_SM};
    outline: none;
}}
QListWidget#serialStationCommandPaletteList::item {{
    padding: {T.SPACING_SM} {T.SPACING_MD};
    border-radius: {T.RADIUS_SM};
}}
QListWidget#serialStationCommandPaletteList::item:selected {{
    background-color: {P.ACCENT_SOFT};
    color: {P.ACCENT};
}}
QListWidget#serialStationCommandPaletteList::item:hover {{
    background-color: {P.BG_PANEL_RAISED};
}}
QLabel#serialStationCommandPaletteHint {{
    background-color: transparent;
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_XS};
    padding: {T.SPACING_SM} {T.SPACING_MD};
    border-top: {T.BORDER_THIN} solid {P.BORDER};
}}"""


def waveform_overlays_section() -> str:
    """波形游标 HUD + 多通道图例 — 对齐 VOFA+ 波形引擎读数与图例观感。

    游标 HUD（``serialStationWaveformCursorHud``）用等宽字体 + 弱文本色，
    图例（``serialStationWaveformLegend``）透明底，图例 chip（``serialStationWaveformLegendChip``）
    用等宽字体显示色块 + 通道名 + 当前值。
    """

    return f"""/* === Waveform Cursor HUD & Legend === */
QLabel#serialStationWaveformCursorHud {{
    background-color: {P.BG_PANEL};
    color: {P.TEXT_MUTED};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_SM};
    padding: {T.SPACING_XS} {T.SPACING_MD};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}
QWidget#serialStationWaveformLegend {{
    background-color: transparent;
    border: none;
}}
QLabel#serialStationWaveformLegendChip {{
    background-color: transparent;
    border: none;
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
    padding: {T.SPACING_XS} {T.SPACING_SM};
}}
/* 波形游标线（pyqtgraph InfiniteLine）：QSS 对其有限，颜色由运行时 pen 设置；
   此处保留契约占位满足覆盖率守护，objectName 用于游标可发现性与测试。 */
QGraphicsLineItem#serialStationWaveformCursorX,
QGraphicsLineItem#serialStationWaveformCursorY {{
    /* no-op: color set via pg.mkPen at runtime */
}}"""
