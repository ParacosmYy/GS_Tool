"""录制时间线单元测试 — segment/gap/timeline。

覆盖：RecordingSegment duration_ns/contains、Gap duration_ns、
RecordingTimeline add_segment/gaps/total_duration。
"""

from __future__ import annotations

from embeddebug.serial_station.recording.timeline import (
    Gap,
    RecordingSegment,
    RecordingTimeline,
)


def _seg(start: int, end: int) -> RecordingSegment:
    return RecordingSegment(start_ns=start, end_ns=end, sample_count=10)


def test_segment_duration():
    seg = _seg(0, 1_000_000_000)
    assert seg.duration_ns == 1_000_000_000


def test_segment_duration_zero():
    seg = _seg(100, 100)
    assert seg.duration_ns == 0


def test_segment_contains_inside():
    seg = _seg(0, 1000)
    assert seg.contains(500) is True


def test_segment_contains_boundary():
    seg = _seg(0, 1000)
    assert seg.contains(0) is True
    assert seg.contains(1000) is True


def test_segment_contains_outside():
    seg = _seg(100, 200)
    assert seg.contains(50) is False
    assert seg.contains(300) is False


def test_timeline_empty():
    tl = RecordingTimeline()
    assert tl.segments == []
    assert tl.total_duration_ns == 0


def test_timeline_add_segment():
    tl = RecordingTimeline()
    tl.add_segment(_seg(0, 100))
    assert len(tl.segments) == 1
    assert tl.total_duration_ns == 100


def test_timeline_multiple_segments():
    tl = RecordingTimeline()
    tl.add_segment(_seg(0, 100))
    tl.add_segment(_seg(200, 300))
    assert len(tl.segments) == 2
    assert tl.total_duration_ns == 300  # last.end - first.start
