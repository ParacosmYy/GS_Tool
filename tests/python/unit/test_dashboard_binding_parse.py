"""DashboardBindingService binding spec 解析单元测试（纯函数）。

覆盖 services/dashboard_binding_service.py 的纯函数（无状态）：
- parse_binding_spec：value/gauge 通道 + led 级别 + slider 三段 + button 命令 + 无效返回 None。
- is_valid_spec：parse 的布尔包装。
- default_spec_for_widget_type：5 种 widget 默认 spec 建议。
"""

from __future__ import annotations

from embeddebug.serial_station.services.dashboard_binding_service import (
    KIND_BUTTON,
    KIND_GAUGE,
    KIND_LED,
    KIND_SLIDER,
    KIND_VALUE,
    default_spec_for_widget_type,
    is_valid_spec,
    parse_binding_spec,
)

# ── parse_binding_spec：value / gauge 通道 ────────────────────────────────


def test_parse_value_channel():
    result = parse_binding_spec("value:ch0")
    assert result is not None
    assert result.kind == KIND_VALUE
    assert result.channel == 0


def test_parse_gauge_channel_multi_digit():
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
    result = parse_binding_spec("led:error")
    assert result is not None
    assert result.kind == KIND_LED
    assert result.level == "error"


def test_parse_led_warning_and_info_levels():
    assert parse_binding_spec("led:warning").level == "warning"
    assert parse_binding_spec("led:info").level == "info"


def test_parse_led_rejects_unknown_level():
    """led:debug（未知级别）→ None。"""

    assert parse_binding_spec("led:debug") is None


# ── parse_binding_spec：slider 三段 ───────────────────────────────────────


def test_parse_slider_full_spec():
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
