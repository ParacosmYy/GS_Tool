"""Error-notice composition for the terminal workspace.

Live observation, send controls, history, and batch controls each have their
own presentation builder so this module remains a small shell leaf.
"""

from __future__ import annotations

from ..action_surface import ActionRailButton
from ..error_surface import ErrorSignalSurface
from ..qt import QHBoxLayout, QLabel, QWidget


def build_error_bar(window) -> QWidget:
    """Build the dismissible application error notice."""

    container = QWidget(window)
    container.setObjectName("errorBar")
    container.setAccessibleName("应用错误通知")
    container.setAccessibleDescription("当前错误会在此显示；清除按钮可移除错误。")
    layout = QHBoxLayout(container)
    layout.setContentsMargins(8, 4, 8, 4)
    window._error_signal = ErrorSignalSurface(container)
    layout.addWidget(window._error_signal)
    window._error_label = QLabel()
    window._error_label.setProperty("role", "error")
    window._error_label.setAccessibleName("应用错误")
    window._error_label.setWordWrap(True)
    layout.addWidget(window._error_label, stretch=1)
    clear_button = ActionRailButton("清除")
    clear_button.setAccessibleName("清除错误")
    clear_button.setToolTip("清除当前错误提示；不会断开连接、清空终端或改变数据。")
    clear_button.setAccessibleDescription(
        "清除当前应用错误提示；不会断开连接、清空终端或改变接收数据。"
    )
    clear_button.clicked.connect(window._view_model.clear_error)
    layout.addWidget(clear_button)
    container.setVisible(False)
    window._error_container = container
    window._clear_error_button = clear_button
    return container


__all__ = ["build_error_bar"]
