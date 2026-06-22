"""TransferEngine 边界扩展测试（mock 协议驱动错误路径）。

ota/test_ota_engine.py 已覆盖成功/NAK 重传/握手失败/cancel（真实 XMODEM）；
本文件用 mock 协议覆盖剩余错误路径 + 帧格式 + _protocol_total 回退。

覆盖：
1. _protocol_total 回退（无 total_blocks/total_data_blocks 属性 → 0）。
2. _protocol_total 优先 total_blocks / 回退 total_data_blocks。
3. run() eot_not_acked 路径（finish 返回 False）。
4. run() 进度回调 on_progress 被调用（blocks_done, total）。
5. _send_block_await_ack 帧格式（header + seq + ~seq + data + checksum）。
6. _send_cancel 发 CAN×2。
7. run() 成功路径 TransferResult 字段（success/blocks_sent/blocks_acked/retries/error=None）。
"""

from __future__ import annotations

from embeddebug.ota.protocols.base import (
    ACK,
    CAN,
    OtaBlock,
)
from embeddebug.ota.transfer_engine import TransferEngine, _protocol_total


class _MockProtocol:
    """可控 mock 协议：预设 next_block 队列 + handle_response 行为。"""

    def __init__(
        self,
        blocks: list[OtaBlock] | None = None,
        ack_responses: list[bytes] | None = None,
        start_ok: bool = True,
        finish_ok: bool = True,
        total_blocks_attr: int | None = 2,
    ) -> None:
        self._blocks = list(blocks) if blocks else []
        self._ack_responses = list(ack_responses) if ack_responses else []
        self._start_ok = start_ok
        self._finish_ok = finish_ok
        self._sent = 0
        self._retries = 0
        self._total_blocks_attr = total_blocks_attr
        if total_blocks_attr is not None:
            self.total_blocks = total_blocks_attr

    def start(self, channel) -> bool:
        return self._start_ok

    def next_block(self) -> OtaBlock | None:
        if not self._blocks:
            return None
        self._sent += 1
        return self._blocks.pop(0)

    def handle_response(self, response: bytes) -> bool:
        # 默认 ACK 确认（channel.read 返回 ACK）。
        return bool(response and response[0] == ACK)

    def finish(self, channel) -> bool:
        return self._finish_ok

    @property
    def blocks_sent(self) -> int:
        return self._sent

    @property
    def retries(self) -> int:
        return self._retries


class _MockChannel:
    def __init__(self) -> None:
        self.written: list[bytes] = []

    def write(self, data: bytes) -> int:
        self.written.append(bytes(data))
        return len(data)

    def read(self, timeout_ms: int) -> bytes:
        return bytes([ACK])  # 默认 ACK


# ── _protocol_total ───────────────────────────────────────────────
def test_protocol_total_prefers_total_blocks():
    p = _MockProtocol(total_blocks_attr=5)
    assert _protocol_total(p) == 5


def test_protocol_total_falls_back_to_total_data_blocks():
    """无 total_blocks 属性时回退 total_data_blocks。"""

    class _Proto:
        total_data_blocks = 7

    assert _protocol_total(_Proto()) == 7


def test_protocol_total_no_attrs_returns_zero():
    """无任何 total 属性 → 0。"""

    class _Proto:
        pass

    assert _protocol_total(_Proto()) == 0


def test_protocol_total_non_int_attr_skipped():
    """total_blocks 非 int → 跳过回退。"""

    class _Proto:
        total_blocks = "not int"

    assert _protocol_total(_Proto()) == 0


# ── run() eot_not_acked 路径 ─────────────────────────────────────
def test_run_eot_not_acked_returns_error():
    """finish 返回 False → eot_not_acked。"""

    block = OtaBlock(sequence=1, header=0x01, data=b"\x00" * 4, checksum=b"\x00\x00")
    proto = _MockProtocol(blocks=[block], finish_ok=False, total_blocks_attr=1)
    engine = TransferEngine(proto, _MockChannel())
    result = engine.run()
    assert result.success is False
    assert result.error == "eot_not_acked"
    assert result.blocks_acked == 1  # 数据块已 ACK


