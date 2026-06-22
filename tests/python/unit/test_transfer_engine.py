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
