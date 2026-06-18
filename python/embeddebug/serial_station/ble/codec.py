"""BLE 通知/读写帧的流式编解码。"""

from __future__ import annotations

from dataclasses import dataclass

FRAME_NOTIFY = 0x01
FRAME_WRITE = 0x02
FRAME_READ_RESPONSE = 0x03

_HEADER_SIZE = 4
_MAX_VALUE = 255


@dataclass(frozen=True)
class BleFrameEvent:
    """解码产出的事件。"""

    type: int
    handle: int
    value: bytes

    @property
    def is_notify(self) -> bool:
        return self.type == FRAME_NOTIFY

    @property
    def is_write(self) -> bool:
        return self.type == FRAME_WRITE

    @property
    def is_read_response(self) -> bool:
        return self.type == FRAME_READ_RESPONSE


class BleFrameCodec:
    """BLE 帧编码器/解码器。"""

    def __init__(self) -> None:
        self._buffer = bytearray()

    @staticmethod
    def encode(event: BleFrameEvent) -> bytes:
        """将事件编码为字节序列。"""
        value = bytes(event.value)[:_MAX_VALUE]
        handle = event.handle & 0xFFFF
        return bytes([event.type & 0xFF, handle & 0xFF, (handle >> 8) & 0xFF, len(value)]) + value

    @staticmethod
    def encode_frame(frame_type: int, handle: int, value: bytes = b"") -> bytes:
        """便捷构造。"""
        return BleFrameCodec.encode(BleFrameEvent(frame_type, handle, value))

    def feed(self, data: bytes) -> list[BleFrameEvent]:
        """喂入字节流，返回事件列表。"""
        self._buffer.extend(data)
        events: list[BleFrameEvent] = []
        while len(self._buffer) >= _HEADER_SIZE:
            length = self._buffer[3]
            total = _HEADER_SIZE + length
            if len(self._buffer) < total:
                break
            frame_type = self._buffer[0]
            handle = self._buffer[1] | (self._buffer[2] << 8)
            value = bytes(self._buffer[_HEADER_SIZE:total])
            events.append(BleFrameEvent(frame_type, handle, value))
            del self._buffer[:total]
        return events

    def reset(self) -> None:
        self._buffer.clear()

    @staticmethod
    def decode(data: bytes) -> list[BleFrameEvent]:
        return BleFrameCodec().feed(data)
