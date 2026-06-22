"""DashboardBindingService 绑定表 + 路由分发单元测试。

覆盖 services/dashboard_binding_service.py 的 DashboardBindingService 类（有状态）：
- bind/unbind/get_binding/all_bindings/clear_all 绑定表管理。
- route_measurement：触发绑定该 channel 的 value/gauge handler。
- route_log：触发绑定该 level 的 led handler（handler 收到 (level, message) 元组）。

纯函数解析（parse_binding_spec/is_valid_spec/default_spec_for_widget_type）在
test_dashboard_binding_parse.py 单独覆盖。
"""

from __future__ import annotations

from embeddebug.serial_station.services.dashboard_binding_service import (
    DashboardBindingService,
)


def _make_service() -> DashboardBindingService:
    """构造无 handler 的绑定服务（route_* 无 handler 时返回命中数但不调用）。"""

    return DashboardBindingService()


# ── 绑定表：bind / unbind / get_binding / all_bindings / clear_all ────────


def test_bind_and_get_binding():
    """bind 后 get_binding 返回 spec。"""

    svc = _make_service()
    svc.bind("w1", "value:ch0", handler=None)
    assert svc.get_binding("w1") == "value:ch0"


def test_get_binding_unknown_returns_none():
    """get_binding 未绑定的 widget 返回 None。"""

    svc = _make_service()
    assert svc.get_binding("nonexistent") is None


def test_unbind_removes_binding():
    """unbind 后 get_binding 返回 None。"""

    svc = _make_service()
    svc.bind("w1", "value:ch0", handler=None)
    svc.unbind("w1")
    assert svc.get_binding("w1") is None


def test_unbind_unknown_widget_no_error():
    """unbind 未绑定的 widget 不抛异常。"""

    svc = _make_service()
    svc.unbind("nonexistent")  # 不应抛


def test_all_bindings_returns_all():
    """all_bindings 返回全部 widget_id → spec 映射。"""

    svc = _make_service()
    svc.bind("w1", "value:ch0", handler=None)
    svc.bind("w2", "led:error", handler=None)
    assert svc.all_bindings() == {"w1": "value:ch0", "w2": "led:error"}


def test_all_bindings_empty_initially():
    """新构造的服务 all_bindings 返回空 dict。"""

    assert _make_service().all_bindings() == {}


def test_clear_all_empties_bindings():
    """clear_all 清空所有绑定。"""

    svc = _make_service()
    svc.bind("w1", "value:ch0", handler=None)
    svc.bind("w2", "led:error", handler=None)
    svc.clear_all()
    assert svc.all_bindings() == {}


def test_bind_overwrites_existing():
    """同 widget_id 二次 bind 覆盖旧 spec。"""

    svc = _make_service()
    svc.bind("w1", "value:ch0", handler=None)
    svc.bind("w1", "gauge:ch1", handler=None)
    assert svc.get_binding("w1") == "gauge:ch1"


# ── route_measurement：值/仪表路由分发 ──────────────────────────────────


def test_route_measurement_calls_matching_handler():
    """route_measurement 触发绑定该 channel 的 widget handler。"""

    calls: list[float] = []
    svc = DashboardBindingService()
    svc.bind("w1", "value:ch0", handler=lambda v: calls.append(v))
    routed = svc.route_measurement(channel=0, value=42.5)
    assert routed == 1
    assert calls == [42.5]


def test_route_measurement_skips_non_matching_channel():
    """route_measurement 不触发绑定其他 channel 的 widget。"""

    calls: list[float] = []
    svc = DashboardBindingService()
    svc.bind("w1", "value:ch0", handler=lambda v: calls.append(v))
    routed = svc.route_measurement(channel=1, value=99.0)
    assert routed == 0
    assert calls == []


def test_route_measurement_hits_multiple_widgets_same_channel():
    """多个 widget 绑定同一 channel，route_measurement 全部触发。"""

    calls_a: list[float] = []
    calls_b: list[float] = []
    svc = DashboardBindingService()
    svc.bind("w1", "value:ch0", handler=lambda v: calls_a.append(v))
    svc.bind("w2", "gauge:ch0", handler=lambda v: calls_b.append(v))
    routed = svc.route_measurement(channel=0, value=1.5)
    assert routed == 2
    assert calls_a == [1.5]
    assert calls_b == [1.5]


def test_route_measurement_handler_exception_does_not_break_others():
    """handler 抛异常不影响其他控件的路由（异常被 catch）。"""

    calls: list[float] = []
    svc = DashboardBindingService()
    svc.bind("w1", "value:ch0", handler=lambda v: (_ for _ in ()).throw(RuntimeError("boom")))
    svc.bind("w2", "value:ch0", handler=lambda v: calls.append(v))
    routed = svc.route_measurement(channel=0, value=1.0)
    assert routed == 2  # 两个都算命中
    assert calls == [1.0]  # w2 的 handler 仍被调用


# ── route_log：LED 级别路由分发 ─────────────────────────────────────────


def test_route_log_triggers_led_handler_for_matching_level():
    """route_log(error) 触发 led:error 绑定的 widget handler（handler 收到 (level, message) 元组）。"""

    calls: list[tuple] = []
    svc = DashboardBindingService()
    svc.bind("w1", "led:error", handler=lambda payload: calls.append(payload))
    routed = svc.route_log(level="error", message="fail")
    assert routed == 1
    assert calls == [("error", "fail")]


def test_route_log_info_level_triggers_info_led_only():
    """route_log(info) 只触发 led:info，不触发 led:error。"""

    error_calls: list[tuple] = []
    info_calls: list[tuple] = []
    svc = DashboardBindingService()
    svc.bind("w1", "led:error", handler=lambda payload: error_calls.append(payload))
    svc.bind("w2", "led:info", handler=lambda payload: info_calls.append(payload))
    routed = svc.route_log(level="info", message="ok")
    assert routed == 1
    assert error_calls == []
    assert info_calls == [("info", "ok")]


def test_route_log_unknown_level_returns_zero():
    """route_log(debug)（非 LED_LEVELS）返回 0，不触发任何 handler。"""

    calls: list[tuple] = []
    svc = DashboardBindingService()
    svc.bind("w1", "led:info", handler=lambda payload: calls.append(payload))
    routed = svc.route_log(level="debug", message="trace")
    assert routed == 0
    assert calls == []
