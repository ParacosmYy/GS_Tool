"""Serial Station 三区分栏主布局（VOFA+ 风格）。

把原单列堆叠重排为三区 QSplitter：
- 左区：连接配置卡（toolbar + 发送/注入命令行）。
- 中区：波形预览 + 日志视图（主信息区，占最大宽度）。
- 右区：日志操作（过滤/搜索/导出/回放）+ Profile（保存/加载）。

设计要点：
- ``sections.build_main_layout`` 先用字面量调用
  ``build_connection_toolbar(owner, controller, root)`` 与
  ``command_section.build_send_row(owner, root)``（满足架构测试），
  再把返回的 layout 交给本模块的 ``assemble_three_zone`` 装配。
- 卡片用 ``layout_cards.wrap_layout`` 包装现有 layout builder，不改 action 契约。
- 所有控件仍以 ``owner``/``root`` 为 parent，``findChild`` 可达。

约束：本模块只装配布局，不访问 controller/transport/protocol/service 的内部状态。
"""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QFrame, QLabel, QLayout, QSplitter, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.layout_cards import build_card, card_body, wrap_layout
from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview


class LayoutMainHost(Protocol):
    def tr(self, source_text: str) -> str: ...


def assemble_three_zone(
    owner: LayoutMainHost,
    root: QWidget,
    toolbar_layout: QLayout,
    send_row_layout: QLayout,
    inject_row_layout: QLayout,
    log_row_layout: QLayout,
    profile_row_layout: QLayout,
    footer_layout: QLayout,
) -> QSplitter:
    """装配三区分栏布局，返回 QSplitter（供 sections 加入根 layout）。

    调用方（sections）先用字面量 builder 构建各 layout，再传入本函数装配。
    """

    splitter = QSplitter(Qt.Orientation.Horizontal, root)
    splitter.setObjectName("serialStationMainSplitter")
    splitter.setHandleWidth(2)
    splitter.setChildrenCollapsible(False)

    left_zone = _build_left_zone(owner, splitter, toolbar_layout, send_row_layout, inject_row_layout)
    log_card, waveform_card = _build_center_zone(owner, splitter)
    right_zone = _build_right_zone(owner, splitter, log_row_layout, profile_row_layout, footer_layout)

    # 三区最小宽度：防止窄栏把横向 toolbar 控件挤压到字符截断。
    # 左栏需容纳连接配置（多 combo），中栏是主信息区，右栏是日志工具。
    left_zone.setMinimumWidth(320)
    splitter.widget(1).setMinimumWidth(360)
    right_zone.setMinimumWidth(260)

    # 中区占主导：左 32% / 中 44% / 右 24%（左栏加宽以容纳连接配置）。
    splitter.setStretchFactor(0, 32)
    splitter.setStretchFactor(1, 44)
    splitter.setStretchFactor(2, 24)

    # 把中区卡片挂到 owner，供 sections 注入 log_view/log_stats_label。
    owner._center_log_card = log_card
    owner._center_waveform_card = waveform_card
    return splitter


def _build_left_zone(
    owner: LayoutMainHost,
    splitter: QSplitter,
    toolbar_layout: QLayout,
    send_row_layout: QLayout,
    inject_row_layout: QLayout,
) -> QWidget:
    """左区：连接配置卡（toolbar 纵向重排）+ 命令卡（发送/注入）。"""

    zone = QWidget(splitter)
    zone.setObjectName("serialStationLeftZone")
    zone_layout = QVBoxLayout(zone)
    zone_layout.setContentsMargins(0, 0, 0, 0)
    zone_layout.setSpacing(10)

    # connection_toolbar 默认返回横向 QHBoxLayout（17 控件一行），
    # 在窄左栏会全部截断；这里把它的控件提取出来纵向重排，并按
    # 语义分组（端口 / 串口参数 / 连接动作 / 端点），保证可读。
    connection_card = _build_connection_card_vertical(owner, zone, toolbar_layout)
    zone_layout.addWidget(connection_card)

    command_card = wrap_layout(zone, send_row_layout)
    command_body = card_body(command_card)
    command_body.addLayout(inject_row_layout)
    zone_layout.addWidget(command_card)

    zone_layout.addStretch(1)
    return zone


def _build_connection_card_vertical(
    owner: LayoutMainHost, parent: QWidget, toolbar_layout: QLayout
) -> QFrame:
    """把横向 toolbar 的控件纵向重排进连接卡片。

    toolbar_layout 由 ``build_connection_toolbar`` 构建为横向一行；这里
    抽取其全部子控件（保留 objectName 与已连接的信号），按端口/串口参数/
    连接动作/端点分组纵向排列，避免窄栏字符截断。控件 parent 不变，
    findChild 与 action 模块契约不受影响。
    """

    card, body = build_card(parent, title=owner.tr("Connection"), icon_name="plug")

    # 提取 toolbar 全部子项（控件 + stretch）。
    widgets = _take_layout_widgets(toolbar_layout)

    # owner 上的控件引用分组（顺序与 build_connection_toolbar 一致）。
    groups = _group_connection_widgets(owner, widgets)
    for label_text, group_widgets in groups:
        if not group_widgets:
            continue
        if label_text is not None:
            body.addWidget(_make_group_label(parent, owner.tr(label_text)))
        for widget in group_widgets:
            body.addWidget(widget)
    return card


