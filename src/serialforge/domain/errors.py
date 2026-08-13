"""Typed error boundaries shared by the application and adapters."""

from __future__ import annotations

from dataclasses import dataclass
from enum import StrEnum


class ErrorCode(StrEnum):
    """Stable categories that presentation code can render or act on."""

    CONFIGURATION = "configuration"
    DEPENDENCY_MISSING = "dependency_missing"
    DISCOVERY = "discovery"
    TRANSPORT = "transport"
    TRANSPORT_OPEN = "transport_open"
    TRANSPORT_READ = "transport_read"
    TRANSPORT_WRITE = "transport_write"
    TRANSPORT_CLOSE = "transport_close"
    TRANSPORT_EOF = "transport_eof"
    TRANSPORT_NOT_OPEN = "transport_not_open"
    DATAGRAM_PEER_REJECTED = "datagram_peer_rejected"
    DATAGRAM_TOO_LARGE = "datagram_too_large"
    TCP_SERVER_PEER_REJECTED = "tcp_server_peer_rejected"
    TCP_SERVER_BUSY = "tcp_server_busy"
    TCP_SERVER_NO_CLIENT = "tcp_server_no_client"
    TCP_SERVER_PEER_NOT_CONNECTED = "tcp_server_peer_not_connected"
    BLE_DEVICE_NOT_FOUND = "ble_device_not_found"
    BLE_CHARACTERISTIC_UNSUPPORTED = "ble_characteristic_unsupported"
    BLE_AUTH_REQUIRED = "ble_auth_required"
    BLE_NOTIFY = "ble_notify"
    BLE_MTU = "ble_mtu"
    PROTOCOL_INPUT = "protocol_input"
    COMPONENT_PROFILE = "component_profile"
    DATASET_CONFIGURATION = "dataset_configuration"
    REPLAY = "replay"
    REPLAY_BUSY = "replay_busy"
    REPLAY_SHUTDOWN_TIMEOUT = "replay_shutdown_timeout"
    COMMAND_BATCH_SHUTDOWN_TIMEOUT = "command_batch_shutdown_timeout"
    ENDPOINT_MISSING = "endpoint_missing"
    ENDPOINT_CHANGED = "endpoint_changed"
    SESSION_BUSY = "session_busy"
    SESSION_NOT_FOUND = "session_not_found"
    SESSION_NOT_OPEN = "session_not_open"
    SESSION_SHUTDOWN_TIMEOUT = "session_shutdown_timeout"
    OUTBOUND_QUEUE_FULL = "outbound_queue_full"
    RECORDING = "recording"
    RECORDING_BUSY = "recording_busy"
    RECORDING_SHUTDOWN_TIMEOUT = "recording_shutdown_timeout"
    BACKPRESSURE = "backpressure"
    CANCELLED = "cancelled"
    UNKNOWN = "unknown"


@dataclass(frozen=True, slots=True)
class ErrorInfo:
    """A UI-safe error summary with optional diagnostic detail."""

    code: ErrorCode
    message: str
    recoverable: bool = False
    detail: str | None = None


class SerialForgeError(Exception):
    """Base exception carrying a structured error for a boundary crossing."""

    def __init__(
        self,
        message: str,
        *,
        code: ErrorCode = ErrorCode.UNKNOWN,
        recoverable: bool = False,
        detail: str | None = None,
    ) -> None:
        super().__init__(message)
        self.info = ErrorInfo(
            code=code,
            message=message,
            recoverable=recoverable,
            detail=detail,
        )


class ConfigurationError(SerialForgeError):
    """A transport or session configuration is invalid."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.CONFIGURATION,
            recoverable=False,
            detail=detail,
        )


class TransportError(SerialForgeError):
    """Base class for infrastructure transport failures."""

    def __init__(
        self,
        message: str,
        *,
        code: ErrorCode = ErrorCode.TRANSPORT,
        recoverable: bool = True,
        detail: str | None = None,
    ) -> None:
        super().__init__(
            message,
            code=code,
            recoverable=recoverable,
            detail=detail,
        )


class TransportDependencyError(TransportError):
    """A concrete adapter dependency is unavailable at runtime."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.DEPENDENCY_MISSING,
            recoverable=False,
            detail=detail,
        )


class TransportDiscoveryError(TransportError):
    """An adapter could not enumerate its endpoints."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.DISCOVERY,
            recoverable=True,
            detail=detail,
        )


class TransportOpenError(TransportError):
    """A transport could not be opened."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.TRANSPORT_OPEN,
            recoverable=True,
            detail=detail,
        )


