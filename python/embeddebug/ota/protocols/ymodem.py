"""YMODEM / YMODEM-g 协议状态机。

YMODEM 基于 XMODEM-CRC：
- 块 0 传文件信息（文件名\0+大小十进制\0），128B。
- 数据块用 1KB（STX）或 128B（SOH，末块）。
- 传输结束发空块名（块 0 全 0）收尾。

YMODEM-g：流式发送，不等待 ACK（接收方不回 ACK），出错发 CAN×2 中止。
速度最快但不可重传；本实现流式发送 + 末尾 EOT。
"""

from __future__ import annotations

import os

from embeddebug.ota.protocols.base import (
    ACK, C, CAN, EOT, NAK, SOH, STX,
    OtaBlock, OtaByteChannel, OtaProtocol, OtaProtocolKind,
    crc16_xmodem, pad_block,
)
from embeddebug.ota.protocols.xmodem import XmodemProtocol

_DATA_BLOCK = 1024     # YMODEM 默认 1KB 数据块
_SMALL_BLOCK = 128     # 末块不足时用 128B
_MAX_RETRIES = 10
_HANDSHAKE_TIMEOUT_MS = 3000
_ACK_TIMEOUT_MS = 1000


class YmodemProtocol(OtaProtocol):
    """YMODEM（CRC + 1KB 块 + 文件信息）或 YMODEM-g（流式无 ACK）。

    帧序列：[块0 文件信息] + [数据块…] + [EOT] + [空块0 收尾]。
    """

    def __init__(
        self, firmware: bytes, filename: str, *, stream: bool = False
    ) -> None:
        self._firmware = firmware
        self._filename = filename
        self._stream = stream  # True = YMODEM-g（无 ACK）
        self._phase = "info"   # info -> data -> eot -> close
        self._blocks = self._slice_data_blocks()
        self._index = 0
        self._sent = 0
        self._retries = 0
        self._channel: OtaByteChannel | None = None

    def _slice_data_blocks(self) -> list[tuple[int, bytes]]:
        """切数据块：前 n-1 块 1KB(STX)，末块 128B(SOH)。"""

        blocks: list[tuple[int, bytes]] = []
        fw = self._firmware
        for i in range(0, max(len(fw), 1), _DATA_BLOCK):
            chunk = fw[i:i + _DATA_BLOCK]
            if i + _DATA_BLOCK > len(fw) and len(chunk) <= _SMALL_BLOCK:
                # 末块不足 128 用 SOH 128B 填充。
                blocks.append((SOH, pad_block(chunk, _SMALL_BLOCK)))
            else:
                blocks.append((STX, pad_block(chunk, _DATA_BLOCK)))
        return blocks

    def _build_info_block(self) -> bytes:
        """块 0：文件名\\0 + 文件大小十进制\\0，填充到 128B。"""

        size_str = str(len(self._firmware)).encode("ascii")
        payload = os.path.basename(self._filename).encode("ascii") + b"\x00" + size_str + b"\x00"
        return pad_block(payload, _SMALL_BLOCK)

    # ── OtaProtocol 实现 ────────────────────────────────────────────
    def start(self, channel: OtaByteChannel) -> bool:
        """握手：YMODEM 等接收方 'C' 启动。"""

        self._channel = channel
        response = channel.read(_HANDSHAKE_TIMEOUT_MS)
        if response and C in response:
            return True
        return False

    def next_block(self) -> OtaBlock | None:
        """按 phase 返回：信息块 → 数据块 → None(触发 EOT)。"""

        if self._phase == "info":
            self._phase = "data"
            data = self._build_info_block()
            self._sent += 1
            crc = crc16_xmodem(data)
            return OtaBlock(0, SOH, data, bytes([(crc >> 8) & 0xFF, crc & 0xFF]))
        if self._phase == "data":
            if self._index >= len(self._blocks):
                return None  # 数据发完，finish 发 EOT
            header, data = self._blocks[self._index]
            seq = (self._index + 1) & 0xFF
            crc = crc16_xmodem(data)
            self._index += 1
            self._sent += 1
            return OtaBlock(seq, header, data, bytes([(crc >> 8) & 0xFF, crc & 0xFF]))
        return None

    def handle_response(self, response: bytes) -> bool:
        """YMODEM：ACK 推进；YMODEM-g：不读响应直接推进（流式）。"""

        if self._stream:
            return True  # 流式模式无 ACK，每块直接推进
        if not response:
            self._retries += 1
            return False
        if response[0] == ACK:
            return True
        if response[0] == NAK:
            self._retries += 1
            return False
        if response.count(CAN) >= 2:
            self._index = len(self._blocks)
            return False
        self._retries += 1
        return False

    def finish(self, channel: OtaByteChannel) -> bool:
        """EOT 确认 + 空块 0 收尾（YMODEM 协议要求结束空块）。"""

        # 1. EOT + ACK。
        eot_ok = False
        if self._stream:
            channel.write(bytes([EOT]))
            eot_ok = True
        else:
            for _ in range(_MAX_RETRIES):
                channel.write(bytes([EOT]))
                response = channel.read(_ACK_TIMEOUT_MS)
                if response and response[0] == ACK:
                    eot_ok = True
                    break
                self._retries += 1
        if not eot_ok:
            return False
        # 2. 空块 0 收尾（接收方再发 'C'，发全 0 块 0，等 ACK）。
        if not self._stream:
            _ = channel.read(_ACK_TIMEOUT_MS)  # 等 'C'
        empty = pad_block(b"", _SMALL_BLOCK)
        crc = crc16_xmodem(empty)
        block = OtaBlock(0, SOH, empty, bytes([(crc >> 8) & 0xFF, crc & 0xFF]))
        channel.write(bytes([block.header, 0x00, 0xFF]) + block.data + block.checksum)
        if not self._stream:
            _ = channel.read(_ACK_TIMEOUT_MS)  # 等最终 ACK
        return True

    @property
    def blocks_sent(self) -> int:
        return self._sent

    @property
    def retries(self) -> int:
        return self._retries

    @property
    def total_data_blocks(self) -> int:
        """数据块数（不含信息块，供进度计算）。"""

        return len(self._blocks)


def make_ymodem(
    firmware: bytes, filename: str, kind: OtaProtocolKind
) -> YmodemProtocol:
    """按协议类型构造 YMODEM 状态机（标准 / YMODEM-g）。"""

    stream = kind == OtaProtocolKind.YMODEM_G
    return YmodemProtocol(firmware, filename, stream=stream)
