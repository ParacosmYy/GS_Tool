"""DashboardBindingService 单元测试 — binding spec 解析 + 路由服务。

覆盖 services/dashboard_binding_service.py 的纯函数 + 路由表：
- parse_binding_spec：value/gauge 通道解析 + led 级别 + slider 三段 + button 命令 + 无效返回 None。
- is_valid_spec：parse 的布尔包装。
- default_spec_for_widget_type：5 种 widget 默认 spec。
- DashboardBindingService：bind/unbind/get_binding/all_bindings/clear_all + route_measurement/route_log 分发。
"""

from __future__ import annotations

from embeddebug.serial_station.services.dashboard_binding_service import (
    KIND_BUTTON,
    KIND_GAUGE,
    KIND_LED,
    KIND_SLIDER,
    KIND_VALUE,
    DashboardBindingService,
    default_spec_for_widget_type,
    is_valid_spec,
    parse_binding_spec,
)


# ── parse_binding_spec：value / gauge 通道 ────────────────────────────────


def test_parse_value_channel():
    """value:ch0 → kind=value, channel=0。"""

    result = parse_binding_spec("value:ch0")
    assert result is not None
    assert result.kind == KIND_VALUE
    assert result.channel == 0


def test_parse_gauge_channel_multi_digit():
    """gauge:ch12 → kind=gauge, channel=12。"""

    result = parse_binding_spec("gauge:ch12")
    assert result is not None
    assert result.kind == KIND_GAUGE
    assert result.channel == 12


def test_parse_channel_kind_case_insensitive():
    """VALUE:CH3 大写也能解析（normalize to lower）。"""

    result = parse_binding_spec("VALUE:CH3")
    assert result is not None
    assert result.kind == KIND_VALUE
    assert result.channel == 3


def test_parse_channel_kind_rejects_missing_ch_prefix():
    """value:0（缺 ch 前缀）→ None。"""

    assert parse_binding_spec("value:0") is None


def test_parse_channel_kind_rejects_negative():
    """value:ch-1（负通道）→ None。"""

    assert parse_binding_spec("value:ch-1") is None


def test_parse_channel_kind_rejects_non_numeric():
    """value:chabc（非数字）→ None。"""

    assert parse_binding_spec("value:chabc") is None


# ── parse_binding_spec：led 级别 ──────────────────────────────────────────


def test_parse_led_error_level():
    """led:error → kind=led, level=error。"""

    result = parse_binding_spec("led:error")
    assert result is not None
    assert result.kind == KIND_LED
    assert result.level == "error"


def test_parse_led_warning_and_info_levels():
    """led:warning / led:info 都可解析。"""

    assert parse_binding_spec("led:warning").level == "warning"
    assert parse_binding_spec("led:info").level == "info"


def test_parse_led_rejects_unknown_level():
    """led:debug（未知级别）→ None。"""

    assert parse_binding_spec("led:debug") is None


# ── parse_binding_spec：slider 三段 ───────────────────────────────────────


def test_parse_slider_full_spec():
    """slider:SET_VOLTAGE,0,100 → prefix/min/max。"""

    result = parse_binding_spec("slider:SET_VOLTAGE,0,100")
    assert result is not None
    assert result.kind == KIND_SLIDER
    assert result.prefix == "SET_VOLTAGE"
    assert result.minimum == 0
    assert result.maximum == 100


def test_parse_slider_rejects_two_parts():
    """slider:SET,0（缺 max）→ None。"""

    assert parse_binding_spec("slider:SET,0") is None


def test_parse_slider_rejects_min_ge_max():
    """slider:SET,100,100（min >= max）→ None。"""

    assert parse_binding_spec("slider:SET,100,100") is None
    assert parse_binding_spec("slider:SET,200,100") is None


def test_parse_slider_rejects_non_integer_bounds():
    """slider:SET,0.5,100（非整数边界）→ None。"""

    assert parse_binding_spec("slider:SET,0.5,100") is None


def test_parse_slider_rejects_empty_prefix():
    """slider:,0,100（空前缀）→ None。"""

    assert parse_binding_spec("slider:,0,100") is None


# ── parse_binding_spec：button 命令 ───────────────────────────────────────


def test_parse_button_command():
    """button:RESET → kind=button, command=RESET。"""

    result = parse_binding_spec("button:RESET")
    assert result is not None
    assert result.kind == KIND_BUTTON
    assert result.command == "RESET"


