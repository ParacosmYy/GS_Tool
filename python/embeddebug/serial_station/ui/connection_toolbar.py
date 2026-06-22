"""Connection toolbar builder for the Serial Station main window."""

from __future__ import annotations

import logging

from typing import Protocol

from PyQt6.QtWidgets import QComboBox, QHBoxLayout, QPushButton, QWidget

from embeddebug.serial_station.controllers import SerialWorkbenchController
from embeddebug.serial_station.ui import connection_actions
from embeddebug.serial_station.ui.serial_config_options import apply_serial_config_options
from embeddebug.serial_station.ui.tcp_controls import build_tcp_controls
from embeddebug.serial_station.ui.udp_controls import build_udp_controls

_log = logging.getLogger(__name__)


def _install_scale_press(button) -> None:
    """Batch 15: 给连接/断开按钮接入 scale 弹性反馈（按下陷下、松手回弹）。

    安全吞异常（按钮构造期失败不阻断工具栏构建），微交互是锦上添花。
    """

    try:
        from embeddebug.serial_station.ui.micro_interactions import install_scale_press

        install_scale_press(button)
    except Exception:
        _log.warning("scale press install failed", exc_info=True)


class ConnectionToolbarHost(Protocol):
    def tr(self, source_text: str) -> str: ...
    def _set_protocol(self, name: str) -> None: ...
    def _refresh_serial_ports(self) -> None: ...
    def _has_serial_ports(self) -> bool: ...
    def _connect_fake(self) -> None: ...
    def _connect_serial(self) -> None: ...
    def _connect_tcp(self) -> None: ...
    def _connect_udp(self) -> None: ...
    def _disconnect(self) -> None: ...


