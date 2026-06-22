"""session/serializer + state 纯 helper 单元测试。

补强 test_session_restore.py 未直接断言的边角：
- _is_supported_version：1=支持 / 0/2/负数=不支持 / 非数字（str/None/float）=不支持。
- FORMAT_VERSION 常量契约。
- _default_geometry / _default_connection_config：默认值结构。
- SessionState：默认值 + from_dict(None) + to_dict round-trip + 字段类型。
- SessionSerializer：非字符串输入 / None / 空白串 / 未知版本仍解析 / 紧凑 JSON（无多余空白）。
"""

from __future__ import annotations

import json

from embeddebug.serial_station.session.serializer import (
    FORMAT_VERSION,
    SessionSerializer,
    _is_supported_version,
)
from embeddebug.serial_station.session.state import (
    SessionState,
    _default_connection_config,
    _default_geometry,
)


# ── _is_supported_version 纯函数 ─────────────────────────────────────────


def test_is_supported_version_one_is_supported():
    """版本 1 在支持区间 [1,1] 内 → True。"""

    assert _is_supported_version(1) is True


def test_is_supported_version_zero_unsupported():
    """版本 0 < _MIN_SUPPORTED_VERSION(1) → False。"""

    assert _is_supported_version(0) is False


def test_is_supported_version_two_unsupported():
    """版本 2 > _MAX_SUPPORTED_VERSION(1) → False。"""

    assert _is_supported_version(2) is False


def test_is_supported_version_negative_unsupported():
    """负版本号 → False。"""

    assert _is_supported_version(-1) is False


def test_is_supported_version_string_numeric():
    """字符串 "1" 能 int() → True。"""

    assert _is_supported_version("1") is True


def test_is_supported_version_string_non_numeric():
    """非数字字符串 → False。"""

    assert _is_supported_version("abc") is False


def test_is_supported_version_none():
    """None → False。"""

    assert _is_supported_version(None) is False


def test_is_supported_version_float():
    """浮点 1.0 → int(1.0)=1 → True（int() 接受浮点）。"""

    assert _is_supported_version(1.0) is True


# ── FORMAT_VERSION 常量契约 ──────────────────────────────────────────────


def test_format_version_is_one():
    """FORMAT_VERSION = 1（当前格式版本）。"""

    assert FORMAT_VERSION == 1


# ── _default_geometry / _default_connection_config ───────────────────────


def test_default_geometry_structure():
    """_default_geometry 含 x/y/width/height 4 键。"""

    geom = _default_geometry()
    assert set(geom.keys()) == {"x", "y", "width", "height"}
    assert geom["x"] == 0
    assert geom["y"] == 0
    assert geom["width"] == 1280
    assert geom["height"] == 800


def test_default_connection_config_structure():
    """_default_connection_config 含 port/baudrate 2 键。"""

    cfg = _default_connection_config()
    assert set(cfg.keys()) == {"port", "baudrate"}
    assert cfg["port"] == ""
    assert cfg["baudrate"] == 115200


def test_default_geometry_returns_new_instance_each_call():
    """每次调用返回新 dict（防 mutable default 共享）。"""

    a = _default_geometry()
    b = _default_geometry()
    assert a == b
    assert a is not b
    a["x"] = 999
    assert b["x"] == 0  # b 不受影响


# ── SessionState 默认值 + from_dict(None) ────────────────────────────────


def test_session_state_defaults():
    """SessionState 默认值匹配 _default_* 工厂 + transport=uart + protocol=raw_data。"""

    state = SessionState()
    assert state.window_geometry == _default_geometry()
    assert state.connection_config == _default_connection_config()
    assert state.transport_mode == "uart"
    assert state.protocol == "raw_data"
    assert state.command_history == []
    assert state.log_filter == ""
    assert state.active_tab == ""
    assert state.timestamp_ns == 0


def test_session_state_from_dict_none_returns_defaults():
    """from_dict(None) → 默认 SessionState。"""

    state = SessionState.from_dict(None)
    assert state.window_geometry == _default_geometry()
    assert state.transport_mode == "uart"


def test_session_state_to_dict_round_trip():
    """to_dict → from_dict round-trip 保持字段等价。"""

    original = SessionState(
        transport_mode="tcp_client",
        protocol="fire_water",
        command_history=["AT", "AT+RESET"],
        log_filter="error",
        active_tab="rtt",
        timestamp_ns=1234567890,
    )
    restored = SessionState.from_dict(original.to_dict())
    assert restored.transport_mode == "tcp_client"
    assert restored.protocol == "fire_water"
    assert restored.command_history == ["AT", "AT+RESET"]
    assert restored.log_filter == "error"
    assert restored.active_tab == "rtt"
    assert restored.timestamp_ns == 1234567890


def test_session_state_to_dict_returns_copy():
    """to_dict 返回的 dict/list 是拷贝（修改不影响原 state）。"""

    state = SessionState(command_history=["A"])
    d = state.to_dict()
    d["command_history"].append("B")
    assert state.command_history == ["A"]  # 原不受影响


# ── SessionSerializer 边界 ───────────────────────────────────────────────


def test_serializer_deserialize_none_returns_defaults():
    """deserialize(None) → 默认（非字符串兜底）。"""

    state = SessionSerializer.deserialize(None)  # type: ignore[arg-type]
    assert state.transport_mode == "uart"


def test_serializer_deserialize_whitespace_only_returns_defaults():
    """deserialize("   ") → 默认（strip 后为空）。"""

    state = SessionSerializer.deserialize("   ")
    assert state.transport_mode == "uart"


def test_serializer_deserialize_non_dict_json_returns_defaults():
    """deserialize JSON 数组/数字 → 默认（非 dict）。"""

    state = SessionSerializer.deserialize("[1, 2, 3]")
    assert state.transport_mode == "uart"
    state2 = SessionSerializer.deserialize("42")
    assert state2.transport_mode == "uart"


def test_serializer_deserialize_unknown_version_still_parses():
    """未知版本号仍解析 payload（向前兼容）。"""

    document = {
        "version": 999,
        "payload": SessionState(transport_mode="ble").to_dict(),
    }
    state = SessionSerializer.deserialize(json.dumps(document))
    assert state.transport_mode == "ble"


def test_serializer_deserialize_string_version():
    """字符串版本号 "1" 被接受（_is_supported_version 支持）。"""

    document = {
        "version": "1",
        "payload": SessionState(protocol="just_float").to_dict(),
    }
    state = SessionSerializer.deserialize(json.dumps(document))
    assert state.protocol == "just_float"


def test_serializer_serialize_produces_compact_json():
    """serialize 输出紧凑 JSON（separators 无多余空白）。"""

    state = SessionState(transport_mode="uart")
    text = SessionSerializer.serialize(state)
    # 紧凑 JSON 不含 ", " 或 ": " 的多余空格。
    assert ", " not in text
    assert ": " not in text


def test_serializer_serialize_contains_version_and_payload():
    """serialize 输出含 version + payload 顶层字段。"""

    text = SessionSerializer.serialize(SessionState())
    document = json.loads(text)
    assert "version" in document
    assert document["version"] == FORMAT_VERSION
    assert "payload" in document
    assert isinstance(document["payload"], dict)


def test_serializer_round_trip_preserves_timestamp():
    """serialize → deserialize round-trip 保留 timestamp_ns。"""

    state = SessionState(timestamp_ns=999999999)
    restored = SessionSerializer.deserialize(SessionSerializer.serialize(state))
    assert restored.timestamp_ns == 999999999
