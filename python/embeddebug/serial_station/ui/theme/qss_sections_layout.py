"""QSS 分区生成器 — 三栏 Shell 布局与玻璃卡片。

把卡片（``serialStationCard``）、三栏 zone 容器、QSplitter 把手与 TopBar
的样式集中在本模块，便于按 EK-OmniProbe ``surface-card`` 玻璃卡片语言统一深化，
同时减轻 ``qss_sections_widgets`` 的行数压力（仓库运行时文件 ≤300 行门禁）。

QSS 无 ``backdrop-filter: blur()``，玻璃感通过分层渐变背景、半透明边框、
顶部高光（``qlineargradient``）与悬浮提亮近似：
- 卡片底：``BG_PANEL``；顶部叠一道极淡白色高光渐变（模拟玻璃反光）。
- 边框：默认 ``BORDER``；悬浮/聚焦转强调青软边（``ACCENT_BORDER``）。
- 标题行：底部细分隔线，标题字重 600，图标透明底（着色由 IconManager 负责）。
- 主信息卡（波形/日志）用更大圆角与更宽松内边距，拉开视觉层级。

颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def cards_section() -> str:
    """玻璃卡片样式 — 对齐 EK-OmniProbe surface-card（深色工业风落地）。"""

    return f"""/* === Glass Cards (EK-OmniProbe surface-card, dark industrial) === */
QFrame#serialStationCard {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_2XL};
    /* 顶部高光渐变：模拟玻璃 inner highlight（QSS 无 blur 的近似手段）。 */
    background-image: qlineargradient(
        x1:0, y1:0, x2:0, y2:1,
        stop:0 {T.CARD_HIGHLIGHT_STOP_0},
        stop:1 {T.CARD_HIGHLIGHT_STOP_1}
    );
    padding: {T.PADDING_CARD};
}}
QFrame#serialStationCard:hover {{
    border-color: {P.ACCENT_BORDER};
    background-color: {P.BG_PANEL_RAISED};
}}
/* 卡片标题行：底部细分隔线，拉开标题与内容。 */
QWidget#serialStationCardHeader {{
    background-color: transparent;
    border: none;
    border-bottom: {T.BORDER_THIN} solid {P.BORDER};
    padding-bottom: {T.SPACING_SM};
}}
QLabel#serialStationCardTitle {{
    background-color: transparent;
    color: {P.TEXT_PRIMARY};
    font-size: {T.FONT_MD};
    font-weight: 600;
    letter-spacing: 0.2px;
}}
QLabel#serialStationCardIcon {{
    background-color: transparent;
    border: none;
}}
/* 连接卡内的小分组标题（Port/Serial/Connect/Endpoints）：弱文本色小字号。 */
QLabel#serialStationCardGroupLabel {{
    background-color: transparent;
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_XS};
    font-weight: 600;
    letter-spacing: 0.6px;
    text-transform: uppercase;
    padding-top: {T.SPACING_SM};
    padding-bottom: {T.SPACING_XS};
}}
/* 日志卡主体容器：保持透明，让 QPlainTextEdit 自身样式生效。 */
QVBoxLayout#serialStationCardBody,
QVBoxLayout#serialStationLogCardBody {{
    background-color: transparent;
    border: none;
}}"""


def zones_section() -> str:
    """三栏 zone 容器 — 透明底，让卡片浮在窗口底色上，并对称内边距。"""

    return f"""/* === Three-Zone Shell Containers === */
QWidget#serialStationLeftZone,
QWidget#serialStationCenterZone,
QWidget#serialStationRightZone {{
    background-color: transparent;
    border: none;
}}"""


def splitter_section() -> str:
    """QSplitter 把手 — 收窄为分隔线色，悬浮转强调青，提升分栏可达性。"""

    return f"""/* === Splitter Handles === */
QSplitter#serialStationMainSplitter::handle {{
    background-color: {P.BORDER};
}}
QSplitter#serialStationMainSplitter::handle:hover {{
    background-color: {P.ACCENT_BORDER};
}}
QSplitter#serialStationMainSplitter::handle:horizontal {{
    width: 2px;
    margin: {T.SPACING_MD} 0;
}}
QSplitter#serialStationMainSplitter::handle:vertical {{
    height: 2px;
    margin: 0 {T.SPACING_MD};
}}"""


def topbar_section() -> str:
    """TopBar — 品牌区 + 状态药丸 + Profile，对齐 EK-OmniProbe TopBar shell。"""

    return f"""/* === TopBar (Brand + Status + Profile) === */
QFrame#serialStationTopBar {{
    background-color: {P.TOPBAR_BG_TOP};
    background-image: qlineargradient(
        x1:0, y1:0, x2:0, y2:1,
        stop:0 {P.TOPBAR_BG_TOP},
        stop:1 {P.TOPBAR_BG_BOTTOM}
    );
    border: {T.BORDER_THIN} solid {P.TOPBAR_BORDER};
    border-radius: {T.RADIUS_XL};
}}
/* 品牌徽标 chip：强调青软底 + 图标。 */
QFrame#serialStationBrandChip {{
    background-color: {P.BRAND_CHIP_BG};
    border: {T.BORDER_THIN} solid {P.BRAND_CHIP_BORDER};
    border-radius: {T.RADIUS_MD};
}}
QLabel#serialStationBrandIcon {{
    background-color: transparent;
    border: none;
}}
QLabel#serialStationBrandName {{
    background-color: transparent;
    color: {P.TEXT_PRIMARY};
    font-size: {T.FONT_MD};
    font-weight: 600;
    letter-spacing: 0.3px;
}}
QLabel#serialStationBrandTagline {{
    background-color: transparent;
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_XS};
}}"""


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
}}"""
