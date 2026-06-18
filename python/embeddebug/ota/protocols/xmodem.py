"""XMODEM / XMODEM-CRC 协议状态机。

XMODEM：128 字节块 + 8 位校验和；接收方发 NAK 启动。
XMODEM-CRC：128 字节块 + CRC-16；接收方发 'C' 启动（更可靠，实际最常用）。

协议帧结构：
- CKSUM: [SOH][seq][~seq][128B data][cksum]
- CRC:   [SOH][seq][~seq][128B data][crc_hi][crc_lo]

握手 → 逐块发送等 ACK → EOT 收尾。NAK 重传当前块，超时重传，CAN×2 中止。
"""

from __future__ import annotations

from embeddebug.ota.protocols.base import (
    ACK, C, CAN, EOT, NAK, SOH,
    OtaBlock, OtaByteChannel, OtaProtocol, OtaProtocolKind,
    checksum_8bit, crc16_xmodem, pad_block,
)

_BLOCK_SIZE = 128
_MAX_RETRIES = 10
_HANDSHAKE_TIMEOUT_MS = 1000
_ACK_TIMEOUT_MS = 1000


class XmodemProtocol(OtaProtocol):
    """XMODEM（CKSUM）或 XMODEM-CRC 状态机。

    ``use_crc=True`` 走 CRC-16（XMODEM-CRC），握手用 'C'；
    ``use_crc=False`` 走 8 位校验和（经典 XMODEM），握手用 NAK。
    """

    def __init__(self, firmware: bytes, *, use_crc: bool = True) -> None:
        self._firmware = firmware
        self._use_crc = use_crc
        self._blocks = self._slice_blocks()
        self._index = 0            # 下一个待发送块序（0 起，帧 seq = index+1）
        self._sent = 0             # 已发送块数（含重传）
        self._retries = 0          # 累计重传
        self._channel: OtaByteChannel | None = None

    def _slice_blocks(self) -> list[bytes]:
        """把固件切成 128B 块，末块用 0x1A 填充。"""

        return [pad_block(self._firmware[i:i + _BLOCK_SIZE], _BLOCK_SIZE)
                for i in range(0, max(len(self._firmware), 1), _BLOCK_SIZE)]

    # ── OtaProtocol 实现 ────────────────────────────────────────────
    def start(self, channel: OtaByteChannel) -> bool:
        """握手：等待接收方发 NAK（CKSUM）或 'C'（CRC）启动。"""

        self._channel = channel
        expected = bytes([C if self._use_crc else NAK])
        response = channel.read(_HANDSHAKE_TIMEOUT_MS)
        if response and expected[0] in response:
            return True
        return False

    def next_block(self) -> OtaBlock | None:
        """返回下一块；固件发完返回 None（触发 finish EOT）。"""

        if self._index >= len(self._blocks):
            return None
        seq = (self._index + 1) & 0xFF
        data = self._blocks[self._index]
        checksum = self._compute_checksum(data)
        self._sent += 1
        return OtaBlock(sequence=seq, header=SOH, data=data, checksum=checksum)

    def handle_response(self, response: bytes) -> bool:
        """处理 ACK/NAK/CAN；返回是否确认（推进 index）。"""

        if not response:
            # 超时：重传当前块（不推进 index），累计重试。
            self._retries += 1
            return False
        if response[0] == ACK:
            self._index += 1
            return True
        if response[0] == NAK:
            self._retries += 1
            return False
        if response.count(CAN) >= 2:
            # 接收方取消，中止。
            self._index = len(self._blocks)  # 强制结束
            return False
        self._retries += 1
        return False

    def finish(self, channel: OtaByteChannel) -> bool:
        """发送 EOT 并等待 ACK（部分实现需双 EOT，这里单 EOT + 重试）。"""

        for _ in range(_MAX_RETRIES):
            channel.write(bytes([EOT]))
            response = channel.read(_ACK_TIMEOUT_MS)
            if response and response[0] == ACK:
                return True
        return False

    @property
    def blocks_sent(self) -> int:
        return self._sent

    @property
    def retries(self) -> int:
        return self._retries

    # ── 内部 ────────────────────────────────────────────────────────
    def _compute_checksum(self, data: bytes) -> bytes:
        if self._use_crc:
            crc = crc16_xmodem(data)
            return bytes([(crc >> 8) & 0xFF, crc & 0xFF])
        return bytes([checksum_8bit(data)])

    @property
    def total_blocks(self) -> int:
        """固件总块数（供进度计算）。"""

        return len(self._blocks)


def make_xmodem(firmware: bytes, kind: OtaProtocolKind) -> XmodemProtocol:
    """按协议类型构造 XMODEM 状态机（CKSUM / CRC）。"""

    use_crc = kind == OtaProtocolKind.XMODEM_CRC
    return XmodemProtocol(firmware, use_crc=use_crc)
