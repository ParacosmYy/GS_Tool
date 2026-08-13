"""Versioned, UI-safe built-in connection presets.

Presets only describe values that are safe to prefill in the presentation
surface.  They never contain a port handle, a BLE identity, a secret, or an
instruction to connect automatically.
"""

from __future__ import annotations

from collections.abc import Iterable
from dataclasses import dataclass

from ..domain.models import (
    MAX_BLE_DEVICE_NAME_LENGTH,
    MAX_NETWORK_HOST_LENGTH,
    MAX_NETWORK_PORT,
    MAX_TCP_SERVER_CLIENTS,
    MAX_UDP_DATAGRAM_BYTES,
    TransportKind,
    UartFlowControl,
    UartParity,
    UartStopBits,
)

MAX_CONNECTION_PRESET_COUNT = 16
MAX_PRESET_KEY_LENGTH = 64
MAX_PRESET_TEXT_LENGTH = 160


def _is_strict_int(value: object) -> bool:
    """Is strict int."""
    return isinstance(value, int) and not isinstance(value, bool)


def _bounded_text(value: object, label: str, maximum: int, *, allow_empty: bool = False) -> str:
    """Bounded text."""
    if not isinstance(value, str):
        raise ValueError(f"{label} must be text")
    normalized = value.strip()
    if not allow_empty and not normalized:
        raise ValueError(f"{label} must not be empty")
    if len(normalized) > maximum:
        raise ValueError(f"{label} exceeds {maximum} characters")
    return normalized


def _bounded_port(value: object, label: str, *, allow_zero: bool = False) -> int:
    """Bounded port."""
    minimum = 0 if allow_zero else 1
    if (
        isinstance(value, bool)
        or not isinstance(value, int)
        or not minimum <= value <= MAX_NETWORK_PORT
    ):
        raise ValueError(f"{label} must be between {minimum} and {MAX_NETWORK_PORT}")
    return value


UART_BAUD_RATE_PRESETS: tuple[int, ...] = (
    110,
    300,
    600,
    1_200,
    2_400,
    4_800,
    9_600,
    14_400,
    19_200,
    28_800,
    38_400,
    57_600,
    76_800,
    115_200,
    128_000,
    230_400,
    250_000,
    460_800,
    500_000,
    921_600,
    1_000_000,
    1_500_000,
    2_000_000,
    3_000_000,
    4_000_000,
)

DEFAULT_UART_BAUD_RATE = 115_200
DEFAULT_CONNECTION_PRESET_KEY = "uart-115200-8n1"


@dataclass(frozen=True, slots=True)
class UartConnectionPresetValues:
    """Safe UART timing values that can be copied into the form."""

    baud_rate: int
    data_bits: int = 8
    parity: UartParity = UartParity.NONE
    stop_bits: UartStopBits = UartStopBits.ONE
    flow_control: UartFlowControl = UartFlowControl.NONE

    def __post_init__(self) -> None:
        if not _is_strict_int(self.baud_rate) or self.baud_rate not in UART_BAUD_RATE_PRESETS:
            raise ValueError("preset UART baud rate is not a built-in option")
        if not _is_strict_int(self.data_bits) or self.data_bits not in {5, 6, 7, 8}:
            raise ValueError("preset UART data bits are invalid")
        if not isinstance(self.parity, UartParity):
            raise ValueError("preset UART parity is invalid")
        if not isinstance(self.stop_bits, UartStopBits):
            raise ValueError("preset UART stop bits are invalid")
        if not isinstance(self.flow_control, UartFlowControl):
            raise ValueError("preset UART flow control is invalid")


@dataclass(frozen=True, slots=True)
class NetworkConnectionPresetValues:
    """Remote peer values shared by TCP Client and RTT."""

    host: str = "127.0.0.1"
    port: int = 9_000

    def __post_init__(self) -> None:
        object.__setattr__(
            self, "host", _bounded_text(self.host, "network host", MAX_NETWORK_HOST_LENGTH)
        )
        object.__setattr__(self, "port", _bounded_port(self.port, "network port"))


@dataclass(frozen=True, slots=True)
class TcpServerConnectionPresetValues:
    """Loopback-safe TCP Server values."""

    bind_host: str = "127.0.0.1"
    listen_port: int = 9_000
    max_clients: int = 4

    def __post_init__(self) -> None:
        object.__setattr__(
            self,
            "bind_host",
            _bounded_text(self.bind_host, "TCP Server bind host", MAX_NETWORK_HOST_LENGTH),
        )
        object.__setattr__(self, "listen_port", _bounded_port(self.listen_port, "TCP Server port"))
        if (
            not _is_strict_int(self.max_clients)
            or not 1 <= self.max_clients <= MAX_TCP_SERVER_CLIENTS
        ):
            raise ValueError("preset TCP Server client count is invalid")


