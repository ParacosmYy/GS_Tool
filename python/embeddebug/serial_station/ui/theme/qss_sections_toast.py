"""QSS 分区生成器 — Toast 通知卡片（Batch 11 新增）。

ToastWidget（级别色条 + 标题 + 描述 + 关闭按钮）的样式。从 qss_sections_widgets 拆出，
避免 widgets 分区超 300 行可维护性门禁。颜色引用 palette，尺寸引用 tokens，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def toast_section() -> str:
    return f"""/* === Toast (Batch 11) === */
QFrame#serialStationToast {{
    background-color: {P.BG_PANEL_RAISED};
    border: {T.BORDER_THIN} solid {P.BORDER_STRONG};
    border-radius: {T.RADIUS_MD};
}}
QFrame#serialStationToastAccent {{
    border-top-left-radius: {T.RADIUS_MD};
    border-bottom-left-radius: {T.RADIUS_MD};
}}
QLabel#serialStationToastGlyph,
QLabel#serialStationToastTitle,
QLabel#serialStationToastMessage {{
    background-color: transparent;
}}
QLabel#serialStationToastTitle {{
    color: {P.TEXT_PRIMARY};
    font-weight: 600;
}}
QLabel#serialStationToastMessage {{
    color: {P.TEXT_SECONDARY};
    font-size: {T.FONT_SM};
}}
QPushButton#serialStationToastCloseButton {{
    background-color: transparent;
    color: {P.TEXT_MUTED};
    border: none;
}}
QPushButton#serialStationToastCloseButton:hover {{
    color: {P.TEXT_PRIMARY};
}}
/* === Toast 容器（Batch 12）=== */
QWidget#serialStationToastContainer {{
    background-color: transparent;
    border: none;
}}
QLabel#serialStationToastPlaceholder {{
    background-color: transparent;
}}"""
