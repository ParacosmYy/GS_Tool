"""QSS 分区 — 日志选项工具条场景实例（Batch 51-52 抽出）。

从 ``qss_sections_controls`` 拆出，守 ≤300 行门禁。集中 log_options_bar wire 的
widget 在日志工具区的具体实例 objectName 样式。自绘控件（Badge/Chip/Toggle/
Segmented/InfoBanner/Drawer）QSS 是契约占位；QPushButton 有完整三态。

颜色引用 palette，尺寸引用 tokens，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def log_options_section() -> str:
    return f"""/* === Log Options Bar（Batch 51-52 场景实例）=== */
/* Badge/Chip/ToggleSwitch/SegmentedControl/InfoBanner/Drawer 自绘，
   QSS 仅契约占位（objectName 覆盖门禁要求文档化）。 */
QWidget#serialStationLogConnectionBadge,
QWidget#serialStationActiveFilterChip,
QWidget#serialStationAutoScrollToggle,
QWidget#serialStationLogViewModeSegmented,
QWidget#serialStationLogInfoBanner,
QWidget#serialStationHistoryDrawer {{
    background-color: transparent;
}}
QLabel#serialStationHistoryPlaceholder {{
    color: {P.TEXT_SECONDARY};
    font-size: {T.FONT_SM};
    padding: {T.SPACING_MD};
}}
QLabel#serialStationAutoScrollLabel {{
    background-color: transparent;
    color: {P.TEXT_SECONDARY};
    font-size: {T.FONT_SM};
}}
QPushButton#serialStationHistoryButton {{
    background-color: {P.BG_PANEL_RAISED};
    color: {P.TEXT_SECONDARY};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
    padding: {T.SPACING_XS} {T.SPACING_MD};
    font-size: {T.FONT_SM};
}}
QPushButton#serialStationHistoryButton:hover {{
    background-color: {P.ACCENT_SOFT};
    color: {P.ACCENT_HOVER};
    border-color: {P.ACCENT_BORDER};
}}
QPushButton#serialStationHistoryButton:pressed {{
    background-color: {P.ACCENT_PRESSED};
}}
QPushButton#serialStationHistoryButton:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
}}"""
