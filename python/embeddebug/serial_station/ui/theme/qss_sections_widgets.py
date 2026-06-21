"""QSS 分区生成器 — 交互控件与展示区。

包含按钮、日志视图、波形、状态标签、滚动条、快捷键契约分区。
卡片/三栏 Shell/TopBar 的样式已迁移到 ``qss_sections_layout``，便于统一深化。
颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T

_CONNECT_BUTTONS = [
    "serialStationConnectButton",
    "serialStationConnectSerialButton",
    "serialStationConnectTcpButton",
    "serialStationConnectUdpButton",
]

_TOOL_BUTTONS = [
    "serialStationRefreshPortsButton",
    "serialStationExportLogButton",
    "serialStationReplayLogButton",
    "serialStationSaveProfileButton",
    "serialStationLoadProfileButton",
    "serialStationClearButton",
    "serialStationInjectButton",
    "serialStationSendButton",
]


def buttons_section() -> str:
    connect_sel = ",\n".join(f"QPushButton#{n}" for n in _CONNECT_BUTTONS)
    connect_hover = ",\n".join(f"QPushButton#{n}:hover" for n in _CONNECT_BUTTONS)
    connect_press = ",\n".join(f"QPushButton#{n}:pressed" for n in _CONNECT_BUTTONS)
    connect_dis = ",\n".join(f"QPushButton#{n}:disabled" for n in _CONNECT_BUTTONS)
    tool_sel = ",\n".join(f"QPushButton#{n}" for n in _TOOL_BUTTONS)
    tool_hover = ",\n".join(f"QPushButton#{n}:hover" for n in _TOOL_BUTTONS)
    tool_press = ",\n".join(f"QPushButton#{n}:pressed" for n in _TOOL_BUTTONS)
    tool_dis = ",\n".join(f"QPushButton#{n}:disabled" for n in _TOOL_BUTTONS)
    return f"""/* === Buttons === */
QPushButton {{
    background-color: {P.BG_PANEL};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_MD};
    min-height: {T.CONTROL_HEIGHT_MD};
    font-size: {T.FONT_BASE};
}}
QPushButton:hover {{
    background-color: {P.BG_PANEL_RAISED};
    border-color: {P.BORDER_STRONG};
}}
QPushButton:pressed {{
    background-color: {P.BG_SELECTION};
    border-color: {P.ACCENT_PRESSED};
}}
QPushButton:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border-color: {P.BORDER};
}}
QPushButton:focus {{
    border: {T.BORDER_THIN} solid {P.ACCENT_BORDER};
    outline: none;
}}
/* 连接类主按钮 — 强调青→蓝品牌渐变（跨色相，打破单一色相单调） */
{connect_sel} {{
    background-color: {P.ACCENT_GRADIENT};
    color: {P.TEXT_ON_ACCENT};
    border: {T.BORDER_NONE};
    font-weight: 600;
}}
{connect_hover} {{
    background-color: {P.ACCENT_HOVER};
}}
{connect_press} {{
    background-color: {P.ACCENT_PRESSED};
}}
{connect_dis} {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
}}
/* 断开按钮 — 危险红 */
QPushButton#serialStationDisconnectButton {{
    background-color: {P.ERROR_SOFT};
    color: {P.ERROR};
    border: {T.BORDER_THIN} solid {P.ERROR};
}}
QPushButton#serialStationDisconnectButton:hover {{
    background-color: {P.ERROR};
    color: {P.TEXT_INVERTED};
}}
QPushButton#serialStationDisconnectButton:pressed {{
    background-color: {P.ERROR_HOVER};
}}
QPushButton#serialStationDisconnectButton:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border-color: {P.BORDER};
}}
/* 次要工具按钮 */
{tool_sel} {{
    background-color: {P.BG_PANEL};
    color: {P.TEXT_SECONDARY};
}}
{tool_hover} {{
    background-color: {P.BG_PANEL_RAISED};
    color: {P.TEXT_PRIMARY};
    border-color: {P.ACCENT_BORDER};
}}
{tool_press} {{
    background-color: {P.BG_SELECTION};
    border-color: {P.ACCENT_PRESSED};
}}
{tool_dis} {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border-color: {P.BORDER};
}}
/* 发送/注入 — 终端 TX 蓝 */
QPushButton#serialStationSendButton,
QPushButton#serialStationInjectButton {{
    color: {P.TERM_TX};
    border-color: {P.TERM_TX};
}}
QPushButton#serialStationSendButton:hover,
QPushButton#serialStationInjectButton:hover {{
    background-color: {P.TERM_TX};
    color: {P.TEXT_INVERTED};
}}
QPushButton#serialStationSendButton:pressed,
QPushButton#serialStationInjectButton:pressed {{
    background-color: {P.ACCENT_PRESSED};
    color: {P.TEXT_INVERTED};
}}
QPushButton#serialStationSendButton:disabled,
QPushButton#serialStationInjectButton:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border-color: {P.BORDER};
}}"""


def log_view_section() -> str:
    return f"""/* === Log View (Terminal) === */
QPlainTextEdit#serialStationLogView {{
    background-color: {P.TERM_BACKGROUND};
    color: {P.TERM_SYSTEM};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_LG};
    padding: {T.SPACING_SM} {T.SPACING_LG};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
    selection-background-color: {P.ACCENT_SOFT};
    selection-color: {P.TEXT_PRIMARY};
}}
QPlainTextEdit#serialStationLogView:focus {{
    border-color: {P.ACCENT_BORDER};
}}"""


def status_section() -> str:
    return f"""/* === Status & Profile Labels === */
QLabel#serialStationStatusLabel {{
    color: {P.WARNING};
    padding: {T.SPACING_XS} {T.SPACING_MD};
    border: {T.BORDER_THIN} solid {P.WARNING_BORDER};
    border-radius: {T.RADIUS_PILL};
    background-color: {P.WARNING_SOFT};
    font-size: {T.FONT_SM};
    font-weight: 600;
}}
QLabel#serialStationProfileLabel {{
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_SM};
    padding: {T.SPACING_XS} {T.SPACING_MD};
}}"""


def scrollbar_section() -> str:
    return f"""/* === Scrollbars (8px capsule, soft hover) === */
QScrollBar:vertical {{
    background: transparent;
    width: 8px;
    margin: 1px;
}}
QScrollBar::handle:vertical {{
    background: {P.SCROLLBAR};
    border-radius: 4px;
    min-height: 30px;
}}
QScrollBar::handle:vertical:hover {{
    background: {P.SCROLLBAR_HOVER};
}}
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical {{
    height: 0;
    border: none;
    background: none;
}}
QScrollBar:horizontal {{
    background: transparent;
    height: 8px;
    margin: 1px;
}}
QScrollBar::handle:horizontal {{
    background: {P.SCROLLBAR};
    border-radius: 4px;
    min-width: 30px;
}}
QScrollBar::handle:horizontal:hover {{
    background: {P.SCROLLBAR_HOVER};
}}
QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal {{
    width: 0;
    border: none;
    background: none;
}}"""


def plaintext_section() -> str:
    return f"""/* === PlainTextEdit Fallback === */
QPlainTextEdit {{
    background-color: {P.BG_INPUT};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.BORDER_STRONG};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT};
}}"""


def shortcut_section() -> str:
    return """/* === Shortcut objectName contract === */
/* QShortcut 无可视样式，objectName 仅用于测试与可发现性，此处保留契约占位。 */
QObject#serialStationSendShortcut,
QObject#serialStationClearShortcut,
QObject#serialStationRefreshPortsShortcut,
QObject#serialStationCommandPaletteShortcut {
    /* no-op */
}"""
