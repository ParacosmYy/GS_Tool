"""Serial Station 会话保存/恢复模块。

提供会话状态快照、JSON 序列化以及原子落盘/崩溃恢复能力。模块对外仅
暴露三个核心类型，调用方应通过本入口 import，避免直接依赖内部文件。
"""

from __future__ import annotations

from embeddebug.serial_station.session.manager import (
    AUTOSAVE_INTERVAL_S,
    SESSION_EXTENSION,
    SessionManager,
)
from embeddebug.serial_station.session.serializer import (
    FORMAT_VERSION,
    SessionSerializer,
)
from embeddebug.serial_station.session.state import SessionState

__all__ = [
    "AUTOSAVE_INTERVAL_S",
    "FORMAT_VERSION",
    "SESSION_EXTENSION",
    "SessionManager",
    "SessionSerializer",
    "SessionState",
]
