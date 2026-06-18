"""多模式应用 — 模式面板协议与注册表。

定义 ``ModePanel`` 协议（每个功能模式实现）和 ``PanelRegistry``（AppShell 装配源）。
新增一个功能模式 = 实现一个 ``ModePanel`` + 调用 ``register_panel`` 注册一行。
AppShell 按注册顺序生成导航图标，点击切换 QStackedWidget 对应页。

约束：本模块只定义协议与注册表，不 import ui 或具体模式实现。
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import TYPE_CHECKING, Protocol

if TYPE_CHECKING:
    from PyQt6.QtWidgets import QWidget

    from embeddebug.app.app_controller import AppController


class ModePanel(Protocol):
    """单个功能模式的 UI 面板契约。

    生命周期：``build`` 在 AppShell 首次装配时调用一次（惰性）；
    ``on_enter`` / ``on_leave`` 在切换到/离开该模式时调用。
    """

    def build(self, app_controller: "AppController") -> "QWidget":
        """构建并返回该模式的主控件（装进 QStackedWidget）。"""
        ...

    def on_enter(self) -> None:
        """切入该模式时调用（默认空实现，可重写做检查如 OTA 连接态）。"""
        ...

    def on_leave(self) -> None:
        """切出该模式时调用（默认空实现，可重写做中止任务）。"""
        ...


@dataclass(frozen=True)
class PanelRegistration:
    """一个模式在导航栏的注册项。"""

    mode_id: str          # 唯一标识（如 "serial" / "ota"）
    icon: str             # lucide 图标名（如 "cable"）
    label: str            # 导航 tooltip / 文字
    factory: "PanelFactory"


# factory 接受 AppController，返回 ModePanel 实例。
PanelFactory = "__factory_placeholder__"  # 真实类型在运行时是 Callable[[AppController], ModePanel]


_REGISTRY: list[PanelRegistration] = []


def register_panel(
    mode_id: str, icon: str, label: str, factory: object
) -> None:
    """注册一个模式面板（按注册顺序追加到导航栏）。

    重复 mode_id 会被忽略（幂等），便于模块多次 import 时不重复注册。
    """

    if any(r.mode_id == mode_id for r in _REGISTRY):
        return
    _REGISTRY.append(PanelRegistration(mode_id=mode_id, icon=icon, label=label, factory=factory))  # type: ignore[arg-type]


def registered_panels() -> tuple[PanelRegistration, ...]:
    """返回已注册模式的有序快照（供 AppShell 装配导航栏）。"""

    return tuple(_REGISTRY)


def reset_registry() -> None:
    """清空注册表（仅测试用，便于隔离用例）。"""

    _REGISTRY.clear()
