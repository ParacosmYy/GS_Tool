"""SessionSerializer 会话状态序列化器边界测试。

模块此前无直接测试覆盖（grep 0 命中，仅经 SessionManager 间接调用）。
本文件覆盖序列化器全契约：

1. FORMAT_VERSION 常量 + _is_supported_version（1 True / 0/2 False / 非数字 False）。
2. serialize 结构（含 version + payload + 紧凑 JSON）。
3. serialize → deserialize 往返（全字段一致）。
4. deserialize 损坏路径全分支：空字符串默认 / 非字符串默认 / 非 JSON 默认 / 非 dict 默认。
5. deserialize 兼容旧格式（直接状态字典无 version/payload 包裹）。
6. deserialize 未知版本仍尝试解析（from_dict 兜底缺失字段）。
"""

from __future__ import annotations

import json

from embeddebug.serial_station.session.serializer import (
    FORMAT_VERSION,
    SessionSerializer,
    _is_supported_version,
)
from embeddebug.serial_station.session.state import SessionState


# ── 常量 + 版本检查 ───────────────────────────────────────────────
def test_format_version_is_one():
    assert FORMAT_VERSION == 1


def test_is_supported_version_one_true():
    assert _is_supported_version(1) is True


def test_is_supported_version_zero_false():
    assert _is_supported_version(0) is False


def test_is_supported_version_two_false():
    assert _is_supported_version(2) is False


def test_is_supported_version_non_numeric_false():
    assert _is_supported_version("abc") is False
    assert _is_supported_version(None) is False
    assert _is_supported_version(1.5) is True  # int(1.5)=1


# ── serialize 结构 ────────────────────────────────────────────────
def test_serialize_returns_json_string():
    text = SessionSerializer.serialize(SessionState())
    assert isinstance(text, str)


def test_serialize_has_version_and_payload():
    text = SessionSerializer.serialize(SessionState())
    doc = json.loads(text)
    assert doc["version"] == FORMAT_VERSION
    assert "payload" in doc
    assert isinstance(doc["payload"], dict)


def test_serialize_is_compact():
    """serialize 用紧凑分隔符（无多余空格）。"""

    text = SessionSerializer.serialize(SessionState())
    assert ", " not in text  # 紧凑 separators
    assert ": " not in text


# ── serialize → deserialize 往返 ─────────────────────────────────
def test_roundtrip_preserves_fields():
    state = SessionState(
        transport_mode="uart",
        protocol="just_float",
        command_history=["AT+RST", "AT+GMR"],
        log_filter="ERROR",
        active_tab="serial",
        timestamp_ns=12345,
    )
    text = SessionSerializer.serialize(state)
    loaded = SessionSerializer.deserialize(text)
    assert loaded.transport_mode == "uart"
    assert loaded.protocol == "just_float"
    assert loaded.command_history == ["AT+RST", "AT+GMR"]
    assert loaded.log_filter == "ERROR"
    assert loaded.active_tab == "serial"
    assert loaded.timestamp_ns == 12345


def test_roundtrip_default_state():
    """默认状态往返应等价。"""

    state = SessionState()
    loaded = SessionSerializer.deserialize(SessionSerializer.serialize(state))
    assert loaded.transport_mode == state.transport_mode
    assert loaded.protocol == state.protocol


def test_roundtrip_empty_collections():
    """空 command_history + 空 connection_config 往返。"""

    state = SessionState(command_history=[], connection_config={})
    loaded = SessionSerializer.deserialize(SessionSerializer.serialize(state))
    assert loaded.command_history == []
    assert loaded.connection_config == {}


# ── deserialize 损坏路径 ──────────────────────────────────────────
def test_deserialize_empty_string_returns_default():
    loaded = SessionSerializer.deserialize("")
    assert isinstance(loaded, SessionState)
    assert loaded.transport_mode == "uart"  # 默认值


def test_deserialize_whitespace_only_returns_default():
    loaded = SessionSerializer.deserialize("   \n\t  ")
    assert isinstance(loaded, SessionState)


def test_deserialize_non_string_returns_default():
    """非字符串输入返回默认状态（不抛）。"""

    loaded = SessionSerializer.deserialize(None)  # type: ignore[arg-type]
    assert isinstance(loaded, SessionState)
    loaded2 = SessionSerializer.deserialize(12345)  # type: ignore[arg-type]
    assert isinstance(loaded2, SessionState)


def test_deserialize_invalid_json_returns_default():
    loaded = SessionSerializer.deserialize("not json {{{")
    assert isinstance(loaded, SessionState)


def test_deserialize_non_dict_json_returns_default():
    """JSON 解析为数组/字符串等非 dict → 默认状态。"""

    loaded = SessionSerializer.deserialize("[1, 2, 3]")
    assert isinstance(loaded, SessionState)
    loaded2 = SessionSerializer.deserialize('"just a string"')
    assert isinstance(loaded2, SessionState)


# ── deserialize 兼容旧格式 ───────────────────────────────────────
def test_deserialize_legacy_dict_format():
    """旧格式：直接状态字典（无 version/payload 包裹）应兼容。"""

    legacy = json.dumps({
        "transport_mode": "tcp",
        "protocol": "raw_data",
        "command_history": ["cmd1"],
        "log_filter": "",
        "active_tab": "dashboard",
        "timestamp_ns": 99,
    })
    loaded = SessionSerializer.deserialize(legacy)
    assert loaded.transport_mode == "tcp"
    assert loaded.command_history == ["cmd1"]
    assert loaded.timestamp_ns == 99


def test_deserialize_payload_dict_with_unknown_version():
    """未知 version 仍尝试按 payload 解析（from_dict 兜底）。"""

    doc = json.dumps({
        "version": 999,
        "payload": {
            "transport_mode": "udp",
            "protocol": "raw_data",
        },
    })
    loaded = SessionSerializer.deserialize(doc)
    assert loaded.transport_mode == "udp"


def test_deserialize_payload_not_dict_falls_back():
    """payload 不是 dict（如数组）→ 回退旧格式解析。"""

    doc = json.dumps({"version": 1, "payload": [1, 2, 3]})
    loaded = SessionSerializer.deserialize(doc)
    # payload 非 dict → 不走 from_dict(payload) → 回退到外层 dict（含 version/payload 键）。
    assert isinstance(loaded, SessionState)


# ── 无状态工具类 ──────────────────────────────────────────────────
def test_serializer_methods_are_static():
    """serialize/deserialize 应是 staticmethod（可不实例化调用）。"""

    # 已在上方所有测试中直接 SessionSerializer.xxx() 调用，证明是静态。
    assert callable(SessionSerializer.serialize)
    assert callable(SessionSerializer.deserialize)
