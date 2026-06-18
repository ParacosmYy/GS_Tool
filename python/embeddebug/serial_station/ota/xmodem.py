"""XModem 传输：块构建/解析、CRC16-CCITT、发送状态机。"""

from __future__ import annotations

from embeddebug.serial_station.ota.config import OtaConfig, OtaProgress

SOH = 0x01
STX = 0x02
EOT = 0x04
ACK = 0x06
NAK = 0x15
CAN = 0x18
C = 0x43


def crc16_ccitt(data: bytes, init: int = 0x0000) -> int:
    """CRC-16/CCITT(XMODEM)。"""
    crc = init
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def checksum_8bit(data: bytes) -> int:
    """8 位算术校验和。"""
    return sum(data) & 0xFF


class XModemTransfer:
    """XModem 单文件发送器。"""

    def __init__(self, config: OtaConfig, progress: OtaProgress | None = None) -> None:
        self.config = config
        self.progress = progress or OtaProgress()
        self._retry = 0

    def build_block(self, index: int, data: bytes) -> bytes:
        """构建数据块。"""
        if not 0 <= index <= 0xFF:
            raise ValueError(f"块序号必须在 0~255 内: {index}")
        size = len(data)
        start = STX if size == 1024 else SOH
        header = bytes([start, index & 0xFF, (~index) & 0xFF])
        trailer = self._build_trailer(data)
        return header + data + trailer

    def _build_trailer(self, data: bytes) -> bytes:
        if self.config.use_crc:
            value = crc16_ccitt(data)
            return bytes([(value >> 8) & 0xFF, value & 0xFF])
        return bytes([checksum_8bit(data)])

    def build_eot(self) -> bytes:
        return bytes([EOT])

    def build_cancel(self) -> bytes:
        return bytes([CAN, CAN])

    @staticmethod
    def parse_ack(byte: int) -> str:
        if byte == ACK:
            return "ack"
        if byte == NAK:
            return "nak"
        if byte == CAN:
            return "can"
        if byte == C:
            return "c"
        return "unknown"

    def send_block(self, transport, index: int, data: bytes) -> int:
        """发送一个块。"""
        frame = self.build_block(index, data)
        transport.write(frame)
        self.progress.mark_sent(len(data))
        return len(frame)

    def handle_response(self, byte: int | None) -> str:
        """根据响应推进状态机。"""
        if byte is None:
            self.progress.mark_error()
            return self._on_negative()
        kind = self.parse_ack(byte)
        if kind == "ack":
            self._retry = 0
            return "advance"
        if kind == "can":
            self.progress.fail()
            return "abort"
        if kind in ("nak", "unknown"):
            self.progress.mark_error()
            return self._on_negative()
        return "idle"

    def _on_negative(self) -> str:
        self._retry += 1
        if self._retry > self.config.retry_count:
            self.progress.fail()
            return "abort"
        return "retry"
