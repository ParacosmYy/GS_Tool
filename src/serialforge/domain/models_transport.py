"""Immutable, transport-neutral values used across layer boundaries."""

from __future__ import annotations

import ipaddress
import math
from dataclasses import dataclass
from enum import StrEnum
from uuid import UUID

from .errors import ConfigurationError


class TransportKind(StrEnum):
    """Transport families retain their distinct wire semantics."""

    UART = "uart"
    TCP_STREAM = "tcp_stream"
    TCP_SERVER = "tcp_server"
    UDP_DATAGRAM = "udp_datagram"
    BLE_GATT = "ble_gatt"
    RTT = "rtt"


class UartParity(StrEnum):
    """UART parity options understood by the adapter."""

    NONE = "none"
    ODD = "odd"
    EVEN = "even"
    MARK = "mark"
    SPACE = "space"


class UartStopBits(StrEnum):
    """UART stop-bit options."""

    ONE = "one"
    ONE_POINT_FIVE = "one_point_five"
    TWO = "two"


class UartFlowControl(StrEnum):
    """UART hardware/software flow-control modes."""

    NONE = "none"
    XON_XOFF = "xon_xoff"
    RTS_CTS = "rts_cts"
    DSR_DTR = "dsr_dtr"


class SessionState(StrEnum):
    """Session lifecycle states emitted to observers."""

    DISCOVERED = "discovered"
    OPENING = "opening"
    OPEN = "open"
    CLOSING = "closing"
    CLOSED = "closed"
    ERROR = "error"


type SessionId = UUID
type PeerId = UUID


MAX_STREAM_PAYLOAD_BYTES = 1_048_576
MAX_RAW_RECORD_BYTES = 1_048_576
MAX_RECORD_FILE_BYTES = 64 * 1024 * 1024
MAX_RECORD_QUEUE_BYTES = 16 * 1024 * 1024
MAX_COMMAND_PAYLOAD_BYTES = 64 * 1024
MAX_COMMAND_NAME_LENGTH = 128
MAX_RECORD_PATH_LENGTH = 4_096
MAX_COMMAND_HISTORY_ITEMS = 100
MAX_QUICK_COMMAND_ITEMS = 64
MAX_ENDPOINT_IDENTITIES = 64
MAX_PREVIEW_BYTES = 256 * 1024
MAX_UDP_DATAGRAM_BYTES = 65_507
DEFAULT_UDP_DATAGRAM_BYTES = 4_096
MAX_NETWORK_PORT = 65_535
MAX_NETWORK_TIMEOUT = 60.0
MAX_NETWORK_HOST_LENGTH = 255
DEFAULT_RTT_TELNET_PORT = 19_021
MAX_SERVER_ALLOWLIST_ENTRIES = 64
MAX_SERVER_ALLOWLIST_ITEM_LENGTH = 64
MAX_TCP_SERVER_CLIENTS = 16
MAX_BLE_DEVICE_ID_LENGTH = 255
MAX_BLE_DEVICE_NAME_LENGTH = 255
MAX_BLE_UUID_LENGTH = 64
MAX_BLE_SERVICES = 256
MAX_BLE_CHARACTERISTICS = 1_024
MAX_BLE_WRITE_BYTES = 4_096
MAX_BLE_WRITE_WITH_RESPONSE_BYTES = 512


@dataclass(frozen=True, slots=True)
class Endpoint:
    """A user-visible endpoint identity without adapter-specific handles."""

    transport: TransportKind
    address: str
    label: str
    identity: str | None = None
    metadata: tuple[tuple[str, str], ...] = ()

    def __post_init__(self) -> None:
        if not self.address.strip():
            raise ConfigurationError("端点地址不能为空。")
        if not self.label.strip():
            raise ConfigurationError("端点名称不能为空。")
        identity = self.identity.strip() if self.identity else ""
        if not identity:
            identity = f"{self.transport.value}:address:{self.address.strip()}"
        normalized_metadata = tuple(
            sorted((str(key).strip(), str(value).strip()) for key, value in self.metadata)
        )
        if any(not key or not value for key, value in normalized_metadata):
            raise ConfigurationError("端点元数据的键和值不能为空。")
        object.__setattr__(self, "identity", identity)
        object.__setattr__(self, "metadata", normalized_metadata)