class TransportReadError(TransportError):
    """A transport read failed."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.TRANSPORT_READ,
            recoverable=True,
            detail=detail,
        )


class TransportWriteError(TransportError):
    """A transport write failed or was incomplete."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.TRANSPORT_WRITE,
            recoverable=True,
            detail=detail,
        )


class TransportCloseError(TransportError):
    """A transport could not be closed cleanly."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.TRANSPORT_CLOSE,
            recoverable=True,
            detail=detail,
        )


class TransportEofError(TransportError):
    """A stream peer closed its connection; no reconnect is attempted."""

    def __init__(self, message: str = "TCP 远端已关闭连接。", *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.TRANSPORT_EOF,
            recoverable=True,
            detail=detail,
        )


class ProtocolInputError(SerialForgeError):
    """A protocol decoder received an invalid or unbounded input unit."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.PROTOCOL_INPUT,
            recoverable=True,
            detail=detail,
        )


class ComponentProfileError(SerialForgeError):
    """A declarative component profile is missing, invalid, or unbounded."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.COMPONENT_PROFILE,
            recoverable=True,
            detail=detail,
        )


class DatasetConfigurationError(SerialForgeError):
    """A declarative dataset configuration is missing, invalid, or unbounded."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.DATASET_CONFIGURATION,
            recoverable=True,
            detail=detail,
        )


class ReplayConfigurationError(SerialForgeError):
    """A historical replay input or option is invalid or unbounded."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.REPLAY,
            recoverable=True,
            detail=detail,
        )


class DatagramPeerRejectedError(TransportError):
    """An outbound UDP operation targeted a different peer."""

    def __init__(
        self,
        message: str = "UDP 目标 peer 与当前配置不匹配。",
        *,
        detail: str | None = None,
    ) -> None:
        super().__init__(
            message,
            code=ErrorCode.DATAGRAM_PEER_REJECTED,
            recoverable=True,
            detail=detail,
        )


class DatagramTooLargeError(TransportError):
    """A UDP payload exceeded the configured limit and was not truncated."""

    def __init__(
        self,
        message: str = "UDP datagram 超过配置的 payload 上限。",
        *,
        detail: str | None = None,
    ) -> None:
        super().__init__(
            message,
            code=ErrorCode.DATAGRAM_TOO_LARGE,
            recoverable=True,
            detail=detail,
        )


class TcpServerPeerRejectedError(TransportError):
    """An incoming TCP client failed the listener's explicit allowlist."""

    def __init__(self, peer: str, *, detail: str | None = None) -> None:
        super().__init__(
            f"已拒绝 TCP client {peer}：不在 allowlist。",
            code=ErrorCode.TCP_SERVER_PEER_REJECTED,
            recoverable=True,
            detail=detail,
        )


class TcpServerBusyError(TransportError):
    """A second incoming client was rejected while the single slot was occupied."""

    def __init__(self, peer: str, *, detail: str | None = None) -> None:
        super().__init__(
            f"已拒绝 TCP client {peer}：当前已有客户端连接。",
            code=ErrorCode.TCP_SERVER_BUSY,
            recoverable=True,
            detail=detail,
        )


class TcpServerNoClientError(TransportError):
    """A server send was requested while no client is connected."""

    def __init__(self, message: str = "当前没有已连接的 TCP client，暂时无法发送。") -> None:
        super().__init__(
            message,
            code=ErrorCode.TCP_SERVER_NO_CLIENT,
            recoverable=True,
        )


class TcpServerPeerNotConnectedError(TransportError):
    """A selected peer disconnected before its queued write was sent."""

    def __init__(self, peer: str, *, detail: str | None = None) -> None:
        super().__init__(
            f"TCP Server client {peer} 已不再连接，发送未执行。",
            code=ErrorCode.TCP_SERVER_PEER_NOT_CONNECTED,
            recoverable=True,
            detail=detail,
        )


class BleGattDeviceNotFoundError(TransportError):
    """The selected BLE device could not be resolved by Windows."""

    def __init__(self, device_id: str, *, detail: str | None = None) -> None:
        super().__init__(
            f"未找到 BLE 设备 {device_id}；请重新扫描后手动连接。",
            code=ErrorCode.BLE_DEVICE_NOT_FOUND,
            recoverable=True,
            detail=detail,
        )


