"""Fake serial transport for tests and D2 substitute verification."""

from __future__ import annotations

from embeddebug.serial_station.drivers.base import BytesCallback, ErrorCallback, SerialPortConfig, SerialTransport


class FakeSerialTransport(SerialTransport):
    """In-memory serial transport with explicit RX injection."""

    def __init__(self, open_error: str | None = None) -> None:
        self._open_error = open_error
        self._is_open = False
        self._bytes_callbacks: list[BytesCallback] = []
        self._error_callbacks: list[ErrorCallback] = []
        self._config: SerialPortConfig | None = None
        self.written: list[bytes] = []

    @property
    def is_open(self) -> bool:
        return self._is_open

    @property
    def config(self) -> SerialPortConfig | None:
        return self._config

    def open(self, config: SerialPortConfig) -> bool:
        self._config = config
        if self._open_error:
            self._emit_error(self._open_error)
            self._is_open = False
            return False
        self._is_open = True
        return True

    def close(self) -> None:
        self._is_open = False

    def write(self, data: bytes) -> int:
        if not self._is_open:
            self._emit_error("transport_not_open")
            return 0
        payload = bytes(data)
        self.written.append(payload)
        return len(payload)

    def inject_rx(self, data: bytes) -> None:
        payload = bytes(data)
        for callback in list(self._bytes_callbacks):
            callback(payload)

    def on_bytes_received(self, callback: BytesCallback) -> None:
        self._bytes_callbacks.append(callback)

    def on_error(self, callback: ErrorCallback) -> None:
        self._error_callbacks.append(callback)

    def _emit_error(self, message: str) -> None:
        for callback in list(self._error_callbacks):
            callback(message)