@dataclass(frozen=True, slots=True)
class PeerAddress:
    """A validated host/port pair that carries network peer identity."""

    host: str
    port: int

    def __post_init__(self) -> None:
        host = self.host.strip()
        if not host:
            raise ConfigurationError("网络 peer 主机不能为空。")
        if len(host) > MAX_NETWORK_HOST_LENGTH:
            raise ConfigurationError("网络 peer 主机名超过长度上限。")
        if isinstance(self.port, bool) or not 1 <= self.port <= MAX_NETWORK_PORT:
            raise ConfigurationError("网络 peer 端口必须在 1 到 65535 之间。")
        object.__setattr__(self, "host", host)

    @property
    def display(self) -> str:
        """Return a compact user-visible peer label."""

        return f"{self.host}:{self.port}"


def _validate_network_timeout(value: float, label: str) -> None:
    if (
        isinstance(value, bool)
        or not isinstance(value, (int, float))
        or not math.isfinite(value)
        or not 0 < value <= MAX_NETWORK_TIMEOUT
    ):
        raise ConfigurationError(f"{label}必须在 (0, {MAX_NETWORK_TIMEOUT}] 秒内。")


@dataclass(frozen=True, slots=True)
class UartTransportConfig:
    """Versioned UART settings; no pyserial objects cross this boundary."""

    port: str
    baud_rate: int = 115_200
    data_bits: int = 8
    parity: UartParity = UartParity.NONE
    stop_bits: UartStopBits = UartStopBits.ONE
    flow_control: UartFlowControl = UartFlowControl.NONE
    read_timeout: float | None = 0.2
    write_timeout: float | None = 1.0
    inter_byte_timeout: float | None = None
    exclusive: bool | None = None
    dtr: bool = True
    rts: bool = True
    read_chunk_size: int = 4_096
    endpoint_identity: str | None = None
    schema_version: int = 1

    def __post_init__(self) -> None:
        port = self.port.strip()
        if not port:
            raise ConfigurationError("UART 端口不能为空。")
        object.__setattr__(self, "port", port)
        if self.baud_rate <= 0:
            raise ConfigurationError("UART 波特率必须为正数。")
        if self.data_bits not in {5, 6, 7, 8}:
            raise ConfigurationError("UART 数据位必须是 5、6、7 或 8。")
        if not isinstance(self.parity, UartParity):
            raise ConfigurationError("UART parity 必须使用 UartParity。")
        if not isinstance(self.stop_bits, UartStopBits):
            raise ConfigurationError("UART stop bits 必须使用 UartStopBits。")
        if not isinstance(self.flow_control, UartFlowControl):
            raise ConfigurationError("UART flow control 必须使用 UartFlowControl。")
        self._validate_timeout(self.read_timeout, "UART 读取超时", 2, allow_none=False)
        self._validate_timeout(self.write_timeout, "UART 写入超时", 3, allow_none=False)
        self._validate_timeout(self.inter_byte_timeout, "UART 字节间隔超时", 2)
        if not 0 < self.read_chunk_size <= 65_536:
            raise ConfigurationError("UART 读取块大小必须在 1 到 65536 字节内。")
        if self.endpoint_identity is not None and not self.endpoint_identity.strip():
            raise ConfigurationError("UART 端点身份不能为空字符串。")
        if self.endpoint_identity is not None:
            object.__setattr__(self, "endpoint_identity", self.endpoint_identity.strip())
        if self.schema_version != 1:
            raise ConfigurationError("不支持的 UART 配置 schema 版本。")

    @staticmethod
    def _validate_timeout(
        value: float | None,
        label: str,
        maximum: float,
        *,
        allow_none: bool = True,
    ) -> None:
        if value is None:
            if allow_none:
                return
            raise ConfigurationError(f"{label}必须是有限正数。")
        if not 0 < value <= maximum:
            suffix = "，或设为 None" if allow_none else ""
            raise ConfigurationError(f"{label}必须在 (0, {maximum}] 秒内{suffix}。")

    @property
    def endpoint(self) -> Endpoint:
        """Return the transport-neutral endpoint used in session events."""

        return Endpoint(
            transport=TransportKind.UART,
            address=self.port,
            label=self.port,
            identity=self.endpoint_identity,
        )


