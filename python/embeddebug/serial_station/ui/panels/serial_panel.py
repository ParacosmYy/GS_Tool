"""串口模式面板 — 把现有 SerialStationMainWindow 作为串口模式接入 AppShell。

由于 SerialStationMainWindow 是完整 QMainWindow（含 TopBar/三栏/快捷键/命令面板），
作为 AppShell 的一页时，取其 centralWidget 内容嵌入 QStackedWidget，并保留
SerialStationMainWindow 实例作为 owner（持有所有 action 委托与 controller 回调）。

约束：不改 SerialStationMainWindow 内部逻辑，零侵入接入多模式 shell。
"""

from __future__ import annotations

from PyQt6.QtWidgets import QWidget

from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.ui.main_window import SerialStationMainWindow


class SerialPanel:
    """串口模式 ModePanel：包装 SerialStationMainWindow。"""

    def __init__(self) -> None:
        self._window: SerialStationMainWindow | None = None

    def build(self, app_controller: AppController) -> QWidget:
        """构建串口模式主控件：实例化串口窗口并取其内容区。"""

        self._window = SerialStationMainWindow(app_controller=app_controller)
        # 取中央内容（三栏 + TopBar），脱离原窗口父级，作为面板内容。
        content = self._window.centralWidget()
        content.setParent(None)
        # 包一层容器，objectName 便于 QSS 与 findChild。
        wrapper = QWidget()
        wrapper.setObjectName("serialStationSerialPanel")
        layout = __import__("PyQt6.QtWidgets", fromlist=["QVBoxLayout"]).QVBoxLayout(wrapper)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(content)
        return wrapper

    def on_enter(self) -> None:
        """切入串口模式（窗口已 show 时聚焦，MVP 无额外动作）。"""

    def on_leave(self) -> None:
        """切出串口模式（保留连接与日志，MVP 无额外动作）。"""

    @property
    def window(self) -> SerialStationMainWindow | None:
        """暴露底层串口窗口（供测试与命令面板访问 action 委托）。"""

        return self._window
