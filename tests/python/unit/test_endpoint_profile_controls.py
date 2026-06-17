from __future__ import annotations

from embeddebug.serial_station.ui.endpoint_profile_controls import (
    apply_endpoint_profile_controls,
)


class TextEdit:
    def __init__(self) -> None:
        self.value = ""

    def setText(self, text: str) -> None:
        self.value = text


def test_apply_endpoint_profile_controls_sets_host_and_port_for_matching_mode():
    host_edit = TextEdit()
    port_edit = TextEdit()

    apply_endpoint_profile_controls(
        host_edit,
        port_edit,
        transport={"mode": "tcp"},
        port_name="127.0.0.1:19000",
        expected_mode="tcp",
    )

    assert host_edit.value == "127.0.0.1"
    assert port_edit.value == "19000"


def test_apply_endpoint_profile_controls_ignores_non_matching_or_invalid_endpoint():
    host_edit = TextEdit()
    port_edit = TextEdit()

    apply_endpoint_profile_controls(
        host_edit,
        port_edit,
        transport={"mode": "udp"},
        port_name="127.0.0.1:19000",
        expected_mode="tcp",
    )
    apply_endpoint_profile_controls(
        host_edit,
        port_edit,
        transport={"mode": "tcp"},
        port_name="COM1",
        expected_mode="tcp",
    )

    assert host_edit.value == ""
    assert port_edit.value == ""
