"""DBC 解析器单元测试 — 报文/信号解析 + 物理值解码。

覆盖：DbcDatabase.parse 解析 BO_/SG_、message/signal 查找、
decode_signal Intel/Motorola 字节序 + factor/offset 转换、扩展帧判定。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.can.dbc import (
    CAN_FD_MAX_DLC,
    DbcDatabase,
    DbcMessage,
    DbcSignal,
    decode_signal,
)

_SAMPLE_DBC = """\
BO_ 100 EngineStatus: 8 ECU1
 SG_ RPM : 0|16@1+ (0.25,0) "rpm"
 SG_ Temp : 16|8@1+ (1,-40) "C"
BO_ 200 WheelSpeed: 4 ECU2
 SG_ FL : 0|16@1+ (0.01,0) "km/h"
"""


def test_parse_finds_messages():
    db = DbcDatabase.parse(_SAMPLE_DBC)
    assert 100 in db.messages
    assert 200 in db.messages
    assert db.messages[100].name == "EngineStatus"
    assert db.messages[200].name == "WheelSpeed"


def test_parse_message_dlc():
    db = DbcDatabase.parse(_SAMPLE_DBC)
    assert db.messages[100].dlc == 8
    assert db.messages[200].dlc == 4


def test_parse_signals_attached():
    db = DbcDatabase.parse(_SAMPLE_DBC)
    engine = db.messages[100]
    assert len(engine.signals) == 2
    rpm = engine.signal("RPM")
    assert rpm.start_bit == 0
    assert rpm.bit_length == 16
    assert rpm.is_little_endian is True
    assert rpm.factor == 0.25
    assert rpm.unit == "rpm"


def test_signal_factor_offset():
    db = DbcDatabase.parse(_SAMPLE_DBC)
    temp = db.messages[100].signal("Temp")
    assert temp.factor == 1.0
    assert temp.offset == -40.0


def test_message_lookup_by_id():
    db = DbcDatabase.parse(_SAMPLE_DBC)
    msg = db.message(100)
    assert msg.name == "EngineStatus"


def test_signal_not_found_raises():
    msg = DbcMessage(id=1, name="x", dlc=8)
    with pytest.raises(KeyError):
        msg.signal("nonexistent")


def test_decode_signal_little_endian():
    """小端 16 位：raw=100, factor=0.25 → 25.0。"""
    sig = DbcSignal(name="x", start_bit=0, bit_length=16, is_little_endian=True, factor=0.25)
    data = bytes([100, 0])  # little-endian 100
    assert decode_signal(data, sig) == 25.0


def test_decode_signal_with_offset():
    """factor=1, offset=-40：raw=60 → 20.0。"""
    sig = DbcSignal(name="x", start_bit=0, bit_length=8, is_little_endian=True, factor=1.0, offset=-40.0)
    data = bytes([60])
    assert decode_signal(data, sig) == 20.0


def test_decode_signal_zero_length_returns_zero():
    sig = DbcSignal(name="x", start_bit=0, bit_length=0, is_little_endian=True)
    assert decode_signal(b"\xFF", sig) == 0.0


def test_decode_signal_empty_bytes_returns_zero():
    sig = DbcSignal(name="x", start_bit=0, bit_length=8, is_little_endian=True)
    assert decode_signal(b"", sig) == 0.0


def test_parse_extended_frame_detection():
    """id > 0x7FF 标记为扩展帧。"""
    dbc = "BO_ 2048 ExtMsg: 8 ECU\n"  # 0x800 > 0x7FF
    db = DbcDatabase.parse(dbc)
    assert db.messages[2048].is_extended is True


def test_parse_standard_frame_not_extended():
    """id <= 0x7FF 标准帧。"""
    dbc = "BO_ 100 StdMsg: 8 ECU\n"
    db = DbcDatabase.parse(dbc)
    assert db.messages[100].is_extended is False


def test_parse_empty_dbc():
    """空文本返回空数据库。"""
    db = DbcDatabase.parse("")
    assert db.messages == {}


def test_can_fd_max_dlc_constant():
    assert CAN_FD_MAX_DLC == 64
