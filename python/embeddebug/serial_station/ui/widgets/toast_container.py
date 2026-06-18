"""Toast 容器 — NotificationManager 信号 → ToastWidget 的渲染编排层。

填补通知子系统（notifications/manager）只有逻辑层而无 UI 渲染的缺口（Batch 12）：
ToastContainer 绑定 NotificationManager 的 notification_added/removed 信号，对每条
新增通知创建 ToastWidget 从右滑入（enter），并在通知 removed 或 toast closed 时
淡出移除。容器自身是浮在父窗口右上角的 QWidget（半透明，不抢焦点）。

设计要点：
- QVBoxLayout 竖排，自底向上堆叠（新 toast 加在顶部），固定最大可见数。
- add(data) → 创建 ToastWidget + enter() + schedule_dismiss；closed 信号回调 dismiss。
- remove(uid) → 找到对应 toast 调 leave()（淡出后自动从布局移除）。
- 容器 setAttribute WA_TransparentForMouseEvents 之外仍保留 toast 可点击关闭
  （toast 自身接收鼠标事件；容器背景透明）。
- 限制最大可见数，超出移除最早一条（配合 manager 的 max_visible 上限）。

约束：UI 编排层，依赖 NotificationManager + ToastWidget + PyQt6；不访问 transport/protocol。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QLabel, QVBoxLayout, QWidget

from embeddebug.serial_station.notifications.data import NotificationData
from embeddebug.serial_station.notifications.manager import NotificationManager
from embeddebug.serial_station.ui.widgets.toast import ToastWidget


class ToastContainer(QWidget):
    """通知渲染容器：把 NotificationManager 的事件渲染成 ToastWidget 堆。

    用法::

        manager = NotificationManager()
        container = ToastContainer(manager, parent=window)
        manager.show(NotificationLevel.SUCCESS, "连接成功", "串口 COM3 已就绪")
        # container 自动渲染 + 自动消失

    Args:
        manager: 通知逻辑层（提供 notification_added/removed 信号）。
        max_visible: 容器同时渲染的最大 toast 数（超出挤掉最早一条）。
        parent: 父控件（通常是应用顶层窗口）。
    """

    def __init__(
        self,
        manager: NotificationManager,
        max_visible: int = 4,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationToastContainer")
        self._manager = manager
        self._max_visible = max(1, int(max_visible))
        # uid → ToastWidget 活跃映射（按加入顺序）。
        self._active: list[tuple[int, ToastWidget]] = []
        # 隐藏的占位：容器无 toast 时不显示，避免空 widget 占位。
        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.Tool)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, False)
        self.setAttribute(Qt.WidgetAttribute.WA_ShowWithoutActivating)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)
        layout.setAlignment(Qt.AlignmentFlag.AlignTop)
        # 空态隐藏占位 label（防止容器被布局压成 0 宽）。
        self._placeholder = QLabel("", self)
        self._placeholder.setObjectName("serialStationToastPlaceholder")
        layout.addWidget(self._placeholder)
        self._layout = layout

        # 接 manager 信号。
        manager.notification_added.connect(self._on_added)
        manager.notification_removed.connect(self._on_removed)
        self.hide()

    # ── manager 信号回调 ────────────────────────────────────────────
    def _on_added(self, data: NotificationData) -> None:
        """manager 新增通知 → 创建 ToastWidget 并入场。"""

        self.show()
        toast = ToastWidget(data, parent=self)
        toast.closed.connect(self._on_toast_closed)
        self._layout.insertWidget(self._active_count_widget_index(), toast)
        self._active.append((data.uid, toast))
        toast.enter()
        toast.schedule_dismiss(data.timeout_ms)
        self._enforce_max_visible()

    def _on_removed(self, data: NotificationData) -> None:
        """manager 移除通知（如超时/主动 dismiss）→ 对应 toast 淡出。"""

        toast = self._find(data.uid)
        if toast is not None and not toast.is_leaving:
            toast.leave()

    # ── toast 自身关闭回调 ──────────────────────────────────────────
    def _on_toast_closed(self, toast: ToastWidget) -> None:
        """ToastWidget 离场完成 → 从布局/映射移除，空了则隐藏容器。"""

        for i, (uid, t) in enumerate(list(self._active)):
            if t is toast:
                del self._active[i]
                self._layout.removeWidget(t)
                t.deleteLater()
                break
        if not self._active:
            self.hide()

    # ── 内部 ────────────────────────────────────────────────────────
    def _find(self, uid: int) -> ToastWidget | None:
        for existing_uid, toast in self._active:
            if existing_uid == uid:
                return toast
        return None

    def _active_count_widget_index(self) -> int:
        """新 toast 插入布局的位置（顶部，placeholder 之后）。"""

        # placeholder 永远在 index 0；新 toast 紧随其后。
        return 1

    def _enforce_max_visible(self) -> None:
        """超过 max_visible 时挤掉最早一条（对最旧的调 leave）。

        单次挤兑一条而非循环清理：leave() 触发淡出动画，toast 仍在 _active 直到
        closed 回调才真正移除，循环 while 会因 _active 不立即收缩而卡住。多余条目
        在 closed 回调后由下一次 add 的挤兑递进清理。
        """

        if len(self._active) <= self._max_visible:
            return
        # 找最早一条未在离场的，触发 leave。
        for existing_uid, toast in self._active:
            if not toast.is_leaving:
                toast.leave()
                return

    # ── 测试/外部访问 ───────────────────────────────────────────────
    @property
    def active_toasts(self) -> tuple[ToastWidget, ...]:
        """当前活跃（未离场）的 toast 列表。"""

        return tuple(t for _, t in self._active)

    @property
    def is_empty(self) -> bool:
        """容器是否无活跃 toast（用于断言/布局隐藏）。"""

        return not self._active

    @property
    def count(self) -> int:
        """活跃 toast 数量（未含正在淡出的）。"""

        return len(self._active)

    def clear_all(self) -> None:
        """对所有活跃 toast 触发淡出离场（与 manager.clear 配合，逐条 leave）。"""

        for _, toast in list(self._active):
            if not toast.is_leaving:
                toast.leave()

    def dismiss_oldest(self) -> bool:
        """挤掉最早一条 toast（淡出）；无活跃 toast 返回 False。

        供快捷键（如 Esc 清通知）或容量管理调用。
        """

        for _, toast in self._active:
            if not toast.is_leaving:
                toast.leave()
                return True
        return False
