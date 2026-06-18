"""连接配置侧栏 — 把横向 connection_toolbar 纵向重排进卡片。

``build_connection_toolbar`` 出于 MVP 历史返回横向 ``QHBoxLayout``（17 控件一行），
在三栏布局的窄左栏会全部截断。本模块把它的控件提取后按 Port / Serial / Connect /
Endpoints 语义分组纵向排列，加 ``serialStationCardGroupLabel`` 小标题，保证可读。

设计要点：
- 控件只换 layout 归属，parent 与 objectName 不变，findChild 与 action 模块契约不受影响。
- status/profile label 归 TopBar 管理，不提取，避免从 TopBar 被拽进连接卡。

约束：本模块只构建 UI 容器，不访问 controller/transport/protocol/service。
"""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QFrame, QLabel, QLayout, QWidget

from embeddebug.serial_station.ui.collapsible_card import CollapsibleCard
from embeddebug.serial_station.ui.layout_cards import build_card


class ConnectionSidebarHost(Protocol):
    def tr(self, source_text: str) -> str: ...


# 常驻组（高频，不折叠）：端口选择 + 连接动作。Serial/Endpoints/Other 默认折叠。
_ALWAYS_ON_GROUPS = {"Port", "Connect"}


def build_connection_card(
    owner: ConnectionSidebarHost, parent: QWidget, toolbar_layout: QLayout
) -> QFrame:
    """把横向 toolbar 的控件纵向重排进连接卡片，返回卡片本体。

    Port / Connect 组常驻（高频）；Serial / Endpoints / 未识别组默认折叠
    （低频），释放左栏空间。折叠卡内容用 setVisible 显隐，控件不脱离 widget
    树，findChild 与 action 模块契约不受影响。
    """

    card, body = build_card(parent, title=owner.tr("Connection"), icon_name="plug")

    widgets = take_layout_widgets(toolbar_layout)
    groups = group_connection_widgets(widgets)
    for label_text, group_widgets in groups:
        if not group_widgets:
            continue
        if label_text in _ALWAYS_ON_GROUPS:
            # 常驻组：分组标签 + 控件平铺。
            body.addWidget(make_group_label(parent, owner.tr(label_text)))
            for widget in group_widgets:
                body.addWidget(widget)
        else:
            # 低频组：包进可折叠卡（默认折叠）。
            title = owner.tr(label_text) if label_text is not None else owner.tr("Other")
            collapsible = CollapsibleCard(card, title=title, expanded=False)
            inner = collapsible.body_layout()
            for widget in group_widgets:
                inner.addWidget(widget)
            body.addWidget(collapsible)
    return card


def take_layout_widgets(layout: QLayout) -> list[QWidget]:
    """从 layout 提取全部子控件，控件从原 layout 移除。

    status/profile label 归 TopBar 管理（已被 reparent），不提取。
    """

    topbar_owned = {"serialStationStatusLabel", "serialStationProfileLabel"}
    widgets: list[QWidget] = []
    remaining: list = []
    while layout.count():
        item = layout.takeAt(0)
        if item is None:
            continue
        widget = item.widget()
        if widget is None:
            continue
        if widget.objectName() in topbar_owned:
            remaining.append(item)
            continue
        widgets.append(widget)
    for item in remaining:
        layout.addItem(item)
    return widgets


def group_connection_widgets(
    widgets: list[QWidget],
) -> list[tuple[str | None, list[QWidget]]]:
    """按 objectName 把控件分到语义组，未识别的归 None 组。"""

    by_name = {w.objectName(): w for w in widgets}
    pick = lambda names: [by_name[n] for n in names if n in by_name]

    port = pick(("serialStationProtocolCombo", "serialStationPortCombo"))
    port += pick(("serialStationRefreshPortsButton",))
    serial_params = pick((
        "serialStationBaudCombo", "serialStationDataBitsCombo",
        "serialStationParityCombo", "serialStationStopBitsCombo",
        "serialStationFlowControlCombo",
    ))
    actions = pick((
        "serialStationConnectButton", "serialStationConnectSerialButton",
        "serialStationDisconnectButton",
    ))
    endpoints = pick((
        "serialStationConnectTcpButton", "serialStationTcpHostEdit",
        "serialStationTcpPortEdit", "serialStationConnectUdpButton",
        "serialStationUdpHostEdit", "serialStationUdpPortEdit",
    ))

    grouped: set[str] = set()
    for grp in (port, serial_params, actions, endpoints):
        for w in grp:
            grouped.add(w.objectName())
    topbar_owned = {"serialStationStatusLabel", "serialStationProfileLabel"}
    rest = [
        w for w in widgets
        if w.objectName() not in grouped and w.objectName() not in topbar_owned
    ]

    result: list[tuple[str | None, list[QWidget]]] = []
    if port:
        result.append(("Port", port))
    if serial_params:
        result.append(("Serial", serial_params))
    if actions:
        result.append(("Connect", actions))
    if endpoints:
        result.append(("Endpoints", endpoints))
    if rest:
        result.append((None, rest))
    return result


def make_group_label(parent: QWidget, text: str) -> QLabel:
    """连接卡片内的小分组标题（弱文本色，小字号，由 QSS 着色）。"""

    label = QLabel(text, parent)
    label.setObjectName("serialStationCardGroupLabel")
    label.setAlignment(Qt.AlignmentFlag.AlignLeft)
    return label
