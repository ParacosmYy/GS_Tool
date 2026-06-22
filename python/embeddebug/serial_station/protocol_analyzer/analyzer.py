"""协议帧分析器。"""

from __future__ import annotations

import re
from collections.abc import Iterable

from embeddebug.serial_station.protocol_analyzer.frame import DIRECTION_RX, DIRECTION_TX, ProtocolFrame
from embeddebug.serial_station.protocol_analyzer.report import AnalysisReport

_MIN_DURATION_S = 1e-3


class FrameAnalyzer:
    """累积帧并产出统计与错误报告的分析器。"""

    def __init__(self, error_patterns: dict[str, str] | None = None) -> None:
        self._frames: list[ProtocolFrame] = []
        self._error_patterns: dict[str, re.Pattern[str]] = {}
        if error_patterns:
            for field_name, pattern in error_patterns.items():
                self._error_patterns[field_name] = re.compile(pattern)

    def feed(self, frame: ProtocolFrame) -> None:
        self._frames.append(frame)

    def feed_many(self, frames: Iterable[ProtocolFrame]) -> None:
        for frame in frames:
            self.feed(frame)

    def clear(self) -> None:
        self._frames.clear()

    @property
    def frame_count(self) -> int:
        return len(self._frames)

    def analyze(self) -> AnalysisReport:
        frames = self._frames
        count_tx = sum(1 for f in frames if f.direction == DIRECTION_TX)
        count_rx = sum(1 for f in frames if f.direction == DIRECTION_RX)
        total_bytes = sum(f.size for f in frames)
        avg_size = (total_bytes / len(frames)) if frames else 0.0
        duration_s = self._compute_duration_s(frames)
        frequency_hz = (len(frames) / duration_s) if duration_s > 0 else 0.0
        errors = self._detect_errors(frames)
        stats = {"frame_count": len(frames), "frame_count_tx": count_tx, "frame_count_rx": count_rx, "total_bytes": total_bytes, "avg_frame_size": avg_size, "frequency_hz": frequency_hz, "duration_s": duration_s, "protocol_errors": len(errors)}
        return AnalysisReport(stats=stats, errors=errors, duration_s=duration_s)

    def _compute_duration_s(self, frames: list[ProtocolFrame]) -> float:
        if not frames:
            return 0.0
        first = min(f.timestamp_ns for f in frames)
        last = max(f.timestamp_ns for f in frames)
        return max((last - first) / 1e9, _MIN_DURATION_S)

    def _detect_errors(self, frames: list[ProtocolFrame]) -> list[str]:
        if not self._error_patterns:
            return []
        errors: list[str] = []
        for index, frame in enumerate(frames):
            for field_name, pattern in self._error_patterns.items():
                if field_name not in frame.decoded:
                    continue
                value = frame.decoded[field_name]
                text = value if isinstance(value, str) else str(value)
                match = pattern.search(text)
                if match:
                    errors.append(f"frame#{index} dir={frame.direction} field={field_name} matched={match.group(0)!r} raw={frame.raw_bytes.hex(' ')}")
        return errors
