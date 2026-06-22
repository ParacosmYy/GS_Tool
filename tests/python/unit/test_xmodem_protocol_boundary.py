"""XmodemProtocol XMODEM/XMODEM-CRC 状态机边界测试。

ota/test_ota_engine.py 已覆盖 TransferEngine 集成；本文件聚焦 XmodemProtocol
单元边界：分块/握手/next_block/handle_response/finish/make_xmodem 工厂。

覆盖：
1. make_xmodem 工厂（XMODEM_CRC → use_crc=True / XMODEM → use_crc=False）。
2. _slice_blocks 128B 分块 + 末块 0x1A 填充 + 空固件至少 1 块。
3. total_blocks / blocks_sent / retries 初始值。
4. start 握手（CRC 期望 'C' / CKSUM 期望 NAK / 错误响应 False / 空响应 False）。
5. next_block sequence 递增 + header SOH + CRC 模式 2 字节 / CKSUM 模式 1 字节 + 发完 None。
6. handle_response ACK 推进 index / NAK 重传 / 空 timeout 重传 / CAN×2 中止。
7. finish EOT + ACK 成功 / 全 NAK 失败。
"""

from __future__ import annotations

from embeddebug.ota.protocols.base import (
    ACK,
    C,
    CAN,
    EOT,
    NAK,
    SOH,
    OtaProtocolKind,
)
from embeddebug.ota.protocols.xmodem import (
    _BLOCK_SIZE,
    _MAX_RETRIES,
    XmodemProtocol,
    make_xmodem,
)


class _FakeChannel:
    """可控字节通道：预设 read 返回队列 + 记录 write。"""

    def __init__(self, reads: list[bytes] | None = None) -> None:
        self._reads = list(reads) if reads else []
        self.written: list[bytes] = []

    def write(self, data: bytes) -> int:
        self.written.append(bytes(data))
        return len(data)

    def read(self, timeout_ms: int) -> bytes:
        if self._reads:
            return self._reads.pop(0)
        return b""


# ── make_xmodem 工厂 ─────────────────────────────────────────────
def test_make_xmodem_crc_kind():
    proto = make_xmodem(b"\x00" * 128, OtaProtocolKind.XMODEM_CRC)
    assert proto._use_crc is True


def test_make_xmodem_cksum_kind():
    proto = make_xmodem(b"\x00" * 128, OtaProtocolKind.XMODEM)
    assert proto._use_crc is False


# ── _slice_blocks 分块 ───────────────────────────────────────────
def test_slice_single_full_block():
    """128 字节正好 1 块，无需填充。"""

    proto = XmodemProtocol(b"\x01" * 128)
    assert proto.total_blocks == 1
    assert len(proto._blocks[0]) == _BLOCK_SIZE


def test_slice_pads_last_block_with_0x1a():
    """末块不足 128 用 0x1A 填充到 128。"""

    proto = XmodemProtocol(b"\xAB" * 130)  # 1 整块 + 2 字节
    assert proto.total_blocks == 2
    last = proto._blocks[1]
    assert len(last) == _BLOCK_SIZE
    assert last[0] == 0xAB and last[1] == 0xAB
    assert last[2] == 0x1A  # 填充起始


def test_slice_empty_firmware_at_least_one_block():
    """空固件仍切出至少 1 块（全填充）。"""

    proto = XmodemProtocol(b"")
    assert proto.total_blocks == 1
    assert proto._blocks[0] == bytes([0x1A] * _BLOCK_SIZE)


# ── 初始计数器 ────────────────────────────────────────────────────
def test_initial_counters():
    proto = XmodemProtocol(b"\x00" * 256)
    assert proto.blocks_sent == 0
    assert proto.retries == 0


# ── start 握手 ────────────────────────────────────────────────────
def test_start_crc_expects_c():
    proto = XmodemProtocol(b"\x00" * 128, use_crc=True)
    assert proto.start(_FakeChannel(reads=[bytes([C])])) is True


def test_start_cksum_expects_nak():
    proto = XmodemProtocol(b"\x00" * 128, use_crc=False)
    assert proto.start(_FakeChannel(reads=[bytes([NAK])])) is True


