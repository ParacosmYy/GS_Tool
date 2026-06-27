"""OTA 传输引擎单元测试 — mock 协议/通道的完整循环。

覆盖：TransferEngine.run 成功路径、cancel 中止、_protocol_total 辅助。
用 fake OtaProtocol + OtaByteChannel 模拟握手/块/ACK。
"""

from __future__ import annotations

from embeddebug.ota.protocols.base import (
    ACK,
    C,
    OtaBlock,
    OtaByteChannel,
    OtaProtocol,
)
from embeddebug.ota.transfer_engine import TransferEngine, _protocol_total


class _FakeChannel(OtaByteChannel):
    """可控读写的假通道。"""

    def __init__(self, responses: list[bytes]) -> None:
        self._responses = list(responses)
        self.written: list[bytes] = []

    def read(self, timeout_ms: int = 0) -> bytes:
        if self._responses:
            return self._responses.pop(0)
        return b""

    def write(self, data: bytes) -> int:
        self.written.append(bytes(data))
        return len(data)


class _FakeProtocol(OtaProtocol):
    """可控块序列的假协议。"""

    def __init__(self, blocks: list[OtaBlock], total: int) -> None:
        self._blocks = list(blocks)
        self.total_blocks = total
        self.eot_sent = False

    def start(self, channel: OtaByteChannel) -> bool:
        return bytes([C]) in channel.read(100)

    def next_block(self) -> OtaBlock | None:
        if self._blocks:
            return self._blocks.pop(0)
        return None

    def handle_response(self, response: bytes) -> bool:
        return bytes([ACK]) in response

    def send_eot(self, channel: OtaByteChannel) -> bool:
        channel.write(b"\x04")
        self.eot_sent = True
        return bytes([ACK]) in channel.read(100)


def _make_block(seq: int, data: bytes = b"\x00" * 128) -> OtaBlock:
    return OtaBlock(sequence=seq, header=1, data=data, checksum=b"\x00\x00")


def test_protocol_total():
    p = _FakeProtocol([], 1024)
    assert _protocol_total(p) == 1024


def test_engine_cancel():
    """cancel 设置标志。"""
    protocol = _FakeProtocol([], 0)
    channel = _FakeChannel([])
    engine = TransferEngine(protocol, channel)
    engine.cancel()
    assert engine._cancelled is True


def test_engine_custom_timeouts():
    """自定义 ack_timeout + max_retries。"""
    protocol = _FakeProtocol([], 0)
    channel = _FakeChannel([])
    engine = TransferEngine(protocol, channel, ack_timeout_ms=500, max_retries=3)
    assert engine._ack_timeout_ms == 500
    assert engine._max_retries == 3


class _MinimalProtocol:
    def __init__(self, blocks=None, start_ok=True, finish_ok=True, total_blocks=1):
        self._blocks = list(blocks or [])
        self._start_ok = start_ok
        self._finish_ok = finish_ok
        self.total_blocks = total_blocks
        self._sent = 0
        self._retries = 0

    def start(self, channel): return self._start_ok
    def next_block(self):
        if not self._blocks: return None
        self._sent += 1
        return self._blocks.pop(0)
    def handle_response(self, response: bytes) -> bool: return bool(response and response[0] == ACK)
    def finish(self, channel) -> bool: return self._finish_ok
    @property
    def blocks_sent(self): return self._sent
    @property
    def retries(self): return self._retries


def test_protocol_total_fallbacks():
    class TotalDataBlocks: total_data_blocks = 7
    class NoTotal: pass
    class BadTotal: total_blocks = "bad"
    assert _protocol_total(TotalDataBlocks()) == 7
    assert _protocol_total(NoTotal()) == 0
    assert _protocol_total(BadTotal()) == 0


def test_run_eot_not_acked_returns_error():
    engine = TransferEngine(_MinimalProtocol([_make_block(1)], finish_ok=False), _FakeChannel([bytes([ACK])]))
    result = engine.run()
    assert result.success is False and result.error == "eot_not_acked"
    assert result.blocks_acked == 1


def test_run_progress_callback_and_success_fields():
    calls = []
    protocol = _MinimalProtocol([_make_block(1), _make_block(2)], total_blocks=2)
    result = TransferEngine(protocol, _FakeChannel([bytes([ACK]), bytes([ACK])]),
                            on_progress=lambda done, total: calls.append((done, total))).run()
    assert result.success is True and result.error is None
    assert result.blocks_sent == 2 and result.blocks_acked == 2 and result.retries == 0
    assert calls == [(1, 2), (2, 2)]


def test_send_block_frame_format_and_cancel_frame():
    block = OtaBlock(sequence=1, header=0x01, data=b"\xAB\xCD", checksum=b"\x12\x34")
    channel = _FakeChannel([bytes([ACK])])
    TransferEngine(_MinimalProtocol([block]), channel).run()
    assert channel.written[0] == b"\x01\x01\xFE\xAB\xCD\x12\x34"
    cancel_channel = _FakeChannel([])
    engine = TransferEngine(_MinimalProtocol([_make_block(1)]), cancel_channel)
    engine.cancel()
    result = engine.run()
    assert result.error == "cancelled"
    assert cancel_channel.written[-1] == bytes([0x18, 0x18])


def test_run_handshake_failure_and_cancel_state():
    engine = TransferEngine(_MinimalProtocol(start_ok=False), _FakeChannel([]))
    assert engine._cancelled is False
    result = engine.run()
    assert result.success is False and result.error == "handshake_failed"
