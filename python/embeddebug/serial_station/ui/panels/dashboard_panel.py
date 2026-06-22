"""仪表盘模式面板 — 拖拽式控件仪表盘（Batch 17 激活 dashboard 死代码）。

把 dashboard 子系统（DashboardTabs + WidgetPalette）装配成用户可访问的 ModePanel：
左侧控件库面板（拖拽源）+ 右侧多标签页画布（放置区）+ 顶栏操作（新增标签页/
清空/保存/加载布局/网格切换）。Batch 16 审计确认 dashboard 491 行代码零外部消费者，
本面板首次让该子系统从骨架变成用户可达。

设计要点：
- ModePanel 契约：build 返回主控件，on_enter/on_leave 播放入场动画 + 通知。
- 左 WidgetPalette 列出可拖控件；右 DashboardTabs 多画布标签页，拖入即放置。
- 顶栏：新增标签页、清空画布、保存/加载布局 JSON、网格切换。
- 保存/加载走 QFileDialog；关键事件经 panel_notify → toast 反馈。

约束：只调 dashboard 子系统公共 API，不访问 controller/transport/protocol；不 import 其他域面板。
"""

from __future__ import annotations

import logging

from PyQt6.QtCore import QPoint, Qt
from PyQt6.QtWidgets import (
    QFileDialog,  # noqa: F401  测试通过 module attr 访问（test_dashboard_core_b）
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.ui.dashboard import DashboardTabs, WidgetPalette

_log = logging.getLogger(__name__)


class DashboardPanel:
    """仪表盘 ModePanel：控件库 + 多画布标签页 + 布局持久化。"""

    def __init__(self) -> None:
        self._app_controller: AppController | None = None
        self._widget: QWidget | None = None
        self._tabs: DashboardTabs | None = None
        # Batch 49: 仪表盘绑定服务（measurement/log → widget 路由）。
        self._binding_service = None

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
        clear_btn = QPushButton(widget.tr("清空画布"), widget); clear_btn.setObjectName("serialStationDashboardClearButton")
        clear_btn.clicked.connect(self._clear_canvas)
        save_btn = QPushButton(widget.tr("保存布局"), widget); save_btn.setObjectName("serialStationDashboardSaveButton")
        save_btn.clicked.connect(self._save_layout)
        load_btn = QPushButton(widget.tr("加载布局"), widget); load_btn.setObjectName("serialStationDashboardLoadButton")
        load_btn.clicked.connect(self._load_layout)
        # Batch 36: 网格可见性切换（checkable，默认显示），委托 _dashboard_widget_menu。
        from embeddebug.serial_station.ui.panels._dashboard_widget_menu import make_grid_toggle
        grid_btn = QPushButton(widget.tr("网格"), widget); grid_btn.setObjectName("serialStationDashboardGridButton")
        grid_btn.setCheckable(True); grid_btn.setChecked(True); grid_btn.clicked.connect(make_grid_toggle(self))
        self._status = QLabel(widget.tr("拖拽左侧控件到画布"), widget)
        self._status.setObjectName("serialStationDashboardStatusLabel")
        for btn in (add_tab_btn, clear_btn, save_btn, load_btn, grid_btn):
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

        # Batch 28: 标签页双击重命名（激活 DashboardTabs.rename_tab，此前零消费者）。
        self._tabs.tabBarDoubleClicked.connect(self._rename_tab_on_double_click)
        # Batch 29: 标签页右键菜单（重命名/复制/关闭）—— customContextMenuRequested。
        self._tabs.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self._tabs.customContextMenuRequested.connect(self._show_tab_context_menu)
        # Batch 30: 标签页拖拽重排序后自动重存（tab 顺序是持久化 key 顺序）。
        self._tabs.tab_moved.connect(lambda _frm, _to: self._autosave_layout())

        # Batch 18: 双击全屏接线 —— 新放置控件自动装 attach_double_click_fullscreen
        # （激活 fullscreen.py 死代码）。每个画布的 item_added 信号 → 给新控件装双击全屏。
        # fullscreen_handlers 防 GC（handler 必须被持有才有效）。
        self._fullscreen_handlers: list = []
        self._wire_canvas_fullscreen(self._tabs.current_canvas())
        self._tabs.canvas_changed.connect(self._wire_canvas_fullscreen)

        # 先赋 _widget，再恢复布局（恢复会触发 item_added → _on_item_added_fullscreen
        # 用 self._widget.tr(...)，必须在恢复前就绪）。
        self._widget = widget

        # Batch 49: 实例化绑定服务 + 订阅 controller 事件（measurement/log/error）。
        from embeddebug.serial_station.services.dashboard_binding_service import (
            DashboardBindingService,
        )
        from embeddebug.serial_station.ui.panels._dashboard_binding_wire import (
            subscribe_controller_events,
        )
        self._binding_service = DashboardBindingService()
        subscribe_controller_events(
            app_controller.serial_controller, self._binding_service
        )

        # Batch 25: 自动恢复上一次仪表盘布局（应用数据目录 JSON）。
        self._restore_layout_on_build()

        return widget

    def _wire_canvas_fullscreen(self, canvas: object) -> None:
        """把给定画布的 item_added/item_removed 信号接到全屏安装 + 自动保存（Batch 18/25）。

        每个画布只接一次（用 _wired_canvases 去重，防止 canvas_changed 重复连接）。
        """

        if canvas is None:
            return
        wired = getattr(self, "_wired_canvases", None)
        if wired is None:
            wired = set()
            self._wired_canvases = wired
        canvas_id = id(canvas)
        if canvas_id in wired:
            return
        wired.add(canvas_id)
        canvas.item_added.connect(self._on_item_added_fullscreen)
        # Batch 25: add/remove 后自动持久化布局。
        canvas.item_added.connect(lambda _id: self._autosave_layout())
        canvas.item_removed.connect(lambda _id: self._autosave_layout())

    def _restore_layout_on_build(self) -> None:
        """build 时恢复布局：委托 _dashboard_layout_store.restore_panel_layout（Batch 49 抽出）。"""

        from embeddebug.serial_station.ui.panels._dashboard_layout_store import (
            restore_panel_layout,
        )
        restore_panel_layout(self)

    def _autosave_layout(self) -> None:
        """add/remove 后自动保存：委托 _dashboard_layout_store.persist_panel_layout（Batch 49 抽出）。"""

        from embeddebug.serial_station.ui.panels._dashboard_layout_store import (
            persist_panel_layout,
        )
        persist_panel_layout(self)

    def _on_item_added_fullscreen(self, item_id: str) -> None:
        """新控件放置 → 给它装双击全屏 + 发 toast 提示（Batch 18）。

        Batch 49：恢复布局时，如果 item 带 binding，登记到 binding service；
        给放置控件装右键菜单时传 ``_configure_widget_binding`` 让用户可后续配置。
        """

        canvas = self._tabs.current_canvas() if self._tabs else None
        if canvas is None:
            return
        item = canvas.items.get(item_id)
        if item is None:
            return
        from embeddebug.serial_station.ui.dashboard import attach_double_click_fullscreen
        from embeddebug.serial_station.ui.panels._notify import panel_notify
        from embeddebug.serial_station.ui.panels._dashboard_widget_menu import (
            attach_widget_delete_menu,
        )
        # Batch 49: 既有 binding 注册到 service（layout 恢复路径）。
        if self._binding_service is not None:
            try:
                from embeddebug.serial_station.ui.panels._dashboard_binding_wire import (
                    register_item_binding,
                )
                register_item_binding(canvas, item_id, self._binding_service)
            except Exception:
                _log.warning("layout restore failed", exc_info=True)

        handler = attach_double_click_fullscreen(item.widget, host=self._widget)
        self._fullscreen_handlers.append(handler)
        # Batch 31/49: 装右键菜单；同时接入「配置数据源...」回调。
        attach_widget_delete_menu(
            item.widget, canvas, item_id,
            on_configure_binding=self._configure_widget_binding,
        )
        panel_notify(
            self._widget, "info",
            self._widget.tr("已添加控件"),
            self._widget.tr("{kind}，双击可全屏").format(kind=item.widget_type),
        )

    def _configure_widget_binding(self, item_id: str) -> None:
        """右键「配置数据源...」入口：弹出对话框，应用结果（Batch 49）。"""

        from embeddebug.serial_station.ui.panels._dashboard_binding_wire import (
            open_binding_config_for_item,
        )
        open_binding_config_for_item(self, item_id)

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

    def _rename_tab_on_double_click(self, index: int) -> None:
        """标签页双击 → 弹输入框重命名（Batch 28：激活 rename_tab，此前零消费者）。

        空/取消保持原名；重命名后自动持久化（多标签页布局按 tab 名为 key）。
        """

        if self._tabs is None or self._widget is None:
            return
        if not (0 <= index < self._tabs.count()):
            return
        current_name = self._tabs.tabText(index)
        from PyQt6.QtWidgets import QInputDialog

        new_name, ok = QInputDialog.getText(
            self._widget,
            self._widget.tr("重命名标签页"),
            self._widget.tr("标签页名称："),
            text=current_name,
        )
        if ok and new_name.strip() and new_name.strip() != current_name:
            self._tabs.rename_tab(index, new_name.strip())
            self._status.setText(self._widget.tr("标签页已重命名：{name}").format(name=new_name.strip()))
            self._autosave_layout()  # tab 名是持久化 key，改名后重存。

    def _show_tab_context_menu(self, pos: QPoint) -> None:
        """标签页右键菜单（重命名/复制/关闭）——委托 _dashboard_tab_menu（守 300 行门禁）。"""

        from embeddebug.serial_station.ui.panels._dashboard_tab_menu import (
            show_tab_context_menu,
        )

        show_tab_context_menu(self, pos)

    def _clear_canvas(self) -> None:
        canvas = self._tabs.current_canvas() if self._tabs else None
        if canvas is None:
            return
        canvas.clear()
        self._status.setText(self._widget.tr("画布已清空"))

    def _save_layout(self) -> None:
        """保存布局：委托 _dashboard_layout_store.save_layout_via_dialog（Batch 49 抽出）。"""

        from embeddebug.serial_station.ui.panels._dashboard_layout_store import (
            save_layout_via_dialog,
        )
        save_layout_via_dialog(self)

    def _load_layout(self) -> None:
        """加载布局：委托 _dashboard_layout_store.load_layout_via_dialog（Batch 49 抽出）。"""

        from embeddebug.serial_station.ui.panels._dashboard_layout_store import (
            load_layout_via_dialog,
        )
        load_layout_via_dialog(self)
