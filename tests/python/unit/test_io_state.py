"""controller_io_state 单元测试 — send_text + inject_received。

用 FakeSerialTransport + fake dispatcher 测发送/注入路径。
"""

from __future__ import annotations

from embeddebug.serial_station.controllers.controller_io_state import (
    inject_received_text_result,
    send_text_result,
)
from embeddebug.serial_station.drivers import FakeSerialTransport, SerialPortConfig


def _open_transport() -> FakeSerialTransport:
    t = FakeSerialTransport()
    t.open(SerialPortConfig(port_name="fake", baud_rate=0))
    return t


class _FakeDispatcher:
    def build_command(self, command, params=None):
        return command.encode("utf-8")


def test_send_text_success():
    transport = _open_transport()
    entries = []
    result = send_text_result(
        transport, _FakeDispatcher(), "AT", [], entries, [],
        handle_error=lambda m: None,
    )
    assert result.ok is True
    assert len(entries) == 1
    assert entries[0].direction == "tx"
    assert entries[0].text == "AT"


def test_send_text_transport_not_open():
    transport = FakeSerialTransport()
    errors = []
    result = send_text_result(
        transport, _FakeDispatcher(), "AT", [], [], [],
        handle_error=lambda m: errors.append(m),
    )
    assert result.failed is True
    assert errors == ["transport_not_open"]


def test_send_text_remembers_command():
    transport = _open_transport()
    history = []
    send_text_result(
        transport, _FakeDispatcher(), "AT", history, [], [],
        handle_error=lambda m: None,
    )
    assert "AT" in history


def test_inject_received_success():
    transport = _open_transport()
    result = inject_received_text_result(
        transport, "hello", [], [],
        handle_error=lambda m: None,
    )
    assert result.ok is True


def test_inject_received_non_fake_transport():
    """非 FakeSerialTransport 注入失败。"""

    class _RealTransport:
        is_open = True

    result = inject_received_text_result(
        _RealTransport(), "x", [], [],  # type: ignore[arg-type]
        handle_error=lambda m: None,
    )
    assert result.failed is True
