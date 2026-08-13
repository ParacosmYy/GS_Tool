"""Versioned, bounded JSON codec for safe connection preset catalogs."""

from __future__ import annotations

import json
from collections.abc import Mapping

from ..domain.models import (
    TransportKind,
    UartFlowControl,
    UartParity,
    UartStopBits,
)
from .connection_presets import (
    BleConnectionPresetValues,
    ConnectionPreset,
    ConnectionPresetCatalog,
    NetworkConnectionPresetValues,
    RttConnectionPresetValues,
    TcpServerConnectionPresetValues,
    UartConnectionPresetValues,
    UdpConnectionPresetValues,
)

CATALOG_CODEC_SCHEMA_VERSION = 1
MAX_CATALOG_JSON_LENGTH = 65_536


def encode_catalog(catalog: ConnectionPresetCatalog) -> str:
    """Encode only validated DTO fields; no handles, identities, or secrets exist here."""

    if not isinstance(catalog, ConnectionPresetCatalog):
        raise TypeError("connection preset catalog is invalid")
    document = {
        "schema_version": CATALOG_CODEC_SCHEMA_VERSION,
        "presets": [_encode_preset(preset) for preset in catalog.presets],
    }
    return json.dumps(document, ensure_ascii=False, sort_keys=True, separators=(",", ":"))


def decode_catalog(payload: object) -> ConnectionPresetCatalog:
    """Decode one schema version and let the store decide the safe fallback."""

    if isinstance(payload, bytes):
        payload = payload.decode("utf-8")
    if not isinstance(payload, str) or not payload.strip():
        raise ValueError("connection preset catalog payload is empty")
    if len(payload) > MAX_CATALOG_JSON_LENGTH:
        raise ValueError("connection preset catalog payload is too large")
    document = _mapping(json.loads(payload), "catalog")
    _require_keys(document, {"schema_version", "presets"}, "catalog")
    schema_version = document["schema_version"]
    if (
        not isinstance(schema_version, int)
        or isinstance(schema_version, bool)
        or schema_version != CATALOG_CODEC_SCHEMA_VERSION
    ):
        raise ValueError("unsupported connection preset catalog schema")
    raw_presets = document["presets"]
    if not isinstance(raw_presets, list):
        raise ValueError("connection preset catalog presets must be a list")
    return ConnectionPresetCatalog(tuple(_decode_preset(item) for item in raw_presets))


def _encode_preset(preset: ConnectionPreset) -> dict[str, object]:
    """Encode preset."""
    return {
        "key": preset.key,
        "label": preset.label,
        "description": preset.description,
        "transport": preset.transport.value,
        "values": _encode_values(preset.values),
    }


def _encode_values(values: object) -> dict[str, object]:
    """Encode values."""
    if isinstance(values, UartConnectionPresetValues):
        return {
            "baud_rate": values.baud_rate,
            "data_bits": values.data_bits,
            "parity": values.parity.value,
            "stop_bits": values.stop_bits.value,
            "flow_control": values.flow_control.value,
        }
    if isinstance(values, NetworkConnectionPresetValues):
        return {"host": values.host, "port": values.port}
    if isinstance(values, TcpServerConnectionPresetValues):
        return {
            "bind_host": values.bind_host,
            "listen_port": values.listen_port,
            "max_clients": values.max_clients,
        }
    if isinstance(values, UdpConnectionPresetValues):
        return {
            "local_host": values.local_host,
            "local_port": values.local_port,
            "remote_host": values.remote_host,
            "remote_port": values.remote_port,
            "max_datagram_size": values.max_datagram_size,
        }
    if isinstance(values, RttConnectionPresetValues):
        return {"host": values.host, "port": values.port, "channel": values.channel}
    if isinstance(values, BleConnectionPresetValues):
        return {"name_filter": values.name_filter, "service_filter": values.service_filter}
    raise TypeError("unsupported connection preset value type")


def _decode_preset(raw: object) -> ConnectionPreset:
    """Decode preset."""
    record = _mapping(raw, "preset")
    _require_keys(record, {"key", "label", "description", "transport", "values"}, "preset")
    try:
        transport = TransportKind(record["transport"])
        values = _decode_values(transport, record["values"])
        return ConnectionPreset(
            key=record["key"],
            label=record["label"],
            description=record["description"],
            transport=transport,
            values=values,
        )
    except (TypeError, ValueError) as exc:
        raise ValueError("connection preset record is invalid") from exc


def _decode_values(transport: TransportKind, raw: object) -> object:
    """Decode values."""
    values = _mapping(raw, "preset values")
    if transport is TransportKind.UART:
        _require_keys(
            values,
            {"baud_rate", "data_bits", "parity", "stop_bits", "flow_control"},
            "UART values",
        )
        return UartConnectionPresetValues(
            baud_rate=values["baud_rate"],
            data_bits=values["data_bits"],
            parity=UartParity(values["parity"]),
            stop_bits=UartStopBits(values["stop_bits"]),
            flow_control=UartFlowControl(values["flow_control"]),
        )
    if transport is TransportKind.TCP_STREAM:
        _require_keys(values, {"host", "port"}, "TCP values")
        return NetworkConnectionPresetValues(host=values["host"], port=values["port"])
    if transport is TransportKind.TCP_SERVER:
        _require_keys(values, {"bind_host", "listen_port", "max_clients"}, "TCP Server values")
        return TcpServerConnectionPresetValues(
            bind_host=values["bind_host"],
            listen_port=values["listen_port"],
            max_clients=values["max_clients"],
        )
    if transport is TransportKind.UDP_DATAGRAM:
        _require_keys(
            values,
            {"local_host", "local_port", "remote_host", "remote_port", "max_datagram_size"},
            "UDP values",
        )
        return UdpConnectionPresetValues(
            local_host=values["local_host"],
            local_port=values["local_port"],
            remote_host=values["remote_host"],
            remote_port=values["remote_port"],
            max_datagram_size=values["max_datagram_size"],
        )
    if transport is TransportKind.RTT:
        _require_keys(values, {"host", "port", "channel"}, "RTT values")
        return RttConnectionPresetValues(
            host=values["host"],
            port=values["port"],
            channel=values["channel"],
        )
    if transport is TransportKind.BLE_GATT:
        _require_keys(values, {"name_filter", "service_filter"}, "BLE values")
        return BleConnectionPresetValues(
            name_filter=values["name_filter"],
            service_filter=values["service_filter"],
        )
    raise ValueError("unsupported connection preset transport")


def _mapping(value: object, label: str) -> dict[str, object]:
    """Mapping."""
    if not isinstance(value, Mapping) or any(not isinstance(key, str) for key in value):
        raise ValueError(f"{label} must be an object")
    return dict(value)


def _require_keys(value: Mapping[str, object], expected: set[str], label: str) -> None:
    """Require keys."""
    if set(value) != expected:
        raise ValueError(f"{label} fields are invalid")


__all__ = [
    "CATALOG_CODEC_SCHEMA_VERSION",
    "MAX_CATALOG_JSON_LENGTH",
    "decode_catalog",
    "encode_catalog",
]
