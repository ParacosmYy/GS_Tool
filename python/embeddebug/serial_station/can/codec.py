"""CAN 帧编解码器：SLCAN(Lawicel) ASCII 风格的流式解析与编码。"""

from __future__ import annotations

from typing import Any

from embeddebug.serial_station.can.frame import (
    CAN_FD_MAX_DLC,
    CanFrame,
    CanId,
)

PROTOCOL_NAME = "can_slcan"


class CanFrameCodec:
    """SLCAN ASCII 风格编解码器，支持流式分片输入与 CAN-FD。"""

    name = PROTOCOL_NAME

    def __init__(self) -> None:
        self._buffer = bytearray()
        self._frame_index = 0

    def encode(self, frame: CanFrame) -> bytes:
        """将 CanFrame 编码为 SLCAN ASCII 字节行(含 \\r 结束符)。"""
        prefix = "T" if frame.can_id.is_extended else "t"
        width = 8 if frame.can_id.is_extended else 3
        head = prefix + format(frame.can_id.value, f"0{width}X")
        body = head + f"{frame.dlc:X}".upper()
        data_hex = frame.data.hex().upper()
        return f"{body}{data_hex}\r".encode("ascii")

    def feed(self, data: bytes) -> list[dict[str, Any]]:
        """喂入字节流，返回零或多个帧事件字典。"""
        if data:
            self._buffer.extend(data)
        events: list[dict[str, Any]] = []
        while True:
            event = self._take_frame()
            if event is None:
                return events
            events.append(event)

    def reset(self) -> None:
        """清空缓冲区与帧计数器。"""
        self._buffer.clear()
        self._frame_index = 0

    def _take_frame(self) -> dict[str, Any] | None:
        end = self._buffer.find(0x0D)
        if end < 0:
            return None
        raw = bytes(self._buffer[: end + 1])
        del self._buffer[: end + 1]
        return self._parse(raw)

    def _parse(self, raw: bytes) -> dict[str, Any]:
        text = raw[:-1].decode("ascii", errors="replace") if raw.endswith(b"\r") else raw.decode("ascii", errors="replace")
        if len(text) < 1:
            return self._error("empty_frame", raw)
        prefix = text[0]
        is_extended = prefix in ("T", "R")
        is_standard = prefix in ("t", "r")
        if not (is_extended or is_standard):
            return self._error(f"unknown_prefix:{prefix}", raw)
        body = text[1:]
        id_width = 8 if is_extended else 3
        if len(body) < id_width + 1:
            return self._error("frame_too_short", raw)
        try:
            can_value = int(body[:id_width], 16)
            dlc = int(body[id_width], 16)
        except ValueError:
            return self._error("invalid_hex", raw)
        data_hex = body[id_width + 1:]
        max_dlc = CAN_FD_MAX_DLC if is_extended and dlc > 8 else 8
        if dlc > max_dlc and not (is_extended and dlc <= CAN_FD_MAX_DLC):
            return self._error("dlc_exceeds_limit", raw)
        expected_len = dlc * 2
        if len(data_hex) != expected_len:
            return self._error("data_length_mismatch", raw)
        try:
            data = bytes.fromhex(data_hex)
        except ValueError:
            return self._error("invalid_data_hex", raw)
        is_fd = is_extended and dlc > 8
        can_id = CanId(can_value, is_extended=is_extended)
        self._frame_index += 1
        frame = CanFrame(can_id=can_id, data=data, is_fd=is_fd, frame_index=self._frame_index)
        return {"type": "frame", "protocol_name": self.name, "payload": frame.to_payload(), "raw": raw}

    def _error(self, reason: str, raw: bytes) -> dict[str, Any]:
        return {"type": "error", "protocol_name": self.name, "payload": {"format": self.name, "reason": reason}, "raw": raw}
