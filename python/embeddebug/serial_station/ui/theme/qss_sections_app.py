"""QSS 分区生成器 — 多模式应用 Shell（导航栏 + OTA 面板 + 占位面板）。

覆盖 AppShell 导航栏、OTA 升级面板、RTT/设置占位面板的 objectName。
颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def nav_rail_section() -> str:
    """左侧图标导航栏 — 深色窄栏，激活项强调青软底。"""

    return f"""/* === App Navigation Rail === */
QFrame#serialStationNavRail {{
    background-color: {P.BG_APP};
    border-right: {T.BORDER_THIN} solid {P.BORDER};
}}
QLabel#serialStationNavBrand {{
    color: {P.ACCENT};
    font-size: {T.FONT_SM};
    font-weight: 700;
    padding: {T.SPACING_SM} 0;
}}
/* 导航图标按钮：方形、无边框、激活态强调青软底。 */
QPushButton[objectName^="serialStationNav"] {{
    background-color: transparent;
    border: none;
    border-radius: {T.RADIUS_MD};
    padding: {T.SPACING_MD};
    margin: 0 {T.SPACING_SM};
    min-width: 40px;
    min-height: 40px;
}}
QPushButton[objectName^="serialStationNav"]:hover {{
    background-color: {P.BG_PANEL_RAISED};
}}
QPushButton[objectName^="serialStationNav"]:checked {{
    background-color: {P.ACCENT_SOFT};
    border-left: {T.BORDER_THICK} solid {P.ACCENT};
}}"""


def ota_section() -> str:
    """OTA 升级面板控件 — 文件/协议/进度/日志/状态/按钮。"""

    return f"""/* === OTA Panel === */
QWidget#serialStationOtaPanel {{
    background-color: {P.BG_APP};
}}
QLabel#serialStationOtaFieldLabel {{
    color: {P.TEXT_SECONDARY};
    font-size: {T.FONT_SM};
    font-weight: 600;
}}
QComboBox#serialStationOtaProtocolCombo {{
    background-color: {P.BG_INPUT};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.BORDER_STRONG};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT};
    min-height: {T.CONTROL_HEIGHT_MD};
}}
QFrame#serialStationOtaFileEdit {{
    background-color: {P.BG_INPUT};
    border: {T.BORDER_THIN} solid {P.BORDER_STRONG};
    border-radius: {T.RADIUS_MD};
    min-height: {T.CONTROL_HEIGHT_MD};
}}
QLabel#serialStationOtaFileLabel {{
    color: {P.TEXT_MUTED};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}
QProgressBar#serialStationOtaProgress {{
    background-color: {P.BG_INPUT};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
    text-align: center;
    color: {P.TEXT_PRIMARY};
    min-height: {T.CONTROL_HEIGHT_MD};
}}
QProgressBar#serialStationOtaProgress::chunk {{
    background-color: {P.ACCENT};
    border-radius: {T.RADIUS_MD};
}}
QPlainTextEdit#serialStationOtaLog {{
    background-color: {P.TERM_BACKGROUND};
    color: {P.TERM_SYSTEM};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}
QLabel#serialStationOtaStatusLabel {{
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_SM};
    padding: 0 {T.SPACING_MD};
}}
QPushButton#serialStationOtaBrowseButton {{
    background-color: {P.BG_PANEL};
    color: {P.TEXT_SECONDARY};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_MD};
}}
QPushButton#serialStationOtaBrowseButton:hover {{
    background-color: {P.BG_PANEL_RAISED};
    border-color: {P.ACCENT_BORDER};
}}
QPushButton#serialStationOtaStartButton {{
    background-color: {P.ACCENT};
    color: {P.TEXT_ON_ACCENT};
    border: none;
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_LG};
    font-weight: 600;
}}
QPushButton#serialStationOtaStartButton:hover {{
    background-color: {P.ACCENT_HOVER};
}}
QPushButton#serialStationOtaStartButton:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
}}"""