def build_connection_toolbar(
    owner: ConnectionToolbarHost,
    controller: SerialWorkbenchController,
    root: QWidget,
) -> QHBoxLayout:
    toolbar = QHBoxLayout()
    toolbar.setSpacing(8)

    owner._protocol_combo = QComboBox(root)
    owner._protocol_combo.setObjectName("serialStationProtocolCombo")
    owner._protocol_combo.setToolTip(owner.tr("Select the active serial protocol"))
    owner._protocol_combo.addItems(controller.available_protocols())
    owner._protocol_combo.setCurrentText("raw_data")
    owner._protocol_combo.currentTextChanged.connect(owner._set_protocol)

    owner._port_combo = QComboBox(root)
    owner._port_combo.setObjectName("serialStationPortCombo")
    owner._port_combo.setToolTip(owner.tr("Select a serial port"))
    connection_actions.populate_serial_port_combo(owner)

    owner._refresh_ports_button = QPushButton(owner.tr("Refresh Ports"), root)
    owner._refresh_ports_button.setObjectName("serialStationRefreshPortsButton")
    owner._refresh_ports_button.setToolTip(owner.tr("Refresh available serial ports (Ctrl+R)"))
    def _refresh_with_loading(_checked: bool = False) -> None:
        # Batch 49-5b: 复用 connection_actions._set_loading（消除重复逻辑）。
        # 原 inline 实现 swap 文字「…」+ disable，现统一走 ProgressRing 加载态。
        connection_actions._set_loading(owner._refresh_ports_button, True)
        owner._refresh_serial_ports()
        connection_actions._set_loading(owner._refresh_ports_button, False)
    owner._refresh_ports_button.clicked.connect(_refresh_with_loading)
    _install_rich_tooltips(owner)

    owner._baud_combo = _serial_config_combo(root, "serialStationBaudCombo", owner.tr("Select baud rate"), "baud")
    owner._data_bits_combo = _serial_config_combo(
        root, "serialStationDataBitsCombo", owner.tr("Select data bits"), "data_bits"
    )
    owner._parity_combo = _serial_config_combo(root, "serialStationParityCombo", owner.tr("Select parity"), "parity")
    owner._stop_bits_combo = _serial_config_combo(
        root, "serialStationStopBitsCombo", owner.tr("Select stop bits"), "stop_bits"
    )
    owner._flow_control_combo = _serial_config_combo(
        root, "serialStationFlowControlCombo", owner.tr("Select flow control"), "flow_control"
    )

    owner._connect_button = QPushButton(owner.tr("Connect Fake"), root)
    owner._connect_button.setObjectName("serialStationConnectButton")
    owner._connect_button.setToolTip(owner.tr("Open the fake loopback transport"))
    owner._connect_button.clicked.connect(owner._connect_fake)
    _install_scale_press(owner._connect_button)

    owner._connect_serial_button = QPushButton(owner.tr("Connect Serial"), root)
    owner._connect_serial_button.setObjectName("serialStationConnectSerialButton")
    owner._connect_serial_button.setToolTip(owner.tr("Open the selected serial port"))
    owner._connect_serial_button.setEnabled(owner._has_serial_ports())
    owner._connect_serial_button.clicked.connect(owner._connect_serial)
    _install_scale_press(owner._connect_serial_button)

    owner._tcp_host_edit, owner._tcp_port_edit, owner._connect_tcp_button = build_tcp_controls(owner, root)
    owner._udp_host_edit, owner._udp_port_edit, owner._connect_udp_button = build_udp_controls(owner, root)
    _install_scale_press(owner._connect_tcp_button)
    _install_scale_press(owner._connect_udp_button)

    owner._disconnect_button = QPushButton(owner.tr("Disconnect"), root)
    owner._disconnect_button.setObjectName("serialStationDisconnectButton")
    owner._disconnect_button.setToolTip(owner.tr("Close the active transport"))
    owner._disconnect_button.setEnabled(False)
    owner._disconnect_button.clicked.connect(owner._disconnect)
    _install_scale_press(owner._disconnect_button)

    for widget in (
        owner._protocol_combo,
        owner._port_combo,
        owner._refresh_ports_button,
        owner._baud_combo,
        owner._data_bits_combo,
        owner._parity_combo,
        owner._stop_bits_combo,
        owner._flow_control_combo,
        owner._connect_button,
        owner._connect_serial_button,
        owner._tcp_host_edit,
        owner._tcp_port_edit,
        owner._connect_tcp_button,
        owner._udp_host_edit,
        owner._udp_port_edit,
        owner._connect_udp_button,
        owner._disconnect_button,
    ):
        toolbar.addWidget(widget)
    toolbar.addStretch(1)
    toolbar.addWidget(owner._profile_label)
    toolbar.addWidget(owner._status_label)
    return toolbar


def _serial_config_combo(root: QWidget, object_name: str, tooltip: str, option_key: str) -> QComboBox:
    combo = QComboBox(root)
    combo.setObjectName(object_name)
    combo.setToolTip(tooltip)
    apply_serial_config_options(combo, option_key)
    return combo


def _install_rich_tooltips(owner) -> None:
    """Batch 47: 为工具栏主按钮挂接 RichTooltip（激活 install_tooltip 死代码）。

    对每个按钮在 setToolTip 基础上叠加富文本 tooltip（标题+正文两行），
    提供更详细的操作指引。失败静默跳过（RichTooltip 是锦上添花）。
    """
    try:
        from embeddebug.serial_station.ui.controls import install_tooltip
        install_tooltip(owner._refresh_ports_button,
                        owner.tr("刷新端口"),
                        owner.tr("重新扫描系统可用的 COM 端口（快捷键 Ctrl+R）"))
        install_tooltip(owner._connect_button,
                        owner.tr("连接（替身）"),
                        owner.tr("打开本地 loopback 替身传输，无需真实硬件即可测试收发"))
        install_tooltip(owner._connect_serial_button,
                        owner.tr("连接串口"),
                        owner.tr("打开选中的 COM 端口，使用上方配置的波特率/数据位/校验/流控参数"))
        install_tooltip(owner._disconnect_button,
                        owner.tr("断开连接"),
                        owner.tr("关闭当前传输连接，停止收发"))
    except Exception:
        _log.warning("scale press install failed", exc_info=True)