@dataclass(frozen=True, slots=True)
class UdpConnectionPresetValues:
    """Explicit local bind and remote peer values for UDP unicast."""

    local_host: str = "0.0.0.0"
    local_port: int = 0
    remote_host: str = "127.0.0.1"
    remote_port: int = 9_000
    max_datagram_size: int = 4_096

    def __post_init__(self) -> None:
        object.__setattr__(
            self,
            "local_host",
            _bounded_text(self.local_host, "UDP local host", MAX_NETWORK_HOST_LENGTH),
        )
        object.__setattr__(
            self, "local_port", _bounded_port(self.local_port, "UDP local port", allow_zero=True)
        )
        object.__setattr__(
            self,
            "remote_host",
            _bounded_text(self.remote_host, "UDP remote host", MAX_NETWORK_HOST_LENGTH),
        )
        object.__setattr__(self, "remote_port", _bounded_port(self.remote_port, "UDP remote port"))
        if (
            not _is_strict_int(self.max_datagram_size)
            or not 1 <= self.max_datagram_size <= MAX_UDP_DATAGRAM_BYTES
        ):
            raise ValueError("preset UDP datagram size is invalid")


@dataclass(frozen=True, slots=True)
class RttConnectionPresetValues:
    """RTT Telnet bridge values; no probe or vendor process is started."""

    host: str = "127.0.0.1"
    port: int = 19_021
    channel: int = 0

    def __post_init__(self) -> None:
        object.__setattr__(
            self, "host", _bounded_text(self.host, "RTT host", MAX_NETWORK_HOST_LENGTH)
        )
        object.__setattr__(self, "port", _bounded_port(self.port, "RTT port"))
        if not _is_strict_int(self.channel) or self.channel not in {0, 1}:
            raise ValueError("preset RTT channel is invalid")


@dataclass(frozen=True, slots=True)
class BleConnectionPresetValues:
    """BLE discovery hints without selecting or persisting a device."""

    name_filter: str = ""
    service_filter: str = ""

    def __post_init__(self) -> None:
        object.__setattr__(
            self,
            "name_filter",
            _bounded_text(
                self.name_filter,
                "BLE name filter",
                MAX_BLE_DEVICE_NAME_LENGTH,
                allow_empty=True,
            ),
        )
        object.__setattr__(
            self,
            "service_filter",
            _bounded_text(
                self.service_filter, "BLE service filter", MAX_PRESET_TEXT_LENGTH, allow_empty=True
            ),
        )


type ConnectionPresetValues = (
    UartConnectionPresetValues
    | NetworkConnectionPresetValues
    | TcpServerConnectionPresetValues
    | UdpConnectionPresetValues
    | RttConnectionPresetValues
    | BleConnectionPresetValues
)


@dataclass(frozen=True, slots=True)
class ConnectionPreset:
    """One bounded preset exposed by the connection form."""

    key: str
    label: str
    description: str
    transport: TransportKind
    values: ConnectionPresetValues
    schema_version: int = 1

    def __post_init__(self) -> None:
        if not _is_strict_int(self.schema_version) or self.schema_version != 1:
            raise ValueError("unsupported connection preset schema")
        if not isinstance(self.transport, TransportKind):
            raise ValueError("connection preset transport is invalid")
        object.__setattr__(
            self, "key", _bounded_text(self.key, "preset key", MAX_PRESET_KEY_LENGTH)
        )
        object.__setattr__(
            self, "label", _bounded_text(self.label, "preset label", MAX_PRESET_TEXT_LENGTH)
        )
        object.__setattr__(
            self,
            "description",
            _bounded_text(self.description, "preset description", MAX_PRESET_TEXT_LENGTH),
        )
        expected_type = {
            TransportKind.UART: UartConnectionPresetValues,
            TransportKind.TCP_STREAM: NetworkConnectionPresetValues,
            TransportKind.TCP_SERVER: TcpServerConnectionPresetValues,
            TransportKind.UDP_DATAGRAM: UdpConnectionPresetValues,
            TransportKind.RTT: RttConnectionPresetValues,
            TransportKind.BLE_GATT: BleConnectionPresetValues,
        }[self.transport]
        if not isinstance(self.values, expected_type):
            raise ValueError(f"preset values do not match {self.transport.value}")


@dataclass(frozen=True, slots=True)
class ConnectionPresetCatalog:
    """Versioned bounded catalog with unique stable keys."""

    presets: tuple[ConnectionPreset, ...]
    schema_version: int = 1

    def __post_init__(self) -> None:
        if not _is_strict_int(self.schema_version) or self.schema_version != 1:
            raise ValueError("unsupported connection preset catalog schema")
        if not isinstance(self.presets, tuple):
            raise ValueError("connection preset catalog must be a tuple")
        if not 1 <= len(self.presets) <= MAX_CONNECTION_PRESET_COUNT:
            raise ValueError("connection preset catalog size is out of bounds")
        if any(not isinstance(preset, ConnectionPreset) for preset in self.presets):
            raise ValueError("connection preset catalog contains an invalid preset")
        keys = tuple(preset.key for preset in self.presets)
        if len(set(keys)) != len(keys):
            raise ValueError("connection preset catalog keys must be unique")

    def __iter__(self):
        return iter(self.presets)


