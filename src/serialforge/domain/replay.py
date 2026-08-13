"""Strict, bounded contracts for historical JSONL replay."""

from __future__ import annotations

import base64
import binascii
import json
import math
from dataclasses import dataclass
from datetime import datetime
from enum import StrEnum
from typing import Final
from uuid import UUID

from .errors import ConfigurationError, ErrorInfo, ReplayConfigurationError
from .models import (
    MAX_RAW_RECORD_BYTES,
    MAX_RECORD_FILE_BYTES,
    MAX_RECORD_PATH_LENGTH,
    Endpoint,
    PeerAddress,
    RawRecord,
    RecordDirection,
    TransportKind,
)

MAX_REPLAY_LINE_BYTES: Final = MAX_RAW_RECORD_BYTES * 2
MAX_REPLAY_RECORDS: Final = 1_000_000
MAX_REPLAY_BYTES: Final = MAX_RECORD_FILE_BYTES


class ReplayState(StrEnum):
    """Lifecycle of one offline replay worker."""

    EMPTY = "empty"
    PLAYING = "playing"
    PAUSED = "paused"
    EOF = "eof"
    STOPPED = "stopped"
    ERROR = "error"


@dataclass(frozen=True, slots=True)
class ReplayRecord:
    """One validated raw JSONL record plus its stable file line index."""

    index: int
    raw: RawRecord

    def __post_init__(self) -> None:
        if isinstance(self.index, bool) or not isinstance(self.index, int) or self.index < 1:
            raise ReplayConfigurationError("replay record index 必须是正整数。")
        if not isinstance(self.raw, RawRecord):
            raise ReplayConfigurationError("replay record 必须包含 RawRecord。")


@dataclass(frozen=True, slots=True)
class ReplayOptions:
    """Bounded playback controls; scheduler timing always uses monotonic time."""

    speed: float = 1.0
    max_records: int = MAX_REPLAY_RECORDS
    max_bytes: int = MAX_REPLAY_BYTES

    def __post_init__(self) -> None:
        if (
            isinstance(self.speed, bool)
            or not isinstance(self.speed, (int, float))
            or not math.isfinite(self.speed)
            or not 0.1 <= self.speed <= 16.0
        ):
            raise ReplayConfigurationError("replay speed 必须在 0.1 到 16 倍之间。")
        if (
            isinstance(self.max_records, bool)
            or not isinstance(self.max_records, int)
            or not 1 <= self.max_records <= MAX_REPLAY_RECORDS
        ):
            raise ReplayConfigurationError(
                f"replay max_records 必须在 1 到 {MAX_REPLAY_RECORDS} 之间。"
            )
        if (
            isinstance(self.max_bytes, bool)
            or not isinstance(self.max_bytes, int)
            or not 1 <= self.max_bytes <= MAX_REPLAY_BYTES
        ):
            raise ReplayConfigurationError(
                f"replay max_bytes 必须在 1 到 {MAX_REPLAY_BYTES} 之间。"
            )
        object.__setattr__(self, "speed", float(self.speed))


@dataclass(frozen=True, slots=True)
class ReplaySnapshot:
    """Observable replay counters and terminal state."""

    state: ReplayState = ReplayState.EMPTY
    replay_id: UUID | None = None
    path: str | None = None
    records_read: int = 0
    records_emitted: int = 0
    skipped_records: int = 0
    invalid_records: int = 0
    bytes_read: int = 0
    blocked_offers: int = 0
    capture_elapsed: float = 0.0
    error: ErrorInfo | None = None

    def __post_init__(self) -> None:
        if not isinstance(self.state, ReplayState):
            raise ConfigurationError("replay state 类型无效。")
        if self.replay_id is not None and not isinstance(self.replay_id, UUID):
            raise ConfigurationError("replay_id 必须使用 UUID。")
        if self.path is not None and len(self.path) > MAX_RECORD_PATH_LENGTH:
            raise ConfigurationError("replay 路径超过长度上限。")
        counters = (
            self.records_read,
            self.records_emitted,
            self.skipped_records,
            self.invalid_records,
            self.bytes_read,
            self.blocked_offers,
        )
        if any(
            isinstance(value, bool) or not isinstance(value, int) or value < 0 for value in counters
        ):
            raise ConfigurationError("replay 计数不能为负数。")
        if (
            isinstance(self.capture_elapsed, bool)
            or not isinstance(self.capture_elapsed, (int, float))
            or not math.isfinite(self.capture_elapsed)
            or self.capture_elapsed < 0
        ):
            raise ConfigurationError("replay capture_elapsed 必须是有限非负数字。")


