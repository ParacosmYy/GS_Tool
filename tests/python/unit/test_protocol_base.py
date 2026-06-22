"""protocols/base ProtocolEvent + SerialProtocol 契约测试。

覆盖：ProtocolEvent 默认值 + frozen、SerialProtocol 抽象方法无法实例化。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.protocols.base import ProtocolEvent, SerialProtocol


def test_protocol_event_defaults():
    e = ProtocolEvent(type="byte", protocol_name="ascii")
    assert e.payload == {}
    assert e.raw == b""


def test_protocol_event_with_payload():
    e = ProtocolEvent(type="frame", protocol_name="modbus", payload={"addr": 1}, raw=b"\x01")
    assert e.payload == {"addr": 1}
    assert e.raw == b"\x01"


def test_protocol_event_frozen():
    e = ProtocolEvent(type="x", protocol_name="p")
    with pytest.raises((AttributeError, TypeError)):
        e.type = "y"


def test_serial_protocol_cannot_instantiate():
    """SerialProtocol 是抽象基类，不能直接实例化。"""
    with pytest.raises(TypeError):
        SerialProtocol()  # type: ignore[abstract]


def test_serial_protocol_subclass_must_implement():
    """子类必须实现所有抽象方法。"""

    class _Incomplete(SerialProtocol):
        name = "incomplete"

    with pytest.raises(TypeError):
        _Incomplete()  # type: ignore[abstract]


def test_serial_protocol_complete_subclass():
    """完整实现所有抽象方法的子类可实例化。"""

    class _Complete(SerialProtocol):
        name = "complete"

        def build_command(self, command, params=None):
            return command.encode()

        def feed(self, data):
            return [ProtocolEvent(type="byte", protocol_name=self.name)]

        def reset(self):
            pass

    p = _Complete()
    assert p.name == "complete"
    assert p.build_command("AT") == b"AT"
    assert len(p.feed(b"x")) == 1
