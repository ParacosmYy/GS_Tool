"""QSS 域面板分区子助手（补充）。

从 ``qss_sections_domain_parts`` 拆出，避免单文件超 300 行。
涵盖设置页 Tab、强调色选择色点、关于页标签三类分区。
颜色引用 ``palette``、尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def _settings_tabs() -> str:
    """设置页 Tab。"""

    return f"""/* 设置页 Tab。 */
QTabWidget#serialStationSettingsTabWidget::pane {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
}}
QTabBar::tab {{
    background-color: {P.BG_INPUT};
    color: {P.TEXT_SECONDARY};
    padding: {T.SPACING_SM} {T.SPACING_LG};
    border-top-left-radius: {T.RADIUS_SM};
    border-top-right-radius: {T.RADIUS_SM};
}}
QTabBar::tab:hover {{
    background-color: {P.BG_PANEL_RAISED};
    color: {P.TEXT_PRIMARY};
}}
QTabBar::tab:selected {{
    background-color: {P.ACCENT_SOFT};
    color: {P.ACCENT};
}}
QTabBar::tab:selected:hover {{
    background-color: {P.ACCENT_SOFT};
    color: {P.ACCENT_HOVER};
}}"""


def _accent_swatches() -> str:
    """Batch 10: 强调色选择色点（7 套 accent 变体）。"""

    return f"""/* Batch 10: 强调色选择色点（7 套 accent 变体）。底色由 inline style 填充（每按钮不同），
   QSS 只控选中态 ring 与 hover。checked = 当前活动 accent，画 2px accent 描边外环。 */
QToolButton#serialStationAccentSwatch0,
QToolButton#serialStationAccentSwatch1,
QToolButton#serialStationAccentSwatch2,
QToolButton#serialStationAccentSwatch3,
QToolButton#serialStationAccentSwatch4,
QToolButton#serialStationAccentSwatch5,
QToolButton#serialStationAccentSwatch6 {{
    border: 2px solid transparent;
    padding: 0;
}}
QToolButton#serialStationAccentSwatch0:checked,
QToolButton#serialStationAccentSwatch1:checked,
QToolButton#serialStationAccentSwatch2:checked,
QToolButton#serialStationAccentSwatch3:checked,
QToolButton#serialStationAccentSwatch4:checked,
QToolButton#serialStationAccentSwatch5:checked,
QToolButton#serialStationAccentSwatch6:checked {{
    border: 2px solid {P.ACCENT};
}}
QToolButton#serialStationAccentSwatch0:hover,
QToolButton#serialStationAccentSwatch1:hover,
QToolButton#serialStationAccentSwatch2:hover,
QToolButton#serialStationAccentSwatch3:hover,
QToolButton#serialStationAccentSwatch4:hover,
QToolButton#serialStationAccentSwatch5:hover,
QToolButton#serialStationAccentSwatch6:hover {{
    margin-top: -2px;
}}"""


def _about_labels() -> str:
    """关于页标签。"""

    return f"""/* 关于页标签。 */
QLabel#serialStationSettingsAppNameLabel {{
    color: {P.TEXT_PRIMARY};
    font-size: {T.FONT_XL};
    font-weight: {T.FONT_WEIGHT_BOLD};
}}
QLabel#serialStationSettingsVersionLabel,
QLabel#serialStationSettingsRemoteLabel {{
    color: {P.TEXT_MUTED};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}"""
