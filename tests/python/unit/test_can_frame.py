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


# ---- Batch 147: CanId/CanFrame/CanFilter 边界扩展 ----


def test_can_id_zero_is_valid():
    """ID=0 是合法的标准帧（边界值）。"""
    cid = CanId(0)
    assert cid.value == 0
    assert cid.is_standard() is True
    assert cid.as_hex() == "000"


def test_can_id_extended_zero_is_valid():
    """扩展帧 ID=0 也合法。"""
    cid = CanId(0, is_extended=True)
    assert cid.as_hex() == "00000000"


def test_can_id_standard_max_boundary():
    """STANDARD_ID_MAX=0x7FF 是合法边界（不抛）。"""
    cid = CanId(STANDARD_ID_MAX)
    assert cid.value == 0x7FF
    assert cid.as_hex() == "7FF"


def test_can_id_extended_max_boundary():
    """EXTENDED_ID_MAX=0x1FFFFFFF 是合法边界（不抛）。"""
    cid = CanId(EXTENDED_ID_MAX, is_extended=True)
    assert cid.value == 0x1FFFFFFF


def test_can_id_is_frozen():
    """CanId 是 frozen dataclass。"""
    cid = CanId(0x100)
    with pytest.raises((AttributeError, TypeError)):
        cid.value = 0x200


def test_can_frame_is_frozen():
    """CanFrame 是 frozen dataclass。"""
    frame = CanFrame(CanId(0x100), b"x")
    with pytest.raises((AttributeError, TypeError)):
        frame.is_fd = True


def test_can_frame_empty_data_dlc_zero():
    """空 data 的帧 DLC=0（合法）。"""
    frame = CanFrame(CanId(0x100), b"")
    assert frame.dlc == 0


def test_can_frame_to_payload_extended_fd():
    """扩展帧 + CAN-FD 的 to_payload 反映 isExtended/isFd。"""
    frame = CanFrame(CanId(0x12345, is_extended=True), b"\x01" * 10, is_fd=True)
    payload = frame.to_payload()
    assert payload["isExtended"] is True
    assert payload["isFd"] is True
    assert payload["canIdHex"] == "00012345"
    assert payload["dlc"] == 10


def test_can_frame_to_payload_empty_data():
    """空 data 的 to_payload dataHex 为空串。"""
    payload = CanFrame(CanId(0x100), b"").to_payload()
    assert payload["dataHex"] == ""
    assert payload["data"] == b""
    assert payload["dlc"] == 0


def test_can_frame_default_timestamp_and_index():
    """CanFrame 默认 timestamp=0.0, frame_index=0。"""
    frame = CanFrame(CanId(0x100), b"x")
    assert frame.timestamp == 0.0
    assert frame.frame_index == 0


def test_can_filter_mask_zero_matches_all():
    """mask=0 匹配所有 ID（不关心任何位）。"""
    f = CanFilter(id=0, mask=0)
    assert f.matches(CanId(0x000)) is True
    assert f.matches(CanId(0x7FF)) is True
    assert f.matches(CanId(0x12345, is_extended=True)) is True


def test_can_filter_explicit_standard_only():
    """is_extended=False 只匹配标准帧。"""
    f = CanFilter(id=0x100, mask=0x7FF, is_extended=False)
    assert f.matches(CanId(0x100)) is True
    assert f.matches(CanId(0x100, is_extended=True)) is False


def test_can_filter_id_bits_outside_mask_ignored():
    """filter.id 中 mask=0 的位被忽略（只比 mask=1 的位）。"""
    # id=0x7FF, mask=0x700 → 只比较高 3 位；0x7FF & 0x700 = 0x700
    f = CanFilter(id=0x7FF, mask=0x700)
    # 0x1FF & 0x700 = 0x100 ≠ 0x700 → 不匹配
    assert f.matches(CanId(0x1FF)) is False
    # 0x7FF & 0x700 = 0x700 → 匹配
    assert f.matches(CanId(0x7FF)) is True


def test_can_filter_extended_mismatch_short_circuits_before_mask():
    """is_extended 不匹配时直接返回 False（不走 mask 比较）。"""
    f = CanFilter(id=0x100, mask=0, is_extended=True)
    # mask=0 会匹配所有，但 is_extended=True 阻止标准帧
    assert f.matches(CanId(0x100)) is False
    assert f.matches(CanId(0x100, is_extended=True)) is True
