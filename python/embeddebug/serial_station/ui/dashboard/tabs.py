"""多标签页仪表盘（对齐 VOFA+ 多仪表盘切换）。

每个标签页持有一个 DashboardCanvas，可新建/重命名/关闭标签页。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtWidgets import QTabWidget, QWidget

from embeddebug.serial_station.ui.dashboard.canvas import DashboardCanvas
from embeddebug.serial_station.ui.theme import palette as P


class DashboardTabs(QTabWidget):
    """多标签页仪表盘容器。"""

    canvas_changed = pyqtSignal(object)  # 当前 canvas

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationDashboardTabs")
        self.setTabsClosable(True)
        self.tabCloseRequested.connect(self._close_tab)
        self.currentChanged.connect(self._on_tab_changed)
        self._add_canvas(self._default_tab_name())

    def _default_tab_name(self, index: int | None = None) -> str:
        count = index if index is not None else self.count()
        return f"Dashboard {count + 1}" if count == 0 else f"Dashboard {count}"

    def _add_canvas(self, name: str) -> DashboardCanvas:
        canvas = DashboardCanvas(self)
        index = self.addTab(canvas, name)
        self.setCurrentIndex(index)
        return canvas

    def add_tab(self, name: str | None = None) -> DashboardCanvas:
        """新增一个标签页，返回其 canvas。"""

        return self._add_canvas(name or self._default_tab_name())

    def current_canvas(self) -> DashboardCanvas | None:
        widget = self.currentWidget()
        if isinstance(widget, DashboardCanvas):
            return widget
        return None

    def rename_tab(self, index: int, name: str) -> None:
        if 0 <= index < self.count():
            self.setTabText(index, name)

    def _close_tab(self, index: int) -> None:
        if self.count() <= 1:
            return  # 至少保留一个标签页
        self.removeTab(index)

    def _on_tab_changed(self, index: int) -> None:
        canvas = self.current_canvas()
        if canvas is not None:
            self.canvas_changed.emit(canvas)

    def tab_names(self) -> list[str]:
        return [self.tabText(i) for i in range(self.count())]
