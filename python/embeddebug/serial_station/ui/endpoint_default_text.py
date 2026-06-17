"""Default endpoint text helpers for Serial Station widgets."""

from __future__ import annotations

from typing import Protocol


DEFAULT_ENDPOINT_HOST = "127.0.0.1"
DEFAULT_ENDPOINT_PORT = "19000"


class EndpointDefaultTextEdit(Protocol):
    """Minimal text edit surface needed by endpoint default helpers."""

    def setText(self, text: str) -> None: ...


def apply_default_endpoint_text(
    host_edit: EndpointDefaultTextEdit,
    port_edit: EndpointDefaultTextEdit,
) -> None:
    host_edit.setText(DEFAULT_ENDPOINT_HOST)
    port_edit.setText(DEFAULT_ENDPOINT_PORT)
