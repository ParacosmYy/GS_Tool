"""性能诊断模块单元测试（PerfMetric / PerfMonitor / PerfSnapshot）。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.diagnostics import (
    PerfMetric,
    PerfMonitor,
    PerfSnapshot,
)


# ----------------------------- PerfMetric -----------------------------


def test_perf_metric_single_update_sets_value_and_stats():
    metric = PerfMetric(name="fps", unit="Hz")
    metric.update(60.0)
    assert metric.value == 60.0
    assert metric.min == 60.0
    assert metric.max == 60.0
    assert metric.avg == 60.0
    assert metric.count == 1
    assert metric.timestamp_ns > 0


def test_perf_metric_running_stats_track_min_max_avg():
    metric = PerfMetric(name="latency_ms", unit="ms")
    for sample in (10.0, 40.0, 25.0):
        metric.update(sample)
    assert metric.count == 3
    assert metric.min == 10.0
    assert metric.max == 40.0
    assert metric.value == 25.0
    assert abs(metric.avg - 25.0) < 1e-9


def test_perf_metric_reset_clears_state():
    metric = PerfMetric(name="fps", unit="Hz")
    metric.update(50.0)
    metric.update(70.0)
    assert metric.count == 2
    metric.reset()
    assert metric.count == 0
    assert metric.value == 0.0
    assert metric.min == 0.0
    assert metric.max == 0.0
    assert metric.avg == 0.0


def test_perf_metric_to_dict_contains_all_fields():
    metric = PerfMetric(name="throughput_bps", unit="Bps")
    metric.update(1024.0)
    data = metric.to_dict()
    assert data["name"] == "throughput_bps"
    assert data["unit"] == "Bps"
    assert data["value"] == 1024.0
    assert data["count"] == 1
    for key in ("min", "max", "avg", "timestamp_ns"):
        assert key in data


def test_perf_metric_default_unit_is_empty():
    metric = PerfMetric(name="x")
    assert metric.unit == ""
    metric.update(1.0)
    assert metric.unit == ""


# ----------------------------- PerfMonitor -----------------------------


def test_monitor_record_metric_creates_and_updates(qtbot):
    monitor = PerfMonitor()
    monitor.record_metric("throughput_bps", 2048.0)
    snapshot = monitor.snapshot()
    metric = snapshot.metrics["throughput_bps"]
    assert metric.unit == "Bps"
    assert metric.value == 2048.0
    assert metric.count == 1


def test_monitor_record_metric_custom_unit_overrides_default(qtbot):
    monitor = PerfMonitor()
    monitor.record_metric("memory_mb", 512.0, unit="megabyte")
    metric = monitor.snapshot().metrics["memory_mb"]
    assert metric.unit == "megabyte"
    assert metric.value == 512.0


def test_monitor_tick_fps_first_frame_returns_zero(qtbot):
    monitor = PerfMonitor()
    fps = monitor.tick_fps()
    assert fps == 0.0
    snapshot = monitor.snapshot()
    metric = snapshot.metrics["fps"]
    assert metric.unit == "Hz"
    assert metric.count == 0


def test_monitor_tick_fps_subsequent_frames_produce_positive(qtbot):
    monitor = PerfMonitor()
    monitor.tick_fps()
    qtbot.wait(5)
    fps = monitor.tick_fps()
    assert fps > 0.0
    snapshot = monitor.snapshot()
    metric = snapshot.metrics["fps"]
    assert metric.count == 1
    assert metric.value == fps


def test_monitor_snapshot_has_uptime_and_timestamp(qtbot):
    monitor = PerfMonitor()
    qtbot.wait(10)
    snapshot = monitor.snapshot()
    assert snapshot.timestamp_ns > 0
    assert snapshot.uptime_s >= 0.0


def test_monitor_reset_clears_metrics_and_fps_state(qtbot):
    monitor = PerfMonitor()
    monitor.record_metric("latency_ms", 10.0)
    monitor.tick_fps()
    qtbot.wait(5)
    monitor.tick_fps()
    assert monitor.snapshot().metrics["latency_ms"].count == 1
    monitor.reset()
    snapshot = monitor.snapshot()
    assert snapshot.metrics["latency_ms"].count == 0
    assert monitor.snapshot().metrics["fps"].count == 0


# ----------------------------- PerfSnapshot -----------------------------


def test_snapshot_format_text_contains_metric_names():
    metric = PerfMetric(name="fps", unit="Hz")
    metric.update(55.0)
    snapshot = PerfSnapshot(uptime_s=1.5, metrics={"fps": metric})
    text = snapshot.format_text()
    assert "PerfSnapshot" in text
    assert "uptime=1.500s" in text
    assert "fps" in text
    assert "n=1" in text


def test_snapshot_is_degraded_when_fps_below_threshold():
    metric = PerfMetric(name="fps", unit="Hz")
    metric.update(20.0)
    snapshot = PerfSnapshot(metrics={"fps": metric})
    assert snapshot.is_degraded() is True


def test_snapshot_not_degraded_when_fps_healthy():
    metric = PerfMetric(name="fps", unit="Hz")
    metric.update(60.0)
    snapshot = PerfSnapshot(metrics={"fps": metric})
    assert snapshot.is_degraded() is False


def test_snapshot_is_degraded_when_latency_above_threshold():
    metric = PerfMetric(name="latency_ms", unit="ms")
    metric.update(150.0)
    snapshot = PerfSnapshot(metrics={"latency_ms": metric})
    assert snapshot.is_degraded() is True


def test_snapshot_not_degraded_when_latency_healthy():
    metric = PerfMetric(name="latency_ms", unit="ms")
    metric.update(30.0)
    snapshot = PerfSnapshot(metrics={"latency_ms": metric})
    assert snapshot.is_degraded() is False


def test_snapshot_empty_metrics_not_degraded():
    snapshot = PerfSnapshot()
    assert snapshot.is_degraded() is False
    assert "metrics: 0" in snapshot.format_text()


def test_snapshot_fps_metric_not_degraded_without_samples():
    metric = PerfMetric(name="fps", unit="Hz")
    snapshot = PerfSnapshot(metrics={"fps": metric})
    assert metric.count == 0
    assert snapshot.is_degraded() is False


# ------------------- 多指标 / QTimer 周期快照集成 -------------------


def test_monitor_multiple_metrics_in_snapshot(qtbot):
    monitor = PerfMonitor()
    monitor.record_metric("throughput_bps", 4096.0)
    monitor.record_metric("latency_ms", 12.0)
    monitor.record_metric("cpu_percent", 35.0)
    snapshot = monitor.snapshot()
    names = set(snapshot.metrics)
    assert {"throughput_bps", "latency_ms", "cpu_percent"} <= names
    assert snapshot.metrics["throughput_bps"].unit == "Bps"
    assert snapshot.metrics["cpu_percent"].unit == "%"


def test_monitor_periodic_snapshot_signal_emits(qtbot):
    monitor = PerfMonitor(snapshot_interval_ms=20)
    monitor.start()
    try:
        snapshots: list[PerfSnapshot] = []
        monitor.snapshot_ready.connect(snapshots.append)
        qtbot.waitUntil(lambda: len(snapshots) >= 1, timeout=500)
        assert snapshots
        assert snapshots[0].timestamp_ns > 0
    finally:
        monitor.stop()
    assert monitor.is_running is False


def test_monitor_stop_then_no_more_snapshots(qtbot):
    monitor = PerfMonitor(snapshot_interval_ms=10)
    monitor.start()
    snapshots: list[PerfSnapshot] = []
    monitor.snapshot_ready.connect(snapshots.append)
    qtbot.waitUntil(lambda: len(snapshots) >= 1, timeout=500)
    first = len(snapshots)
    monitor.stop()
    qtbot.wait(40)
    assert len(snapshots) == first
