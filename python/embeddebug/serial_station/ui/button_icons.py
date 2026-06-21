"""按 objectName 为 Serial Station 按钮装配 lucide 图标。

集中管理按钮→图标的映射，避免在 action 模块或 section builder 中散落
setIcon 调用。在窗口构建完成后调用一次即可。

Batch 7-3：新增 ``apply_focus_rings`` 为所有可聚焦输入控件（QLineEdit/QComboBox/
QPlainTextEdit/QSpinBox）安装 focus_ring 微交互，激活 ``micro_interactions.install_focus_ring``
死代码。诊断报告：focus_ring 定义了但 0 调用方。

图标着色复用 IconManager + palette。连接类主按钮用强调青，
断开用错误红，其余用次文本色，与 QSS 三态保持视觉一致。

约束：本模块只读 owner 上的控件引用，不访问 controller/transport/protocol。
"""

from __future__ import annotations

from typing import Protocol

from embeddebug.serial_station.ui.icons import IconManager
from embeddebug.serial_station.ui.micro_interactions import install_focus_ring
from embeddebug.serial_station.ui.theme import palette as P


class ButtonIconHost(Protocol):
    """持有需装饰图标的按钮引用的宿主（SerialStationMainWindow）。"""

    def __getattr__(self, name: str) -> object: ...


# objectName -> (lucide 图标名, 着色)。颜色与 QSS 按钮文字色对齐。
# apply_button_icons 用 findChild 查找，按钮不存在时静默跳过；icon 不存在时
# IconManager 返回 null QIcon，亦跳过。所以即多写几条 entry 也安全。
_BUTTON_ICON_MAP: dict[str, tuple[str, str]] = {
    # ── 连接类主按钮 ──
    "serialStationConnectButton": ("plug-zap", P.TEXT_ON_ACCENT),
    "serialStationConnectSerialButton": ("cable", P.TEXT_ON_ACCENT),
    "serialStationConnectTcpButton": ("ethernet", P.TEXT_ON_ACCENT),
    "serialStationConnectUdpButton": ("radio", P.TEXT_ON_ACCENT),
    "serialStationDisconnectButton": ("unlink", P.ERROR),
    # ── 工具栏（serial station 通用）──
    "serialStationRefreshPortsButton": ("refresh-cw", P.TEXT_SECONDARY),
    "serialStationSendButton": ("send", P.TERM_TX),
    "serialStationInjectButton": ("download", P.TERM_TX),
    "serialStationExportLogButton": ("save", P.TEXT_SECONDARY),
    "serialStationReplayLogButton": ("play", P.TEXT_SECONDARY),
    "serialStationSaveProfileButton": ("save", P.TEXT_SECONDARY),
    "serialStationLoadProfileButton": ("folder-open", P.TEXT_SECONDARY),
    "serialStationClearButton": ("trash-2", P.TEXT_MUTED),
    # ── Batch 44: domain panel 按钮（UI 审计 §4.1 P0 — 27+ 按钮零图标）──
    # BLE panel
    "serialStationBleScanButton": ("scan", P.TEXT_SECONDARY),
    "serialStationBleConnectButton": ("bluetooth", P.TEXT_ON_ACCENT),
    "serialStationBleReadButton": ("eye", P.TEXT_SECONDARY),
    "serialStationBleWriteButton": ("pencil", P.TEXT_SECONDARY),
    "serialStationBleNotifyButton": ("bell", P.TEXT_SECONDARY),
    # CAN panel
    "serialStationCanSendButton": ("send", P.TERM_TX),
    "serialStationCanClearButton": ("trash-2", P.TEXT_MUTED),
    "serialStationCanDemoButton": ("play", P.TEXT_MUTED),
    # RTT panel
    "serialStationRttStartButton": ("zap", P.TEXT_ON_ACCENT),
    "serialStationRttClearButton": ("trash-2", P.TEXT_MUTED),
    # Automation panel
    "serialStationAutomationRunButton": ("play", P.TEXT_ON_ACCENT),
    "serialStationAutomationFireButton": ("zap", P.WARNING),
    "serialStationAutomationRefreshButton": ("refresh-cw", P.TEXT_SECONDARY),
    # SVD panel
    "serialStationSvdLoadButton": ("folder-open", P.TEXT_SECONDARY),
    "serialStationSvdDemoButton": ("cpu", P.TEXT_MUTED),
    # OTA panel
    "serialStationOtaBrowseButton": ("folder-open", P.TEXT_SECONDARY),
    "serialStationOtaStartButton": ("upload", P.TEXT_ON_ACCENT),
    # Dashboard panel
    "serialStationDashboardAddTabButton": ("plus", P.TEXT_SECONDARY),
    "serialStationDashboardClearButton": ("trash-2", P.TEXT_MUTED),
    "serialStationDashboardSaveButton": ("save", P.TEXT_SECONDARY),
    "serialStationDashboardLoadButton": ("folder-open", P.TEXT_SECONDARY),
    "serialStationDashboardGridButton": ("grid", P.TEXT_SECONDARY),
    # Settings panel
    "serialStationSettingsApplyButton": ("check", P.TEXT_ON_ACCENT),
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


# Batch 7-3: 需要装 focus_ring 的可聚焦输入控件类型。
_FOCUSABLE_WIDGET_TYPES = ("QLineEdit", "QComboBox", "QPlainTextEdit", "QSpinBox", "QDoubleSpinBox")


def apply_focus_rings(owner: ButtonIconHost) -> int:
    """为 owner 子树所有可聚焦输入控件安装 focus_ring（Batch 7-3）。

    遍历 owner 下所有 QLineEdit/QComboBox/QPlainTextEdit/QSpinBox/QDoubleSpinBox，
    调用 ``micro_interactions.install_focus_ring`` 装 focus 光环动画，激活该死代码。
    返回已装饰的控件数。

    在窗口构建完成后调用一次（与 apply_button_icons 同时机）。
    """

    find_children = getattr(owner, "findChildren", None)
    if find_children is None:
        return 0
    applied = 0
    for type_name in _FOCUSABLE_WIDGET_TYPES:
        cls = _resolve_widget_class(type_name)
        if cls is None:
            continue
        for widget in find_children(cls):
            # 跳过只读控件（focus ring 对只读无意义）。
            if _is_readonly(widget):
                continue
            try:
                install_focus_ring(widget)
                applied += 1
            except Exception:
                # 装配失败不阻塞（focus ring 是锦上添花）。
                pass
    return applied


def _resolve_widget_class(type_name: str):
    """惰性解析 QWidget 子类名 → 类对象。"""

    from PyQt6 import QtWidgets

    return getattr(QtWidgets, type_name, None)


def _is_readonly(widget) -> bool:
    """判断控件是否只读（focus ring 对只读控件无意义）。"""

    for attr in ("isReadOnly",):
        check = getattr(widget, attr, None)
        if callable(check):
            try:
                if check():
                    return True
            except Exception:
                pass
    return False
