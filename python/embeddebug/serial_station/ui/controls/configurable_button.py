"""可配置按钮控件（对齐 VOFA+ 按钮控件 + 命令模板）。

带文本 + 图标 + 命令模板的按钮，点击发出 command 信号携带格式化命令。
可配置图标（lucide 名称）、强调色、命令模板。

约束：本模块只依赖 PyQt6 + icons + theme.palette，不访问 controller/transport。
"""

from __future__ import annotations

from collections.abc import Callable

from PyQt6.QtCore import pyqtSignal
from PyQt6.QtWidgets import QPushButton, QWidget

from embeddebug.serial_station.ui.icons import IconManager
from embeddebug.serial_station.ui.theme import palette as P


class ConfigurableButton(QPushButton):
    """可配置命令按钮：文本 + 图标 + 命令模板，点击发出 command 信号。"""

    command = pyqtSignal(str)

    def __init__(
        self,
        text: str = "",
        command_template: str = "",
        icon_name: str | None = None,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(text, parent)
        self.setObjectName("serialStationConfigurableButton")
        self._command_template = command_template
        self._formatter: Callable[[], str] | None = None
        self.clicked.connect(self._emit_command)
        if icon_name:
            self.set_icon(icon_name)

    def set_icon(self, icon_name: str, color: str = P.TEXT_SECONDARY) -> None:
        """设置 lucide 图标。"""

        icon = IconManager().icon(icon_name, color=color)
        if not icon.isNull():
            self.setIcon(icon)

    def set_command_template(self, template: str) -> None:
        self._command_template = template

    def set_formatter(self, formatter: Callable[[], str]) -> None:
        """设置自定义命令格式化函数（覆盖 command_template）。"""

        self._formatter = formatter

    def _emit_command(self) -> None:
        if self._formatter is not None:
            self.command.emit(self._formatter())
        elif self._command_template:
            self.command.emit(self._command_template)

    def get_command(self) -> str:
        """返回当前会发出的命令文本（不实际发出）。"""

        if self._formatter is not None:
            return self._formatter()
        return self._command_template
