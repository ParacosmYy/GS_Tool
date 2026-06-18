"""命令↔控件绑定层（对齐 VOFA+ 控件↔命令优雅绑定）。

把控件信号（滑块值变化、按钮点击）绑定到命令发送，通过 protocol.build_command
格式化命令后交给发送回调。支持声明式绑定配置与运行时增删。

约束：本模块只依赖 PyQt6 + 标准库，不直接访问 controller/transport；
发送动作通过注入的 send_callable 完成，便于测试。
"""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, field
from typing import Protocol

from PyQt6.QtCore import QObject, pyqtSignal
from PyQt6.QtWidgets import QPushButton, QSlider, QWidget

SendCallable = Callable[[str], None]


class _BindableWidget(Protocol):
    """可绑定命令的控件协议（有 command 信号或可连 clicked/valueChanged）。"""

    def connect(self, slot: Callable[..., None]) -> None: ...


@dataclass
class Binding:
    """一条控件↔命令绑定。"""

    widget: QWidget
    command_template: str
    formatter: Callable[[], str] | None = None
    enabled: bool = True


class CommandBinder(QObject):
    """管理控件↔命令绑定的集合。

    用法：
        binder = CommandBinder(send_callable=lambda cmd: transport.write(cmd.encode()))
        binder.bind_slider(slider, "PWM={value}")
        binder.bind_button(button, "RESET")
    """

    binding_added = pyqtSignal(str)
    binding_removed = pyqtSignal(str)

    def __init__(self, send_callable: SendCallable, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self._send = send_callable
        self._bindings: dict[str, Binding] = {}

    @property
    def bindings(self) -> dict[str, Binding]:
        return dict(self._bindings)

    def bind_slider(
        self,
        slider: QSlider,
        command_template: str,
        binding_id: str | None = None,
    ) -> str:
        """绑定滑块：值变化时发送 command_template.format(value=v)。"""

        bid = binding_id or f"slider_{id(slider)}"
        slider.valueChanged.connect(lambda v: self._emit_formatted(bid, command_template.format(value=v)))
        self._bindings[bid] = Binding(widget=slider, command_template=command_template)
        self.binding_added.emit(bid)
        return bid

    def bind_button(
        self,
        button: QPushButton,
        command_template: str,
        binding_id: str | None = None,
    ) -> str:
        """绑定按钮：点击时发送 command_template。"""

        bid = binding_id or f"button_{id(button)}"
        button.clicked.connect(lambda: self._emit_formatted(bid, command_template))
        self._bindings[bid] = Binding(widget=button, command_template=command_template)
        self.binding_added.emit(bid)
        return bid

    def bind_custom(
        self,
        widget: QWidget,
        formatter: Callable[[], str],
        signal: pyqtSignal,
        binding_id: str | None = None,
    ) -> str:
        """绑定任意控件：信号触发时用 formatter 生成命令。"""

        bid = binding_id or f"custom_{id(widget)}"
        signal.connect(lambda: self._emit_custom(bid, formatter))
        self._bindings[bid] = Binding(widget=widget, command_template="", formatter=formatter)
        self.binding_added.emit(bid)
        return bid

    def remove(self, binding_id: str) -> bool:
        binding = self._bindings.pop(binding_id, None)
        if binding is None:
            return False
        self.binding_removed.emit(binding_id)
        return True

    def enable(self, binding_id: str) -> None:
        binding = self._bindings.get(binding_id)
        if binding is not None:
            binding.enabled = True

    def disable(self, binding_id: str) -> None:
        binding = self._bindings.get(binding_id)
        if binding is not None:
            binding.enabled = False

    def binding_ids(self) -> list[str]:
        return sorted(self._bindings)

    def _emit_formatted(self, binding_id: str, command: str) -> None:
        binding = self._bindings.get(binding_id)
        if binding is None or not binding.enabled:
            return
        self._send(command)

    def _emit_custom(self, binding_id: str, formatter: Callable[[], str]) -> None:
        binding = self._bindings.get(binding_id)
        if binding is None or not binding.enabled:
            return
        self._send(formatter())
