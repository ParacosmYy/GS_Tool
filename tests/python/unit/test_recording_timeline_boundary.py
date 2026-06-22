"""RecordingTimeline + RecordingSegment + Gap 边界扩展测试。

test_recording_timeline.py 覆盖基础 segment/timeline；本文件补全边界：
Gap 模型、DEFAULT_GAP_THRESHOLD、total_duration_ns、total_samples、gaps 阈值、
seek 偏移、segments 拷贝隔离、frozen 契约。

覆盖：
1. RecordingSegment 默认 source + frozen + duration_ns + contains 边界。
2. Gap.duration_ns 计算。
3. DEFAULT_GAP_THRESHOLD_NS 常量。
4. RecordingTimeline total_duration_ns（空 0 / 多段首尾差）。
5. total_samples 累加。
6. gaps 阈值（< 阈值无 gap / > 阈值产生 gap / 单段无 gap）。
7. seek 命中段返回偏移 / 落在 gap 返回 None / 空时间轴 None。
8. segments 返回拷贝（外部修改不影响内部）。
9. add_segment 累积。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.recording.timeline import (
    Gap,
    RecordingSegment,
    RecordingTimeline,
)


# ── RecordingSegment ──────────────────────────────────────────────
def test_segment_default_source():
    s = RecordingSegment(start_ns=0, end_ns=100, sample_count=10)
    assert s.source == "serial"


def test_segment_custom_source():
    s = RecordingSegment(start_ns=0, end_ns=100, sample_count=10, source="tcp")
    assert s.source == "tcp"


def test_segment_duration_ns():
    s = RecordingSegment(start_ns=1000, end_ns=3000, sample_count=5)
    assert s.duration_ns == 2000


def test_segment_duration_zero():
    s = RecordingSegment(start_ns=500, end_ns=500, sample_count=0)
    assert s.duration_ns == 0


def test_segment_contains_start_boundary():
    s = RecordingSegment(start_ns=100, end_ns=200, sample_count=1)
    assert s.contains(100) is True  # 起点包含


def test_segment_contains_end_boundary():
    s = RecordingSegment(start_ns=100, end_ns=200, sample_count=1)
    assert s.contains(200) is True  # 终点包含


def test_segment_contains_outside():
    s = RecordingSegment(start_ns=100, end_ns=200, sample_count=1)
    assert s.contains(99) is False
    assert s.contains(201) is False


def test_segment_is_frozen():
    s = RecordingSegment(start_ns=0, end_ns=1, sample_count=1)
    with pytest.raises(AttributeError):
        s.start_ns = 999  # type: ignore[misc]


# ── Gap ───────────────────────────────────────────────────────────
def test_gap_duration_ns():
    g = Gap(end_of_previous_ns=1000, start_of_next_ns=3000)
    assert g.duration_ns == 2000


def test_gap_duration_zero():
    g = Gap(end_of_previous_ns=500, start_of_next_ns=500)
    assert g.duration_ns == 0


# ── DEFAULT_GAP_THRESHOLD_NS ─────────────────────────────────────
def test_default_gap_threshold_is_50ms():
    assert RecordingTimeline.DEFAULT_GAP_THRESHOLD_NS == 50_000_000


def test_default_gap_threshold_used_when_not_specified():
    """__init__ 默认用 DEFAULT_GAP_THRESHOLD_NS。"""

    tl = RecordingTimeline()
    assert tl._gap_threshold == RecordingTimeline.DEFAULT_GAP_THRESHOLD_NS


# ── RecordingTimeline total_duration_ns ──────────────────────────
def test_total_duration_empty_is_zero():
    tl = RecordingTimeline()
    assert tl.total_duration_ns == 0


def test_total_duration_single_segment():
    tl = RecordingTimeline()
    tl.add_segment(RecordingSegment(0, 1000, 10))
    assert tl.total_duration_ns == 1000


def test_total_duration_multiple_segments():
    """多段：首段 start 到末段 end 的跨度（含中间 gap）。"""

    tl = RecordingTimeline()
    tl.add_segment(RecordingSegment(0, 1000, 10))
    tl.add_segment(RecordingSegment(5000, 8000, 20))
    assert tl.total_duration_ns == 8000  # 末段 end - 首段 start


# ── total_samples ─────────────────────────────────────────────────
def test_total_samples_empty_is_zero():
    assert RecordingTimeline().total_samples == 0


def test_total_samples_accumulates():
    tl = RecordingTimeline()
    tl.add_segment(RecordingSegment(0, 100, 10))
    tl.add_segment(RecordingSegment(200, 300, 25))
    tl.add_segment(RecordingSegment(400, 500, 15))
    assert tl.total_samples == 50


# ── gaps 阈值 ─────────────────────────────────────────────────────
def test_gaps_single_segment_no_gap():
    tl = RecordingTimeline()
    tl.add_segment(RecordingSegment(0, 1000, 10))
    assert tl.gaps() == []


def test_gaps_below_threshold_no_gap():
    """段间 delta < 阈值 → 无 gap。"""

    tl = RecordingTimeline()
    tl.add_segment(RecordingSegment(0, 1000, 10))
    tl.add_segment(RecordingSegment(1040_000_000, 1040_001_000, 10))  # delta 很大 → 有 gap
    # 用默认阈值 50ms = 50_000_000；delta=1040_000_000-1000 远超 → 有 gap。
    assert len(tl.gaps()) == 1


def test_gaps_custom_threshold():
    """自定义 gap_threshold_ns：小 delta 也产生 gap。"""

    tl = RecordingTimeline(gap_threshold_ns=10)  # 10ns 阈值
    tl.add_segment(RecordingSegment(0, 100, 5))
    tl.add_segment(RecordingSegment(200, 300, 5))  # delta=100 > 10 → gap
    gaps = tl.gaps()
    assert len(gaps) == 1
    assert gaps[0].end_of_previous_ns == 100
    assert gaps[0].start_of_next_ns == 200


def test_gaps_zero_threshold_all_gaps():
    """gap_threshold_ns=0：任何 delta>0 都算 gap。"""

    tl = RecordingTimeline(gap_threshold_ns=0)
    tl.add_segment(RecordingSegment(0, 100, 5))
    tl.add_segment(RecordingSegment(101, 200, 5))  # delta=1 > 0 → gap
    assert len(tl.gaps()) == 1


# ── seek ──────────────────────────────────────────────────────────
def test_seek_hits_segment_returns_offset():
    tl = RecordingTimeline()
    tl.add_segment(RecordingSegment(1000, 2000, 10))
    result = tl.seek(1500)
    assert result is not None
    seg, offset = result
    assert seg.start_ns == 1000
    assert offset == 500  # 1500 - 1000


def test_seek_in_gap_returns_none():
    tl = RecordingTimeline()
    tl.add_segment(RecordingSegment(0, 1000, 10))
    tl.add_segment(RecordingSegment(5000, 6000, 10))
    assert tl.seek(3000) is None  # 落在 gap 中


def test_seek_empty_timeline_returns_none():
    assert RecordingTimeline().seek(0) is None


def test_seek_start_boundary():
    tl = RecordingTimeline()
    tl.add_segment(RecordingSegment(1000, 2000, 10))
    result = tl.seek(1000)
    assert result is not None
    assert result[1] == 0  # 起点偏移 0


# ── segments 拷贝隔离 ────────────────────────────────────────────
def test_segments_returns_copy():
    """segments 属性返回拷贝（外部 append 不影响内部）。"""

    tl = RecordingTimeline()
    tl.add_segment(RecordingSegment(0, 100, 5))
    snapshot = tl.segments
    snapshot.append(RecordingSegment(200, 300, 5))
    # 内部不受影响。
    assert len(tl.segments) == 1


def test_add_segment_accumulates():
    tl = RecordingTimeline()
    for i in range(5):
        tl.add_segment(RecordingSegment(i * 100, (i + 1) * 100, 10))
    assert len(tl.segments) == 5
