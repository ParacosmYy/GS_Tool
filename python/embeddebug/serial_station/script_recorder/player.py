"""脚本回放器。"""
from __future__ import annotations
from PyQt6.QtCore import QObject, pyqtSignal
from embeddebug.serial_station.script_recorder.action import ScriptAction
from embeddebug.serial_station.script_recorder.recording import ScriptRecording

class ScriptPlayer(QObject):
    """按时间轴回放 ScriptRecording。"""
    action_ready = pyqtSignal(object)
    finished = pyqtSignal()
    position_changed = pyqtSignal(int)
    MIN_SPEED = 0.25
    MAX_SPEED = 4.0

    def __init__(self, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self._recording: ScriptRecording | None = None
        self._index = 0
        self._speed = 1.0
        self._is_playing = False

    def load(self, recording: ScriptRecording) -> None:
        self.stop()
        self._recording = recording
        self._index = 0

    @property
    def is_loaded(self) -> bool:
        return self._recording is not None

    @property
    def action_count(self) -> int:
        return 0 if self._recording is None else self._recording.action_count

    @property
    def current_index(self) -> int:
        return self._index

    @property
    def speed(self) -> float:
        return self._speed

    def set_speed(self, speed: float) -> None:
        self._speed = min(self.MAX_SPEED, max(self.MIN_SPEED, float(speed)))

    def play(self) -> None:
        if self._recording is None or self._index >= self.action_count:
            return
        self._is_playing = True

    def pause(self) -> None:
        self._is_playing = False

    def stop(self) -> None:
        self._is_playing = False
        self._index = 0

    def step_next(self) -> ScriptAction | None:
        if self._recording is None or self._index >= self.action_count:
            self._is_playing = False
            self.finished.emit()
            return None
        action = self._recording.actions[self._index]
        self._index += 1
        self.action_ready.emit(action)
        self.position_changed.emit(self._index)
        return action