def test_parse_button_preserves_spaces_in_command():
    """button 命令文本保留空格（命令可含空格）。"""

    result = parse_binding_spec("button:SET VOLTAGE 5")
    assert result is not None
    assert result.command == "SET VOLTAGE 5"


# ── parse_binding_spec：无效输入 ──────────────────────────────────────────


def test_parse_empty_spec_returns_none():
    assert parse_binding_spec("") is None


def test_parse_spec_without_colon_returns_none():
    """valuech0（缺冒号分隔）→ None。"""

    assert parse_binding_spec("valuech0") is None


def test_parse_spec_with_empty_detail_returns_none():
    """value:（detail 空）→ None。"""

    assert parse_binding_spec("value:") is None


def test_parse_unknown_kind_returns_none():
    """unknown:ch0（未知 kind）→ None。"""

    assert parse_binding_spec("unknown:ch0") is None


# ── is_valid_spec ────────────────────────────────────────────────────────


def test_is_valid_spec_true_for_valid():
    assert is_valid_spec("value:ch0") is True
    assert is_valid_spec("led:error") is True
    assert is_valid_spec("slider:SET,0,100") is True


def test_is_valid_spec_false_for_invalid():
    assert is_valid_spec("") is False
    assert is_valid_spec("value:0") is False
    assert is_valid_spec("unknown:ch0") is False


# ── default_spec_for_widget_type ─────────────────────────────────────────


def test_default_spec_for_all_widget_types():
    """5 种 widget 都有默认 spec 建议。"""

    assert default_spec_for_widget_type("value_display") == "value:ch0"
    assert default_spec_for_widget_type("gauge") == "gauge:ch0"
    assert default_spec_for_widget_type("led") == "led:info"
    assert default_spec_for_widget_type("slider") == "slider:SET,0,100"
    assert default_spec_for_widget_type("button") == "button:RESET"


def test_default_spec_unknown_widget_returns_none():
    assert default_spec_for_widget_type("nonexistent") is None


# ── DashboardBindingService：绑定表 ──────────────────────────────────────


def _make_service() -> DashboardBindingService:
    """构造无 handler 的绑定服务（route_* 无 handler 时返回 0）。"""

    return DashboardBindingService()


def test_bind_and_get_binding():
    """bind 后 get_binding 返回 spec。"""

    svc = _make_service()
    svc.bind("w1", "value:ch0", handler=None)
    assert svc.get_binding("w1") == "value:ch0"


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


# ── DashboardBindingService：路由分发 ────────────────────────────────────


def test_route_measurement_calls_matching_handler():
    """route_measurement 触发绑定该 channel 的 widget handler。"""

    calls: list[float] = []
    svc = DashboardBindingService()
    svc.bind("w1", "value:ch0", handler=lambda v: calls.append(v))
    routed = svc.route_measurement(channel=0, value=42.5)
    assert routed == 1  # 1 个 widget 被路由
    assert calls == [42.5]


def test_route_measurement_skips_non_matching_channel():
    """route_measurement 不触发绑定其他 channel 的 widget。"""

    calls: list[float] = []
    svc = DashboardBindingService()
    svc.bind("w1", "value:ch0", handler=lambda v: calls.append(v))
    routed = svc.route_measurement(channel=1, value=99.0)
    assert routed == 0
    assert calls == []


def test_route_log_triggers_led_handler_for_matching_level():
    """route_log(error) 触发 led:error 绑定的 widget handler（handler 收到 level+message）。"""

    calls: list[tuple] = []
    svc = DashboardBindingService()
    svc.bind("w1", "led:error", handler=lambda lvl, msg: calls.append((lvl, msg)))
    routed = svc.route_log(level="error", message="fail")
    assert routed == 1
    assert calls == [("error", "fail")]


def test_route_log_info_level_triggers_info_led_only():
    """route_log(info) 只触发 led:info，不触发 led:error。"""

    error_calls: list[tuple] = []
    info_calls: list[tuple] = []
    svc = DashboardBindingService()
    svc.bind("w1", "led:error", handler=lambda lvl, msg: error_calls.append((lvl, msg)))
    svc.bind("w2", "led:info", handler=lambda lvl, msg: info_calls.append((lvl, msg)))
    routed = svc.route_log(level="info", message="ok")
    assert routed == 1
    assert error_calls == []
    assert info_calls == [("info", "ok")]
