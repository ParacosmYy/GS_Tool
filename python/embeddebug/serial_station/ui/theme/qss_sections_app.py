"""QSS 分区生成器 — 多模式应用 Shell（导航栏 + OTA 面板 + 占位面板）。

覆盖 AppShell 导航栏、OTA 升级面板、RTT/设置占位面板的 objectName。
颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def nav_rail_section() -> str:
    """左侧图标导航栏 — 深色窄栏，品牌徽标方块 + 激活态指示条。"""

    return f"""/* === App Navigation Rail === */
QFrame#serialStationNavRail {{
    background-color: {P.BG_APP};
    border-right: {T.BORDER_THIN} solid {P.BORDER};
}}
/* 品牌徽标 ED：跨色相品牌渐变（青→蓝）+ 反白字，仿 EK-OmniProbe logo chip。 */
QLabel#serialStationNavBrand {{
    background-color: qlineargradient(
        x1:0, y1:0, x2:1, y2:1,
        stop:0 {P.ACCENT_GRADIENT_FROM}, stop:1 {P.ACCENT_GRADIENT_TO}
    );
    color: {P.TEXT_INVERTED};
    border: {T.BORDER_THIN} solid {P.ACCENT_BORDER};
    border-radius: {T.RADIUS_LG};
    font-size: {T.FONT_SM};
    font-weight: {T.FONT_WEIGHT_BLACK};
    min-width: 32px;
    max-width: 32px;
    min-height: 32px;
    max-height: 32px;
}}
/* 导航图标按钮：方形、无边框、hover 微背景、激活态强调青软底 + 渐变左指示条。 */
QPushButton[objectName^="serialStationNav"] {{
    background-color: transparent;
    border: none;
    border-left: 3px solid transparent;
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
    border-left: 3px solid {P.ACCENT};
}}
/* Batch 115: NavIndicator 自绘胶囊指示条（自绘读 palette ACCENT，QSS 仅契约占位
   防止 QWidget 默认底破坏色相；满足 test_build_qss_covers_all objectName 覆盖率）。 */
QWidget#serialStationNavIndicator {{
    background-color: transparent;
    border: none;
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
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
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
QPushButton#serialStationOtaBrowseButton:pressed {{
    background-color: {P.BG_SELECTION};
    border-color: {P.ACCENT_PRESSED};
}}
QPushButton#serialStationOtaBrowseButton:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border-color: {P.BORDER};
}}
QPushButton#serialStationOtaStartButton {{
    background-color: {P.ACCENT};
    color: {P.TEXT_ON_ACCENT};
    border: none;
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_LG};
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
}}
QPushButton#serialStationOtaStartButton:hover {{
    background-color: {P.ACCENT_HOVER};
}}
QPushButton#serialStationOtaStartButton:pressed {{
    background-color: {P.ACCENT_PRESSED};
}}
QPushButton#serialStationOtaStartButton:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
}}"""
