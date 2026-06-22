"""CAN/CAN-FD 帧模型单元测试 — CanId/CanFrame/CanFilter。

覆盖：CanId 标准/扩展 ID 验证 + as_hex、CanFrame DLC 限制 + to_payload、
CanFilter mask 匹配。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.can.frame import (
    EXTENDED_ID_MAX,
    STANDARD_ID_MAX,
    CanFilter,
    CanFrame,
    CanId,
)


def test_can_id_standard():
    cid = CanId(0x123)
    assert cid.is_standard() is True
    assert cid.is_extended is False


def test_can_id_extended():
    cid = CanId(0x1FFFFFFF, is_extended=True)
    assert cid.is_extended is True
    assert cid.is_standard() is False


def test_can_id_as_hex_standard():
    cid = CanId(0x7B)
    assert cid.as_hex() == "07B"


def test_can_id_as_hex_extended():
    cid = CanId(0x12345, is_extended=True)
    assert cid.as_hex() == "00012345"


def test_can_id_standard_overflow():
    with pytest.raises(ValueError):
        CanId(STANDARD_ID_MAX + 1)


def test_can_id_extended_overflow():
    with pytest.raises(ValueError):
        CanId(EXTENDED_ID_MAX + 1, is_extended=True)


def test_can_id_negative():
    with pytest.raises(ValueError):
        CanId(-1)


def test_can_frame_dlc():
    frame = CanFrame(CanId(0x100), b"\x01\x02\x03")
    assert frame.dlc == 3


def test_can_frame_max_dlc_can():
    """CAN 最大 8 字节。"""
    frame = CanFrame(CanId(0x100), b"\x00" * 8)
    assert frame.dlc == 8


def test_can_frame_over_dlc_can_raises():
    with pytest.raises(ValueError):
        CanFrame(CanId(0x100), b"\x00" * 9)


def test_can_frame_fd_allows_64():
    """CAN-FD 允许 64 字节。"""
    frame = CanFrame(CanId(0x100), b"\x00" * 64, is_fd=True)
    assert frame.dlc == 64


def test_can_frame_fd_over_64_raises():
    with pytest.raises(ValueError):
        CanFrame(CanId(0x100), b"\x00" * 65, is_fd=True)


def test_can_frame_to_payload():
    frame = CanFrame(CanId(0x123), b"\xAA\xBB", timestamp=1.5, frame_index=42)
    payload = frame.to_payload()
    assert payload["canId"] == 0x123
    assert payload["canIdHex"] == "123"
    assert payload["dlc"] == 2
    assert payload["dataHex"] == "aa bb"
    assert payload["timestamp"] == 1.5
    assert payload["frameIndex"] == 42


def test_can_filter_exact_match():
    f = CanFilter(id=0x100, mask=0x7FF)
    assert f.matches(CanId(0x100)) is True
    assert f.matches(CanId(0x101)) is False


def test_can_filter_mask_partial():
    """mask=0x700 匹配高 3 位。"""
    f = CanFilter(id=0x100, mask=0x700)
    assert f.matches(CanId(0x1FF)) is True
    assert f.matches(CanId(0x200)) is False


def test_can_filter_extended_flag():
    f = CanFilter(id=0x100, mask=0x7FF, is_extended=True)
    assert f.matches(CanId(0x100, is_extended=True)) is True
    assert f.matches(CanId(0x100)) is False  # standard 不匹配


def test_can_filter_no_extended_check():
    """is_extended=None 不检查帧类型。"""
    f = CanFilter(id=0x100, mask=0x7FF)
    assert f.matches(CanId(0x100, is_extended=True)) is True
    assert f.matches(CanId(0x100)) is True
