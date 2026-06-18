"""按 objectName 为 Serial Station 按钮装配 lucide 图标。

集中管理按钮→图标的映射，避免在 action 模块或 section builder 中散落
setIcon 调用。在窗口构建完成后调用一次即可。

图标着色复用 IconManager + palette。连接类主按钮用强调青，
断开用错误红，其余用次文本色，与 QSS 三态保持视觉一致。

约束：本模块只读 owner 上的控件引用，不访问 controller/transport/protocol。
"""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.icons import IconManager
from embeddebug.serial_station.ui.theme import palette as P


class ButtonIconHost(Protocol):
    """持有需装饰图标的按钮引用的宿主（SerialStationMainWindow）。"""

    def __getattr__(self, name: str) -> object: ...


# objectName -> (lucide 图标名, 着色)。颜色与 QSS 按钮文字色对齐。
_BUTTON_ICON_MAP: dict[str, tuple[str, str]] = {
    "serialStationConnectButton": ("plug-zap", P.TEXT_ON_ACCENT),
    "serialStationConnectSerialButton": ("cable", P.TEXT_ON_ACCENT),
    "serialStationConnectTcpButton": ("ethernet", P.TEXT_ON_ACCENT),
    "serialStationConnectUdpButton": ("radio", P.TEXT_ON_ACCENT),
    "serialStationDisconnectButton": ("unlink", P.ERROR),
    "serialStationRefreshPortsButton": ("refresh-cw", P.TEXT_SECONDARY),
    "serialStationSendButton": ("send", P.TERM_TX),
    "serialStationInjectButton": ("download", P.TERM_TX),
    "serialStationExportLogButton": ("save", P.TEXT_SECONDARY),
    "serialStationReplayLogButton": ("play", P.TEXT_SECONDARY),
    "serialStationSaveProfileButton": ("save", P.TEXT_SECONDARY),
    "serialStationLoadProfileButton": ("folder-open", P.TEXT_SECONDARY),
    "serialStationClearButton": ("trash-2", P.TEXT_MUTED),
}


def apply_button_icons(owner: ButtonIconHost) -> int:
    """为 owner 上匹配 objectName 的按钮设置图标，返回已装饰的数量。"""

    manager = IconManager()
    applied = 0
    for object_name, (icon_name, color) in _BUTTON_ICON_MAP.items():
        button = _find_child_by_object_name(owner, object_name)
        if button is None:
            continue
        from PyQt6.QtGui import QIcon

        icon = manager.icon(icon_name, color=color)
        if isinstance(icon, QIcon) and not icon.isNull():
            button.setIcon(icon)
            applied += 1
    return applied


def _find_child_by_object_name(owner: ButtonIconHost, object_name: str) -> object | None:
    """按 objectName 在 owner 子树查找控件（兼容 PyQt QWidget.findChild）。"""

    find = getattr(owner, "findChild", None)
    if find is None:
        return None
    from PyQt6.QtWidgets import QPushButton

    return find(QPushButton, object_name)
