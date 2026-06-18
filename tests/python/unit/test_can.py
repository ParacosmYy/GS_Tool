"""CAN 子模块单测。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.can import (
    CanFilter,
    CanFrame,
    CanFrameCodec,
    CanId,
    DbcDatabase,
    DbcSignal,
    decode_signal,
)

DBC_SAMPLE = """\
VERSION ""

BO_ 100 EngineStatus: 8 Engine
 SG_ EngineSpeed : 0|16@1+ (0.25,0) [0|16383] "rpm" ECU
 SG_ EngineTemp : 16|8@1+ (1,-40) [-40|215] "degC" ECU

BO_ 256 BrakeStatus: 4 Brake
 SG_ BrakePressure : 0|16@1+ (0.1,0) [0|6553.5] "kPa" BrakeECU
"""


def test_can_id_standard_and_extended_validation():
    std = CanId(0x123)
    assert std.is_standard()
    assert std.as_hex() == "123"
    ext = CanId(0x1ABCDEEF, is_extended=True)
    assert ext.is_extended is True


def test_can_id_rejects_out_of_range():
    with pytest.raises(ValueError):
        CanId(0x800)
    with pytest.raises(ValueError):
        CanId(0x20000000, is_extended=True)


def test_can_frame_dlc_and_payload():
    frame = CanFrame(can_id=CanId(0x10), data=b"\x01\x02")
    assert frame.dlc == 2
    assert frame.to_payload()["dataHex"] == "01 02"


def test_can_frame_rejects_oversize_data():
    with pytest.raises(ValueError):
        CanFrame(can_id=CanId(0x10), data=b"\x00" * 9)


def test_can_fd_accepts_up_to_64():
    frame = CanFrame(can_id=CanId(0x10), data=b"\x00" * 64, is_fd=True)
    assert frame.dlc == 64


def test_codec_roundtrip_standard_frame():
    original = CanFrame(can_id=CanId(0x7A), data=bytes.fromhex("DEADBEEF"))
    events = CanFrameCodec().feed(CanFrameCodec().encode(original))
    assert len(events) == 1
    assert events[0]["payload"]["canId"] == 0x7A
    assert events[0]["payload"]["dataHex"].lower() == "de ad be ef"


def test_codec_roundtrip_extended_frame():
    original = CanFrame(can_id=CanId(0x18FEF100, is_extended=True), data=bytes.fromhex("00"))
    events = CanFrameCodec().feed(CanFrameCodec().encode(original))
    assert events[0]["payload"]["canId"] == 0x18FEF100
    assert events[0]["payload"]["isExtended"] is True


def test_codec_streaming_handles_split_frames():
    codec = CanFrameCodec()
    encoded = codec.encode(CanFrame(can_id=CanId(0x05), data=b"\xAA\xBB"))
    mid = len(encoded) // 2
    assert codec.feed(encoded[:mid]) == []
    events = codec.feed(encoded[mid:])
    assert len(events) == 1


def test_codec_multiple_frames_in_one_feed():
    codec = CanFrameCodec()
    f1 = codec.encode(CanFrame(can_id=CanId(0x1), data=b"\x01"))
    f2 = codec.encode(CanFrame(can_id=CanId(0x2), data=b"\x02"))
    events = codec.feed(f1 + f2)
    assert len(events) == 2


def test_codec_reports_invalid_frame():
    events = CanFrameCodec().feed(b"Zgarbage\r")
    assert events[0]["type"] == "error"


def test_filter_mask_matching():
    flt = CanFilter(id=0x100, mask=0x700)
    assert flt.matches(CanId(0x123)) is True
    assert flt.matches(CanId(0x200)) is False


def test_dbc_parses_messages_and_signals():
    db = DbcDatabase.parse(DBC_SAMPLE)
    msg = db.message(100)
    assert msg.name == "EngineStatus"
    speed = msg.signal("EngineSpeed")
    assert speed.bit_length == 16
    assert speed.factor == pytest.approx(0.25)
    temp = msg.signal("EngineTemp")
    assert temp.offset == pytest.approx(-40)


def test_decode_signal_intel():
    sig = DbcSignal(name="n", start_bit=0, bit_length=16, is_little_endian=True, factor=0.25)
    assert decode_signal(bytes.fromhex("0001"), sig) == pytest.approx(64.0)


def test_decode_signal_with_offset():
    sig = DbcSignal(name="t", start_bit=0, bit_length=8, is_little_endian=True, factor=1.0, offset=-40.0)
    assert decode_signal(bytes([100]), sig) == pytest.approx(60.0)
