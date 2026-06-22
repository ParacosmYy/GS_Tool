"""OTA 协议状态机与传输引擎单元测试。

用 FakeChannel 回放接收方响应（ACK/NAK/超时/'C'），断言块序、重传、CRC、EOT 收尾。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.ota.protocols import (
    OtaProtocolKind,
    make_protocol,
)
from embeddebug.ota.protocols.base import ACK, C, NAK, crc16_xmodem, checksum_8bit
from embeddebug.ota.transfer_engine import TransferEngine


class FakeChannel:
    """回放式字节通道：按预设响应序列返回，并记录所有写入。"""

    def __init__(self, responses: list[bytes]) -> None:
        self._responses = list(responses)
        self.written: list[bytes] = []

    def write(self, data: bytes) -> int:
        self.written.append(data)
        return len(data)

    def read(self, timeout_ms: int) -> bytes:
        if self._responses:
            return self._responses.pop(0)
        return b""  # 超时


def _fw(size: int) -> bytes:
    """构造 size 字节固件（可重复模式）。"""

    return bytes(i % 256 for i in range(size))


# ── 协议构造 ──────────────────────────────────────────────────────────
def test_make_protocol_supports_all_kinds():
    fw = _fw(300)
    for kind in OtaProtocolKind:
        proto = make_protocol(kind, fw, filename="fw.bin")
        assert proto is not None


# ── XMODEM-CRC 完整传输 ──────────────────────────────────────────────
def test_xmodem_crc_transfers_all_blocks():
    """握手 'C' → 3 块（300B → 3×128）每块 ACK → EOT ACK。"""

    fw = _fw(300)  # 3 块（末块填充）
    proto = make_protocol(OtaProtocolKind.XMODEM_CRC, fw)
    # 'C' 握手 + 3×ACK + EOT 的 ACK。
    channel = FakeChannel([bytes([C]), bytes([ACK]), bytes([ACK]), bytes([ACK]), bytes([ACK])])
    engine = TransferEngine(proto, channel)
    result = engine.run()
    assert result.success
    assert result.blocks_acked == 3
    assert result.error is None


def test_xmodem_crc_retries_on_nak():
    """NAK 触发重传，最终 ACK 后推进。"""

    fw = _fw(128)  # 1 块
    proto = make_protocol(OtaProtocolKind.XMODEM_CRC, fw)
    # 'C' + NAK(重传) + ACK + EOT ACK。
    channel = FakeChannel([bytes([C]), bytes([NAK]), bytes([ACK]), bytes([ACK])])
    engine = TransferEngine(proto, channel)
    result = engine.run()
    assert result.success
    assert result.retries >= 1


def test_xmodem_crc_aborts_after_max_retries():
    """连续 NAK 超过 max_retries → 中止 + 发 CAN×2。"""

    fw = _fw(128)
    proto = make_protocol(OtaProtocolKind.XMODEM_CRC, fw)
    # 'C' + 持续 NAK（超过重试上限）。
    channel = FakeChannel([bytes([C])] + [bytes([NAK])] * 20)
    engine = TransferEngine(proto, channel, max_retries=3)
    result = engine.run()
    assert not result.success
    assert result.error == "max_retries_exceeded"
    # 应发了 CAN×2 中止。
    assert any(bytes([0x18, 0x18]) == w for w in channel.written)


# ── XMODEM (CKSUM) ───────────────────────────────────────────────────
def test_xmodem_checksum_uses_nak_handshake():
    """经典 XMODEM 用 NAK 握手（非 'C'）。"""

    fw = _fw(128)
    proto = make_protocol(OtaProtocolKind.XMODEM, fw)
    channel = FakeChannel([bytes([NAK]), bytes([ACK]), bytes([ACK])])
    engine = TransferEngine(proto, channel)
    result = engine.run()
    assert result.success


def test_handshake_failure_returns_error():
    """握手无响应 → handshake_failed。"""

    fw = _fw(128)
    proto = make_protocol(OtaProtocolKind.XMODEM_CRC, fw)
    channel = FakeChannel([])  # 无响应，超时
    engine = TransferEngine(proto, channel)
    result = engine.run()
    assert not result.success
    assert result.error == "handshake_failed"


# ── YMODEM ───────────────────────────────────────────────────────────
def test_ymodem_transfers_with_info_block():
    """YMODEM：'C' 握手 → 信息块 ACK → 数据块 ACK → EOT ACK。"""

    fw = _fw(1100)  # 2 个 1KB 块（第二块填充）
    proto = make_protocol(OtaProtocolKind.YMODEM, fw, filename="app.bin")
    # 'C' + 信息块ACK + 数据块ACK×2 + EOT ACK + 收尾 'C' + 空0 ACK。
    channel = FakeChannel([
        bytes([C]),
        bytes([ACK]), bytes([ACK]), bytes([ACK]),  # 信息块 + 2 数据块
        bytes([ACK]),  # EOT
        bytes([C]),    # 收尾 'C'
        bytes([ACK]),  # 空0 ACK
    ])
    engine = TransferEngine(proto, channel)
    result = engine.run()
    assert result.success


def test_ymodem_g_streams_without_acks():
    """YMODEM-g：流式发送，不等 ACK，EOT 后收尾。"""

    fw = _fw(500)
    proto = make_protocol(OtaProtocolKind.YMODEM_G, fw, filename="app.bin")
    # YMODEM-g 只需初始 'C'，之后流式不读响应。
    channel = FakeChannel([bytes([C])] + [b""] * 20)
    engine = TransferEngine(proto, channel)
    result = engine.run()
    assert result.success


# ── 校验工具 ──────────────────────────────────────────────────────────
def test_crc16_xmodem_known_vectors():
    """CRC-16/XMODEM 已知向量（'123456789' → 0x31C3）。"""

    assert crc16_xmodem(b"123456789") == 0x31C3


def test_checksum_8bit_wraps_at_256():
    assert checksum_8bit(bytes(range(256))) == (sum(range(256)) & 0xFF)
    assert checksum_8bit(b"\xFF\x01") == 0x00  # 256 & 0xFF = 0


# ── 进度回调 ──────────────────────────────────────────────────────────
def test_progress_callback_reports_blocks_done():
    fw = _fw(300)  # 3 块
    proto = make_protocol(OtaProtocolKind.XMODEM_CRC, fw)
    channel = FakeChannel([bytes([C]), bytes([ACK]), bytes([ACK]), bytes([ACK]), bytes([ACK])])
    progress: list[tuple[int, int]] = []
    engine = TransferEngine(proto, channel, on_progress=lambda done, total: progress.append((done, total)))
    engine.run()
    assert progress == [(1, 3), (2, 3), (3, 3)]


def test_cancel_aborts_transfer():
    fw = _fw(500)
    proto = make_protocol(OtaProtocolKind.XMODEM_CRC, fw)
    channel = FakeChannel([bytes([C])] + [bytes([ACK])] * 50)
    engine = TransferEngine(proto, channel)
    engine.cancel()
    result = engine.run()
    assert not result.success
    assert result.error == "cancelled"
