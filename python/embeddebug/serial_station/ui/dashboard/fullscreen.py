"""仪表盘控件全屏与交互增强（对齐 VOFA+ 双击全屏）。

- ``toggle_fullscreen``：双击控件全屏显示，再双击恢复。
- ``WidgetFullscreenHandler``：附加到控件，管理全屏状态与几何恢复。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QWidget


class WidgetFullscreenHandler:
    """管理单个控件的双击全屏/恢复。

    用法：控件的 mouseDoubleClickEvent 调用 handler.toggle(widget)。
    """

    def __init__(self) -> None:
        self._fullscreen: bool = False
        self._saved_parent: QWidget | None = None
        self._saved_geometry = None
        self._target: QWidget | None = None

    @property
    def is_fullscreen(self) -> bool:
        return self._fullscreen

    def toggle(self, widget: QWidget, host: QWidget | None = None) -> bool:
        """切换 widget 全屏/恢复，返回切换后是否全屏。"""

        if self._fullscreen:
            self.restore()
            return False
        self.enter(widget, host)
        return True

    def enter(self, widget: QWidget, host: QWidget | None = None) -> None:
        """进入全屏：保存几何，把 widget 提升到 host 全屏覆盖。"""

        self._target = widget
        self._saved_parent = widget.parent()
        self._saved_geometry = widget.geometry()
        target_host = host or self._find_top_level(widget)
        if target_host is not None:
            widget.setParent(target_host)
            widget.setGeometry(target_host.rect())
            widget.raise_()
            widget.show()
        self._fullscreen = True

    def restore(self) -> None:
        """恢复全屏前的几何与 parent。"""

        if self._target is None:
            return
        if self._saved_parent is not None:
            self._target.setParent(self._saved_parent)
        if self._saved_geometry is not None:
            self._target.setGeometry(self._saved_geometry)
        self._target.show()
        self._fullscreen = False
        self._target = None
        self._saved_parent = None
        self._saved_geometry = None

    @staticmethod
    def _find_top_level(widget: QWidget) -> QWidget | None:
        parent = widget
        while parent is not None:
            p = parent.parent()
            if p is None:
                return parent
            parent = p
        return None


def attach_double_click_fullscreen(widget: QWidget, host: QWidget | None = None) -> WidgetFullscreenHandler:
    """给 widget 安装双击全屏处理器，返回处理器供外部持有。"""

    handler = WidgetFullscreenHandler()
    original_event = widget.mouseDoubleClickEvent

    def _on_double_click(event: object) -> None:
        if event.button() == Qt.MouseButton.LeftButton:
            handler.toggle(widget, host)
            event.accept()
            return
        original_event(event)

    widget.mouseDoubleClickEvent = _on_double_click  # type: ignore[method-assign]
    return handler
