"""录制时间轴：分段、间隙、书签。"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class RecordingSegment:
    start_ns: int
    end_ns: int
    sample_count: int
    source: str = "serial"

    @property
    def duration_ns(self) -> int:
        return self.end_ns - self.start_ns

    def contains(self, position_ns: int) -> bool:
        return self.start_ns <= position_ns <= self.end_ns


@dataclass
class Gap:
    end_of_previous_ns: int
    start_of_next_ns: int

    @property
    def duration_ns(self) -> int:
        return self.start_of_next_ns - self.end_of_previous_ns


class RecordingTimeline:
    """多段录制时间轴。"""

    DEFAULT_GAP_THRESHOLD_NS = 50_000_000

    def __init__(self, gap_threshold_ns: int = DEFAULT_GAP_THRESHOLD_NS) -> None:
        self._segments: list[RecordingSegment] = []
        self._gap_threshold = gap_threshold_ns

    def add_segment(self, segment: RecordingSegment) -> None:
        self._segments.append(segment)

    @property
    def segments(self) -> list[RecordingSegment]:
        return list(self._segments)

    @property
    def total_duration_ns(self) -> int:
        if not self._segments:
            return 0
        return self._segments[-1].end_ns - self._segments[0].start_ns

    @property
    def total_samples(self) -> int:
        return sum(s.sample_count for s in self._segments)

    def gaps(self) -> list[Gap]:
        result: list[Gap] = []
        for prev, curr in zip(self._segments, self._segments[1:]):
            delta = curr.start_ns - prev.end_ns
            if delta > self._gap_threshold:
                result.append(Gap(prev.end_ns, curr.start_ns))
        return result

    def seek(self, position_ns: int) -> tuple[RecordingSegment, int] | None:
        for seg in self._segments:
            if seg.contains(position_ns):
                return seg, position_ns - seg.start_ns
        return None
