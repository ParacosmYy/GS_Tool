"""OTA 升级配置与进度数据模型。"""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum


class OtaProtocol(str, Enum):
    XMODEM = "xmodem"
    YMODEM = "ymodem"
    ZMODEM = "zmodem"


class OtaState(str, Enum):
    IDLE = "idle"
    TRANSFERRING = "transferring"
    COMPLETE = "complete"
    FAILED = "failed"


_VALID_BLOCK_SIZES = (128, 1024)


@dataclass
class OtaConfig:
    """OTA 升级配置。"""
    file_path: str
    protocol: OtaProtocol = OtaProtocol.XMODEM
    block_size: int = 128
    use_crc: bool = True
    retry_count: int = 10

    def validate(self) -> None:
        if not self.file_path or not str(self.file_path).strip():
            raise ValueError("file_path 不能为空")
        if self.block_size not in _VALID_BLOCK_SIZES:
            raise ValueError(f"block_size 必须为 {_VALID_BLOCK_SIZES} 之一")
        if self.retry_count <= 0:
            raise ValueError("retry_count 必须大于 0")


@dataclass
class OtaProgress:
    """升级进度状态。"""
    sent_bytes: int = 0
    total_bytes: int = 0
    block_index: int = 0
    errors: int = 0
    state: OtaState = OtaState.IDLE

    @property
    def percent(self) -> float:
        if self.total_bytes <= 0:
            return 0.0
        return round(min(self.sent_bytes / self.total_bytes, 1.0) * 100.0, 2)

    def mark_sent(self, block_bytes: int) -> None:
        self.sent_bytes += max(block_bytes, 0)
        self.block_index += 1

    def mark_error(self) -> None:
        self.errors += 1

    def begin(self, total_bytes: int) -> None:
        self.state = OtaState.TRANSFERRING
        self.total_bytes = max(total_bytes, 0)
        self.sent_bytes = 0
        self.block_index = 0
        self.errors = 0

    def complete(self) -> None:
        self.state = OtaState.COMPLETE

    def fail(self) -> None:
        self.state = OtaState.FAILED
