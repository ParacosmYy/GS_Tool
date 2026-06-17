"""TCP connection helpers for the Serial Station UI."""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtWidgets import QLineEdit, QPushButton, QWidget

from embeddebug.serial_station.ui.endpoint_control_text import endpoint_control_text
from embeddebug.serial_station.ui.endpoint_default_text import apply_default_endpoint_text
from embeddebug.serial_station.ui.endpoint_profile_controls import (
    apply_endpoint_profile_controls,
)


class SerialStationTcpHost(Protocol):
    def tr(self, source_text: str) -> str: ...
    def _connect_tcp(self) -> None: ...


def build_tcp_controls(owner: SerialStationTcpHost, root: QWidget) -> tuple[QLineEdit, QLineEdit, QPushButton]:
    text = endpoint_control_text("tcp")

    host_edit = QLineEdit(root)
    host_edit.setObjectName("serialStationTcpHostEdit")
    host_edit.setPlaceholderText(owner.tr(text.host_placeholder))
    host_edit.setToolTip(owner.tr(text.host_tooltip))

    port_edit = QLineEdit(root)
    port_edit.setObjectName("serialStationTcpPortEdit")
    port_edit.setPlaceholderText(owner.tr(text.port_placeholder))
    port_edit.setToolTip(owner.tr(text.port_tooltip))
    port_edit.returnPressed.connect(owner._connect_tcp)
    apply_default_endpoint_text(host_edit, port_edit)

    connect_button = QPushButton(owner.tr(text.connect_label), root)
    connect_button.setObjectName("serialStationConnectTcpButton")
    connect_button.setToolTip(owner.tr(text.connect_tooltip))
    connect_button.clicked.connect(owner._connect_tcp)

    return host_edit, port_edit, connect_button


def apply_tcp_profile_controls(window: object, transport: dict[str, object], port_name: str) -> None:
    apply_endpoint_profile_controls(
        window._tcp_host_edit,
        window._tcp_port_edit,
        transport=transport,
        port_name=port_name,
        expected_mode="tcp",
    )