def _take_layout_widgets(layout: QLayout) -> list[QWidget]:
    """从 layout 提取全部子控件（跳过 stretch / spacer），控件从原 layout 移除。

    status/profile label 归 TopBar 管理（已被 reparent），不提取，
    避免从 TopBar 被拽进连接卡。
    """

    topbar_owned = {"serialStationStatusLabel", "serialStationProfileLabel"}
    widgets: list[QWidget] = []
    # 先收集要保留的 item（label），只提取其余控件。
    remaining: list = []
    while layout.count():
        item = layout.takeAt(0)
        if item is None:
            continue
        widget = item.widget()
        if widget is None:
            continue
        if widget.objectName() in topbar_owned:
            # 保留在原位（实际由 TopBar 管理），不提取。
            remaining.append(item)
            continue
        widgets.append(widget)
    # 把保留的 label item 放回 toolbar（它们实际由 TopBar parent）。
    for item in remaining:
        layout.addItem(item)
    return widgets


def _group_connection_widgets(
    owner: LayoutMainHost, widgets: list[QWidget]
) -> list[tuple[str | None, list[QWidget]]]:
    """按 objectName 把控件分到语义组，未识别的归到 None 组（保留显示）。"""

    by_name = {w.objectName(): w for w in widgets}
    get = lambda name: [by_name[name]] if name in by_name else []

    # 端口选择：protocol + port + refresh。
    port = []
    for n in ("serialStationProtocolCombo", "serialStationPortCombo"):
        port.extend(get(n))
    port.extend(get("serialStationRefreshPortsButton"))

    # 串口参数：baud / data bits / parity / stop bits / flow control。
    serial_params = []
    for n in (
        "serialStationBaudCombo",
        "serialStationDataBitsCombo",
        "serialStationParityCombo",
        "serialStationStopBitsCombo",
        "serialStationFlowControlCombo",
    ):
        serial_params.extend(get(n))

    # 连接动作：fake / serial / disconnect。
    actions = []
    for n in ("serialStationConnectButton", "serialStationConnectSerialButton", "serialStationDisconnectButton"):
        actions.extend(get(n))

    # 端点：TCP / UDP。
    endpoints = []
    for n in (
        "serialStationConnectTcpButton",
        "serialStationTcpHostEdit",
        "serialStationTcpPortEdit",
        "serialStationConnectUdpButton",
        "serialStationUdpHostEdit",
        "serialStationUdpPortEdit",
    ):
        endpoints.extend(get(n))

    # 已分组的 objectName 集合，剩余归 None 组。
    grouped = set()
    for grp in (port, serial_params, actions, endpoints):
        for w in grp:
            grouped.add(w.objectName())
    # status/profile label 归 TopBar，不进连接卡（避免从 TopBar 被拽走）。
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


def _make_group_label(parent: QWidget, text: str) -> QLabel:
    """连接卡片内的小分组标题（弱文本色，小字号）。"""

    label = QLabel(text, parent)
    label.setObjectName("serialStationCardGroupLabel")
    label.setAlignment(Qt.AlignmentFlag.AlignLeft)
    return label


def _build_center_zone(owner: LayoutMainHost, splitter: QSplitter) -> tuple[QWidget, QWidget]:
    """中区：波形预览 + 日志视图（主信息区）。

    返回 (log_card, waveform_card)，波形卡内已嵌入 waveform_preview。
    """

    zone = QWidget(splitter)
    zone.setObjectName("serialStationCenterZone")
    zone_layout = QVBoxLayout(zone)
    zone_layout.setContentsMargins(0, 0, 0, 0)
    zone_layout.setSpacing(10)

    waveform_card, wave_body = build_card(
        zone, title=owner.tr("Waveform"), icon_name="activity"
    )
    owner._waveform_preview = SerialWaveformPreview(waveform_card)
    wave_body.addWidget(owner._waveform_preview, 1)
    zone_layout.addWidget(waveform_card, 1)

    log_card, log_body = build_card(zone, title=owner.tr("Log"), icon_name="terminal")
    zone_layout.addWidget(log_card, 1)
    return log_card, waveform_card


def _build_right_zone(
    owner: LayoutMainHost,
    splitter: QSplitter,
    log_row_layout: QLayout,
    profile_row_layout: QLayout,
    footer_layout: QLayout,
) -> QWidget:
    """右区：日志操作卡 + Profile 卡 + 底部 Clear。"""

    zone = QWidget(splitter)
    zone.setObjectName("serialStationRightZone")
    zone_layout = QVBoxLayout(zone)
    zone_layout.setContentsMargins(0, 0, 0, 0)
    zone_layout.setSpacing(10)

    log_ops_card, log_ops_body = build_card(
        zone, title=owner.tr("Log Tools"), icon_name="filter"
    )
    log_ops_body.addLayout(log_row_layout)
    zone_layout.addWidget(log_ops_card)

    profile_card, profile_body = build_card(
        zone, title=owner.tr("Profile"), icon_name="save"
    )
    profile_body.addLayout(profile_row_layout)
    zone_layout.addWidget(profile_card)

    zone_layout.addLayout(footer_layout)
    return zone
