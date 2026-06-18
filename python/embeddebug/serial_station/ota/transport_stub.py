"""内存型 OTA 传输替身。"""

from __future__ import annotations

from collections.abc import Callable


class OtaTransportStub:
    """纯内存传输通道。"""

    def __init__(self) -> None:
        self.written: list[bytes] = []
        self._total: bytearray = bytearray()
        self._rx_queue: bytearray = bytearray()
        self._on_write: Callable[[bytes], None] | None = None

    def write(self, data: bytes) -> int:
        payload = bytes(data)
        self.written.append(payload)
        self._total.extend(payload)
        if self._on_write is not None:
            self._on_write(payload)
        return len(payload)

    @property
    def total_bytes(self) -> bytes:
        return bytes(self._total)

    def reset(self) -> None:
        self.written.clear()
        self._total.clear()
        self._rx_queue.clear()

    def inject_ack(self, count: int = 1) -> None:
        self._rx_queue.extend(bytes([0x06]) * count)

    def inject_nak(self, count: int = 1) -> None:
        self._rx_queue.extend(bytes([0x15]) * count)

    def read_byte(self, timeout_ms: int = 0) -> int | None:
        del timeout_ms
        if self._rx_queue:
            return self._rx_queue.pop(0)
        return None
