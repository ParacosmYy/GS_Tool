"""PerfMetric 性能指标单元测试 — update/reset 统计。

覆盖：update 更新 min/max/avg/count、首次采样初始化 min=max、
多次采样均值、reset 清空。
"""

from __future__ import annotations

from embeddebug.serial_station.diagnostics.metrics import PerfMetric


def test_perfmetric_defaults():
    m = PerfMetric(name="fps")
    assert m.value == 0.0
    assert m.count == 0
    assert m.avg == 0.0


def test_update_first_sample():
    m = PerfMetric(name="fps")
    m.update(60.0)
    assert m.value == 60.0
    assert m.count == 1
    assert m.min == 60.0
    assert m.max == 60.0
    assert m.avg == 60.0


def test_update_multiple_samples():
    m = PerfMetric(name="latency")
    m.update(10.0)
    m.update(20.0)
    m.update(30.0)
    assert m.count == 3
    assert m.min == 10.0
    assert m.max == 30.0
    assert m.avg == 20.0


def test_update_new_min():
    m = PerfMetric(name="fps")
    m.update(60.0)
    m.update(30.0)
    assert m.min == 30.0
    assert m.max == 60.0


def test_update_new_max():
    m = PerfMetric(name="fps")
    m.update(60.0)
    m.update(120.0)
    assert m.min == 60.0
    assert m.max == 120.0


def test_reset():
    m = PerfMetric(name="fps")
    m.update(60.0)
    m.update(30.0)
    m.reset()
    assert m.count == 0
    assert m.value == 0.0
    assert m.avg == 0.0
    assert m.min == 0.0
    assert m.max == 0.0


def test_reset_then_update():
    """reset 后可重新采样。"""
    m = PerfMetric(name="fps")
    m.update(100.0)
    m.reset()
    m.update(50.0)
    assert m.count == 1
    assert m.min == 50.0
    assert m.max == 50.0
    assert m.avg == 50.0


def test_unit_field():
    m = PerfMetric(name="latency", unit="ms")
    assert m.unit == "ms"


def test_update_negative_value():
    """负值正常记录。"""
    m = PerfMetric(name="temp")
    m.update(-40.0)
    assert m.min == -40.0
    assert m.value == -40.0