@dataclass(frozen=True, slots=True)
class TcpTransportConfig:
    """Versioned TCP client settings with bounded blocking operations."""

    remote_peer: PeerAddress
    connect_timeout: float = 3.0
    read_timeout: float = 0.2
    write_timeout: float = 3.0
    read_chunk_size: int = 4_096
    endpoint_identity: str | None = None
    schema_version: int = 1

    def __post_init__(self) -> None:
        if not isinstance(self.remote_peer, PeerAddress):
            raise ConfigurationError("TCP remote peer 必须使用 PeerAddress。")
        _validate_network_timeout(self.connect_timeout, "TCP 连接超时")
        _validate_network_timeout(self.read_timeout, "TCP 读取超时")
        _validate_network_timeout(self.write_timeout, "TCP 写入超时")
        if not 0 < self.read_chunk_size <= MAX_STREAM_PAYLOAD_BYTES:
            raise ConfigurationError("TCP 读取块大小必须在 1 到 1 MiB 之间。")
        if self.endpoint_identity is not None and not self.endpoint_identity.strip():
            raise ConfigurationError("TCP 端点身份不能为空字符串。")
        if self.endpoint_identity is not None:
            object.__setattr__(self, "endpoint_identity", self.endpoint_identity.strip())
        if self.schema_version != 1:
            raise ConfigurationError("不支持的 TCP 配置 schema 版本。")

    @property
    def endpoint(self) -> Endpoint:
        return Endpoint(
            transport=TransportKind.TCP_STREAM,
            address=self.remote_peer.display,
            label=f"TCP {self.remote_peer.display}",
            identity=self.endpoint_identity,
            metadata=(
                ("remote_host", self.remote_peer.host),
                ("remote_port", str(self.remote_peer.port)),
            ),
        )


@dataclass(frozen=True, slots=True)
class RttTransportConfig:
    """Versioned J-Link RTT Telnet bridge settings.

    This is deliberately a TCP boundary to an already-running J-Link RTT
    Telnet service. It does not own a probe, a debug connection, or a vendor
    DLL. The channel selector is encoded in SEGGER's short connection config
    string before normal byte-stream traffic begins.
    """

    remote_peer: PeerAddress = PeerAddress("127.0.0.1", DEFAULT_RTT_TELNET_PORT)
    channel: int = 0
    connect_timeout: float = 3.0
    read_timeout: float = 0.2
    write_timeout: float = 3.0
    read_chunk_size: int = 4_096
    endpoint_identity: str | None = None
    schema_version: int = 1

    def __post_init__(self) -> None:
        if not isinstance(self.remote_peer, PeerAddress):
            raise ConfigurationError("RTT Telnet peer 必须使用 PeerAddress。")
        if isinstance(self.channel, bool) or self.channel not in {0, 1}:
            raise ConfigurationError("RTT 首版只支持 channel 0 或 1。")
        _validate_network_timeout(self.connect_timeout, "RTT 连接超时")
        _validate_network_timeout(self.read_timeout, "RTT 读取超时")
        _validate_network_timeout(self.write_timeout, "RTT 写入超时")
        if not 0 < self.read_chunk_size <= MAX_STREAM_PAYLOAD_BYTES:
            raise ConfigurationError("RTT 读取块大小必须在 1 到 1 MiB 之间。")
        if self.endpoint_identity is not None and not self.endpoint_identity.strip():
            raise ConfigurationError("RTT 端点身份不能为空字符串。")
        if self.endpoint_identity is not None:
            object.__setattr__(self, "endpoint_identity", self.endpoint_identity.strip())
        if self.schema_version != 1:
            raise ConfigurationError("不支持的 RTT 配置 schema 版本。")

    @property
    def endpoint(self) -> Endpoint:
        return Endpoint(
            transport=TransportKind.RTT,
            address=self.remote_peer.display,
            label=f"J-Link RTT ch{self.channel} · {self.remote_peer.display}",
            identity=self.endpoint_identity or f"rtt:{self.remote_peer.display}:ch{self.channel}",
            metadata=(
                ("bridge", "SEGGER RTT Telnet"),
                ("remote_host", self.remote_peer.host),
                ("remote_port", str(self.remote_peer.port)),
                ("channel", str(self.channel)),
            ),
        )


