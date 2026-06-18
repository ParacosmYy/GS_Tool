"""响应式断点：窗口 < 900px 折叠侧栏。

对齐 05-ui-standard §十七（响应式布局：窗口 <900px 自动折叠导航、断点动画过渡）。

Batch 4 (B2) 改进：修复 AppShell 模式下响应式功能性失效。
- 旧版依赖 ``SerialStationMainWindow.resizeEvent`` 驱动，但在多模式 shell 下
  SerialStationMainWindow 的内容被 reparent 到 AppShell 的 QStackedWidget，
  真正 resize 的是 AppShell，旧 resizeEvent 不触发，响应式控制器从未运行。
- 新增 ``attach_to_top_level(window)``：用事件过滤器监听**顶层窗口**的 resize，
  无论内容被 reparent 到哪都生效。SerialStationMainWindow 装配时调用一次即可。

设计要点：
- ``ResponsiveLayout`` 持有 QSplitter 与断点阈值，监听窗口 resize。
- 跨过断点时折叠左区（设为最小宽度）并发出 ``sidebar_collapsed`` 信号；
  跨回时恢复并发出 ``sidebar_expanded`` 信号，供 UI 做过渡动画。
- 不直接销毁控件，只调整 splitter sizes，保证 findChild 仍可达。

约束：本模块只依赖 PyQt6 + 标准库，不访问 controller/transport/protocol。
"""

from __future__ import annotations

from PyQt6.QtCore import QEvent, QObject, pyqtSignal
from PyQt6.QtWidgets import QSplitter, QWidget

BREAKPOINT_COLLAPSE = 900   # 窗口宽度 < 900 折叠侧栏
BREAKPOINT_EXPAND = 980     # 窗口宽度 >= 980 恢复（滞后，避免边界抖动）
COLLAPSED_WIDTH = 0         # 折叠时左区宽度
LEFT_ZONE_INDEX = 0         # 三区中左区索引


class ResponsiveLayout(QObject):
    """响应式布局控制器：按窗口宽度折叠/展开左区侧栏。

    支持两种驱动方式：
    1. ``on_window_resized(width)``：调用方手动转发窗口宽度（向后兼容）。
    2. ``attach_to_top_level(window)``：自动监听顶层窗口 resize（推荐），
       解决 AppShell 多模式 shell 下内容被 reparent 导致旧 resizeEvent 不触发的问题。
    """

    sidebar_collapsed = pyqtSignal()
    sidebar_expanded = pyqtSignal()

    def __init__(self, splitter: QSplitter, parent: QWidget | None = None) -> None:
        super().__init__(parent or splitter)
        self._splitter = splitter
        self._collapsed = False
        self._saved_sizes: list[int] = []
        self._top_level: QWidget | None = None

    @property
    def is_collapsed(self) -> bool:
        return self._collapsed

    def attach_to_top_level(self, window: QWidget) -> None:
        """绑定顶层窗口，监听其 resize 事件自动驱动断点。

        用于 AppShell 多模式 shell：SerialStationMainWindow 内容被 reparent 后，
        本控制器仍能通过监听真正 resize 的顶层窗口（AppShell）工作。
        """

        self._top_level = window
        window.installEventFilter(self)

    def eventFilter(self, watched: object, event: object) -> bool:
        """监听顶层窗口 resize，转发到断点逻辑。"""

        if watched is self._top_level and getattr(event, "type", lambda: None)() == QEvent.Type.Resize:
            width = getattr(event, "size", lambda: None)
            if width is not None:
                try:
                    self.on_window_resized(width().width())
                except Exception:
                    pass
        return False

    def on_window_resized(self, window_width: int) -> None:
        """窗口 resize 时调用，按断点折叠/展开侧栏。"""

        if not self._collapsed and window_width < BREAKPOINT_COLLAPSE:
            self._collapse()
        elif self._collapsed and window_width >= BREAKPOINT_EXPAND:
            self._expand()

    def _collapse(self) -> None:
        """折叠左区：保存当前 sizes，把左区设为 0。"""

        if self._splitter.count() <= LEFT_ZONE_INDEX:
            return
        self._saved_sizes = list(self._splitter.sizes())
        sizes = list(self._splitter.sizes())
        total = sum(sizes) or 1
        sizes[LEFT_ZONE_INDEX] = COLLAPSED_WIDTH
        remaining = total - COLLAPSED_WIDTH
        # 把左区让出的宽度按原比例分给中右区。
        others = [s for i, s in enumerate(sizes) if i != LEFT_ZONE_INDEX]
        others_total = sum(others) or 1
        for i in range(len(sizes)):
            if i == LEFT_ZONE_INDEX:
                continue
            sizes[i] = int(remaining * (sizes[i] / others_total))
        self._splitter.setSizes(sizes)
        self._collapsed = True
        self.sidebar_collapsed.emit()

    def _expand(self) -> None:
        """展开左区：恢复保存的 sizes。"""

        if self._saved_sizes:
            self._splitter.setSizes(self._saved_sizes)
        self._collapsed = False
        self.sidebar_expanded.emit()
