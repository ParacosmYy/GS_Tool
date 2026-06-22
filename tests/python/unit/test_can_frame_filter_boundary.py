"""CanFrame/CanId/CanFilter 边界扩展测试。

test_can.py 覆盖基础构造 + 编解码 + 单一 mask 匹配；本文件补 CanFilter.matches
边界（is_extended 过滤 + 多 bit mask + 边界 ID）+ CanId.is_standard/as_hex 边界
+ CanFrame.to_payload 全键 + frozen 契约 + DLC 边界。

覆盖：
1. CanFilter.matches is_extended=None（忽略帧类型）/ True（仅扩展）/ False（仅标准）。
2. CanFilter.matches 多 bit mask 精确匹配 + 全 0 mask 通配。
3. CanFilter.matches 扩展帧与标准帧互斥过滤。
4. CanId.is_standard（标准 True/扩展 False）+ as_hex 宽度（标准 3 位/扩展 8 位）。
5. CanId 边界值（0 / STANDARD_ID_MAX / EXTENDED_ID_MAX）+ 超限 ValueError。
6. CanFrame.to_payload 全键 + dataHex 空格分隔 + frozen 不可变。
7. CanFrame DLC 边界（0 空数据 / CAN_MAX_DLC / CAN_FD_MAX_DLC / 超限 ValueError）。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.can.frame import (
    CAN_FD_MAX_DLC,
    CAN_MAX_DLC,
    EXTENDED_ID_MAX,
    STANDARD_ID_MAX,
    CanFilter,
    CanFrame,
    CanId,
)


# ── CanFilter.matches is_extended 过滤 ───────────────────────────
def test_filter_is_extended_none_ignores_frame_type():
    """is_extended=None 不按帧类型过滤（只看 id & mask）。"""

    flt = CanFilter(id=0x100, mask=0x700, is_extended=None)
    assert flt.matches(CanId(0x123, is_extended=False)) is True
    assert flt.matches(CanId(0x123, is_extended=True)) is True


def test_filter_is_extended_true_only_matches_extended():
    """is_extended=True 仅匹配扩展帧。"""

    flt = CanFilter(id=0x100, mask=0x700, is_extended=True)
    assert flt.matches(CanId(0x123, is_extended=True)) is True
    assert flt.matches(CanId(0x123, is_extended=False)) is False


def test_filter_is_extended_false_only_matches_standard():
    """is_extended=False 仅匹配标准帧。"""

    flt = CanFilter(id=0x100, mask=0x700, is_extended=False)
    assert flt.matches(CanId(0x123, is_extended=False)) is True
    assert flt.matches(CanId(0x123, is_extended=True)) is False


# ── CanFilter.matches mask 边界 ──────────────────────────────────
def test_filter_all_zero_mask_matches_everything():
    """mask=0 → 任意 ID 都匹配（通配）。"""

    flt = CanFilter(id=0x000, mask=0x000)
    assert flt.matches(CanId(0x123)) is True
    assert flt.matches(CanId(0x7FF)) is True
    assert flt.matches(CanId(0x000)) is True


def test_filter_full_mask_exact_match():
    """mask=0x7FF → 精确匹配标准 11 位。"""

    flt = CanFilter(id=0x123, mask=0x7FF)
    assert flt.matches(CanId(0x123)) is True
    assert flt.matches(CanId(0x124)) is False


def test_filter_multi_bit_mask_partial_match():
    """mask=0x0F0 → 仅匹配 bit 4-7。"""

    flt = CanFilter(id=0x120, mask=0x0F0)
    assert flt.matches(CanId(0x123)) is True  # 0x12? 都匹配
    assert flt.matches(CanId(0x12F)) is True
    assert flt.matches(CanId(0x100)) is False  # 0x10? 不匹配 0x12?


# ── CanId.is_standard / as_hex ───────────────────────────────────
def test_can_id_is_standard_true_for_standard():
    assert CanId(0x123).is_standard() is True


def test_can_id_is_standard_false_for_extended():
    assert CanId(0x123, is_extended=True).is_standard() is False


def test_can_id_as_hex_standard_3_width():
    assert CanId(0x1).as_hex() == "001"
    assert CanId(0x7FF).as_hex() == "7FF"


def test_can_id_as_hex_extended_8_width():
    assert CanId(0x1, is_extended=True).as_hex() == "00000001"
    assert CanId(0x12345ABC, is_extended=True).as_hex() == "12345ABC"


# ── CanId 边界值 + 超限 ──────────────────────────────────────────
def test_can_id_zero_valid():
    cid = CanId(0x000)
    assert cid.value == 0


def test_can_id_standard_max_valid():
    cid = CanId(STANDARD_ID_MAX)
    assert cid.value == STANDARD_ID_MAX


def test_can_id_extended_max_valid():
    cid = CanId(EXTENDED_ID_MAX, is_extended=True)
    assert cid.value == EXTENDED_ID_MAX


def test_can_id_standard_over_max_raises():
    with pytest.raises(ValueError):
        CanId(STANDARD_ID_MAX + 1)


def test_can_id_extended_over_max_raises():
    with pytest.raises(ValueError):
        CanId(EXTENDED_ID_MAX + 1, is_extended=True)


def test_can_id_negative_raises():
    with pytest.raises(ValueError):
        CanId(-1)


# ── CanFrame.to_payload 全键 ─────────────────────────────────────
def test_to_payload_has_all_keys():
    frame = CanFrame(can_id=CanId(0x123), data=b"\xAB", timestamp=1.5, frame_index=7)
    payload = frame.to_payload()
    for key in ("canId", "canIdHex", "isExtended", "isFd", "dlc", "data", "dataHex", "timestamp", "frameIndex"):
        assert key in payload


def test_to_payload_data_hex_space_separated():
    """dataHex 用 bytes.hex(' ') 小写空格分隔。"""

    frame = CanFrame(can_id=CanId(0x1), data=b"\xDE\xAD\xBE\xEF")
    assert frame.to_payload()["dataHex"] == "de ad be ef"


def test_to_payload_empty_data():
    frame = CanFrame(can_id=CanId(0x1), data=b"")
    payload = frame.to_payload()
    assert payload["dlc"] == 0
    assert payload["data"] == b""
    assert payload["dataHex"] == ""


# ── CanFrame frozen 契约 ──────────────────────────────────────────
def test_can_frame_is_frozen():
    """frozen dataclass 赋值应抛 AttributeError（dataclasses.FrozenInstanceError 子类）。"""

    from dataclasses import FrozenInstanceError

    frame = CanFrame(can_id=CanId(0x1), data=b"\x01")
    with pytest.raises((AttributeError, FrozenInstanceError)):
        frame.can_id = CanId(0x2)  # type: ignore[misc]


# ── CanFrame DLC 边界 ─────────────────────────────────────────────
def test_can_frame_zero_data_valid():
    frame = CanFrame(can_id=CanId(0x1), data=b"")
    assert frame.dlc == 0


def test_can_frame_max_dlc_valid():
    frame = CanFrame(can_id=CanId(0x1), data=b"\x00" * CAN_MAX_DLC)
    assert frame.dlc == CAN_MAX_DLC


def test_can_frame_over_max_dlc_raises():
    with pytest.raises(ValueError):
        CanFrame(can_id=CanId(0x1), data=b"\x00" * (CAN_MAX_DLC + 1))


def test_can_fd_frame_max_dlc_valid():
    frame = CanFrame(can_id=CanId(0x1, is_extended=True), data=b"\x00" * CAN_FD_MAX_DLC, is_fd=True)
    assert frame.dlc == CAN_FD_MAX_DLC
    assert frame.is_fd is True


def test_can_fd_frame_over_max_dlc_raises():
    with pytest.raises(ValueError):
        CanFrame(
            can_id=CanId(0x1, is_extended=True),
            data=b"\x00" * (CAN_FD_MAX_DLC + 1),
            is_fd=True,
        )
