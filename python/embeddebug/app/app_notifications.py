"""AppShell 通知子系统 helper（Batch 12 引入，Batch 15 抽离守 300 行门禁）。

把 AppShell 的通知装配、notify() 入口和 toast 快捷键处理从 app_shell.py 抽出，
避免 app_shell 超 300 行可维护性门禁。AppShell 持有一个 ``NotificationHost`` 实例，
委托装配/notify/Esc 处理。

职责边界：
- ``build(shell)``：实例化 NotificationManager + ToastContainer，定位右上角。
- ``show(shell, ...)``：level 字符串 → NotificationLevel → manager.show。
- ``handle_key_press(shell, event)``：Esc 关闭最早 toast，Ctrl+Shift+Esc 清空。
- ``reposition(shell)``：窗口 resize 时把容器对齐右上角。

约束：只依赖 PyQt6 + notifications + widgets，不访问 transport/protocol。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QMainWindow

from embeddebug.serial_station.notifications.data import NotificationLevel

_LEVEL_MAP = {
    "info": NotificationLevel.INFO,
    "success": NotificationLevel.SUCCESS,
    "warning": NotificationLevel.WARNING,
    "error": NotificationLevel.ERROR,
}


def build(shell: QMainWindow) -> None:
    """装配通知子系统：manager + 浮层 toast 容器（右上角）。"""

    from embeddebug.serial_station.notifications import NotificationManager
    from embeddebug.serial_station.ui.widgets.toast_container import ToastContainer

    shell._notification_manager = NotificationManager(parent=shell)
    shell._toast_container = ToastContainer(shell._notification_manager, parent=shell)
    # 容器定位到窗口右上角（resizeEvent 重新对齐）。
    shell._toast_container.setFixedWidth(340)
    shell._toast_container.raise_()
    shell._toast_container.hide()


def reposition(shell: QMainWindow) -> None:
    """窗口尺寸变化时把 toast 容器对齐到右上角。"""

    container = getattr(shell, "_toast_container", None)
    if container is None:
        return
    margin = 16
    try:
        width = shell.width()
        container.move(width - container.width() - margin, margin)
    except Exception:
        pass


def show(
    shell: QMainWindow,
    level: str,
    title: str,
    message: str,
    timeout_ms: int = 3000,
) -> None:
    """向用户弹一条非模态通知（level 字符串 → NotificationLevel → manager.show）。"""

    resolved = _LEVEL_MAP.get(level, NotificationLevel.INFO)
    manager = getattr(shell, "_notification_manager", None)
    if manager is None:
        return
    manager.show(resolved, title, message, timeout_ms=timeout_ms)


def handle_key_press(shell: QMainWindow, event: object) -> bool:
    """处理 toast 快捷键：Esc 关闭最早，Ctrl+Shift+Esc 清空。

    Returns:
        True 表示已处理（调用方应 event.accept），False 表示非 toast 快捷键（交给父类）。
    """

    # event.key 是方法，需调用取值（QKeyEvent.key()）。
    key_method = getattr(event, "key", None)
    key = key_method() if callable(key_method) else None
    if key is None:
        return False
    container = getattr(shell, "_toast_container", None)
    if container is None:
        return False
    modifiers = event.modifiers()
    has_ctrl = bool(modifiers & Qt.KeyboardModifier.ControlModifier)
    has_shift = bool(modifiers & Qt.KeyboardModifier.ShiftModifier)
    # Ctrl+Shift+Esc → clear_all（通知风暴一键清空）。
    if key == Qt.Key.Key_Escape and has_ctrl and has_shift:
        container.clear_all()
        return True
    # 单 Esc（无 Ctrl/Shift）→ dismiss_oldest（关闭最早一条 toast）。
    if key == Qt.Key.Key_Escape and not has_ctrl and not has_shift:
        return bool(container.dismiss_oldest())
    return False
