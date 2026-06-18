"""可配置按钮控件（对齐 VOFA+ 按钮控件 + 命令模板）。

带文本 + 图标 + 命令模板的按钮，点击发出 command 信号携带格式化命令。
可配置图标（lucide 名称）、强调色、命令模板。

Batch 3 (B1) 改进：接入 ``ScaleAnimation.press`` 按压回弹动画，点击时有
物理按压反馈（缩小到 0.94 再 OutBack 回弹），符合 05-ui-standard 铁律 17
（按钮必须有 hover/pressed/disabled 三态）的动效层。

Batch 5 (C1) 改进：默认接入 ``install_hover_lift``（hover 时上浮 + accent tint
阴影），可通过 ``set_hover_lift(False)`` 关闭（表格密集场景）。按压 + hover
双重微交互，达到 Linear/Vercel 级按钮质感。

约束：本模块只依赖 PyQt6 + icons + theme.palette + animations，不访问 controller/transport。
"""

from __future__ import annotations

from collections.abc import Callable

from PyQt6.QtCore import pyqtSignal
from PyQt6.QtWidgets import QPushButton, QWidget

from embeddebug.serial_station.ui.animations.scale import ScaleAnimation
from embeddebug.serial_station.ui.icons import IconManager
from embeddebug.serial_station.ui.micro_interactions import install_hover_lift
from embeddebug.serial_station.ui.theme import palette as P


class ConfigurableButton(QPushButton):
    """可配置命令按钮：文本 + 图标 + 命令模板，点击发出 command 信号。

    点击时触发按压回弹动画（ScaleAnimation.press），产生物理反馈感。
    可通过 ``set_press_animation(False)`` 关闭（如表格内密集按钮场景）。
    """

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
        self._press_animation_enabled = True
        self._hover_lift_enabled = True
        self.clicked.connect(self._on_clicked)
        if icon_name:
            self.set_icon(icon_name)
        # 默认装 hover lift（Batch 5）：控件库按钮通常在仪表盘/独立容器，不在会被
        # QSplitter 挤压的主布局里，hover 上浮 + accent tint 阴影安全。
        if self._hover_lift_enabled:
            install_hover_lift(self)

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

    def set_press_animation(self, enabled: bool) -> None:
        """启用/禁用按压回弹动画（表格内密集按钮可关闭以减少视觉干扰）。"""

        self._press_animation_enabled = enabled

    def set_hover_lift(self, enabled: bool) -> None:
        """启用/禁用 hover 上浮动画。

        构造时已默认安装；若需在会被父布局挤压的位置（如 QSplitter 紧邻区）
        关闭以避免位移引起重排，调用本方法。
        """

        self._hover_lift_enabled = enabled
        if not enabled:
            # 移除已装的 graphics effect（hover lift 通过 QGraphicsDropShadowEffect 实现）。
            self.setGraphicsEffect(None)

    def _on_clicked(self) -> None:
        """点击处理：先播放按压动画，再发出命令。"""

        if self._press_animation_enabled:
            # ScaleAnimation.press 自动防 GC（类级活跃列表），无需持有引用。
            ScaleAnimation.press(self).start()
        self._emit_command()

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
