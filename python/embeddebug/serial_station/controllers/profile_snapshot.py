"""Profile snapshot builders for Serial Station controllers."""

from __future__ import annotations

from typing import Any

from embeddebug.serial_station.drivers import SerialPortConfig


def build_profile_snapshot(
    name: str,
    mode: str,
    config: SerialPortConfig | None,
    is_connected: bool,
    protocol: str,
    command_history: tuple[str, ...],
) -> dict[str, Any]:
    return {
        "name": name,
        "transport": _transport_snapshot(mode, config, is_connected),
        "protocol": protocol,
        "commandHistory": list(command_history),
    }


def _transport_snapshot(
    mode: str,
    config: SerialPortConfig | None,
    is_connected: bool,
) -> dict[str, object]:
    return {
        "mode": mode,
        "portName": config.port_name if config else "",
        "baudRate": config.baud_rate if config else 0,
        "dataBits": config.data_bits if config else 8,
        "parity": config.parity if config else "none",
        "stopBits": config.stop_bits if config else "1",
        "flowControl": config.flow_control if config else "none",
        "connected": is_connected,
    }
