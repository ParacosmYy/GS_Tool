"""UDP connection helpers for the Serial Station UI."""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtWidgets import QLineEdit, QPushButton, QWidget

from embeddebug.serial_station.ui.endpoint_profile_controls import (
    apply_endpoint_profile_controls,
)


class SerialStationUdpHost(Protocol):
    def tr(self, source_text: str) -> str: ...
    def _connect_udp(self) -> None: ...


def build_udp_controls(owner: SerialStationUdpHost, root: QWidget) -> tuple[QLineEdit, QLineEdit, QPushButton]:
    host_edit = QLineEdit(root)
    host_edit.setObjectName("serialStationUdpHostEdit")
    host_edit.setPlaceholderText(owner.tr("UDP host"))
    host_edit.setToolTip(owner.tr("UDP remote host name or address"))
    host_edit.setText("127.0.0.1")

    port_edit = QLineEdit(root)
    port_edit.setObjectName("serialStationUdpPortEdit")
    port_edit.setPlaceholderText(owner.tr("UDP port"))
    port_edit.setToolTip(owner.tr("UDP remote port number"))
    port_edit.setText("19000")
    port_edit.returnPressed.connect(owner._connect_udp)

    connect_button = QPushButton(owner.tr("Connect UDP"), root)
    connect_button.setObjectName("serialStationConnectUdpButton")
    connect_button.setToolTip(owner.tr("Open a UDP datagram connection"))
    connect_button.clicked.connect(owner._connect_udp)

    return host_edit, port_edit, connect_button


def apply_udp_profile_controls(window: object, transport: dict[str, object], port_name: str) -> None:
    apply_endpoint_profile_controls(
        window._udp_host_edit,
        window._udp_port_edit,
        transport=transport,
        port_name=port_name,
        expected_mode="udp",
    )
