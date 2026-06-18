"""OTA 协议基础 — 数据类型与协议契约。

定义 X/YMODEM 家族共享的控制字节、块结构、传输结果与 ``OtaProtocol`` 协议。
具体协议（XMODEM/XMODEM-CRC/YMODEM/YMODEM-g）在 ``xmodem.py`` / ``ymodem.py`` 实现，
由 ``transfer_engine`` 驱动。

约束：本模块只定义协议数据与契约，不 import ui 或具体传输实现。
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Protocol


# ── X/YMODEM 控制字节 ────────────────────────────────────────────────
SOH = 0x01   # 块头：128 字节数据块
STX = 0x02   # 块头：1024 字节数据块（YMODEM-1K）
EOT = 0x04   # 传输结束
ACK = 0x06   # 确认
NAK = 0x15   # 否认（请求重传，校验和模式）
CAN = 0x18   # 取消（连续两个 CAN 中止传输）
C = 0x43     # 'C'：请求 CRC 模式握手


class OtaProtocolKind(str, Enum):
    """支持的 OTA 协议变体。"""

    XMODEM = "xmodem"          # 128B 块 + CKSUM
    XMODEM_CRC = "xmodem-crc"  # 128B 块 + CRC-16
    YMODEM = "ymodem"          # 1KB 块 + CRC-16 + 文件信息块 0
    YMODEM_G = "ymodem-g"      # 流式无 ACK（最高速，出错 CAN 中止）


@dataclass(frozen=True)
class OtaBlock:
    """单个传输块（协议无关）。"""

    sequence: int              # 块序号（1 起，0 = 文件信息块）
    header: int                # SOH(128B) 或 STX(1KB)
    data: bytes                # 有效载荷（128 或 1024）
    checksum: bytes            # 校验（1B CKSUM 或 2B CRC-16，大端）


@dataclass(frozen=True)
class TransferResult:
    """一次传输的结果（成功/失败 + 统计）。"""

    success: bool
    blocks_sent: int           # 已发送块数（含重传）
    blocks_acked: int          # 已确认块数
    retries: int               # 重传总次数
    error: str | None = None   # 失败原因（成功时 None）


class OtaByteChannel(Protocol):
    """字节传输通道契约（OTA 引擎与具体 transport 解耦）。

    ``SerialTransportAdapter`` 把 ``SerialTransport`` 适配成本协议：
    callback 推送的接收字节收集到队列，``read`` 带超时取出。
    """

    def write(self, data: bytes) -> int:
        """发送字节，返回写入数。"""
        ...

    def read(self, timeout_ms: int) -> bytes:
        """阻塞读字节，超时返回 b""（供协议等待 ACK/NAK）。"""
        ...


class OtaProtocol(Protocol):
    """单次传输的协议状态机契约（由 transfer_engine 驱动）。

    生命周期：``start`` 握手 → 反复 ``next_block`` + ``handle_response`` →
    ``finish`` 收尾（EOT 确认）。每个协议变体实现本接口。
    """

    def start(self, channel: OtaByteChannel) -> bool:
        """执行握手（等待接收方 NAK/'C'），成功返回 True。"""
        ...

    def next_block(self) -> OtaBlock | None:
        """返回下一块；无更多数据返回 None（触发 EOT）。"""
        ...

    def handle_response(self, response: bytes) -> bool:
        """处理接收方响应（ACK/NAK/CAN）；返回是否确认当前块。

        返回 True=ACK（推进到下一块），False=NAK（重传），协议内部更新重试计数。
        """
        ...

    def finish(self, channel: OtaByteChannel) -> bool:
        """发送 EOT 并等待最终确认，成功返回 True。"""
        ...

    @property
    def blocks_sent(self) -> int:
        """已发送块数（含重传）。"""
        ...

    @property
    def retries(self) -> int:
        """累计重传次数。"""
        ...


# ── 校验工具 ─────────────────────────────────────────────────────────
def checksum_8bit(data: bytes) -> int:
    """XMODEM 校验和：所有字节模 256 求和的低 8 位。"""

    return sum(data) & 0xFF


def crc16_xmodem(data: bytes) -> int:
    """XMODEM/CRC-16 多项式 0x1021，初始 0x0000（CCITT，X/YMODEM 标准）。"""

    crc = 0x0000
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc


def pad_block(data: bytes, block_size: int) -> bytes:
    """把不足 block_size 的数据用 0x1A (CPM EOF) 填充到完整块。"""

    if len(data) >= block_size:
        return data[:block_size]
    return data + bytes([0x1A]) * (block_size - len(data))


__all__ = [
    "ACK", "CAN", "C", "EOT", "NAK", "SOH", "STX",
    "OtaBlock", "OtaByteChannel", "OtaProtocol", "OtaProtocolKind",
    "TransferResult", "checksum_8bit", "crc16_xmodem", "pad_block",
]
