"""会话保存/恢复管理器。

负责将 :class:`SessionState` 以原子写入方式持久化到 ``.edsession``
文件，并提供崩溃恢复检测、自动节流保存与会话清理能力。所有 IO 操作
均带有失败返回值，便于上层根据布尔结果给出错误提示。
"""

from __future__ import annotations

import os
import time
from pathlib import Path

from embeddebug.serial_station.session.serializer import SessionSerializer
from embeddebug.serial_station.session.state import SessionState

SESSION_EXTENSION = ".edsession"
"""会话文件统一扩展名。"""

AUTOSAVE_INTERVAL_S = 5.0
"""自动保存最小间隔（秒），避免高频抖动落盘。"""

_MARKER_SUFFIX = ".lock"


class SessionManager:
    """管理会话文件的生命周期。

    该实例保存最近一次自动保存的时间戳，用于实现节流；实例本身是
    非线程安全对象，调用方需保证单线程或加锁访问。
    """

    def __init__(self, now_ns: int = 0) -> None:
        self._last_autosave_ns: int = int(now_ns)
        self._has_autosaved = False

    def save(self, state: SessionState, path: Path) -> bool:
        """原子写入会话文件。

        Args:
            state: 待保存的会话状态。
            path: 目标路径，建议使用 ``.edsession`` 扩展名。

        Returns:
            写入成功返回 ``True``，任何 IO/序列化错误返回 ``False``。
        """

        try:
            payload = SessionSerializer.serialize(state)
            directory = Path(path).expanduser().parent
            directory.mkdir(parents=True, exist_ok=True)
            tmp_path = Path(f"{path}{_MARKER_SUFFIX}.tmp")
            tmp_path.write_text(payload, encoding="utf-8")
            os.replace(tmp_path, path)
        except (OSError, ValueError, TypeError):
            return False
        return True

    def load(self, path: Path) -> SessionState | None:
        """从磁盘读取并反序列化会话状态。

        Args:
            path: ``.edsession`` 文件路径。

        Returns:
            解析后的状态；文件不存在或读取失败返回 ``None``。
        """

        target = Path(path).expanduser()
        if not target.is_file():
            return None
        try:
            text = target.read_text(encoding="utf-8")
        except OSError:
            return None
        return SessionSerializer.deserialize(text)

    def autosave(self, state: SessionState, path: Path) -> bool:
        """节流自动保存。

        首次调用立即落盘，后续调用受 :data:`AUTOSAVE_INTERVAL_S` 节流。
        超出间隔才会写盘并刷新时间戳，从而避免拖累 UI 线程。

        Args:
            state: 当前会话状态。
            path: 目标路径。

        Returns:
            本次调用是否实际触发落盘。
        """

        now_ns = time.time_ns()
        if self._has_autosaved and (now_ns - self._last_autosave_ns) < int(AUTOSAVE_INTERVAL_S * 1_000_000_000):
            return False
        if not self.save(state, path):
            return False
        self._last_autosave_ns = now_ns
        self._has_autosaved = True
        return True

    def has_crash_recovery(self, path: Path) -> bool:
        """判断指定路径是否存在可用于崩溃恢复的会话文件。

        Args:
            path: 会话文件路径。

        Returns:
            文件存在且非空视为可恢复。
        """

        target = Path(path).expanduser()
        try:
            return target.is_file() and target.stat().st_size > 0
        except OSError:
            return False

    def clear(self, path: Path) -> bool:
        """删除会话文件（含遗留临时文件）。

        Args:
            path: 会话文件路径。

        Returns:
            清理后目标文件不再存在返回 ``True``。
        """

        target = Path(path).expanduser()
        tmp_path = Path(f"{target}{_MARKER_SUFFIX}.tmp")
        for candidate in (target, tmp_path):
            try:
                if candidate.exists():
                    candidate.unlink()
            except OSError:
                pass
        return not target.exists()
