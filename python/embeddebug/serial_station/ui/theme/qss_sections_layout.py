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
    """玻璃卡片样式 — 3-stop 光带 + per-side border 近似 EK-OmniProbe surface-card。"""

    return f"""/* === Glass Cards (EK-OmniProbe surface-card, dark industrial) === */
QFrame#serialStationCard {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-top-color: {P.CARD_INNER_TOP_EDGE};
    border-bottom-color: {P.CARD_GROUND_SHADOW};
    border-radius: {T.RADIUS_2XL};
    /* 3-stop 顶部光带：压缩到顶部 ~18%，模拟玻璃 inner highlight + 反光带。 */
    background-image: qlineargradient(
        x1:0, y1:0, x2:0, y2:1,
        stop:0 {P.CARD_SHEEN_TOP},
        stop:0.18 {P.CARD_SHEEN_MID},
        stop:0.5 {P.CARD_SHEEN_BOTTOM}
    );
    padding: {T.PADDING_CARD};
}}
QFrame#serialStationCard:hover {{
    border: {T.BORDER_THIN} solid {P.CARD_HOVER_RING};
    background-color: {P.BG_PANEL_RAISED};
}}
/* 卡片标题行：底部柔和分隔线（半透明 + hover 联动转强调青）。 */
QWidget#serialStationCardHeader {{
    background-color: transparent;
    border: none;
    border-bottom: {T.BORDER_THIN} solid {P.BORDER};
    padding-bottom: {T.SPACING_SM};
}}
QFrame#serialStationCard:hover QWidget#serialStationCardHeader {{
    border-bottom-color: {P.ACCENT_BORDER};
}}
QLabel#serialStationCardTitle {{
    background-color: transparent;
    color: {P.TEXT_PRIMARY};
    font-size: {T.FONT_MD};
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
    letter-spacing: {T.LETTER_SPACING_SUBTLE};
}}
QLabel#serialStationCardIcon {{
    background-color: transparent;
    border: none;
}}
/* 可折叠面板 */
QWidget#serialStationCollapsiblePanel {{
    background-color: transparent;
    border: none;
}}
/* 连接卡内的小分组标题（Port/Serial/Connect/Endpoints）：弱文本色小字号。 */
QLabel#serialStationCardGroupLabel {{
    background-color: transparent;
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_XS};
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
    letter-spacing: {T.LETTER_SPACING_UPPERCASE};
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
}}
/* 串口配置面板容器（连接侧栏包装层） */
QWidget#serialStationSerialPanel {{
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
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
    letter-spacing: {T.LETTER_SPACING_BRAND};
}}
QLabel#serialStationBrandTagline {{
    background-color: transparent;
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_XS};
}}"""


def collapsible_section() -> str:
    """可折叠卡 — 嵌套在连接卡内的低频组容器，标题行可点击切换。"""

    return f"""/* === Collapsible Card (connection sidebar low-freq groups) === */
QFrame#serialStationCollapsibleCard {{
    background-color: {P.BG_INPUT};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
}}
QWidget#serialStationCollapsibleHeader {{
    background-color: transparent;
    border: none;
    padding: {T.SPACING_XS} {T.SPACING_SM};
}}
QWidget#serialStationCollapsibleHeader:hover {{
    background-color: {P.BG_PANEL_RAISED};
    border-radius: {T.RADIUS_SM};
}}
QLabel#serialStationCollapsibleArrow {{
    background-color: transparent;
    border: none;
}}
QLabel#serialStationCollapsibleTitle {{
    background-color: transparent;
    color: {P.TEXT_SECONDARY};
    font-size: {T.FONT_XS};
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
    letter-spacing: {T.LETTER_SPACING_WIDE};
    text-transform: uppercase;
}}
QWidget#serialStationCollapsibleBody {{
    background-color: transparent;
    border: none;
}}"""