class BleGattCharacteristicUnsupportedError(TransportError):
    """The selected characteristic does not expose the requested capability."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.BLE_CHARACTERISTIC_UNSUPPORTED,
            recoverable=True,
            detail=detail,
        )


class BleGattAuthorizationError(TransportError):
    """The Windows BLE operation needs pairing, encryption, or authorization."""

    def __init__(
        self,
        message: str = "BLE 设备需要配对或授权，请在 Windows 中完成后重试。",
        *,
        detail: str | None = None,
    ) -> None:
        super().__init__(
            message,
            code=ErrorCode.BLE_AUTH_REQUIRED,
            recoverable=True,
            detail=detail,
        )


class BleGattNotificationError(TransportError):
    """A notification subscription operation failed."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.BLE_NOTIFY,
            recoverable=True,
            detail=detail,
        )


class BleGattMtuError(TransportError):
    """A GATT payload exceeded the adapter's known write capability."""

    def __init__(self, message: str, *, detail: str | None = None) -> None:
        super().__init__(
            message,
            code=ErrorCode.BLE_MTU,
            recoverable=True,
            detail=detail,
        )


class TransportNotOpenError(TransportError):
    """An I/O operation was attempted before opening the transport."""

    def __init__(self, message: str = "传输尚未打开。") -> None:
        super().__init__(
            message,
            code=ErrorCode.TRANSPORT_NOT_OPEN,
            recoverable=False,
        )


class SessionBusyError(SerialForgeError):
    """A second session was requested while one is still running."""

    def __init__(self, message: str = "已有会话正在运行。") -> None:
        super().__init__(message, code=ErrorCode.SESSION_BUSY, recoverable=True)


class SessionNotFoundError(SerialForgeError):
    """The requested session identity is not known."""

    def __init__(self, message: str = "会话不存在或已经结束。") -> None:
        super().__init__(message, code=ErrorCode.SESSION_NOT_FOUND, recoverable=True)


class SessionNotOpenError(SerialForgeError):
    """A command was requested before the session reached the open state."""

    def __init__(self, message: str = "会话尚未连接。") -> None:
        super().__init__(message, code=ErrorCode.SESSION_NOT_OPEN, recoverable=True)


class SessionShutdownTimeoutError(SerialForgeError):
    """The worker did not stop within the caller's bounded shutdown budget."""

    def __init__(self, message: str = "会话 worker 未能在关闭时限内退出。") -> None:
        super().__init__(
            message,
            code=ErrorCode.SESSION_SHUTDOWN_TIMEOUT,
            recoverable=False,
        )


class OutboundQueueFullError(SerialForgeError):
    """The bounded outbound queue rejected a new write."""

    def __init__(self, message: str = "发送队列已满，请稍后重试。") -> None:
        super().__init__(
            message,
            code=ErrorCode.OUTBOUND_QUEUE_FULL,
            recoverable=True,
        )


class RecordingBusyError(SerialForgeError):
    """A second recorder was requested while the previous writer is active."""

    def __init__(self, message: str = "原始记录器仍在运行，请先等待上一份记录关闭。") -> None:
        super().__init__(message, code=ErrorCode.RECORDING_BUSY, recoverable=True)


class ReplayBusyError(SerialForgeError):
    """A second historical replay was requested while one is active."""

    def __init__(self, message: str = "已有历史回放正在运行。") -> None:
        super().__init__(message, code=ErrorCode.REPLAY_BUSY, recoverable=True)


class CommandBatchShutdownTimeoutError(SerialForgeError):
    """The bounded command batch worker did not stop within its budget."""

    def __init__(self, message: str = "批量命令 worker 未能在关闭时限内退出。") -> None:
        super().__init__(
            message,
            code=ErrorCode.COMMAND_BATCH_SHUTDOWN_TIMEOUT,
            recoverable=False,
        )


class ReplayShutdownTimeoutError(SerialForgeError):
    """The replay worker did not stop within its bounded shutdown budget."""

    def __init__(self, message: str = "历史回放 worker 未能在关闭时限内退出。") -> None:
        super().__init__(
            message,
            code=ErrorCode.REPLAY_SHUTDOWN_TIMEOUT,
            recoverable=False,
        )


class RecordingShutdownTimeoutError(SerialForgeError):
    """The recorder did not stop within its bounded shutdown budget."""

    def __init__(self, message: str = "原始记录器未能在关闭时限内退出。") -> None:
        super().__init__(
            message,
            code=ErrorCode.RECORDING_SHUTDOWN_TIMEOUT,
            recoverable=False,
        )
