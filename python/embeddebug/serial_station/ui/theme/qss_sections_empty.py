"""QSS 分区生成器 — 空状态与骨架屏（Batch 5 新增）。

EmptyStateWidget（图标+标题+描述+CTA）与 SkeletonWidget（shimmer 骨架）的样式。
颜色引用 palette，尺寸引用 tokens，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def empty_state_section() -> str:
    return f"""/* === EmptyState & Skeleton (Batch 5) === */
QWidget#serialStationEmptyState {{
    background-color: transparent;
    border: none;
}}
QLabel#serialStationEmptyStateIcon {{
    background-color: transparent;
    color: {P.TEXT_MUTED};
    margin-bottom: {T.SPACING_MD};
}}
QLabel#serialStationEmptyStateTitle {{
    background-color: transparent;
    color: {P.TEXT_PRIMARY};
    font-size: {T.FONT_LG};
    font-weight: 600;
}}
QLabel#serialStationEmptyStateDescription {{
    background-color: transparent;
    color: {P.TEXT_SECONDARY};
    font-size: {T.FONT_SM};
}}
QPushButton#serialStationEmptyStateCta {{
    background-color: {P.ACCENT_GRADIENT};
    color: {P.TEXT_ON_ACCENT};
    border: {T.BORDER_NONE};
    border-radius: {T.RADIUS_MD};
    padding: {T.SPACING_SM} {T.SPACING_LG};
    font-weight: 600;
    min-width: 120px;
    max-width: 240px;
}}
QPushButton#serialStationEmptyStateCta:hover {{
    background-color: {P.ACCENT_HOVER};
}}
QWidget#serialStationSkeleton,
QWidget#serialStationSkeletonBlock {{
    background-color: transparent;
    border: none;
}}"""
