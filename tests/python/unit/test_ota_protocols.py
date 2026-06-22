"""OTA 协议辅助函数单元测试 — crc16_xmodem + pad_block + Ymodem 块切分。

覆盖：crc16_xmodem 已知值、pad_block 填充、YmodemProtocol._slice_data_blocks
块切分（1KB/128B/末块）+ _build_info_block 文件信息。
"""

from __future__ import annotations

from embeddebug.ota.protocols.base import (
    SOH,
    STX,
    crc16_xmodem,
    pad_block,
)
from embeddebug.ota.protocols.ymodem import YmodemProtocol


def test_crc16_xmodem_known_value():
    """CRC-16/XMODEM of "123456789" = 0x31C3。"""
    assert crc16_xmodem(b"123456789") == 0x31C3


def test_crc16_xmodem_empty():
    assert crc16_xmodem(b"") == 0x0000


def test_crc16_xmodem_single_byte():
    result = crc16_xmodem(b"\x00")
    assert isinstance(result, int)


def test_pad_block_short():
    """短数据填充到 block_size。"""
    result = pad_block(b"\x01", 128)
    assert len(result) == 128
    assert result[0] == 0x01


def test_pad_block_exact():
    """正好 block_size 不填充。"""
    data = b"\x00" * 128
    assert pad_block(data, 128) == data


def test_pad_block_already_longer():
    """超长数据截断到 block_size。"""
    result = pad_block(b"\x00" * 200, 128)
    assert len(result) == 128


def test_ymodem_slice_empty_firmware():
    """空 firmware 至少 1 个块。"""
    proto = YmodemProtocol(b"", "empty.bin")
    assert len(proto._blocks) >= 1


def test_ymodem_slice_single_1kb_block():
    """正好 1024 字节 → 1 个 STX 块。"""
    proto = YmodemProtocol(b"\x00" * 1024, "fw.bin")
    assert len(proto._blocks) == 1
    header, data = proto._blocks[0]
    assert header == STX
    assert len(data) == 1024


def test_ymodem_slice_multiple_blocks():
    """2048 字节 → 2 个 STX 块。"""
    proto = YmodemProtocol(b"\x00" * 2048, "fw.bin")
    assert len(proto._blocks) == 2
    for header, _ in proto._blocks:
        assert header == STX


def test_ymodem_slice_last_block_small():
    """末块不足 128B → SOH 128B。"""
    proto = YmodemProtocol(b"\x00" * 1100, "fw.bin")  # 1KB + 76 bytes
    assert len(proto._blocks) == 2
    last_header, last_data = proto._blocks[-1]
    assert last_header == SOH
    assert len(last_data) == 128


def test_ymodem_build_info_block():
    """块 0 含文件名 + 大小。"""
    proto = YmodemProtocol(b"\x00" * 100, "firmware.bin")
    info = proto._build_info_block()
    assert len(info) == 128
    assert b"firmware.bin" in info
    assert b"100" in info