class ReplayRecordCodec:
    """Decode the current recorder JSONL shape without executing user content."""

    _KEYS: Final = {
        "wall_time",
        "occurred_at",
        "session_id",
        "transport",
        "address",
        "identity",
        "peer",
        "peer_id",
        "channel",
        "direction",
        "payload_b64",
    }

    @classmethod
    def loads_line(cls, text: str, index: int) -> ReplayRecord:
        """Loads line."""
        if not isinstance(text, str):
            raise ReplayConfigurationError("replay JSONL 行必须是文本。")
        if len(text.encode("utf-8")) > MAX_REPLAY_LINE_BYTES:
            raise ReplayConfigurationError("replay JSONL 行超过 2 MiB 上限。")
        try:
            value = json.loads(
                text,
                object_pairs_hook=_reject_duplicate_keys,
                parse_constant=_reject_constant,
            )
            if not isinstance(value, dict) or set(value) != cls._KEYS:
                raise ReplayConfigurationError("replay JSONL 字段集合不受支持。")
            occurred_at = _finite_nonnegative(value["occurred_at"], "occurred_at")
            session_id = UUID(_required_string(value["session_id"], "session_id"))
            transport = TransportKind(_required_string(value["transport"], "transport"))
            address = _required_string(value["address"], "address")
            identity = _optional_string(value["identity"], "identity")
            endpoint = Endpoint(
                transport=transport,
                address=address,
                label=address,
                identity=identity,
            )
            direction = RecordDirection(_required_string(value["direction"], "direction"))
            wall_time = datetime.fromisoformat(_required_string(value["wall_time"], "wall_time"))
            if wall_time.tzinfo is None or wall_time.utcoffset() is None:
                raise ReplayConfigurationError("wall_time 必须包含时区。")
            payload = _decode_payload(value["payload_b64"])
            peer = _decode_peer(value["peer"])
            peer_id = _decode_uuid(value["peer_id"], "peer_id")
            channel = _optional_string(value["channel"], "channel")
            return ReplayRecord(
                index=index,
                raw=RawRecord(
                    session_id=session_id,
                    endpoint=endpoint,
                    direction=direction,
                    payload=payload,
                    occurred_at=occurred_at,
                    wall_time=wall_time,
                    peer=peer,
                    peer_id=peer_id,
                    channel=channel,
                ),
            )
        except ReplayConfigurationError:
            raise
        except (binascii.Error, ConfigurationError, KeyError, TypeError, ValueError) as exc:
            raise ReplayConfigurationError(
                f"replay JSONL 第 {index} 行无效。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc


def _required_string(value: object, label: str) -> str:
    """Required string."""
    if not isinstance(value, str) or not value.strip():
        raise ReplayConfigurationError(f"{label} 必须是非空字符串。")
    return value.strip()


def _optional_string(value: object, label: str) -> str | None:
    """Optional string."""
    if value is None:
        return None
    if not isinstance(value, str) or not value.strip():
        raise ReplayConfigurationError(f"{label} 必须是字符串或 null。")
    return value.strip()


def _decode_uuid(value: object, label: str) -> UUID | None:
    """Decode uuid."""
    if value is None:
        return None
    try:
        return UUID(_required_string(value, label))
    except (ReplayConfigurationError, ValueError) as exc:
        raise ReplayConfigurationError(f"{label} UUID 无效。") from exc


def _decode_peer(value: object) -> PeerAddress | None:
    """Decode peer."""
    if value is None:
        return None
    if not isinstance(value, dict) or set(value) != {"host", "port"}:
        raise ReplayConfigurationError("peer 必须是 host/port 对象或 null。")
    try:
        return PeerAddress(host=_required_string(value["host"], "peer.host"), port=value["port"])
    except (ConfigurationError, ReplayConfigurationError, TypeError, ValueError) as exc:
        raise ReplayConfigurationError("peer 定义无效。", detail=str(exc)) from exc


def _decode_payload(value: object) -> bytes:
    """Decode payload."""
    encoded = _required_string(value, "payload_b64")
    try:
        payload = base64.b64decode(encoded.encode("ascii"), validate=True)
    except (UnicodeEncodeError, binascii.Error) as exc:
        raise ReplayConfigurationError("payload_b64 不是有效 base64。") from exc
    if not payload:
        raise ReplayConfigurationError("payload 不能为空。")
    if len(payload) > MAX_RAW_RECORD_BYTES:
        raise ReplayConfigurationError("payload 超过 1 MiB 上限。")
    return payload


def _finite_nonnegative(value: object, label: str) -> float:
    """Finite nonnegative."""
    if (
        isinstance(value, bool)
        or not isinstance(value, (int, float))
        or not math.isfinite(value)
        or value < 0
    ):
        raise ReplayConfigurationError(f"{label} 必须是有限非负数字。")
    return float(value)


def _reject_duplicate_keys(pairs: list[tuple[str, object]]) -> dict[str, object]:
    """Reject duplicate keys."""
    result: dict[str, object] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"重复 JSON key：{key}")
        result[key] = value
    return result


def _reject_constant(value: str) -> None:
    """Reject constant."""
    raise ValueError(f"JSON 不允许特殊数字：{value}")


__all__ = [
    "MAX_REPLAY_BYTES",
    "MAX_REPLAY_LINE_BYTES",
    "MAX_REPLAY_RECORDS",
    "ReplayOptions",
    "ReplayRecord",
    "ReplayRecordCodec",
    "ReplaySnapshot",
    "ReplayState",
]
