"""Batch 49-2: 日志区连接加载态 helper。

独立模块，保持 ``sections.py`` ≤ 260 行（architecture test 守护）。

连接开始时日志区显示 SkeletonBlock（3 行闪烁）+ 「正在建立连接…」文案，
告知用户正在建立连接；首帧 RX 或连接失败时淡出，让位给真实日志或空态。

提供：
- ``build_log_loading_state(parent)``：构造覆盖层（SkeletonBlock + 文案）。
- ``show_log_loading_state(host)`` / ``hide_log_loading_state(host)``：
  从 host 上挂载的 ``_log_loading_state`` 安全切换显隐。

触发点：
- connect_fake/connect_serial/connect_tcp/connect_udp 开始 → show（connection_actions）。
- 首帧 RX / 连接失败 / disconnect → hide。

约束：只依赖 PyQt6 + widgets + theme，不访问 controller/transport/protocol。
"""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QLabel, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.widgets.skeleton import SkeletonBlock


class _LogLoadingStateHost(Protocol):
    """host 上访问 ``_log_loading_state`` 所需的最小表面（getattr 防御）。"""

    _log_loading_state: QWidget | None


class LogLoadingOverlay(QWidget):
    """日志区连接加载态覆盖层：SkeletonBlock（3 行闪烁）+ 文案。

    作为日志卡的子控件覆盖层，由调用方的 resizeEvent 居中（或简单 addWidget
    到 body 末尾，QVBoxLayout 自然堆叠在 log_view 下方）。当前采用 addWidget
    策略：loading 时 log_view 隐藏、overlay 显示；hide 时反之。
    """

    def __init__(self, parent: QWidget) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationLogLoadingOverlay")
        layout = QVBoxLayout(self)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # SkeletonBlock：3 行闪烁，模拟即将出现的日志行。
        self._skeleton = SkeletonBlock(parent=self, rows=3, row_height=14, with_title=False)
        self._skeleton.setObjectName("serialStationLogLoadingSkeleton")
        layout.addWidget(self._skeleton)

        self._label = QLabel(self.tr("正在建立连接…"), self)
        self._label.setObjectName("serialStationLogLoadingLabel")
        self._label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        label_font = self._label.font()
        from embeddebug.serial_station.ui.theme import tokens as T
        label_font.setPointSize(T.FONT_POINT_BODY)
        self._label.setFont(label_font)
        self._label.setStyleSheet(f"color: {P.TEXT_MUTED};")
        layout.addWidget(self._label)
        self.hide()


def build_log_loading_state(parent: QWidget) -> LogLoadingOverlay:
    """构建日志区连接加载态覆盖层。

    Args:
        parent: 日志卡 body 容器。

    Returns:
        配置好的 LogLoadingOverlay，调用方负责 addWidget + 初始 hide。
    """

    return LogLoadingOverlay(parent)


def show_log_loading_state(host: _LogLoadingStateHost) -> None:
    """显示日志连接加载态（连接开始时调用）。"""

    overlay = getattr(host, "_log_loading_state", None)
    if overlay is None:
        return
    overlay.show()
    overlay.raise_()


def hide_log_loading_state(host: _LogLoadingStateHost) -> None:
    """隐藏日志连接加载态（首帧 RX / 连接失败 / disconnect 时调用）。"""

    overlay = getattr(host, "_log_loading_state", None)
    if overlay is None:
        return
    overlay.hide()
