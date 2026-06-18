"""OTA 传输通道适配器 — 把 SerialTransport 适配成 OtaByteChannel。

SerialTransport 是 callback 推送（on_bytes_received），无阻塞 read；
OTA 协议需要带超时的阻塞读（等 ACK/NAK）。本适配器：
- 订阅 transport 的 on_bytes_received，把字节收进队列。
- ``read(timeout_ms)`` 从队列取字节，超时返回 b""。
- ``write`` 直接转发到 transport.write。

约束：本模块依赖 SerialTransport 抽象与标准库，不 import ui。
"""

from __future__ import annotations

import queue
import threading

from embeddebug.ota.protocols.base import OtaByteChannel
from embeddebug.serial_station.drivers import SerialTransport


class SerialTransportAdapter(OtaByteChannel):
    """把 SerialTransport 适配为 OTA 字节通道（带超时读）。"""

    def __init__(self, transport: SerialTransport) -> None:
        self._transport = transport
        self._rx: queue.Queue[int] = queue.Queue()
        self._lock = threading.Lock()
        transport.on_bytes_received(self._on_bytes)

    def _on_bytes(self, data: bytes) -> None:
        """transport 推送回调：字节入队。"""

        for byte in data:
            self._rx.put(byte)

    def write(self, data: bytes) -> int:
        """转发写入到 transport。"""

        return self._transport.write(data)

    def read(self, timeout_ms: int) -> bytes:
        """带超时读字节：尽可能读出已缓冲字节，至少等一个或超时返回 b""。"""

        # 先阻塞等第一个字节（超时即返回空）。
        try:
            first = self._rx.get(timeout=timeout_ms / 1000.0)
        except queue.Empty:
            return b""
        buffer = bytearray([first])
        # 非阻塞把后续已缓冲字节全部读出（同一 ACK 帧的多字节）。
        while True:
            try:
                buffer.append(self._rx.get_nowait())
            except queue.Empty:
                break
        return bytes(buffer)

    def drain(self) -> bytes:
        """非阻塞读出全部已缓冲字节（清空接收队列，握手前调用）。"""

        buffer = bytearray()
        while True:
            try:
                buffer.append(self._rx.get_nowait())
            except queue.Empty:
                break
        return bytes(buffer)