@dataclass(frozen=True, slots=True)
class UdpTransportConfig:
    """Versioned UDP unicast settings with explicit local bind and peer."""

    local_host: str
    local_port: int
    remote_peer: PeerAddress
    read_timeout: float = 0.2
    write_timeout: float = 3.0
    max_datagram_size: int = DEFAULT_UDP_DATAGRAM_BYTES
    endpoint_identity: str | None = None
    schema_version: int = 1

    def __post_init__(self) -> None:
        local_host = self.local_host.strip()
        if not local_host:
            raise ConfigurationError("UDP 本地绑定主机不能为空。")
        if len(local_host) > MAX_NETWORK_HOST_LENGTH:
            raise ConfigurationError("UDP 本地绑定主机名超过长度上限。")
        if isinstance(self.local_port, bool) or not 0 <= self.local_port <= MAX_NETWORK_PORT:
            raise ConfigurationError("UDP 本地绑定端口必须在 0 到 65535 之间。")
        if not isinstance(self.remote_peer, PeerAddress):
            raise ConfigurationError("UDP remote peer 必须使用 PeerAddress。")
        _validate_network_timeout(self.read_timeout, "UDP 读取超时")
        _validate_network_timeout(self.write_timeout, "UDP 写入超时")
        if not 0 < self.max_datagram_size <= MAX_UDP_DATAGRAM_BYTES:
            raise ConfigurationError(
                f"UDP payload 上限必须在 1 到 {MAX_UDP_DATAGRAM_BYTES} 字节之间。"
            )
        if self.endpoint_identity is not None and not self.endpoint_identity.strip():
            raise ConfigurationError("UDP 端点身份不能为空字符串。")
        if self.endpoint_identity is not None:
            object.__setattr__(self, "endpoint_identity", self.endpoint_identity.strip())
        if self.schema_version != 1:
            raise ConfigurationError("不支持的 UDP 配置 schema 版本。")
        object.__setattr__(self, "local_host", local_host)

    @property
    def endpoint(self) -> Endpoint:
        address = f"{self.local_host}:{self.local_port}->{self.remote_peer.display}"
        return Endpoint(
            transport=TransportKind.UDP_DATAGRAM,
            address=address,
            label=f"UDP {self.remote_peer.display}",
            identity=self.endpoint_identity,
            metadata=(
                ("local_host", self.local_host),
                ("local_port", str(self.local_port)),
                ("remote_host", self.remote_peer.host),
                ("remote_port", str(self.remote_peer.port)),
                ("max_datagram_size", str(self.max_datagram_size)),
            ),
        )


def _normalize_ble_uuid(value: str, label: str) -> str:
    if not isinstance(value, str):
        raise ConfigurationError(f"{label}必须是字符串。")
    normalized = value.strip().lower()
    if not normalized or len(normalized) > MAX_BLE_UUID_LENGTH:
        raise ConfigurationError(f"{label}不能为空且不能超过 {MAX_BLE_UUID_LENGTH} 个字符。")
    return normalized