def test_start_wrong_response_returns_false():
    proto = XmodemProtocol(b"\x00" * 128, use_crc=True)
    assert proto.start(_FakeChannel(reads=[bytes([NAK])])) is False  # CRC 要 C


def test_start_empty_response_returns_false():
    proto = XmodemProtocol(b"\x00" * 128, use_crc=True)
    assert proto.start(_FakeChannel(reads=[b""])) is False


# ── next_block ────────────────────────────────────────────────────
def test_next_block_crc_checksum_is_2_bytes():
    proto = XmodemProtocol(b"\x00" * 128, use_crc=True)
    block = proto.next_block()
    assert block is not None
    assert block.header == SOH
    assert block.sequence == 1
    assert len(block.checksum) == 2  # CRC-16 hi+lo


def test_next_block_cksum_checksum_is_1_byte():
    proto = XmodemProtocol(b"\x00" * 128, use_crc=False)
    block = proto.next_block()
    assert block is not None
    assert len(block.checksum) == 1  # 8-bit


def test_next_block_sequence_increments():
    """next_block 的 sequence 随 _index 递增（需 ACK 推进 index）。"""

    proto = XmodemProtocol(b"\x00" * 256)  # 2 块
    b1 = proto.next_block()
    assert b1.sequence == 1
    proto.handle_response(bytes([ACK]))  # 推进 index
    b2 = proto.next_block()
    assert b2.sequence == 2


def test_next_block_none_after_all_sent():
    """所有块 ACK 后 next_block 返回 None（触发 finish EOT）。"""

    proto = XmodemProtocol(b"\x00" * 128)  # 1 块
    proto.next_block()
    proto.handle_response(bytes([ACK]))  # 推进 index 到 1
    assert proto.next_block() is None  # index 1 >= total_blocks 1


def test_next_block_increments_blocks_sent():
    proto = XmodemProtocol(b"\x00" * 256)
    proto.next_block()
    proto.next_block()
    assert proto.blocks_sent == 2


# ── handle_response ───────────────────────────────────────────────
def test_handle_response_ack_advances():
    proto = XmodemProtocol(b"\x00" * 256)
    proto.next_block()
    assert proto.handle_response(bytes([ACK])) is True
    assert proto._index == 1


def test_handle_response_nak_increments_retries():
    proto = XmodemProtocol(b"\x00" * 128)
    proto.next_block()
    assert proto.handle_response(bytes([NAK])) is False
    assert proto.retries == 1
    assert proto._index == 0  # 未推进


def test_handle_response_empty_timeout_increments_retries():
    proto = XmodemProtocol(b"\x00" * 128)
    proto.next_block()
    assert proto.handle_response(b"") is False
    assert proto.retries == 1


def test_handle_response_double_can_aborts():
    """CAN×2 中止：_index 强制到末尾。"""

    proto = XmodemProtocol(b"\x00" * 256)
    proto.next_block()
    assert proto.handle_response(bytes([CAN, CAN])) is False
    assert proto._index == proto.total_blocks  # 强制结束


# ── finish ────────────────────────────────────────────────────────
def test_finish_eot_ack_success():
    proto = XmodemProtocol(b"\x00" * 128)
    ch = _FakeChannel(reads=[bytes([ACK])])
    assert proto.finish(ch) is True
    assert ch.written == [bytes([EOT])]


def test_finish_all_nak_fails_after_max_retries():
    """finish 重试 _MAX_RETRIES 次全失败返回 False。"""

    proto = XmodemProtocol(b"\x00" * 128)
    ch = _FakeChannel(reads=[bytes([NAK])] * _MAX_RETRIES)
    assert proto.finish(ch) is False
    assert len(ch.written) == _MAX_RETRIES  # 每次重试都 write EOT


def test_finish_empty_response_fails():
    proto = XmodemProtocol(b"\x00" * 128)
    ch = _FakeChannel(reads=[b""] * _MAX_RETRIES)
    assert proto.finish(ch) is False
