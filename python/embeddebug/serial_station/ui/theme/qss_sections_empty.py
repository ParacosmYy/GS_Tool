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
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
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
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
    min-width: 120px;
    max-width: 240px;
}}
QPushButton#serialStationEmptyStateCta:hover {{
    background-color: {P.ACCENT_HOVER};
}}
QPushButton#serialStationEmptyStateCta:pressed {{
    background-color: {P.ACCENT_PRESSED};
}}
QPushButton#serialStationEmptyStateCta:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border: none;
}}
QWidget#serialStationSkeleton,
QWidget#serialStationSkeletonBlock {{
    background-color: transparent;
    border: none;
}}
/* Batch 49-2: 日志连接加载态覆盖层（SkeletonBlock + 「正在建立连接…」文案）。
   复用 BG_OVERLAY 模态遮罩色，覆盖在 log_view 上方。 */
QWidget#serialStationLogLoadingOverlay {{
    background-color: {P.BG_OVERLAY};
    border-radius: {T.RADIUS_LG};
}}
QWidget#serialStationLogLoadingSkeleton {{
    background-color: transparent;
    border: none;
}}
QLabel#serialStationLogLoadingLabel {{
    background-color: transparent;
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_SM};
    padding: {T.SPACING_XS} {T.SPACING_MD};
}}
/* Batch 49-5: 连接按钮内嵌 ProgressRing（加载态旋转指示器）。
   透明背景，由按钮 background 透出。 */
QWidget#serialStationButtonLoadingRing {{
    background-color: transparent;
    border: none;
}}"""
