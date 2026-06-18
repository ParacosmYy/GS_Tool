"""仪表盘模式面板 — 拖拽式控件仪表盘（Batch 17 激活 dashboard 死代码）。

把 dashboard 子系统（DashboardTabs + WidgetPalette）装配成用户可访问的 ModePanel：
左侧控件库面板（拖拽源）+ 右侧多标签页画布（放置区）+ 顶栏操作（新增标签页/
清空/保存/加载布局）。Batch 16 审计确认 dashboard 491 行代码零外部消费者，本面板
首次让该子系统从骨架变成用户可达。

设计要点：
- ModePanel 契约：build 返回主控件，on_enter/on_leave 播放入场动画 + 通知。
- 左 WidgetPalette 列出可拖控件（led/slider/button/gauge/value_display）。
- 右 DashboardTabs 多画布标签页，拖入即放置。
- 顶栏：新增标签页、清空当前画布、保存布局 JSON、加载布局 JSON。
- 保存/加载走 QFileDialog，复用 canvas.save_layout/load_layout。
- 主题切换/关键事件经 panel_notify → toast 反馈。

约束：只调 dashboard 子系统公共 API（DashboardTabs/WidgetPalette/canvas），
不访问 controller/transport/protocol；不 import 其他域面板。
"""

from __future__ import annotations

from PyQt6.QtWidgets import (
    QFileDialog,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.ui.dashboard import DashboardTabs, WidgetPalette


class DashboardPanel:
    """仪表盘 ModePanel：控件库 + 多画布标签页 + 布局持久化。"""

    def __init__(self) -> None:
        self._app_controller: AppController | None = None
        self._widget: QWidget | None = None
        self._tabs: DashboardTabs | None = None

    def build(self, app_controller: AppController) -> QWidget:
        self._app_controller = app_controller
        widget = QWidget()
        widget.setObjectName("serialStationDashboardPanel")
        layout = QVBoxLayout(widget)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        # 顶栏：操作按钮 + 状态。
        top = QHBoxLayout()
        add_tab_btn = QPushButton(widget.tr("新增标签页"), widget)
        add_tab_btn.setObjectName("serialStationDashboardAddTabButton")
        add_tab_btn.clicked.connect(self._add_tab)
        clear_btn = QPushButton(widget.tr("清空画布"), widget)
        clear_btn.setObjectName("serialStationDashboardClearButton")
        clear_btn.clicked.connect(self._clear_canvas)
        save_btn = QPushButton(widget.tr("保存布局"), widget)
        save_btn.setObjectName("serialStationDashboardSaveButton")
        save_btn.clicked.connect(self._save_layout)
        load_btn = QPushButton(widget.tr("加载布局"), widget)
        load_btn.setObjectName("serialStationDashboardLoadButton")
        load_btn.clicked.connect(self._load_layout)
        self._status = QLabel(widget.tr("拖拽左侧控件到画布"), widget)
        self._status.setObjectName("serialStationDashboardStatusLabel")
        for btn in (add_tab_btn, clear_btn, save_btn, load_btn):
            top.addWidget(btn)
        top.addStretch(1)
        top.addWidget(self._status)
        layout.addLayout(top)

        # 主体：左控件库 + 右多画布标签页。
        body = QHBoxLayout()
        body.setSpacing(12)
        self._palette = WidgetPalette(widget)
        self._palette.setObjectName("serialStationDashboardPalette")
        self._palette.setFixedWidth(200)
        body.addWidget(self._palette)
        self._tabs = DashboardTabs(widget)
        body.addWidget(self._tabs, 1)
        layout.addLayout(body, 1)

        self._widget = widget
        return widget

    def on_enter(self) -> None:
        """切入仪表盘页：播放入场动画。"""

        from embeddebug.serial_station.ui.panels._enter_anim import play_panel_enter

        play_panel_enter(self)

    def on_leave(self) -> None:
        """切出仪表盘页：停止入场动画。"""

        from embeddebug.serial_station.ui.panels._enter_anim import stop_panel_enter

        stop_panel_enter(self)

    # ── 操作 ────────────────────────────────────────────────────────
    def _add_tab(self) -> None:
        if self._tabs is not None:
            self._tabs.add_tab()
            self._status.setText(self._widget.tr("已新增标签页"))

    def _clear_canvas(self) -> None:
        canvas = self._tabs.current_canvas() if self._tabs else None
        if canvas is None:
            return
        canvas.clear()
        self._status.setText(self._widget.tr("画布已清空"))

    def _save_layout(self) -> None:
        canvas = self._tabs.current_canvas() if self._tabs else None
        if canvas is None or self._widget is None:
            return
        path, _ = QFileDialog.getSaveFileName(
            self._widget, self._widget.tr("保存仪表盘布局"), "",
            self._widget.tr("Dashboard layout (*.json)"),
        )
        if not path:
            return
        try:
            canvas.save_layout(path)
            self._status.setText(self._widget.tr("布局已保存：{path}").format(path=path))
        except OSError as exc:
            self._status.setText(self._widget.tr("保存失败：{err}").format(err=exc))

    def _load_layout(self) -> None:
        canvas = self._tabs.current_canvas() if self._tabs else None
        if canvas is None or self._widget is None:
            return
        path, _ = QFileDialog.getOpenFileName(
            self._widget, self._widget.tr("加载仪表盘布局"), "",
            self._widget.tr("Dashboard layout (*.json)"),
        )
        if not path:
            return
        try:
            count = canvas.load_layout(path)
            self._status.setText(
                self._widget.tr("已加载 {n} 个控件").format(n=count)
            )
        except (OSError, ValueError, KeyError) as exc:
            self._status.setText(self._widget.tr("加载失败：{err}").format(err=exc))