class BleGattWriteMode(StrEnum):
    """The two GATT characteristic write semantics exposed by the UI."""

    WITH_RESPONSE = "with_response"
    WITHOUT_RESPONSE = "without_response"


@dataclass(frozen=True, slots=True)
class BleGattDevice:
    """UI-safe BLE advertisement snapshot; no Bleak object crosses this boundary."""

    device_id: str
    name: str = ""
    rssi: int | None = None
    service_uuids: tuple[str, ...] = ()

    def __post_init__(self) -> None:
        device_id = self.device_id.strip()
        name = self.name.strip()
        if not device_id or len(device_id) > MAX_BLE_DEVICE_ID_LENGTH:
            raise ConfigurationError("BLE 设备 ID 不能为空且不能超过 255 个字符。")
        if len(name) > MAX_BLE_DEVICE_NAME_LENGTH:
            raise ConfigurationError("BLE 设备名称不能超过 255 个字符。")
        normalized = tuple(
            sorted({_normalize_ble_uuid(value, "BLE service UUID") for value in self.service_uuids})
        )
        object.__setattr__(self, "device_id", device_id)
        object.__setattr__(self, "name", name)
        object.__setattr__(self, "service_uuids", normalized)

    @property
    def display(self) -> str:
        """Return a compact stable device label for a combo box."""

        label = self.name or "未命名 BLE 设备"
        rssi = f" · RSSI {self.rssi} dBm" if self.rssi is not None else ""
        return f"{label} · {self.device_id}{rssi}"


@dataclass(frozen=True, slots=True)
class BleGattDiscoveryConfig:
    """Bounded manual BLE scan filters."""

    scan_timeout: float = 5.0
    name_filter: str = ""
    service_uuids: tuple[str, ...] = ()
    schema_version: int = 1

    def __post_init__(self) -> None:
        _validate_network_timeout(self.scan_timeout, "BLE 扫描超时")
        name_filter = self.name_filter.strip()
        if len(name_filter) > MAX_BLE_DEVICE_NAME_LENGTH:
            raise ConfigurationError("BLE 名称过滤不能超过 255 个字符。")
        normalized = tuple(
            sorted({_normalize_ble_uuid(value, "BLE service UUID") for value in self.service_uuids})
        )
        if len(normalized) > MAX_BLE_SERVICES:
            raise ConfigurationError(f"BLE service UUID 最多 {MAX_BLE_SERVICES} 项。")
        if self.schema_version != 1:
            raise ConfigurationError("不支持的 BLE 扫描配置 schema 版本。")
        object.__setattr__(self, "name_filter", name_filter)
        object.__setattr__(self, "service_uuids", normalized)


@dataclass(frozen=True, slots=True)
class BleGattCharacteristicRef:
    """Stable characteristic instance identity used by GATT commands."""

    service_uuid: str
    characteristic_uuid: str
    handle: int

    def __post_init__(self) -> None:
        object.__setattr__(
            self, "service_uuid", _normalize_ble_uuid(self.service_uuid, "BLE service UUID")
        )
        object.__setattr__(
            self,
            "characteristic_uuid",
            _normalize_ble_uuid(self.characteristic_uuid, "BLE characteristic UUID"),
        )
        if isinstance(self.handle, bool) or not isinstance(self.handle, int) or self.handle < 0:
            raise ConfigurationError("BLE characteristic handle 必须是非负整数。")

    @property
    def key(self) -> str:
        """Return a stable string key that disambiguates duplicate UUIDs."""

        return f"{self.service_uuid}|{self.characteristic_uuid}|{self.handle}"


