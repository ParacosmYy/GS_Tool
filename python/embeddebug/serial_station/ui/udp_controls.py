"""UDP connection helpers for the Serial Station UI."""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtWidgets import QLineEdit, QPushButton, QWidget

from embeddebug.serial_station.ui.endpoint_control_text import endpoint_control_text
from embeddebug.serial_station.ui.endpoint_default_text import apply_default_endpoint_text
from embeddebug.serial_station.ui.endpoint_profile_controls import (
    apply_endpoint_profile_controls,
)


class SerialStationUdpHost(Protocol):
    def tr(self, source_text: str) -> str: ...
    def _connect_udp(self) -> None: ...


def build_udp_controls(owner: SerialStationUdpHost, root: QWidget) -> tuple[QLineEdit, QLineEdit, QPushButton]:
    text = endpoint_control_text("udp")

    host_edit = QLineEdit(root)
    host_edit.setObjectName("serialStationUdpHostEdit")
    host_edit.setPlaceholderText(owner.tr(text.host_placeholder))
    host_edit.setToolTip(owner.tr(text.host_tooltip))

    port_edit = QLineEdit(root)
    port_edit.setObjectName("serialStationUdpPortEdit")
    port_edit.setPlaceholderText(owner.tr(text.port_placeholder))
    port_edit.setToolTip(owner.tr(text.port_tooltip))
    port_edit.returnPressed.connect(owner._connect_udp)
    apply_default_endpoint_text(host_edit, port_edit)

    connect_button = QPushButton(owner.tr(text.connect_label), root)
    connect_button.setObjectName("serialStationConnectUdpButton")
    connect_button.setToolTip(owner.tr(text.connect_tooltip))
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
