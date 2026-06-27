"""CAN 子模块单测。"""

from __future__ import annotations

from dataclasses import FrozenInstanceError
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
from embeddebug.serial_station.can.frame import CAN_FD_MAX_DLC, CAN_MAX_DLC, EXTENDED_ID_MAX, STANDARD_ID_MAX

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
    with pytest.raises(ValueError):
        CanId(-1)


@pytest.mark.parametrize(
    ("can_id", "is_extended", "expected_hex", "is_standard"),
    [
        (0x001, False, "001", True),
        (STANDARD_ID_MAX, False, "7FF", True),
        (0x00000001, True, "00000001", False),
        (0x12345ABC, True, "12345ABC", False),
    ],
)
def test_can_id_hex_width_and_standard_flag(can_id, is_extended, expected_hex, is_standard):
    cid = CanId(can_id, is_extended=is_extended)
    assert cid.as_hex() == expected_hex
    assert cid.is_standard() is is_standard


def test_can_frame_dlc_and_payload():
    frame = CanFrame(can_id=CanId(0x10), data=b"\x01\x02")
    assert frame.dlc == 2
    assert frame.to_payload()["dataHex"] == "01 02"


def test_can_frame_rejects_oversize_data():
    with pytest.raises(ValueError):
        CanFrame(can_id=CanId(0x10), data=b"\x00" * (CAN_MAX_DLC + 1))


def test_can_fd_accepts_up_to_64():
    frame = CanFrame(can_id=CanId(0x10), data=b"\x00" * CAN_FD_MAX_DLC, is_fd=True)
    assert frame.dlc == CAN_FD_MAX_DLC


def test_can_fd_rejects_oversize_data():
    with pytest.raises(ValueError):
        CanFrame(can_id=CanId(0x10), data=b"\x00" * (CAN_FD_MAX_DLC + 1), is_fd=True)


def test_can_frame_payload_contract_and_frozen():
    frame = CanFrame(can_id=CanId(0x123), data=b"\xAB", timestamp=1.5, frame_index=7)
    payload = frame.to_payload()
    assert set(payload) == {
        "canId",
        "canIdHex",
        "isExtended",
        "isFd",
        "dlc",
        "data",
        "dataHex",
        "timestamp",
        "frameIndex",
    }
    assert payload["dataHex"] == "ab"
    with pytest.raises((AttributeError, FrozenInstanceError)):
        frame.can_id = CanId(0x2)  # type: ignore[misc]


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


@pytest.mark.parametrize(
    ("filter_id", "mask", "candidate_id", "expected"),
    [
        (0x100, 0x700, 0x123, True),
        (0x100, 0x700, 0x200, False),
        (0x000, 0x000, 0x123, True),
        (0x000, 0x000, 0x7FF, True),
        (0x000, 0x000, 0x000, True),
        (0x123, 0x7FF, 0x123, True),
        (0x123, 0x7FF, 0x124, False),
        (0x120, 0x0F0, 0x123, True),
        (0x120, 0x0F0, 0x12F, True),
        (0x120, 0x0F0, 0x100, False),
    ],
)
def test_filter_mask_matching_boundaries(filter_id, mask, candidate_id, expected):
    flt = CanFilter(id=filter_id, mask=mask)
    assert flt.matches(CanId(candidate_id)) is expected


@pytest.mark.parametrize(
    ("filter_extended", "frame_extended", "expected"),
    [
        (None, False, True),
        (None, True, True),
        (True, True, True),
        (True, False, False),
        (False, False, True),
        (False, True, False),
    ],
)
def test_filter_frame_type_gate(filter_extended, frame_extended, expected):
    flt = CanFilter(id=0x100, mask=0x700, is_extended=filter_extended)
    assert flt.matches(CanId(0x123, is_extended=frame_extended)) is expected


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


# ---- Batch A: frame/filter 边界（合并自 test_can_frame.py）----


def test_can_id_zero_and_max_boundaries():
    """ID=0 合法；STANDARD_ID_MAX / EXTENDED_ID_MAX 是合法边界。"""
    assert CanId(0).value == 0
    assert CanId(STANDARD_ID_MAX).value == 0x7FF
    assert CanId(EXTENDED_ID_MAX, is_extended=True).value == 0x1FFFFFFF
    with pytest.raises(ValueError):
        CanId(STANDARD_ID_MAX + 1)
    with pytest.raises(ValueError):
        CanId(EXTENDED_ID_MAX + 1, is_extended=True)


def test_can_frame_empty_data_and_fd_payload():
    """空 data DLC=0；扩展帧+FD 的 to_payload。"""
    assert CanFrame(CanId(0x100), b"").dlc == 0
    frame = CanFrame(CanId(0x12345, is_extended=True), b"\x01" * 10, is_fd=True)
    payload = frame.to_payload()
    assert payload["isExtended"] is True and payload["isFd"] is True


def test_can_filter_extended_mismatch_short_circuits():
    """is_extended 不匹配时短路返回 False。"""
    f = CanFilter(id=0x100, mask=0, is_extended=True)
    assert not f.matches(CanId(0x100))  # 标准帧被拒绝
    assert f.matches(CanId(0x100, is_extended=True))
