"""性能快照 PerfSnapshot 单元测试 — is_degraded + format_text。

覆盖：is_degraded fps/latency 阈值检测、format_text 格式化输出、
空 metrics 不劣化、阈值常量。
"""

from __future__ import annotations

from embeddebug.serial_station.diagnostics.metrics import PerfMetric
from embeddebug.serial_station.diagnostics.snapshot import (
    FPS_DEGRADED_THRESHOLD,
    LATENCY_DEGRADED_THRESHOLD_MS,
    PerfSnapshot,
)


def test_thresholds():
    assert FPS_DEGRADED_THRESHOLD == 30.0
    assert LATENCY_DEGRADED_THRESHOLD_MS == 100.0


def test_is_degraded_empty_metrics():
    snap = PerfSnapshot()
    assert snap.is_degraded() is False


def test_is_degraded_fps_low():
    snap = PerfSnapshot(metrics={"fps": PerfMetric(name="fps", value=20.0, count=1)})
    assert snap.is_degraded() is True


def test_is_degraded_fps_ok():
    snap = PerfSnapshot(metrics={"fps": PerfMetric(name="fps", value=60.0, count=1)})
    assert snap.is_degraded() is False


def test_is_degraded_latency_high():
    snap = PerfSnapshot(metrics={"latency_ms": PerfMetric(name="latency_ms", value=150.0, count=1)})
    assert snap.is_degraded() is True


def test_is_degraded_latency_ok():
    snap = PerfSnapshot(metrics={"latency_ms": PerfMetric(name="latency_ms", value=50.0, count=1)})
    assert snap.is_degraded() is False


def test_is_degraded_fps_count_zero_ignored():
    """count=0 的指标被忽略（无采样数据）。"""
    snap = PerfSnapshot(metrics={"fps": PerfMetric(name="fps", value=10.0, count=0)})
    assert snap.is_degraded() is False


def test_is_degraded_both_metrics():
    snap = PerfSnapshot(metrics={
        "fps": PerfMetric(name="fps", value=60.0, count=1),
        "latency_ms": PerfMetric(name="latency_ms", value=200.0, count=1),
    })
    assert snap.is_degraded() is True


def test_format_text_contains_metrics():
    snap = PerfSnapshot(uptime_s=1.5, metrics={
        "fps": PerfMetric(name="fps", value=60.0, min=30.0, max=90.0, count=100),
    })
    text = snap.format_text()
    assert "uptime=1.500s" in text
    assert "fps" in text
    assert "degraded: False" in text


def test_format_text_empty_metrics():
    snap = PerfSnapshot()
    text = snap.format_text()
    assert "metrics: 0" in text
