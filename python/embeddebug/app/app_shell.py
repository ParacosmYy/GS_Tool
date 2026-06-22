"""多模式应用主壳 — 左侧图标导航栏 + 右侧可切换工作区。

AppShell 是应用顶层 ``QMainWindow``：左侧 ``NavRail``（56px 竖排图标按钮）+
右侧 ``QStackedWidget``（每个模式一页）。点导航图标切换当前页并触发面板
``on_enter``/``on_leave`` 生命周期。

模式面板通过 ``mode_panel.register_panel`` 注册，AppShell 按注册顺序装配导航。
首批：串口（现有 SerialStationMainWindow 内容）/ OTA（X/YMODEM）/ RTT（占位）/ 设置（占位）。

约束：AppShell 只做装配与切换编排，不写业务逻辑；业务在各 ModePanel 内。
"""

from __future__ import annotations

import logging
from PyQt6.QtCore import QSize, Qt, QTimer
from PyQt6.QtWidgets import (
    QButtonGroup,
    QFrame,
    QHBoxLayout,
    QLabel,
    QMainWindow,
    QPushButton,
    QStackedWidget,
    QVBoxLayout,
    QWidget,
)

from embeddebug.app.app_controller import AppController
from embeddebug.app.mode_panel import registered_panels
from embeddebug.serial_station.ui.icons import button_icon
from embeddebug.serial_station.ui.theme import palette as P

_log = logging.getLogger(__name__)

