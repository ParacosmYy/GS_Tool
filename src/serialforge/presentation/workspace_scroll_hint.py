"""Presentation-only scroll discoverability for the workspace route strip.

The hint observes only the active settings page's vertical scrollbar. It does
not own page content, navigation, business state, or a second timer, which
keeps the compact shell affordance reusable across every workspace tab.
"""

from __future__ import annotations

from .property_refresh import refresh_dynamic_property
from .qt import QLabel, QScrollArea, Qt, QWidget


class WorkspaceScrollHint(QLabel):
    """Show a bounded, accessible cue when the active page can scroll."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("workspaceScrollHint")
        self.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setFixedHeight(22)
        self.setMinimumWidth(112)
        self.setMaximumWidth(158)
        self.setAccessibleName("工作区滚动位置")
        self._scroll_area: QScrollArea | None = None
        self._scroll_bar = None
        self._set_state("hidden", "", "")

    def bind_page(self, page: QWidget | None) -> None:
        """Observe one active settings page and detach from the previous one."""

        scroll = page if isinstance(page, QScrollArea) else None
        if scroll is self._scroll_area:
            self._refresh()
            return
        self._disconnect_scroll_bar()
        self._scroll_area = scroll
        if scroll is None:
            self._set_state("hidden", "", "")
            return
        self._scroll_bar = scroll.verticalScrollBar()
        self._scroll_bar.valueChanged.connect(self._refresh)
        self._scroll_bar.rangeChanged.connect(self._refresh)
        self._refresh()

    def _disconnect_scroll_bar(self) -> None:
        """Disconnect scroll bar."""
        bar = self._scroll_bar
        self._scroll_bar = None
        if bar is None:
            return
        for signal in (bar.valueChanged, bar.rangeChanged):
            try:
                signal.disconnect(self._refresh)
            except (RuntimeError, TypeError):
                pass

    def _refresh(self, *_args: object) -> None:
        """Map the native scrollbar range to a small semantic shell state."""

        bar = self._scroll_bar
        if bar is None:
            self._set_state("hidden", "", "")
            return
        maximum = max(0, int(bar.maximum()))
        value = max(0, min(maximum, int(bar.value())))
        if maximum == 0:
            self._set_state(
                "complete",
                "全显 · 内容已全部显示",
                "当前工作区内容已完整显示，无需滚动。",
            )
        elif value == 0:
            self._set_state(
                "top",
                "顶部 · ↓ 向下查看",
                "当前位于工作区顶部；向下滚动可查看其余配置。",
            )
        elif value >= maximum:
            self._set_state(
                "bottom",
                "底部 · ↑ 返回顶部",
                "当前位于工作区底部；向上滚动可返回顶部配置。",
            )
        else:
            self._set_state(
                "middle",
                "中段 · ↕ 上下滚动",
                "当前工作区内容超出可视范围，可向上或向下滚动。",
            )

    def _set_state(self, state: str, text: str, description: str) -> None:
        """Set state."""
        refresh_dynamic_property(self, "state", state)
        self.setText(text)
        self.setAccessibleDescription(description)
        self.setToolTip(description)
        self.setVisible(state != "hidden")


__all__ = ["WorkspaceScrollHint"]
