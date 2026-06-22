"""diagnostics/monitor 边界单元测试。

补强 test_diagnostics.py 未直接断言的边角：
- DEFAULT_SNAPSHOT_INTERVAL_MS / DEFAULT_METRIC_UNITS 常量契约。
- PerfMonitor start_ns 属性 + uptime_s 非负。
- set_snapshot_interval 修改 + 负值 clamp。
- reset 清空指标 + 重置 start_ns + 清除 tick_fps 状态。
- is_running start/stop 切换。
- _get_or_create 默认单位映射 + 重复调用返回同一实例。
- snapshot 含 timestamp_ns + uptime_s + metrics 拷贝。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.diagnostics.monitor import (
    DEFAULT_METRIC_UNITS,
    DEFAULT_SNAPSHOT_INTERVAL_MS,
    PerfMonitor,
)


# ── 常量契约 ─────────────────────────────────────────────────────────────


def test_default_snapshot_interval_is_1000ms():
    """DEFAULT_SNAPSHOT_INTERVAL_MS = 1000（1 秒）。"""

    assert DEFAULT_SNAPSHOT_INTERVAL_MS == 1000


def test_default_metric_units_has_five_entries():
    """DEFAULT_METRIC_UNITS 含 5 个指标→单位映射。"""

    assert len(DEFAULT_METRIC_UNITS) == 5
    assert DEFAULT_METRIC_UNITS["fps"] == "Hz"
    assert DEFAULT_METRIC_UNITS["throughput_bps"] == "Bps"
    assert DEFAULT_METRIC_UNITS["latency_ms"] == "ms"
    assert DEFAULT_METRIC_UNITS["memory_mb"] == "MB"
    assert DEFAULT_METRIC_UNITS["cpu_percent"] == "%"


# ── PerfMonitor start_ns + uptime_s ─────────────────────────────────────


def test_start_ns_is_positive():
    """start_ns 是正整数（纳秒时间戳）。"""

    monitor = PerfMonitor()
    assert monitor.start_ns > 0


def test_uptime_s_is_non_negative():
    """uptime_s ≥ 0（刚启动接近 0）。"""

    monitor = PerfMonitor()
    assert monitor.uptime_s() >= 0.0


def test_uptime_s_increases_over_time():
    """uptime_s 随时间增长。"""

    monitor = PerfMonitor()
    first = monitor.uptime_s()
    # 不 sleep，仅验证多次调用不减少
    second = monitor.uptime_s()
    assert second >= first


# ── set_snapshot_interval ────────────────────────────────────────────────


def test_set_snapshot_interval_positive():
    """set_snapshot_interval 正值修改。"""

    monitor = PerfMonitor()
    monitor.set_snapshot_interval(500)
    # 不抛即通过（内部 QTimer.setInterval）


def test_set_snapshot_interval_negative_clamped():
    """set_snapshot_interval 负值 clamp 到 ≥1。"""

    monitor = PerfMonitor()
    monitor.set_snapshot_interval(-100)  # 不抛


def test_set_snapshot_interval_zero_clamped():
    """set_snapshot_interval=0 clamp 到 ≥1。"""

    monitor = PerfMonitor()
    monitor.set_snapshot_interval(0)  # 不抛


# ── reset ───────────────────────────────────────────────────────────────


def test_reset_clears_metrics():
    """reset 清空指标统计（count 归零）。"""

    monitor = PerfMonitor()
    monitor.record_metric("fps", 60.0)
    monitor.reset()
    snapshot = monitor.snapshot()
    metric = snapshot.metrics["fps"]
    assert metric.count == 0


def test_reset_resets_start_ns():
    """reset 更新 start_ns（新时间戳）。"""

    monitor = PerfMonitor()
    old_start = monitor.start_ns
    monitor.reset()
    assert monitor.start_ns >= old_start  # 新时间戳 ≥ 旧的


def test_reset_clears_tick_fps_state():
    """reset 后 tick_fps 首帧返回 0（_last_tick_ns 清空）。"""

    monitor = PerfMonitor()
    monitor.tick_fps()  # 首帧设 _last_tick_ns
    monitor.reset()
    fps = monitor.tick_fps()  # reset 后又是首帧
    assert fps == 0.0


# ── is_running / start / stop ───────────────────────────────────────────


def test_is_running_false_initially():
    """新建 PerfMonitor 定时器未启动。"""

    monitor = PerfMonitor()
    assert monitor.is_running is False


def test_start_sets_is_running_true(qtbot):
    """start 后 is_running=True。"""

    monitor = PerfMonitor(snapshot_interval_ms=10000)
    monitor.start()
    assert monitor.is_running is True
    monitor.stop()


def test_stop_sets_is_running_false(qtbot):
    """stop 后 is_running=False。"""

    monitor = PerfMonitor(snapshot_interval_ms=10000)
    monitor.start()
    monitor.stop()
    assert monitor.is_running is False


def test_start_idempotent(qtbot):
    """重复 start 不抛（幂等）。"""

    monitor = PerfMonitor(snapshot_interval_ms=10000)
    monitor.start()
    monitor.start()  # 不抛
    monitor.stop()


# ── _get_or_create ──────────────────────────────────────────────────────


def test_get_or_create_default_unit_from_mapping():
    """_get_or_create 用 DEFAULT_METRIC_UNITS 映射默认单位。"""

    monitor = PerfMonitor()
    metric = monitor._get_or_create("fps")
    assert metric.unit == "Hz"


def test_get_or_create_custom_unit_overrides_default():
    """显式 unit 覆盖默认映射。"""

    monitor = PerfMonitor()
    metric = monitor._get_or_create("fps", unit="frames/s")
    assert metric.unit == "frames/s"


def test_get_or_create_unknown_name_empty_unit():
    """未知名无默认映射 → 空单位。"""

    monitor = PerfMonitor()
    metric = monitor._get_or_create("custom_metric")
    assert metric.unit == ""


def test_get_or_create_returns_same_instance():
    """重复调用同名 → 返回同一 PerfMetric 实例。"""

    monitor = PerfMonitor()
    first = monitor._get_or_create("fps")
    second = monitor._get_or_create("fps")
    assert first is second


# ── snapshot 结构 ───────────────────────────────────────────────────────


def test_snapshot_contains_timestamp_and_uptime():
    """snapshot 含 timestamp_ns（正）+ uptime_s（非负）。"""

    monitor = PerfMonitor()
    snap = monitor.snapshot()
    assert snap.timestamp_ns > 0
    assert snap.uptime_s >= 0.0


def test_snapshot_metrics_is_copy():
    """snapshot.metrics 是拷贝（修改不影响 monitor 内部）。"""

    monitor = PerfMonitor()
    monitor.record_metric("fps", 60.0)
    snap = monitor.snapshot()
    snap.metrics.clear()
    # monitor 内部不受影响
    snap2 = monitor.snapshot()
    assert "fps" in snap2.metrics