@dataclass(frozen=True, slots=True)
class BleGattCharacteristic:
    """UI-safe characteristic capability snapshot."""

    ref: BleGattCharacteristicRef
    properties: tuple[str, ...] = ()
    max_write_without_response_size: int | None = None

    def __post_init__(self) -> None:
        if not isinstance(self.ref, BleGattCharacteristicRef):
            raise ConfigurationError("BLE characteristic ref 类型无效。")
        properties = tuple(
            sorted({str(value).strip().lower() for value in self.properties if str(value).strip()})
        )
        if self.max_write_without_response_size is not None and (
            isinstance(self.max_write_without_response_size, bool)
            or not isinstance(self.max_write_without_response_size, int)
            or self.max_write_without_response_size <= 0
        ):
            raise ConfigurationError("BLE without-response 最大写长必须是正整数或 None。")
        object.__setattr__(self, "properties", properties)

    @property
    def display(self) -> str:
        """Return a compact characteristic label with capability hints."""

        capabilities = ",".join(self.properties) or "无属性"
        return f"{self.ref.characteristic_uuid} · h={self.ref.handle} · {capabilities}"

    def supports(self, property_name: str) -> bool:
        return property_name.strip().lower() in self.properties


@dataclass(frozen=True, slots=True)
class BleGattService:
    """UI-safe service snapshot containing immutable characteristic capabilities."""

    uuid: str
    characteristics: tuple[BleGattCharacteristic, ...] = ()

    def __post_init__(self) -> None:
        object.__setattr__(self, "uuid", _normalize_ble_uuid(self.uuid, "BLE service UUID"))
        if len(self.characteristics) > MAX_BLE_CHARACTERISTICS:
            raise ConfigurationError(f"BLE characteristic 最多 {MAX_BLE_CHARACTERISTICS} 项。")
        if any(not isinstance(value, BleGattCharacteristic) for value in self.characteristics):
            raise ConfigurationError("BLE service characteristics 类型无效。")

    @property
    def display(self) -> str:
        return f"{self.uuid} · {len(self.characteristics)} characteristics"


@dataclass(frozen=True, slots=True)
class BleGattTransportConfig:
    """Versioned single-device BLE GATT session settings."""

    device_id: str
    device_name: str = ""
    connect_timeout: float = 30.0
    pair: bool = False
    use_cached_services: bool | None = None
    service_uuids: tuple[str, ...] = ()
    schema_version: int = 1

    def __post_init__(self) -> None:
        device_id = self.device_id.strip()
        device_name = self.device_name.strip()
        if not device_id or len(device_id) > MAX_BLE_DEVICE_ID_LENGTH:
            raise ConfigurationError("BLE 设备 ID 不能为空且不能超过 255 个字符。")
        if len(device_name) > MAX_BLE_DEVICE_NAME_LENGTH:
            raise ConfigurationError("BLE 设备名称不能超过 255 个字符。")
        _validate_network_timeout(self.connect_timeout, "BLE 连接超时")
        if not isinstance(self.pair, bool):
            raise ConfigurationError("BLE pair 必须是 bool。")
        if self.use_cached_services is not None and not isinstance(self.use_cached_services, bool):
            raise ConfigurationError("BLE use_cached_services 必须是 bool 或 None。")
        normalized = tuple(
            sorted({_normalize_ble_uuid(value, "BLE service UUID") for value in self.service_uuids})
        )
        if len(normalized) > MAX_BLE_SERVICES:
            raise ConfigurationError(f"BLE service UUID 最多 {MAX_BLE_SERVICES} 项。")
        if self.schema_version != 1:
            raise ConfigurationError("不支持的 BLE 配置 schema 版本。")
        object.__setattr__(self, "device_id", device_id)
        object.__setattr__(self, "device_name", device_name)
        object.__setattr__(self, "service_uuids", normalized)

    @property
    def endpoint(self) -> Endpoint:
        label = self.device_name or self.device_id
        return Endpoint(
            transport=TransportKind.BLE_GATT,
            address=self.device_id,
            label=f"BLE {label}",
            identity=f"ble:{self.device_id}",
            metadata=(
                ("device_id", self.device_id),
                ("device_name", self.device_name or "unknown"),
                ("pair", str(self.pair).lower()),
                ("use_cached_services", str(self.use_cached_services).lower()),
                ("service_uuids", ",".join(self.service_uuids) or "all"),
            ),
        )


