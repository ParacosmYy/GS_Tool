"""protocols build_command + registry 错误路径单元测试。

补强 test_protocols.py 未覆盖的边角：
- RawDataProtocol.build_command：默认 utf-8 / hex 模式 / 自定义 encoding
- FireWaterProtocol.build_command：默认 \n / 自定义 line_ending（\r\n）
- JustFloatProtocol.build_command：默认 utf-8 / hex 模式 / 自定义 encoding
- SerialProtocolRegistry.register：空 name 抛 ValueError
- SerialProtocolRegistry.create：未知 name 抛 KeyError（带提示信息）
- SerialProtocolRegistry.names：按字母序返回
- create_default_registry：注册全部 3 个内置协议
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.protocols import (
    FireWaterProtocol,
    JustFloatProtocol,
    RawDataProtocol,
    create_default_registry,
)
from embeddebug.serial_station.protocols.registry import SerialProtocolRegistry


# ----------------------- RawDataProtocol.build_command -----------------------


def test_raw_data_build_command_default_utf8():
    protocol = RawDataProtocol()
    assert protocol.build_command("AT") == b"AT"


def test_raw_data_build_command_with_none_params_uses_utf8():
    protocol = RawDataProtocol()
    assert protocol.build_command("ping", None) == b"ping"


def test_raw_data_build_command_hex_mode_decodes_hex_string():
    """params={'hex': True} 时 command 视作十六进制字符串。"""
    protocol = RawDataProtocol()
    assert protocol.build_command("48656c6c6f", {"hex": True}) == b"Hello"


def test_raw_data_build_command_custom_encoding():
    """params={'encoding': 'ascii'} 使用指定编码。"""
    protocol = RawDataProtocol()
    assert protocol.build_command("ABC", {"encoding": "ascii"}) == b"ABC"


def test_raw_data_build_command_empty_string():
    protocol = RawDataProtocol()
    assert protocol.build_command("") == b""


# ----------------------- FireWaterProtocol.build_command -----------------------


def test_fire_water_build_command_appends_default_newline():
    protocol = FireWaterProtocol()
    assert protocol.build_command("status?") == b"status?\n"


def test_fire_water_build_command_with_none_params_uses_newline():
    protocol = FireWaterProtocol()
    assert protocol.build_command("status?", None) == b"status?\n"


def test_fire_water_build_command_custom_crlf_line_ending():
    """params={'line_ending': '\\r\\n'} 附加 CRLF。"""
    protocol = FireWaterProtocol()
    assert protocol.build_command("reset", {"line_ending": "\r\n"}) == b"reset\r\n"


def test_fire_water_build_command_empty_command_still_appends_ending():
    protocol = FireWaterProtocol()
    assert protocol.build_command("") == b"\n"


# ----------------------- JustFloatProtocol.build_command -----------------------


def test_just_float_build_command_default_utf8():
    protocol = JustFloatProtocol()
    assert protocol.build_command("PING") == b"PING"


def test_just_float_build_command_with_none_params_uses_utf8():
    protocol = JustFloatProtocol()
    assert protocol.build_command("PING", None) == b"PING"


def test_just_float_build_command_hex_mode_decodes_hex_string():
    protocol = JustFloatProtocol()
    assert protocol.build_command("41542d", {"hex": True}) == b"AT-"


def test_just_float_build_command_custom_encoding():
    protocol = JustFloatProtocol()
    assert protocol.build_command("HELLO", {"encoding": "ascii"}) == b"HELLO"


def test_just_float_build_command_empty_string():
    protocol = JustFloatProtocol()
    assert protocol.build_command("") == b""


# ----------------------- SerialProtocolRegistry -----------------------


def test_registry_register_rejects_empty_name():
    registry = SerialProtocolRegistry()
    with pytest.raises(ValueError, match="protocol name must not be empty"):
        registry.register("", RawDataProtocol)


def test_registry_create_unknown_name_raises_keyerror():
    registry = SerialProtocolRegistry()
    with pytest.raises(KeyError, match="unknown protocol: ghost"):
        registry.create("ghost")


def test_registry_names_returns_sorted_list():
    registry = SerialProtocolRegistry()
    registry.register("zeta", RawDataProtocol)
    registry.register("alpha", FireWaterProtocol)
    registry.register("middle", JustFloatProtocol)
    assert registry.names() == ["alpha", "middle", "zeta"]


def test_registry_register_overwrites_existing_name():
    """同名注册覆盖旧 factory（不报错）。"""
    registry = SerialProtocolRegistry()
    registry.register("dup", RawDataProtocol)
    registry.register("dup", FireWaterProtocol)
    protocol = registry.create("dup")
    assert protocol.name == "fire_water"


def test_registry_create_returns_fresh_instance_each_call():
    """每次 create() 返回新实例（factory 而非缓存单例）。"""
    registry = SerialProtocolRegistry()
    registry.register("raw", RawDataProtocol)
    first = registry.create("raw")
    second = registry.create("raw")
    assert first is not second


def test_registry_names_empty_when_no_protocols_registered():
    assert SerialProtocolRegistry().names() == []


# ----------------------- create_default_registry -----------------------


def test_create_default_registry_registers_all_builtin_protocols():
    registry = create_default_registry()
    assert registry.names() == ["fire_water", "just_float", "raw_data"]
    assert isinstance(registry.create("raw_data"), RawDataProtocol)
    assert isinstance(registry.create("fire_water"), FireWaterProtocol)
    assert isinstance(registry.create("just_float"), JustFloatProtocol)


def test_create_default_registry_each_create_returns_new_instance():
    registry = create_default_registry()
    a = registry.create("fire_water")
    b = registry.create("fire_water")
    assert a is not b
    # reset 一个不应影响另一个（独立 buffer）
    a.reset()
    events_a = a.feed(b"1,2\n")
    events_b = b.feed(b"3,4\n")
    assert events_a[0].payload["values"] == [1.0, 2.0]
    assert events_b[0].payload["values"] == [3.0, 4.0]
    assert events_a[0].payload["frameIndex"] == 1
    assert events_b[0].payload["frameIndex"] == 1  # 独立计数器