class AppShell(QMainWindow):
    """多模式应用顶层窗口。"""

    def __init__(self) -> None:
        super().__init__()
        self.setObjectName("embeddebugAppShell")
        self.setWindowTitle(self.tr("EmbedDebug"))
        self.resize(1360, 840)

        self._app_controller = AppController()
        self._panels: dict[str, object] = {}
        self._nav_buttons: dict[str, QPushButton] = {}
        # Batch 23: 页面离场淡出动画引用（防 GC），_stop_page_anims 清理。
        self._leave_anims: list = []
        # Batch 46: 延迟切换定时器（让离场淡出可见，setCurrentIndex 不再立即隐藏老页）。
        self._switch_timer: QTimer | None = None
        self._switch_index: int = -1

        central = QWidget(self)
        layout = QHBoxLayout(central)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        nav = self._build_nav_rail(central)
        layout.addWidget(nav)

        self._stack = QStackedWidget(central)
        self._stack.setObjectName("serialStationAppStack")
        layout.addWidget(self._stack, 1)

        self._build_pages(nav)
        self.setCentralWidget(central)

        # 通知子系统（Batch 12/15）：委托 app_notifications 装配，避免本文件超 300 行。
        from embeddebug.app import app_notifications

        self._app_notifications = app_notifications
        app_notifications.build(self)

        # 默认切到第一个模式（串口）。
        if self._stack.count() > 0:
            self._switch_to(0)

        # Batch 12: 启动就绪 toast（真实用户可见通知，验证端到端 manager→toast 闭环）。
        # 用 QTimer 延迟，让窗口先完成首次布局再弹 toast，避免浮层位置错位。
        from PyQt6.QtCore import QTimer

        QTimer.singleShot(50, self._show_ready_toast)

    def _show_ready_toast(self) -> None:
        """启动就绪通知：证明通知子系统端到端可用。"""

        try:
            self.notify("success", self.tr("EmbedDebug 已就绪"),
                        self.tr("串口工站已加载，选择端口或 endpoint 开始调试"), timeout_ms=4000)
        except Exception:
            _log.warning("startup ready toast failed", exc_info=True)  # toast 是锦上添花，失败不阻塞启动。

    def resizeEvent(self, event: object) -> None:
        """窗口 resize 时把 toast 容器对齐到右上角（委托 app_notifications）。"""

        super().resizeEvent(event)
        self._app_notifications.reposition(self)

    def notify(self, level: str, title: str, message: str, timeout_ms: int = 3000) -> None:
        """向用户弹一条非模态通知（委托 app_notifications.show）。"""

        self._app_notifications.show(self, level, title, message, timeout_ms=timeout_ms)

    def notification_manager(self):
        """暴露 NotificationManager（供面板/测试驱动通知）。"""

        return getattr(self, "_notification_manager", None)

    def keyPressEvent(self, event: object) -> None:
        """全局快捷键：Esc 关闭最早 toast，Ctrl+Shift+Esc 清空（委托 app_notifications）。

        非 toast 快捷键交给父类默认处理。
        """

        if self._app_notifications.handle_key_press(self, event):
            event.accept()
            return
        super().keyPressEvent(event)


    def _build_nav_rail(self, parent: QWidget) -> QFrame:
        """构建左侧图标导航栏（56px 竖排）。"""

        rail = QFrame(parent)
        rail.setObjectName("serialStationNavRail")
        rail.setFixedWidth(56)
        rail_layout = QVBoxLayout(rail)
        rail_layout.setContentsMargins(0, 10, 0, 10)
        rail_layout.setSpacing(6)
        rail_layout.setAlignment(Qt.AlignmentFlag.AlignTop)

        # 顶部品牌徽标（应用名首字母，强调青渐变方块）。
        brand = QLabel("ED", rail)
        brand.setObjectName("serialStationNavBrand")
        brand.setAlignment(Qt.AlignmentFlag.AlignCenter)
        brand.setFixedSize(32, 32)
        rail_layout.addWidget(brand)
        rail_layout.addSpacing(10)

        self._nav_group = QButtonGroup(rail)
        self._nav_group.setExclusive(True)
        return rail

    def _build_pages(self, nav_rail: QFrame) -> None:
        """按注册顺序为每个模式构建页 + 导航按钮。

        导航按钮以 nav_rail 为父级（**不能**以 self._stack 为父级，否则
        QStackedWidget 会把按钮当成页面吸收，导致空白屏）。
        """

        registrations = registered_panels()
        for index, reg in enumerate(registrations):
            # 导航按钮：父级是 nav_rail，挂进 rail 的竖排布局。
            btn = QPushButton(nav_rail)
            btn.setObjectName(f"serialStationNav{reg.mode_id.capitalize()}Btn")
            btn.setToolTip(reg.label)
            btn.setCheckable(True)
            btn.setCursor(Qt.CursorShape.PointingHandCursor)
            icon = button_icon(reg.icon, color=P.TEXT_SECONDARY)
            if not icon.isNull():
                btn.setIcon(icon)
                btn.setIconSize(QSize(20, 20))
            self._nav_buttons[reg.mode_id] = btn
            self._nav_group.addButton(btn, index)
            nav_rail.layout().addWidget(btn)
            # Batch 15: NavRail 图标按钮接入 scale 弹性反馈（按下陷下）+
            # hover scale 高亮（图标弹出），落地「动态化动画缩放」。
            from embeddebug.serial_station.ui.micro_interactions import (
                install_nav_hover_scale,
                install_scale_press,
            )

            install_scale_press(btn)
            install_nav_hover_scale(btn)
            btn.clicked.connect(lambda _checked, idx=index: self._switch_to(idx))

        # 模式页：只把面板控件 addWidget 进 stack（按钮不进 stack）。
        for reg in registrations:
            panel = reg.factory(self._app_controller)
            widget = panel.build(self._app_controller)
            self._panels[reg.mode_id] = panel
            self._stack.addWidget(widget)

    def _switch_to(self, index: int) -> None:
        """切换到第 index 页。离场淡出（160ms）→ 切 index → 入场淡入+上滑（240ms）。"""

        registrations = registered_panels()
        if not (0 <= index < len(registrations)):
            return
        old_index = self._stack.currentIndex()
        # 同页不重复切换。
        if old_index == index and self._stack.currentWidget() is not None:
            return

        # 停止进行中的离场/入场动画。
        self._stop_page_anims()

        # 离开当前页。
        if 0 <= old_index < len(registrations):
            leaving = registrations[old_index]
            self._call_panel(leaving.mode_id, "on_leave")
            # Batch 23: 老页面快速淡出（激活 panel_animations.fade_out 死代码，
            # 实现 docstring 承诺的「两端都有过渡」，满足铁律 18 禁止突然消失）。
            leaving_widget = self._stack.widget(old_index)
            if leaving_widget is not None:
                self._animate_page_leave(leaving_widget)

        # 导航按钮立即更新（给用户即时反馈，不等动画）。
        self._nav_buttons[registrations[index].mode_id].setChecked(True)

        # Batch 46: 延迟 setCurrentIndex 让离场淡出可见（160ms = DURATION_FAST）。
        # 此前同步 setCurrentIndex 立即隐藏老页面，fade_out 在不可见控件上运行。
        self._switch_index = index
        if self._switch_timer is not None:
            self._switch_timer.stop()
        self._switch_timer = QTimer(self)
        self._switch_timer.setSingleShot(True)
        self._switch_timer.timeout.connect(self._complete_switch)
        self._switch_timer.start(160)

    def _complete_switch(self) -> None:
        """离场淡出完成后执行 setCurrentIndex + 入场动画。"""

        self._switch_timer = None
        index = self._switch_index
        registrations = registered_panels()
        if not (0 <= index < len(registrations)):
            return
        self._stack.setCurrentIndex(index)
        entering = registrations[index]
        self._call_panel(entering.mode_id, "on_enter")
        # 入场动画：淡入 + 轻微上滑（240ms OutCubic，对齐 mode-stage-enter）。
        self._animate_page_enter(self._stack.currentWidget())

    def _stop_page_anims(self) -> None:
        """停止所有进行中的页面离场/入场动画。"""

        for anim in getattr(self, "_page_anims", []):
            try:
                anim.stop()
            except Exception:
                _log.warning("startup ready toast failed", exc_info=True)
        self._page_anims = []
        # Batch 23: 也停止离场淡出动画。
        for anim in getattr(self, "_leave_anims", []):
            try:
                anim.stop()
            except Exception:
                _log.warning("startup ready toast failed", exc_info=True)
        self._leave_anims = []
        # Batch 46: 取消待执行的延迟切换定时器（快速连续切换时不串页）。
        if getattr(self, "_switch_timer", None) is not None:
            self._switch_timer.stop()
            self._switch_timer = None

    def _animate_page_enter(self, page: QWidget) -> None:
        """页面入场动画（fade + slide），持有引用防 GC。"""

        try:
            from embeddebug.serial_station.ui.panel_animations import card_enter

            self._page_anims = card_enter(page)
            for anim in self._page_anims:
                anim.start()
        except Exception:
            _log.warning("startup ready toast failed", exc_info=True)  # 动画是锦上添花，失败不阻塞切换。

    def _animate_page_leave(self, page: QWidget) -> None:
        """页面离场快速淡出（Batch 23：激活 fade_out 死代码）。"""

        if page is None:
            return
        try:
            from embeddebug.serial_station.ui.panel_animations import fade_out

            anim = fade_out(page)
            self._leave_anims.append(anim)
            anim.start()
        except Exception:
            _log.warning("startup ready toast failed", exc_info=True)  # 离场淡出是锦上添花，失败不阻塞切换。

    def _call_panel(self, mode_id: str, method: str) -> None:
        """安全调用面板生命周期方法（缺失或异常不中断切换）。"""

        panel = self._panels.get(mode_id)
        if panel is None:
            return
        fn = getattr(panel, method, None)
        if fn is None:
            return
        try:
            fn()
        except Exception:
            _log.warning("startup ready toast failed", exc_info=True)

    def app_controller(self) -> AppController:
        """返回共享 AppController（供测试与命令面板访问）。"""

        return self._app_controller
