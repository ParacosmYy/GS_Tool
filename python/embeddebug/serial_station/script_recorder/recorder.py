"""脚本录制器。"""
from __future__ import annotations
import time
from collections.abc import Iterable
from embeddebug.serial_station.script_recorder.action import ScriptAction
from embeddebug.serial_station.script_recorder.recording import ScriptRecording

class ScriptRecorder:
    """录制器：捕获动作流并自动打时间戳。"""

    def __init__(self, name: str = "", description: str = "", record_only: Iterable[str] | None = None) -> None:
        self._recording = ScriptRecording(name=name, description=description)
        self._is_recording = False
        self._start: float | None = None
        self._filter: set[str] | None = set(record_only) if record_only is not None else None

    @property
    def is_recording(self) -> bool:
        return self._is_recording

    @property
    def recording(self) -> ScriptRecording:
        return self._recording

    def start(self) -> None:
        if self._is_recording:
            return
        self._is_recording = True
        self._start = time.perf_counter()
        self._recording.created_at = time.strftime("%Y-%m-%d %H:%M:%S")

    def stop(self) -> ScriptRecording:
        self._is_recording = False
        self._start = None
        return self._recording

    def record(self, action: ScriptAction) -> bool:
        if not self._is_recording:
            return False
        if self._filter is not None and action.type not in self._filter:
            return False
        ts = int((time.perf_counter() - self._start) * 1000) if self._start else 0
        self._recording.add(ScriptAction(type=action.type, payload=action.payload, timestamp_ms=ts, label=action.label))
        return True
