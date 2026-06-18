"""RTT 内存传输桩：用于测试与 D2 替身验证。"""

from __future__ import annotations

from collections.abc import Callable

BytesCallback = Callable[[bytes], None]
ErrorCallback = Callable[[str], None]


class RttTransportStub:
    """内存传输桩：默认已连接，支持显式注入收向数据。"""

    def __init__(self, *, open: bool = True) -> None:
        self._is_open = open
        self._bytes_callbacks: list[BytesCallback] = []
        self._error_callbacks: list[ErrorCallback] = []
        self.written: list[bytes] = []

    @property
    def is_open(self) -> bool:
        return self._is_open

    def open(self) -> bool:
        """打开传输桩。"""
        self._is_open = True
        return True

    def close(self) -> None:
        """关闭传输桩。"""
        self._is_open = False

    def write(self, data: bytes) -> int:
        """记录待发数据并返回字节数；关闭时报错并返回 0。"""
        if not self._is_open:
            self._emit_error("transport_not_open")
            return 0
        payload = bytes(data)
        self.written.append(payload)
        return len(payload)

    def inject_received(self, data: bytes) -> None:
        """模拟目标→主机数据，分发给所有 bytes 回调。"""
        payload = bytes(data)
        for callback in list(self._bytes_callbacks):
            callback(payload)

    def on_bytes_received(self, callback: BytesCallback) -> None:
        """注册收向字节回调。"""
        self._bytes_callbacks.append(callback)

    def on_error(self, callback: ErrorCallback) -> None:
        """注册错误回调。"""
        self._error_callbacks.append(callback)

    def _emit_error(self, message: str) -> None:
        for callback in list(self._error_callbacks):
            callback(message)
