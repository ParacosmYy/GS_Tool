"""TCP connection helpers for the Serial Station UI."""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtWidgets import QLineEdit, QPushButton, QWidget


class SerialStationTcpHost(Protocol):
    def tr(self, source_text: str) -> str: ...
    def _connect_tcp(self) -> None: ...


def build_tcp_controls(owner: SerialStationTcpHost, root: QWidget) -> tuple[QLineEdit, QLineEdit, QPushButton]:
    host_edit = QLineEdit(root)
    host_edit.setObjectName("serialStationTcpHostEdit")
    host_edit.setPlaceholderText(owner.tr("TCP host"))
    host_edit.setToolTip(owner.tr("TCP host name or address"))
    host_edit.setText("127.0.0.1")

    port_edit = QLineEdit(root)
    port_edit.setObjectName("serialStationTcpPortEdit")
    port_edit.setPlaceholderText(owner.tr("TCP port"))
    port_edit.setToolTip(owner.tr("TCP port number"))
    port_edit.setText("19000")
    port_edit.returnPressed.connect(owner._connect_tcp)

    connect_button = QPushButton(owner.tr("Connect TCP"), root)
    connect_button.setObjectName("serialStationConnectTcpButton")
    connect_button.setToolTip(owner.tr("Open a TCP client connection"))
    connect_button.clicked.connect(owner._connect_tcp)

    return host_edit, port_edit, connect_button


def apply_tcp_profile_controls(window: object, transport: dict[str, object], port_name: str) -> None:
    if str(transport.get("mode", "")) != "tcp" or ":" not in port_name:
        return
    host, _, port_text = port_name.rpartition(":")
    window._tcp_host_edit.setText(host)
    window._tcp_port_edit.setText(port_text)