_BUILTIN_CONNECTION_PRESETS: tuple[ConnectionPreset, ...] = (
    ConnectionPreset(
        key=DEFAULT_CONNECTION_PRESET_KEY,
        label="UART · 115200 8N1",
        description="常用调试串口；只填入 UART 时序，不自动连接。",
        transport=TransportKind.UART,
        values=UartConnectionPresetValues(baud_rate=115_200),
    ),
    ConnectionPreset(
        key="uart-9600-8e1",
        label="UART · 9600 8E1",
        description="常见 Modbus RTU 时序；协议 framing 仍需在协议页选择。",
        transport=TransportKind.UART,
        values=UartConnectionPresetValues(
            baud_rate=9_600,
            parity=UartParity.EVEN,
        ),
    ),
    ConnectionPreset(
        key="tcp-client-localhost",
        label="TCP Client · 127.0.0.1:9000",
        description="本机 TCP Client 调试端点。",
        transport=TransportKind.TCP_STREAM,
        values=NetworkConnectionPresetValues(),
    ),
    ConnectionPreset(
        key="tcp-server-loopback",
        label="TCP Server · 127.0.0.1:9000",
        description="仅监听回环地址；应用时清除旧 LAN 确认与 allowlist。",
        transport=TransportKind.TCP_SERVER,
        values=TcpServerConnectionPresetValues(),
    ),
    ConnectionPreset(
        key="udp-localhost",
        label="UDP · 127.0.0.1:9000",
        description="固定远端单播；本地端口 0 交给系统分配。",
        transport=TransportKind.UDP_DATAGRAM,
        values=UdpConnectionPresetValues(),
    ),
    ConnectionPreset(
        key="rtt-terminal",
        label="RTT · Terminal · 127.0.0.1:19021",
        description="连接已有 J-Link RTT Telnet 服务，不启动 SEGGER 工具。",
        transport=TransportKind.RTT,
        values=RttConnectionPresetValues(),
    ),
    ConnectionPreset(
        key="ble-gatt-scan",
        label="BLE GATT · 扫描设备",
        description="切换到 BLE GATT；扫描并选择设备后再连接。",
        transport=TransportKind.BLE_GATT,
        values=BleConnectionPresetValues(),
    ),
)

BUILTIN_CONNECTION_PRESET_CATALOG = ConnectionPresetCatalog(_BUILTIN_CONNECTION_PRESETS)
BUILTIN_CONNECTION_PRESETS = BUILTIN_CONNECTION_PRESET_CATALOG.presets
BUILTIN_CONNECTION_PRESET_KEYS = frozenset(
    preset.key for preset in BUILTIN_CONNECTION_PRESET_CATALOG
)
MAX_CUSTOM_CONNECTION_PRESET_COUNT = (
    MAX_CONNECTION_PRESET_COUNT - len(BUILTIN_CONNECTION_PRESET_CATALOG.presets)
)


def custom_connection_presets(
    catalog: ConnectionPresetCatalog,
) -> tuple[ConnectionPreset, ...]:
    """Return only user-owned entries; builtin keys remain immutable."""

    if not isinstance(catalog, ConnectionPresetCatalog):
        raise TypeError("connection preset catalog is invalid")
    return tuple(preset for preset in catalog if preset.key not in BUILTIN_CONNECTION_PRESET_KEYS)


def merge_connection_preset_catalog(
    custom_presets: Iterable[ConnectionPreset],
) -> ConnectionPresetCatalog:
    """Merge validated custom entries after the immutable builtin catalog."""

    custom = tuple(custom_presets)
    if any(not isinstance(preset, ConnectionPreset) for preset in custom):
        raise ValueError("custom connection preset is invalid")
    if any(preset.key in BUILTIN_CONNECTION_PRESET_KEYS for preset in custom):
        raise ValueError("custom connection preset key is reserved")
    return ConnectionPresetCatalog(BUILTIN_CONNECTION_PRESET_CATALOG.presets + custom)


__all__ = [
    "BUILTIN_CONNECTION_PRESETS",
    "BUILTIN_CONNECTION_PRESET_CATALOG",
    "BUILTIN_CONNECTION_PRESET_KEYS",
    "DEFAULT_CONNECTION_PRESET_KEY",
    "DEFAULT_UART_BAUD_RATE",
    "MAX_CONNECTION_PRESET_COUNT",
    "MAX_CUSTOM_CONNECTION_PRESET_COUNT",
    "UART_BAUD_RATE_PRESETS",
    "BleConnectionPresetValues",
    "ConnectionPreset",
    "ConnectionPresetCatalog",
    "ConnectionPresetValues",
    "NetworkConnectionPresetValues",
    "RttConnectionPresetValues",
    "TcpServerConnectionPresetValues",
    "UartConnectionPresetValues",
    "UdpConnectionPresetValues",
    "custom_connection_presets",
    "merge_connection_preset_catalog",
]
