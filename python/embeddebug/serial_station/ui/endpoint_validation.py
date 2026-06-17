"""Endpoint field validation for Serial Station connection controls."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class EndpointValidationResult:
    """Validated host:port fields or a user-facing validation message."""

    ok: bool
    host: str
    port: int
    message: str = ""


def validate_endpoint_fields(host_text: str, port_text: str, label: str) -> EndpointValidationResult:
    host = host_text.strip()
    if not host:
        return EndpointValidationResult(False, "", 0, f"{label} host is empty")
    try:
        port = int(port_text.strip())
    except ValueError:
        return EndpointValidationResult(False, "", 0, f"{label} port is invalid")
    if port < 1 or port > 65535:
        return EndpointValidationResult(False, "", 0, f"{label} port is invalid")
    return EndpointValidationResult(True, host, port)
