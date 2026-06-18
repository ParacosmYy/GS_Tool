"""多模式应用 — 应用级控制器。

持有串口 ``SerialWorkbenchController``，并暴露共享 transport 访问，供其他模式
（如 OTA）复用已连接的串口连接，无需各模式独立建连。

约束：本模块只持有 controller 引用与 transport 访问，不 import ui。
"""

from __future__ import annotations

from embeddebug.serial_station.controllers import SerialWorkbenchController
from embeddebug.serial_station.drivers import SerialTransport


class AppController:
    """应用级控制器：串口连接的唯一持有者 + 跨模式共享 transport 入口。

    各模式面板通过 ``active_transport()`` 拿到当前已连接的串口（未连接返回 None），
    实现「串口模式连接 → OTA 模式直接复用该连接」的共享工作流。
    """

    def __init__(self) -> None:
        self._serial_controller = SerialWorkbenchController()

    @property
    def serial_controller(self) -> SerialWorkbenchController:
        """串口模式 controller（面板用它做连接/发送/日志）。"""

        return self._serial_controller

    def active_transport(self) -> SerialTransport | None:
        """当前已连接的串口 transport；未连接返回 None（跨模式共享通道）。"""

        return self._serial_controller.active_transport

    def is_connected(self) -> bool:
        """串口是否已连接（供 OTA 等模式做连接态 gating）。"""

        return self._serial_controller.is_connected