class ServerRejectReason(StrEnum):
    """Why a TCP server accepted a socket and then rejected its peer."""

    UNAUTHORIZED_PEER = "unauthorized_peer"
    BUSY = "busy"


class ServerReadKind(StrEnum):
    """Outcome of one bounded TCP server operation."""

    TIMEOUT = "timeout"
    CLIENT_CONNECTED = "client_connected"
    DATA = "data"
    CLIENT_EOF = "client_eof"
    PEER_REJECTED = "peer_rejected"
    WRITE_COMPLETED = "write_completed"
    CLIENT_ERROR = "client_error"


class TcpServerClientState(StrEnum):
    """Lifecycle of one client owned by a TCP server session."""

    CONNECTED = "connected"
    DISCONNECTED = "disconnected"


@dataclass(frozen=True, slots=True)
class TcpServerPeerSnapshot:
    """Stable identity and bounded queue summary for one accepted connection."""

    peer_id: PeerId
    address: PeerAddress
    state: TcpServerClientState = TcpServerClientState.CONNECTED
    queued_items: int = 0
    queued_bytes: int = 0

    def __post_init__(self) -> None:
        if not isinstance(self.peer_id, UUID):
            raise ConfigurationError("TCP Server peer_id 必须使用 UUID。")
        if not isinstance(self.address, PeerAddress):
            raise ConfigurationError("TCP Server peer address 必须使用 PeerAddress。")
        if not isinstance(self.state, TcpServerClientState):
            raise ConfigurationError("TCP Server peer state 必须使用 TcpServerClientState。")
        if (
            isinstance(self.queued_items, bool)
            or not isinstance(self.queued_items, int)
            or self.queued_items < 0
        ):
            raise ConfigurationError("TCP Server peer 队列条数不能为负数。")
        if (
            isinstance(self.queued_bytes, bool)
            or not isinstance(self.queued_bytes, int)
            or self.queued_bytes < 0
        ):
            raise ConfigurationError("TCP Server peer 队列字节数不能为负数。")

    @property
    def display(self) -> str:
        """Return the address used by the compact UI and diagnostics."""

        return self.address.display


