"""Batch 10: 设置页强调色选择行 —— 7 套 accent 变体色点 + 切换过渡动画。

从 ``settings_panel`` 抽出以守 300 行运行时文件门禁。提供：
- ``build_accent_row(panel, parent)``：构建色点行（含当前态高亮）。
- ``select_accent(panel, accent_id)``：切换 accent，带 windowOpacity 过渡动画
  + 色点 checked 同步 + toast 反馈。

约束：本模块只消费 theme_switcher/accents/theme_transition/_notify，不碰
controller/transport（与 settings_panel 同一 docstring 边界）。
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QHBoxLayout, QLabel, QToolButton, QWidget

from embeddebug.serial_station.ui.theme.accents import (
    ACCENTS,
    get_accent_by_id,
    get_active_accent_id,
)

if TYPE_CHECKING:
    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel


def build_accent_row(panel: SettingsPanel, parent: QWidget) -> QHBoxLayout:
    """构建 7 色点选择行。色点存到 ``panel._accent_swatches`` 供后续 checked 同步。"""

    row = QHBoxLayout()
    label = QLabel(panel._widget.tr("强调色"), parent)
    label.setObjectName("serialStationSettingsFieldLabel")
    row.addWidget(label)

    active_id = get_active_accent_id()
    for index, variant in enumerate(ACCENTS):
        swatch = QToolButton(parent)
        swatch.setObjectName(f"serialStationAccentSwatch{index}")
        swatch.setToolTip(panel._widget.tr(variant.label_key))
        # 用 inline style 填色：背景 = 该 accent 的 dark base。
        # 只设 background-color，border 留给应用级 QSS（checked 时画 accent ring）；
        # inline 设 border 会覆盖应用级 QSS（widget-level 优先级更高）。
        swatch.setStyleSheet(
            f"QToolButton#serialStationAccentSwatch{index} {{"
            f" background-color: {variant.dark.base}; border-radius: 12px; }}"
        )
        swatch.setCheckable(True)
        swatch.setChecked(variant.id == active_id)
        swatch.setFixedSize(26, 26)
        swatch.setCursor(Qt.CursorShape.PointingHandCursor)
        # 闭包捕获 variant.id（lambda 默认绑定循环变量会全部指向最后一个）。
        swatch.clicked.connect(
            lambda _checked=False, vid=variant.id: select_accent(panel, vid)
        )
        panel._accent_swatches.append(swatch)
        row.addWidget(swatch)
    row.addStretch(1)
    return row


def select_accent(panel: SettingsPanel, accent_id: str) -> None:
    """切换强调色，带 windowOpacity 暗淡+回亮过渡。"""

    from PyQt6.QtWidgets import QApplication

    from embeddebug.serial_station.ui.panels._notify import panel_notify
    from embeddebug.serial_station.ui.theme.theme_switcher import (
        THEME_DARK,
        apply_theme_by_name,
    )

    def apply_fn() -> None:
        # 保留当前主题（深/浅），只换 accent。
        from embeddebug.serial_station.ui.theme.manager import ThemeManager

        current = ThemeManager().current_theme or THEME_DARK
        apply_theme_by_name(QApplication.instance(), current, accent=accent_id)

    # 过渡在透明度谷值时执行 QSS 重着色（掩盖硬切闪烁）；
    # 防重入：上次过渡未完成时 transition_theme 同步执行 apply_fn（不丢操作）。
    from embeddebug.serial_station.ui.theme.theme_transition import transition_theme

    transition_theme(QApplication.instance(), apply_fn)
    _sync_checked(panel, accent_id)

    # Batch 23: 把选择持久化到 SettingsManager（双写 theme_store 镜像）。
    try:
        from embeddebug.serial_station.services.settings_service import SettingsManager

        SettingsManager.instance().update(accent=accent_id)
    except Exception:  # noqa: BLE001  偏好持久化失败不阻塞 accent 切换
        pass

    variant = get_accent_by_id(accent_id)
    panel_notify(panel._widget, "success", panel._widget.tr("强调色已切换"),
                 panel._widget.tr("已应用 {accent} 强调色").format(
                     accent=panel._widget.tr(variant.label_key)))


def _sync_checked(panel: SettingsPanel, accent_id: str) -> None:
    """同步色点 checked 态到指定 accent（被选中的画 ring）。"""

    for index, variant in enumerate(ACCENTS):
        if index < len(panel._accent_swatches):
            panel._accent_swatches[index].setChecked(variant.id == accent_id)
