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


# ---- Batch 155: OTA protocol 边界扩展 ----


def test_crc16_xmodem_two_bytes():
    """CRC-16/XMODEM of 2 bytes 0x01 0x02 = 0x1373（确定性已知值）。"""
    assert crc16_xmodem(b"\x01\x02") == 0x1373


def test_crc16_xmodem_idempotent():
    """同输入两次 crc 一致。"""
    data = b"test data"
    assert crc16_xmodem(data) == crc16_xmodem(data)


def test_crc16_xmodem_distinct_inputs():
    """不同输入不同 CRC。"""
    assert crc16_xmodem(b"\x01") != crc16_xmodem(b"\x02")


def test_pad_block_empty_data():
    """空数据填充满 block_size 的 0x1A。"""
    result = pad_block(b"", 128)
    assert len(result) == 128
    assert all(b == 0x1A for b in result)


def test_pad_block_fill_character_is_0x1a():
    """填充字符是 CPM EOF (0x1A)，不是 0x00。"""
    result = pad_block(b"\x01", 4)
    assert result == b"\x01\x1A\x1A\x1A"


def test_pad_block_zero_block_size():
    """block_size=0 时截断到 0 长度（len >= 0 总成立）。"""
    assert pad_block(b"data", 0) == b""


def test_checksum_8bit_known_value():
    """checksum_8bit 已知值。"""
    from embeddebug.ota.protocols.base import checksum_8bit
    assert checksum_8bit(b"\x01\x02\x03") == 6  # 1+2+3
    assert checksum_8bit(b"") == 0


def test_checksum_8bit_wraps_at_256():
    """校验和模 256。"""
    from embeddebug.ota.protocols.base import checksum_8bit
    # 256 = 0x100 → & 0xFF = 0
    assert checksum_8bit(bytes([255, 1])) == 0
    # 255 + 2 = 257 → & 0xFF = 1
    assert checksum_8bit(bytes([255, 2])) == 1


def test_ota_block_is_frozen():
    """OtaBlock 是 frozen dataclass。"""
    import pytest
    from embeddebug.ota.protocols.base import OtaBlock
    block = OtaBlock(sequence=1, header=SOH, data=b"x", checksum=b"\x00\x00")
    with pytest.raises((AttributeError, TypeError)):
        block.sequence = 2


def test_transfer_result_is_frozen():
    """TransferResult 是 frozen dataclass。"""
    import pytest
    from embeddebug.ota.protocols.base import TransferResult
    result = TransferResult(success=True, blocks_sent=10, blocks_acked=10, retries=0)
    with pytest.raises((AttributeError, TypeError)):
        result.success = False


def test_transfer_result_default_error_is_none():
    """TransferResult 默认 error=None。"""
    from embeddebug.ota.protocols.base import TransferResult
    result = TransferResult(success=True, blocks_sent=1, blocks_acked=1, retries=0)
    assert result.error is None


def test_ota_protocol_kind_enum_values():
    """OtaProtocolKind 4 种协议变体。"""
    from embeddebug.ota.protocols.base import OtaProtocolKind
    assert OtaProtocolKind.XMODEM.value == "xmodem"
    assert OtaProtocolKind.XMODEM_CRC.value == "xmodem-crc"
    assert OtaProtocolKind.YMODEM.value == "ymodem"
    assert OtaProtocolKind.YMODEM_G.value == "ymodem-g"


def test_control_byte_constants():
    """X/YMODEM 控制字节常量。"""
    from embeddebug.ota.protocols.base import ACK, C, CAN, EOT, NAK
    assert SOH == 0x01
    assert STX == 0x02
    assert EOT == 0x04
    assert ACK == 0x06
    assert NAK == 0x15
    assert CAN == 0x18
    assert C == 0x43


def test_ymodem_slice_exactly_128_bytes():
    """正好 128 字节 firmware → 1 个 SOH 块（末块路径）。"""
    proto = YmodemProtocol(b"\x00" * 128, "fw.bin")
    assert len(proto._blocks) == 1
    header, data = proto._blocks[0]
    assert header == SOH
    assert len(data) == 128


def test_ymodem_total_data_blocks_property():
    """total_data_blocks 返回数据块数（不含信息块）。"""
    proto = YmodemProtocol(b"\x00" * 2048, "fw.bin")
    assert proto.total_data_blocks == 2


def test_ymodem_build_info_block_basename_only():
    """_build_info_block 用 basename（去掉路径）。"""
    proto = YmodemProtocol(b"\x00" * 10, "/path/to/firmware.bin")
    info = proto._build_info_block()
    assert b"firmware.bin" in info
    assert b"/path/to/" not in info


def test_ymodem_build_info_block_empty_filename():
    """空文件名时 info block 仍 128 字节。"""
    proto = YmodemProtocol(b"\x00" * 10, "")
    info = proto._build_info_block()
    assert len(info) == 128
