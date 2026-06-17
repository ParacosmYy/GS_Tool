"""Connection result helpers for Serial Station controllers."""

from __future__ import annotations

from embeddebug.serial_station.drivers import SerialPortConfig, SerialTransport
from embeddebug.shared import OperationResult


def open_transport_result(
    transport: SerialTransport,
    config: SerialPortConfig,
    mode: str,
) -> OperationResult[SerialPortConfig]:
    if transport.open(config):
        return OperationResult.success(config)
    return OperationResult.failure(
        "transport_open_failed",
        f"Failed to open {mode} transport",
    )