# ── 进度回调 ──────────────────────────────────────────────────────
def test_run_progress_callback_invoked():
    """on_progress 应被调用（blocks_done, total）。"""

    block = OtaBlock(sequence=1, header=0x01, data=b"\x00" * 4, checksum=b"\x00\x00")
    proto = _MockProtocol(blocks=[block], total_blocks_attr=1)
    progress_calls = []
    engine = TransferEngine(proto, _MockChannel(), on_progress=lambda done, total: progress_calls.append((done, total)))
    engine.run()
    assert len(progress_calls) == 1
    assert progress_calls[0] == (1, 1)


# ── _send_block_await_ack 帧格式 ─────────────────────────────────
def test_send_block_frame_format():
    """帧 = header + seq + ~seq + data + checksum。"""

    block = OtaBlock(sequence=1, header=0x01, data=b"\xAB\xCD", checksum=b"\x12\x34")
    proto = _MockProtocol(blocks=[block], total_blocks_attr=1)
    ch = _MockChannel()
    engine = TransferEngine(proto, ch)
    engine.run()
    # 第一个 write 是数据帧。
    frame = ch.written[0]
    assert frame[0] == 0x01  # header
    assert frame[1] == 1  # seq
    assert frame[2] == (~1) & 0xFF  # ~seq = 0xFE
    assert frame[3:5] == b"\xAB\xCD"  # data
    assert frame[5:7] == b"\x12\x34"  # checksum


# ── _send_cancel ──────────────────────────────────────────────────
def test_send_cancel_writes_double_can():
    """cancel 后 run() 发 CAN×2。"""

    block = OtaBlock(sequence=1, header=0x01, data=b"\x00" * 4, checksum=b"\x00\x00")
    proto = _MockProtocol(blocks=[block], total_blocks_attr=1)
    ch = _MockChannel()
    engine = TransferEngine(proto, ch)
    engine.cancel()
    result = engine.run()
    assert result.success is False
    assert result.error == "cancelled"
    # 最后写入应为 CAN×2。
    assert ch.written[-1] == bytes([CAN, CAN])


# ── run() 成功路径 TransferResult ────────────────────────────────
def test_run_success_result_fields():
    """成功路径 TransferResult 字段完整。"""

    b1 = OtaBlock(sequence=1, header=0x01, data=b"\x00" * 4, checksum=b"\x00\x00")
    b2 = OtaBlock(sequence=2, header=0x01, data=b"\x00" * 4, checksum=b"\x00\x00")
    proto = _MockProtocol(blocks=[b1, b2], total_blocks_attr=2)
    engine = TransferEngine(proto, _MockChannel())
    result = engine.run()
    assert result.success is True
    assert result.error is None
    assert result.blocks_sent == 2
    assert result.blocks_acked == 2
    assert result.retries == 0


# ── 握手失败 ──────────────────────────────────────────────────────
def test_run_handshake_failure():
    """start 返回 False → handshake_failed。"""

    proto = _MockProtocol(start_ok=False, total_blocks_attr=1)
    engine = TransferEngine(proto, _MockChannel())
    result = engine.run()
    assert result.success is False
    assert result.error == "handshake_failed"
    assert result.blocks_acked == 0


# ── cancel 初始状态 ───────────────────────────────────────────────
def test_engine_initial_not_cancelled():
    proto = _MockProtocol(total_blocks_attr=1)
    engine = TransferEngine(proto, _MockChannel())
    assert engine._cancelled is False


def test_engine_cancel_sets_flag():
    proto = _MockProtocol(total_blocks_attr=1)
    engine = TransferEngine(proto, _MockChannel())
    engine.cancel()
    assert engine._cancelled is True
