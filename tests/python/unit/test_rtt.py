"""SEGGER RTT 模块单元测试：通道配置 / 会话生命周期 / 环形缓冲 / 传输桩。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.rtt import (
    SEGGER_RTT_MAGIC,
    SEGGER_RTT_MAGIC_BYTES,
    RttChannel,
    RttConfig,
    RttSession,
    RttTransportStub,
    control_block_layout,
)


def _make_config() -> RttConfig:
    """构造三通道配置：terminal/log 上行，cmd 下行。"""
    return RttConfig(
        channels=(
            RttChannel(name="terminal", buffer_size=8, mode="up"),
            RttChannel(name="log", buffer_size=16, mode="up"),
            RttChannel(name="cmd", buffer_size=32, mode="down"),
        ),
        ram_base=0x2000_0000,
    )


def test_rtt_magic_constant_is_null_terminated_token():
    assert SEGGER_RTT_MAGIC == "RTT\0"
    assert SEGGER_RTT_MAGIC_BYTES == b"RTT\0"
    assert SEGGER_RTT_MAGIC_BYTES.endswith(b"\0")


def test_channel_config_index_and_ram_base_default():
    config = _make_config()
    names = [channel.name for channel in config.channels]

    assert names == ["terminal", "log", "cmd"]
    assert config.channels[0].mode == "up"
    assert config.channels[2].mode == "down"
    assert config.ram_base == 0x2000_0000
    assert RttConfig(channels=(config.channels[0],)).ram_base == 0x2000_0000


def test_control_block_layout_offsets_and_headers():
    layout = control_block_layout(max_up=2, max_down=1)

    assert layout[0] == ("acID", 0, 16)
    assert layout[1] == ("MaxNumUpBuffers", 16, 4)
    assert layout[2] == ("MaxNumDownBuffers", 20, 4)
    assert layout[3] == ("aUp[0].header", 24, 16)
    assert layout[4] == ("aUp[1].header", 40, 16)
    assert layout[5] == ("aDown[0].header", 56, 16)


def test_session_open_close_lifecycle():
    transport = RttTransportStub()
    session = RttSession(transport, _make_config())

    assert not session.is_open
    assert session.open() is True
    assert session.is_open
    session.close()
    assert not session.is_open


def test_session_send_routes_down_channel_to_transport():
    transport = RttTransportStub()
    session = RttSession(transport, _make_config())
    session.open()

    written = session.send("cmd", b"hello")

    assert written == 5
    assert transport.written == [b"hello"]
    session.close()


def test_session_send_validates_mode_and_unknown_channel():
    transport = RttTransportStub()
    session = RttSession(transport, _make_config())
    session.open()
    errors: list[str] = []
    session.on_error(errors.append)

    assert session.send("terminal", b"x") == 0
    assert session.send("missing", b"x") == 0

    assert errors == ["wrong_mode", "unknown_channel"]
    assert transport.written == []
    session.close()


def test_session_send_when_closed_emits_error():
    transport = RttTransportStub()
    session = RttSession(transport, _make_config())
    errors: list[str] = []
    session.on_error(errors.append)

    assert session.send("cmd", b"x") == 0

    assert errors == ["session_not_open"]


def test_session_recv_consumes_injected_bytes():
    transport = RttTransportStub()
    session = RttSession(transport, _make_config())
    session.open()

    transport.inject_received(b"abc")

    assert session.available("terminal") == 3
    assert session.recv("terminal") == b"abc"
    assert session.recv("terminal") == b""
    session.close()


def test_ring_buffer_wraparound_keeps_newest_in_order():
    transport = RttTransportStub()
    session = RttSession(transport, _make_config())
    session.open()

    transport.inject_received(b"ABCDEFGHIJKL")

    assert session.available("terminal") == 8
    assert session.recv("terminal") == b"EFGHIJKL"
    session.close()


def test_max_buffer_enforcement_caps_available():
    transport = RttTransportStub()
    session = RttSession(transport, _make_config())
    session.open()

    transport.inject_received(b"0123456789" * 20)

    assert session.available("terminal") == 8
    assert len(session.recv("terminal")) == 8
    session.close()


def test_session_recv_unknown_channel_emits_error():
    transport = RttTransportStub()
    session = RttSession(transport, _make_config())
    session.open()
    errors: list[str] = []
    session.on_error(errors.append)

    assert session.recv("cmd") == b""

    assert errors == ["unknown_channel"]
    session.close()


def test_session_re_emits_received_bytes_to_subscribers():
    transport = RttTransportStub()
    session = RttSession(transport, _make_config())
    session.open()
    seen: list[bytes] = []
    session.on_bytes_received(seen.append)

    transport.inject_received(b"ping")

    assert seen == [b"ping"]
    session.close()


def test_set_active_up_channel_redirects_incoming_stream():
    transport = RttTransportStub()
    session = RttSession(transport, _make_config())
    session.open()

    assert session.set_active_up_channel("log") is True
    transport.inject_received(b"to-log")

    assert session.recv("terminal") == b""
    assert session.recv("log") == b"to-log"
    session.close()


def test_stub_write_and_inject_surface():
    transport = RttTransportStub()
    received: list[bytes] = []
    errors: list[str] = []
    transport.on_bytes_received(received.append)
    transport.on_error(errors.append)

    assert transport.write(b"out") == 3
    transport.inject_received(b"in")

    assert transport.written == [b"out"]
    assert received == [b"in"]

    transport.close()
    assert transport.write(b"x") == 0
    assert errors == ["transport_not_open"]
