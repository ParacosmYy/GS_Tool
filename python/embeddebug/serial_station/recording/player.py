"""录制回放器。"""

from __future__ import annotations

from pathlib import Path

from PyQt6.QtCore import QObject, pyqtSignal

from embeddebug.serial_station.core.measurements import ChannelBatch
from embeddebug.serial_station.recording.format import RecordingFormat, RecordingReader


class RecordingPlayer(QObject):
    """QTimer 驱动的回放器。"""

    batch_available = pyqtSignal(object)
    position_changed = pyqtSignal(int)
    finished = pyqtSignal()

    def __init__(self, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self._pending: list[ChannelBatch] = []
        self._index = 0
        self._is_playing = False
        self._speed = 1.0

    def load(self, path: str | Path, fmt: RecordingFormat) -> bool:
        try:
            self.stop()
            reader = RecordingReader(fmt)
            reader.open(path)
            self._pending = list(reader.iter_batches())
            reader.close()
            self._index = 0
            return True
        except (OSError, ValueError, KeyError):
            return False

    @property
    def total_batches(self) -> int:
        return len(self._pending)

    @property
    def current_index(self) -> int:
        return self._index

    @property
    def progress(self) -> float:
        total = len(self._pending)
        if total == 0:
            return 0.0
        return self._index / total

    @property
    def is_playing(self) -> bool:
        return self._is_playing

    @property
    def speed(self) -> float:
        return self._speed

    def set_speed(self, speed: float) -> None:
        self._speed = min(4.0, max(0.25, float(speed)))

    def play(self) -> None:
        self._is_playing = True

    def pause(self) -> None:
        self._is_playing = False

    def stop(self) -> None:
        self._is_playing = False
        self._index = 0

    def step_next(self) -> ChannelBatch | None:
        """推进一个批次并返回（测试确定性钩子）。"""
        if self._index >= len(self._pending):
            self._is_playing = False
            self.finished.emit()
            return None
        batch = self._pending[self._index]
        self._index += 1
        self.batch_available.emit(batch)
        self.position_changed.emit(self._index)
        return batch

    def seek(self, batch_index: int) -> None:
        self._index = max(0, min(batch_index, len(self._pending)))
        self.position_changed.emit(self._index)
