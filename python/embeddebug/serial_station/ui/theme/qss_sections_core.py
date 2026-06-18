"""QSS 分区生成器 — 核心容器与输入控件。

包含全局、主窗口、标签、输入框、下拉框分区。
颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def global_section() -> str:
    return f"""/* === Global === */
QWidget {{
    background-color: {P.BG_WINDOW};
    color: {P.TEXT_PRIMARY};
    font-family: {T.FONT_FAMILY};
    font-size: {T.FONT_BASE};
}}
QWidget:disabled {{
    color: {P.TEXT_DISABLED};
}}"""


def window_section() -> str:
    return f"""/* === Window & Root === */
QMainWindow,
QMainWindow#embeddebugPySerialStationWindow {{
    background-color: {P.BG_WINDOW};
}}
QWidget#serialStationPyRoot {{
    background-color: {P.BG_APP};
}}
QLabel#serialStationPyTitle {{
    color: {P.TEXT_PRIMARY};
    font-size: {T.FONT_XL};
    font-weight: 600;
    padding: {T.SPACING_SM} {T.SPACING_NONE};
}}"""


def labels_section() -> str:
    return f"""/* === Labels === */
QLabel {{
    background-color: transparent;
    color: {P.TEXT_SECONDARY};
}}
QLabel#serialStationLogStatsLabel {{
    color: {P.TEXT_MUTED};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
    padding: 0 {T.SPACING_MD};
}}"""


_INPUT_NAMES = [
    "serialStationSendEdit",
    "serialStationInjectEdit",
    "serialStationLogSearchEdit",
    "serialStationLogPathEdit",
    "serialStationProfilePathEdit",
    "serialStationProfileNameEdit",
    "serialStationTcpHostEdit",
    "serialStationTcpPortEdit",
    "serialStationUdpHostEdit",
    "serialStationUdpPortEdit",
]


def inputs_section() -> str:
    selectors = ",\n".join(f"QLineEdit#{name}" for name in _INPUT_NAMES)
    return f"""/* === Line Edits === */
QLineEdit {{
    background-color: {P.BG_INPUT};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.BORDER_STRONG};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT};
    selection-background-color: {P.TERM_SELECTION};
    selection-color: {P.TEXT_INVERTED};
}}
QLineEdit:focus {{
    border: {T.BORDER_THIN} solid {P.BORDER_FOCUS};
    background-color: {P.BG_INPUT_FOCUS};
}}
QLineEdit:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border-color: {P.BORDER};
}}
QLineEdit::placeholder {{
    color: {P.TEXT_MUTED};
}}
{selectors} {{
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}"""


_COMBO_NAMES = [
    "serialStationProtocolCombo",
    "serialStationPortCombo",
    "serialStationBaudCombo",
    "serialStationDataBitsCombo",
    "serialStationParityCombo",
    "serialStationStopBitsCombo",
    "serialStationFlowControlCombo",
    "serialStationCommandHistoryCombo",
    "serialStationLogFilterCombo",
]


def combos_section() -> str:
    selectors = ",\n".join(f"QComboBox#{name}" for name in _COMBO_NAMES)
    return f"""/* === ComboBoxes === */
QComboBox {{
    background-color: {P.BG_INPUT};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.BORDER_STRONG};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT};
    min-height: {T.CONTROL_HEIGHT_MD};
    selection-background-color: {P.BG_SELECTION};
}}
QComboBox:hover {{
    border-color: {P.ACCENT_BORDER};
}}
QComboBox:focus {{
    border: {T.BORDER_THIN} solid {P.BORDER_FOCUS};
}}
QComboBox:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border-color: {P.BORDER};
}}
QComboBox::drop-down {{
    border: none;
    width: 22px;
}}
QComboBox::down-arrow {{
    width: 10px;
    height: 10px;
}}
QComboBox QAbstractItemView {{
    background-color: {P.BG_PANEL};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
    padding: {T.SPACING_SM};
    selection-background-color: {P.BG_SELECTION};
    selection-color: {P.ACCENT};
    outline: none;
}}
{selectors} {{
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}"""
