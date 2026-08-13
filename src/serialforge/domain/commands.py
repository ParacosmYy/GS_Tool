"""Bounded, declarative command batches shared by application and UI."""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import StrEnum
from uuid import UUID, uuid4

from .errors import ConfigurationError, ErrorInfo
from .models import (
    BleGattCharacteristicRef,
    BleGattWriteMode,
    CommandEntry,
    PeerId,
    SessionId,
    TransportConfig,
)

MAX_COMMAND_BATCHES = 16
MAX_COMMAND_BATCH_STEPS = 32
MAX_COMMAND_BATCH_STEP_BYTES = 512
MAX_COMMAND_BATCH_TOTAL_BYTES = 16 * 1024
MAX_COMMAND_BATCH_STEP_DELAY_MS = 2_000
MAX_COMMAND_BATCH_TOTAL_DELAY_MS = 180_000


class CommandBatchState(StrEnum):
    """Observable lifecycle of one bounded command batch run."""

    IDLE = "idle"
    RUNNING = "running"
    COMPLETED = "completed"
    STOPPED = "stopped"
    FAILED = "failed"


@dataclass(frozen=True, slots=True)
class CommandBatchStep:
    """One static command and the optional delay before the next command."""

    entry: CommandEntry
    delay_after_ms: int = 0

    def __post_init__(self) -> None:
        if not isinstance(self.entry, CommandEntry):
            raise ConfigurationError("批量命令步骤必须使用 CommandEntry。")
        if (
            isinstance(self.delay_after_ms, bool)
            or not isinstance(self.delay_after_ms, int)
            or not 0 <= self.delay_after_ms <= MAX_COMMAND_BATCH_STEP_DELAY_MS
        ):
            raise ConfigurationError(
                f"批量命令步骤延时必须在 0 到 {MAX_COMMAND_BATCH_STEP_DELAY_MS} ms 之间。"
            )

    @property
    def wire_payload(self) -> bytes:
        """Return the exact bounded payload submitted to the transport queue."""

        return self.entry.payload + (b"\r\n" if self.entry.append_newline else b"")


@dataclass(frozen=True, slots=True)
class CommandBatch:
    """An immutable named sequence; it contains no code or dynamic behavior."""

    name: str
    steps: tuple[CommandBatchStep, ...]
    batch_id: UUID = field(default_factory=uuid4)

    def __post_init__(self) -> None:
        name = self.name.strip()
        if not name:
            raise ConfigurationError("批量命令名称不能为空。")
        if len(name) > 128:
            raise ConfigurationError("批量命令名称超过 128 个字符。")
        if not isinstance(self.batch_id, UUID):
            raise ConfigurationError("批量命令 ID 必须使用 UUID。")
        steps = tuple(self.steps)
        if not 0 < len(steps) <= MAX_COMMAND_BATCH_STEPS:
            raise ConfigurationError(f"批量命令步骤数必须在 1 到 {MAX_COMMAND_BATCH_STEPS} 之间。")
        if any(not isinstance(step, CommandBatchStep) for step in steps):
            raise ConfigurationError("批量命令只接受 CommandBatchStep。")
        total_bytes = sum(len(step.wire_payload) for step in steps)
        if any(len(step.wire_payload) > MAX_COMMAND_BATCH_STEP_BYTES for step in steps):
            raise ConfigurationError(
                f"批量命令单步 payload 不能超过 {MAX_COMMAND_BATCH_STEP_BYTES} 字节。"
            )
        if total_bytes > MAX_COMMAND_BATCH_TOTAL_BYTES:
            raise ConfigurationError(
                f"批量命令总 payload 不能超过 {MAX_COMMAND_BATCH_TOTAL_BYTES} 字节。"
            )
        total_delay = sum(step.delay_after_ms for step in steps[:-1])
        if total_delay > MAX_COMMAND_BATCH_TOTAL_DELAY_MS:
            raise ConfigurationError(
                f"批量命令总等待不能超过 {MAX_COMMAND_BATCH_TOTAL_DELAY_MS // 1000} 秒。"
            )
        object.__setattr__(self, "name", name)
        object.__setattr__(self, "steps", steps)

    @property
    def total_bytes(self) -> int:
        """Return the total wire payload size including optional CRLF."""

        return sum(len(step.wire_payload) for step in self.steps)

    @property
    def total_delay_ms(self) -> int:
        """Return delays that can occur between steps, excluding the final row."""

        return sum(step.delay_after_ms for step in self.steps[:-1])


@dataclass(frozen=True, slots=True)
class CommandBatchRequest:
    """A run-time snapshot that binds a batch to one active session target."""

    session_id: SessionId
    config: TransportConfig
    batch: CommandBatch
    target_peer_id: PeerId | None = None
    ble_characteristic: BleGattCharacteristicRef | None = None
    ble_write_mode: BleGattWriteMode | None = None

    def __post_init__(self) -> None:
        if not isinstance(self.session_id, UUID):
            raise ConfigurationError("批量命令 session_id 必须使用 UUID。")
        if not isinstance(self.batch, CommandBatch):
            raise ConfigurationError("批量命令 request 必须使用 CommandBatch。")
        if self.target_peer_id is not None and not isinstance(self.target_peer_id, UUID):
            raise ConfigurationError("批量命令 TCP peer 必须使用 UUID。")
        if self.ble_characteristic is not None and not isinstance(
            self.ble_characteristic, BleGattCharacteristicRef
        ):
            raise ConfigurationError("批量命令 BLE characteristic 类型无效。")
        if self.ble_write_mode is not None and not isinstance(
            self.ble_write_mode, BleGattWriteMode
        ):
            raise ConfigurationError("批量命令 BLE write mode 类型无效。")


@dataclass(frozen=True, slots=True)
class CommandBatchSnapshot:
    """Bounded progress state; completion means local queue acceptance only."""

    batch_id: UUID | None = None
    state: CommandBatchState = CommandBatchState.IDLE
    total_steps: int = 0
    accepted_steps: int = 0
    current_step: int | None = None
    failed_step: int | None = None
    message: str = ""
    error: ErrorInfo | None = None

    def __post_init__(self) -> None:
        if self.batch_id is not None and not isinstance(self.batch_id, UUID):
            raise ConfigurationError("批量命令 snapshot ID 必须使用 UUID。")
        if not isinstance(self.state, CommandBatchState):
            raise ConfigurationError("批量命令 snapshot 状态无效。")
        if min(self.total_steps, self.accepted_steps) < 0:
            raise ConfigurationError("批量命令 snapshot 计数不能为负数。")
        if self.total_steps > MAX_COMMAND_BATCH_STEPS:
            raise ConfigurationError("批量命令 snapshot 超过步骤上限。")
        if self.accepted_steps > self.total_steps:
            raise ConfigurationError("批量命令 snapshot accepted_steps 无效。")
        if self.current_step is not None and not 1 <= self.current_step <= self.total_steps:
            raise ConfigurationError("批量命令 snapshot current_step 无效。")
        if self.failed_step is not None and not 1 <= self.failed_step <= self.total_steps:
            raise ConfigurationError("批量命令 snapshot failed_step 无效。")
        if len(self.message) > 256:
            raise ConfigurationError("批量命令 snapshot message 过长。")


CommandBatchId = UUID
