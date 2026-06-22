"""YmodemProtocol 生命周期 + make_ymodem 工厂边界测试。

test_ota_protocols.py 已覆盖 _slice_data_blocks/_build_info_block/total_data_blocks
/常量/枚举；本文件聚焦未经测试的生命周期：start/handle_response/finish/make_ymodem
+ next_block phase 状态机。

覆盖：
1. make_ymodem 工厂（YMODEM → stream=False / YMODEM_G → stream=True）。
2. start 握手（'C' 成功 / NAK 失败 / 空响应 False）。
3. next_block phase 状态机（info 块 sequence=0 SOH → data 块 STX → 发完 None）。
4. handle_response（ACK 推进 / NAK 重传 / 空 timeout / CAN×2 中止 / stream 模式直推）。
5. finish EOT+ACK 成功（YMODEM 写 EOT + 空块0）/ stream 模式直写 EOT。
6. blocks_sent / retries / total_data_blocks 属性。
"""

from __future__ import annotations

from embeddebug.ota.protocols.base import ACK, C, CAN, EOT, NAK, SOH, STX, OtaProtocolKind
from embeddebug.ota.protocols.ymodem import (
    _DATA_BLOCK,
    _MAX_RETRIES,
    _SMALL_BLOCK,
    YmodemProtocol,
    make_ymodem,
)


class _FakeChannel:
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


# ── make_ymodem 工厂 ─────────────────────────────────────────────
def test_make_ymodem_standard_not_stream():
    proto = make_ymodem(b"\x00" * 128, "fw.bin", OtaProtocolKind.YMODEM)
    assert proto._stream is False


def test_make_ymodem_g_is_stream():
    proto = make_ymodem(b"\x00" * 128, "fw.bin", OtaProtocolKind.YMODEM_G)
    assert proto._stream is True


# ── start 握手 ────────────────────────────────────────────────────
def test_start_expects_c():
    proto = YmodemProtocol(b"\x00" * 128, "fw.bin")
    assert proto.start(_FakeChannel(reads=[bytes([C])])) is True


def test_start_nak_returns_false():
    proto = YmodemProtocol(b"\x00" * 128, "fw.bin")
    assert proto.start(_FakeChannel(reads=[bytes([NAK])])) is False


def test_start_empty_returns_false():
    proto = YmodemProtocol(b"\x00" * 128, "fw.bin")
    assert proto.start(_FakeChannel(reads=[b""])) is False


# ── next_block phase 状态机 ──────────────────────────────────────
def test_next_block_info_block_sequence_zero_soh():
    proto = YmodemProtocol(b"\x00" * 128, "fw.bin")
    block = proto.next_block()
    assert block.sequence == 0
    assert block.header == SOH
    assert proto._phase == "data"


def test_next_block_data_block_stx():
    """信息块后，数据块用 STX（1KB）header（需 >= 1KB 固件）。"""

    proto = YmodemProtocol(b"\x00" * 1024, "fw.bin")
    proto.next_block()  # info 块
    block = proto.next_block()  # data 块
    assert block.header == STX
    assert block.sequence == 1


def test_next_block_none_after_all_data():
    """信息块 + 所有数据块发完后返回 None（触发 EOT）。"""

    proto = YmodemProtocol(b"\x00" * 128, "fw.bin")
    proto.next_block()  # info
    proto.next_block()  # data block 1
    assert proto.next_block() is None  # 数据发完


def test_next_block_increments_blocks_sent():
    proto = YmodemProtocol(b"\x00" * 256, "fw.bin")
    proto.next_block()  # info → sent=1
    proto.next_block()  # data → sent=2
    assert proto.blocks_sent == 2


# ── handle_response ───────────────────────────────────────────────
def test_handle_response_ack_advances():
    proto = YmodemProtocol(b"\x00" * 256, "fw.bin")
    proto.next_block()  # info
    proto.next_block()  # data 0
    assert proto.handle_response(bytes([ACK])) is True
    assert proto._index == 1


def test_handle_response_nak_increments_retries():
    proto = YmodemProtocol(b"\x00" * 128, "fw.bin")
    assert proto.handle_response(bytes([NAK])) is False
    assert proto.retries == 1


def test_handle_response_empty_increments_retries():
    proto = YmodemProtocol(b"\x00" * 128, "fw.bin")
    assert proto.handle_response(b"") is False
    assert proto.retries == 1


def test_handle_response_double_can_aborts():
    proto = YmodemProtocol(b"\x00" * 256, "fw.bin")
    assert proto.handle_response(bytes([CAN, CAN])) is False
    assert proto._index == proto.total_data_blocks


def test_handle_response_stream_mode_always_advances():
    """YMODEM-g stream 模式：handle_response 不读，直接 True。"""

    proto = YmodemProtocol(b"\x00" * 128, "fw.bin", stream=True)
    assert proto.handle_response(b"") is True  # 即使空也推进
    assert proto.handle_response(bytes([NAK])) is True


# ── finish ────────────────────────────────────────────────────────
def test_finish_ymodem_eot_then_empty_block():
    """YMODEM finish：EOT+ACK 成功 + 空块0 收尾（读 'C' + 读 ACK）。"""

    proto = YmodemProtocol(b"\x00" * 128, "fw.bin")
    ch = _FakeChannel(reads=[bytes([ACK]), bytes([C]), bytes([ACK])])
    assert proto.finish(ch) is True
    # 第一个 write 是 EOT，后续是空块0（header+seq+~seq+data+crc）。
    assert ch.written[0] == bytes([EOT])
    assert len(ch.written) == 2  # EOT + 空块0


def test_finish_all_nak_fails():
    """EOT 全 NAK → finish 返回 False（重试 _MAX_RETRIES）。"""

    proto = YmodemProtocol(b"\x00" * 128, "fw.bin")
    ch = _FakeChannel(reads=[bytes([NAK])] * _MAX_RETRIES)
    assert proto.finish(ch) is False


def test_finish_stream_mode_writes_eot_only():
    """YMODEM-g stream：finish 直接写 EOT（不等 ACK）+ 空块0（不等读）。"""

    proto = YmodemProtocol(b"\x00" * 128, "fw.bin", stream=True)
    ch = _FakeChannel()
    assert proto.finish(ch) is True
    assert ch.written[0] == bytes([EOT])
    assert len(ch.written) == 2  # EOT + 空块0


# ── 属性 ──────────────────────────────────────────────────────────
def test_initial_counters():
    proto = YmodemProtocol(b"\x00" * 256, "fw.bin")
    assert proto.blocks_sent == 0
    assert proto.retries == 0
    assert proto.total_data_blocks == 1


def test_constants_contract():
    assert _DATA_BLOCK == 1024
    assert _SMALL_BLOCK == 128
    assert _MAX_RETRIES == 10
