"""Profile endpoint helpers shared by TCP and UDP controls."""

from __future__ import annotations

from typing import Protocol


class EndpointTextEdit(Protocol):
    """Minimal text edit surface needed by endpoint profile helpers."""

    def setText(self, text: str) -> None: ...


def apply_endpoint_profile_controls(
    host_edit: EndpointTextEdit,
    port_edit: EndpointTextEdit,
    *,
    transport: dict[str, object],
    port_name: str,
    expected_mode: str,
) -> None:
    if str(transport.get("mode", "")) != expected_mode or ":" not in port_name:
        return
    host, _, port_text = port_name.rpartition(":")
    host_edit.setText(host)
    port_edit.setText(port_text)
