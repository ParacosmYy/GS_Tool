"""Session, recorder, and command value objects.

The compatibility module re-exports these values through ``domain.models``.
"""

from __future__ import annotations

from dataclasses import dataclass
from datetime import datetime
from enum import StrEnum
from uuid import UUID

from .errors import ConfigurationError, ErrorInfo
from .models_transport import (
    MAX_COMMAND_NAME_LENGTH,
    MAX_COMMAND_PAYLOAD_BYTES,
    MAX_RAW_RECORD_BYTES,
    MAX_RECORD_FILE_BYTES,
    MAX_RECORD_PATH_LENGTH,
    Endpoint,
    PeerAddress,
    PeerId,
    SessionId,
    SessionState,
)


@dataclass(frozen=True, slots=True)
class SessionSnapshot:
    """Read-only session state suitable for presentation or diagnostics."""

    session_id: SessionId
    endpoint: Endpoint
    state: SessionState
    error: ErrorInfo | None = None


class RecordDirection(StrEnum):
    """Direction of a raw stream record."""

    RECEIVE = "rx"
    SEND = "tx"


@dataclass(frozen=True, slots=True)
class RawRecord:
    """A bounded, timestamped raw UART record suitable for append-only storage."""

    session_id: SessionId
    endpoint: Endpoint
    direction: RecordDirection
    payload: bytes
    occurred_at: float
    wall_time: datetime
    peer: PeerAddress | None = None
    peer_id: PeerId | None = None
    channel: str | None = None

    def __post_init__(self) -> None:
        if not isinstance(self.direction, RecordDirection):
            raise ConfigurationError("原始记录方向必须使用 RecordDirection。")
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ConfigurationError("原始记录 payload 必须是 bytes-like。")
        payload = bytes(self.payload)
        if not payload:
            raise ConfigurationError("原始记录 payload 不能为空。")
        if len(payload) > MAX_RAW_RECORD_BYTES:
            raise ConfigurationError("原始记录 payload 超过 1 MiB 上限。")
        if self.occurred_at < 0:
            raise ConfigurationError("原始记录单调时间不能为负数。")
        if self.wall_time.tzinfo is None or self.wall_time.utcoffset() is None:
            raise ConfigurationError("原始记录墙上时间必须包含时区。")
        if self.peer is not None and not isinstance(self.peer, PeerAddress):
            raise ConfigurationError("原始记录 peer 必须使用 PeerAddress。")
        if self.peer_id is not None and not isinstance(self.peer_id, UUID):
            raise ConfigurationError("原始记录 peer_id 必须使用 UUID。")
        if self.channel is not None and not self.channel.strip():
            raise ConfigurationError("原始记录 channel 不能为空字符串。")
        object.__setattr__(self, "payload", payload)


class RecordingState(StrEnum):
    """Lifecycle of the bounded raw recorder."""

    STOPPED = "stopped"
    STARTING = "starting"
    ACTIVE = "active"
    STOPPING = "stopping"
    ERROR = "error"


@dataclass(frozen=True, slots=True)
class RecordingSnapshot:
    """Observable recorder state with bounded counters."""

    state: RecordingState = RecordingState.STOPPED
    path: str | None = None
    queued_records: int = 0
    written_records: int = 0
    dropped_records: int = 0
    bytes_written: int = 0
    error: ErrorInfo | None = None

    def __post_init__(self) -> None:
        if self.path is not None and len(self.path) > MAX_RECORD_PATH_LENGTH:
            raise ConfigurationError("原始记录文件路径超过长度上限。")
        if (
            min(
                self.queued_records,
                self.written_records,
                self.dropped_records,
                self.bytes_written,
            )
            < 0
        ):
            raise ConfigurationError("原始记录计数不能为负数。")
        if self.bytes_written > MAX_RECORD_FILE_BYTES:
            raise ConfigurationError("原始记录文件超过 64 MiB 上限。")


class CommandMode(StrEnum):
    """Presentation encoding used to edit a command."""

    TEXT = "text"
    HEX = "hex"


@dataclass(frozen=True, slots=True)
class CommandEntry:
    """A bounded send-history or quick-command item."""

    name: str
    payload: bytes
    mode: CommandMode
    append_newline: bool
    created_at: float

    def __post_init__(self) -> None:
        name = self.name.strip()
        if not name:
            raise ConfigurationError("命令名称不能为空。")
        if len(name) > MAX_COMMAND_NAME_LENGTH:
            raise ConfigurationError("命令名称超过长度上限。")
        if not isinstance(self.mode, CommandMode):
            raise ConfigurationError("命令模式必须使用 CommandMode。")
        if not isinstance(self.payload, (bytes, bytearray, memoryview)):
            raise ConfigurationError("命令 payload 必须是 bytes-like。")
        payload = bytes(self.payload)
        if not payload:
            raise ConfigurationError("命令 payload 不能为空。")
        if len(payload) > MAX_COMMAND_PAYLOAD_BYTES:
            raise ConfigurationError("命令 payload 超过 64 KiB 历史上限。")
        object.__setattr__(self, "name", name)
        object.__setattr__(self, "payload", payload)
