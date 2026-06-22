"""SerialDispatcher 单元测试 — 协议分发器。

覆盖：protocol_name、set_protocol 切换 + reset、build_command 委托、feed 委托。
用 fake SerialProtocol mock。
"""

from __future__ import annotations

from embeddebug.serial_station.core.dispatcher import SerialDispatcher
from embeddebug.serial_station.protocols.base import ProtocolEvent


class _FakeProtocol:
    """可控的假协议。"""

    def __init__(self, name: str = "fake") -> None:
        self._name = name
        self.reset_count = 0
        self.fed: list[bytes] = []

    @property
    def name(self) -> str:
        return self._name

    def reset(self) -> None:
        self.reset_count += 1

    def build_command(self, command: str, params=None) -> bytes:
        return command.encode("utf-8")

    def feed(self, data: bytes) -> list[ProtocolEvent]:
        self.fed.append(data)
        return [ProtocolEvent(type="byte", protocol_name=self._name, payload={"data": data.hex()})]


def test_dispatcher_protocol_name():
    d = SerialDispatcher(_FakeProtocol("ascii"))
    assert d.protocol_name == "ascii"


def test_dispatcher_set_protocol_resets():
    p1 = _FakeProtocol("ascii")
    d = SerialDispatcher(p1)
    p2 = _FakeProtocol("modbus")
    d.set_protocol(p2)
    assert d.protocol_name == "modbus"
    assert p2.reset_count == 1


def test_dispatcher_build_command():
    d = SerialDispatcher(_FakeProtocol())
    result = d.build_command("AT")
    assert result == b"AT"


def test_dispatcher_build_command_with_params():
    d = SerialDispatcher(_FakeProtocol())
    result = d.build_command("SET", {"addr": 1})
    assert result == b"SET"


def test_dispatcher_feed_returns_events():
    p = _FakeProtocol()
    d = SerialDispatcher(p)
    events = d.feed(b"\x01\x02")
    assert len(events) == 1
    assert events[0].protocol_name == "fake"
    assert p.fed == [b"\x01\x02"]


def test_dispatcher_feed_empty():
    p = _FakeProtocol()
    d = SerialDispatcher(p)
    events = d.feed(b"")
    assert isinstance(events, list)


def test_dispatcher_multiple_feeds_accumulate():
    p = _FakeProtocol()
    d = SerialDispatcher(p)
    d.feed(b"a")
    d.feed(b"b")
    assert p.fed == [b"a", b"b"]