@dataclass(frozen=True, slots=True)
class TcpServerTransportConfig:
    """Versioned IPv4 TCP listener settings with bounded client fan-out."""

    bind_host: str = "127.0.0.1"
    listen_port: int = 9_000
    accept_timeout: float = 0.2
    read_timeout: float = 0.2
    write_timeout: float = 3.0
    read_chunk_size: int = 4_096
    allowlist: tuple[str, ...] = ()
    lan_confirmed: bool = False
    schema_version: int = 1
    max_clients: int = 4

    def __post_init__(self) -> None:
        if not isinstance(self.bind_host, str):
            raise ConfigurationError("TCP Server 监听主机必须是字符串。")
        bind_host = self.bind_host.strip()
        if not bind_host:
            raise ConfigurationError("TCP Server 监听主机不能为空。")
        if len(bind_host) > MAX_NETWORK_HOST_LENGTH:
            raise ConfigurationError("TCP Server 监听主机超过长度上限。")
        try:
            bind_ip = ipaddress.ip_address(bind_host)
        except ValueError as exc:
            raise ConfigurationError("TCP Server 仅支持 IPv4 字面量，不支持主机名。") from exc
        if bind_ip.version != 4:
            raise ConfigurationError("TCP Server 当前仅支持 IPv4，不支持 IPv6。")
        object.__setattr__(self, "bind_host", str(bind_ip))

        if (
            isinstance(self.listen_port, bool)
            or not isinstance(self.listen_port, int)
            or not 1 <= self.listen_port <= MAX_NETWORK_PORT
        ):
            raise ConfigurationError("TCP Server 监听端口必须在 1 到 65535 之间。")
        _validate_network_timeout(self.accept_timeout, "TCP Server 接收超时")
        _validate_network_timeout(self.read_timeout, "TCP Server 读取超时")
        _validate_network_timeout(self.write_timeout, "TCP Server 写入超时")
        if (
            isinstance(self.read_chunk_size, bool)
            or not isinstance(self.read_chunk_size, int)
            or not 0 < self.read_chunk_size <= MAX_STREAM_PAYLOAD_BYTES
        ):
            raise ConfigurationError("TCP Server 读取块大小必须在 1 到 1 MiB 之间。")
        if (
            isinstance(self.max_clients, bool)
            or not isinstance(self.max_clients, int)
            or not 1 <= self.max_clients <= MAX_TCP_SERVER_CLIENTS
        ):
            raise ConfigurationError(
                f"TCP Server 最大 client 数必须在 1 到 {MAX_TCP_SERVER_CLIENTS} 之间。"
            )
        if not isinstance(self.lan_confirmed, bool):
            raise ConfigurationError("TCP Server LAN 确认必须是 bool。")

        if not isinstance(self.allowlist, (tuple, list)):
            raise ConfigurationError("TCP Server allowlist 必须是 IPv4/CIDR 列表。")
        normalized_allowlist: list[str] = []
        for item in self.allowlist:
            if not isinstance(item, str):
                raise ConfigurationError("TCP Server allowlist 项必须是字符串。")
            value = item.strip()
            if not value:
                raise ConfigurationError("TCP Server allowlist 不能包含空项。")
            if len(value) > MAX_SERVER_ALLOWLIST_ITEM_LENGTH:
                raise ConfigurationError("TCP Server allowlist 项超过长度上限。")
            try:
                network = ipaddress.ip_network(value, strict=False)
            except ValueError as exc:
                raise ConfigurationError(
                    f"TCP Server allowlist 项无效：{value}；请输入 IPv4 或 CIDR。"
                ) from exc
            if network.version != 4:
                raise ConfigurationError("TCP Server allowlist 当前仅支持 IPv4/CIDR。")
            canonical = str(network)
            if canonical not in normalized_allowlist:
                normalized_allowlist.append(canonical)
        if len(normalized_allowlist) > MAX_SERVER_ALLOWLIST_ENTRIES:
            raise ConfigurationError(
                f"TCP Server allowlist 最多 {MAX_SERVER_ALLOWLIST_ENTRIES} 项。"
            )
        object.__setattr__(self, "allowlist", tuple(normalized_allowlist))

        if not bind_ip.is_loopback and (not self.lan_confirmed or not normalized_allowlist):
            raise ConfigurationError(
                "TCP Server 监听非回环地址前，必须确认 LAN 监听并填写非空 allowlist。"
            )
        if self.schema_version != 1:
            raise ConfigurationError("不支持的 TCP Server 配置 schema 版本。")

    @property
    def is_loopback(self) -> bool:
        """Whether the listener is restricted to a loopback interface."""

        return ipaddress.ip_address(self.bind_host).is_loopback

    def allows(self, peer: PeerAddress) -> bool:
        """Return whether an incoming numeric IPv4 peer passes this policy."""

        try:
            peer_ip = ipaddress.ip_address(peer.host)
        except ValueError:
            return False
        if peer_ip.version != 4:
            return False
        if not self.allowlist:
            return self.is_loopback and peer_ip.is_loopback
        return any(peer_ip in ipaddress.ip_network(item) for item in self.allowlist)

    @property
    def endpoint(self) -> Endpoint:
        allowlist = ",".join(self.allowlist) if self.allowlist else "loopback"
        address = f"{self.bind_host}:{self.listen_port}"
        return Endpoint(
            transport=TransportKind.TCP_SERVER,
            address=address,
            label=f"TCP Server {address}",
            metadata=(
                ("bind_host", self.bind_host),
                ("listen_port", str(self.listen_port)),
                ("max_clients", str(self.max_clients)),
                ("allowlist", allowlist),
                ("lan_confirmed", str(self.lan_confirmed).lower()),
            ),
        )
