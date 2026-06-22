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


# ---- Batch 133: format_text 排序 / unit 空字符串 / 边界值 / degraded True ----


def test_format_text_sorts_metrics_alphabetically():
    """format_text 按字母序输出 metrics（sorted(self.metrics)）。"""
    snap = PerfSnapshot(metrics={
        "zebra": PerfMetric(name="zebra", value=1.0, count=1),
        "alpha": PerfMetric(name="alpha", value=2.0, count=1),
        "mid": PerfMetric(name="mid", value=3.0, count=1),
    })
    text = snap.format_text()
    alpha_pos = text.index("alpha")
    mid_pos = text.index("mid")
    zebra_pos = text.index("zebra")
    assert alpha_pos < mid_pos < zebra_pos


def test_format_text_metric_without_unit_omits_unit_segment():
    """metric.unit 为空时 format_text 不附加 unit 段（unit = ""）。"""
    snap = PerfSnapshot(metrics={
        "counter": PerfMetric(name="counter", value=5.0, count=1),  # unit 默认 ""
    })
    text = snap.format_text()
    line = next(ln for ln in text.split("\n") if "counter" in ln)
    # 格式："- counter: now=5.000 (min=...)"，now= 后直接接 "(" 而非 "Hz ("
    assert "now=5.000 (" in line


def test_format_text_shows_degraded_true_when_unhealthy():
    """degraded=True 时 format_text 显示 'degraded: True'。"""
    snap = PerfSnapshot(metrics={"fps": PerfMetric(name="fps", value=10.0, count=1)})
    assert "degraded: True" in snap.format_text()


def test_is_degraded_fps_exactly_at_threshold_not_degraded():
    """fps == 30.0（阈值边界）不算劣化（严格 <）。"""
    snap = PerfSnapshot(metrics={"fps": PerfMetric(name="fps", value=30.0, count=1)})
    assert snap.is_degraded() is False


def test_is_degraded_latency_exactly_at_threshold_not_degraded():
    """latency == 100.0（阈值边界）不算劣化（严格 >）。"""
    snap = PerfSnapshot(metrics={
        "latency_ms": PerfMetric(name="latency_ms", value=100.0, count=1)
    })
    assert snap.is_degraded() is False


def test_format_text_includes_min_max_avg_count():
    """format_text 输出 min/max/avg/n 统计段。

    PerfMetric 构造时不计算 avg（需 update() 累积）；这里用 3 次 update
    让 avg = (30+60+90)/3 = 60.0。
    """
    metric = PerfMetric(name="fps", unit="Hz")
    metric.update(30.0)
    metric.update(60.0)
    metric.update(90.0)  # value=90, min=30, max=90, avg=60, count=3
    snap = PerfSnapshot(metrics={"fps": metric})
    text = snap.format_text()
    assert "min=30.000" in text
    assert "max=90.000" in text
    assert "avg=60.000" in text
    assert "n=3" in text


def test_snapshot_default_timestamp_ns_is_recent():
    """PerfSnapshot 默认 timestamp_ns 落在构造前后区间（time.time_ns 工厂）。"""
    import time
    before = time.time_ns()
    snap = PerfSnapshot()
    after = time.time_ns()
    assert before <= snap.timestamp_ns <= after


def test_snapshot_default_uptime_is_zero():
    """PerfSnapshot 默认 uptime_s=0.0。"""
    snap = PerfSnapshot()
    assert snap.uptime_s == 0.0


def test_is_degraded_unknown_metric_name_ignored():
    """非 fps/latency_ms 的指标不参与劣化判定。"""
    snap = PerfSnapshot(metrics={
        "custom": PerfMetric(name="custom", value=99999.0, count=1),
    })
    assert snap.is_degraded() is False
