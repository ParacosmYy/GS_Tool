from __future__ import annotations

from embeddebug.serial_station.ui.endpoint_default_text import (
    apply_default_endpoint_text,
)


class TextEdit:
    def __init__(self) -> None:
        self.value = ""

    def setText(self, text: str) -> None:
        self.value = text


def test_apply_default_endpoint_text_sets_shared_host_and_port_defaults():
    host_edit = TextEdit()
    port_edit = TextEdit()

    apply_default_endpoint_text(host_edit, port_edit)

    assert host_edit.value == "127.0.0.1"
    assert port_edit.value == "19000"
