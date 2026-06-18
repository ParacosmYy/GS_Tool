"""通知数据模型：级别枚举与不可变通知载荷。"""

from __future__ import annotations

import time
from collections.abc import Callable
from dataclasses import dataclass, field
from enum import Enum


class NotificationLevel(Enum):
    """通知级别。"""

    INFO = "info"
    SUCCESS = "success"
    WARNING = "warning"
    ERROR = "error"


@dataclass
class NotificationData:
    """一条通知的不可变载荷。"""

    level: NotificationLevel
    title: str
    message: str
    timestamp_ns: int
    timeout_ms: int = 3000
    action_label: str = ""
    action_callback: Callable[[], None] | None = None
    uid: int = field(default=0, repr=False)

    def is_expired(self, now_ns: int) -> bool:
        """判断是否已过期。"""
        if self.timeout_ms <= 0:
            return False
        return (now_ns - self.timestamp_ns) // 1_000_000 >= self.timeout_ms

    @classmethod
    def create(cls, level, title, message, timeout_ms=3000, action_label="", action_callback=None):
        return cls(level=level, title=title, message=message, timestamp_ns=time.monotonic_ns(), timeout_ms=timeout_ms, action_label=action_label, action_callback=action_callback)
