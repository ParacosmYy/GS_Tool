"""RTT 会话：在注入的传输层上驱动多通道收发。"""

from __future__ import annotations

from collections.abc import Callable

from embeddebug.serial_station.rtt.protocol import RttConfig, channel_index

BytesCallback = Callable[[bytes], None]
ErrorCallback = Callable[[str], None]


class _Ring:
    """定容字节环形缓冲：实现 FIFO + 写满覆盖最旧数据的环形语义。"""

    def __init__(self, capacity: int) -> None:
        if capacity <= 0:
            raise ValueError("capacity must be positive")
        self._capacity = capacity
        self._buf: bytearray = bytearray()

    @property
    def capacity(self) -> int:
        return self._capacity

    @property
    def available(self) -> int:
        return len(self._buf)

    def write(self, data: bytes) -> None:
        """追加数据；超出容量时丢弃最旧的溢出段，保留最近 capacity 字节。"""
        if not data:
            return
        self._buf.extend(data)
        overflow = len(self._buf) - self._capacity
        if overflow > 0:
            del self._buf[:overflow]

    def read(self, size: int = -1) -> bytes:
        """读取并消费最多 size 字节；size<0 读取全部可用。"""
        if size < 0 or size > len(self._buf):
            size = len(self._buf)
        out = bytes(self._buf[:size])
        del self._buf[:size]
        return out


class RttSession:
    """RTT 会话：在注入传输层上驱动多通道收发。"""

    def __init__(self, transport, config: RttConfig, *, default_receive_channel: str | None = None) -> None:
        self._transport = transport
        self._config = config
        self._is_open = False
        self._rings: dict[str, _Ring] = {
            channel.name: _Ring(channel.buffer_size)
            for channel in config.channels
            if channel.mode == "up"
        }
        up_names = [channel.name for channel in config.channels if channel.mode == "up"]
        self._active_up: str | None = default_receive_channel or (up_names[0] if up_names else None)
        self._byte_subs: list[BytesCallback] = []
        self._error_subs: list[ErrorCallback] = []

    @property
    def is_open(self) -> bool:
        return self._is_open

    @property
    def config(self) -> RttConfig:
        return self._config

    def open(self) -> bool:
        """注册传输回调并打开会话。"""
        self._transport.on_bytes_received(self._on_transport_bytes)
        self._transport.on_error(self._on_transport_error)
        self._is_open = True
        return True

    def close(self) -> None:
        """关闭会话。"""
        self._is_open = False

    def set_active_up_channel(self, name: str) -> bool:
        """切换接收路由目标的上行通道；不存在时报错返回 False。"""
        if name not in self._rings:
            self._emit_error("unknown_channel")
            return False
        self._active_up = name
        return True

    def send(self, channel_name: str, data: bytes) -> int:
        """向 down 通道写入数据并透传到传输层；返回接受字节数。"""
        if not self._is_open:
            self._emit_error("session_not_open")
            return 0
        try:
            index = channel_index(self._config.channels, channel_name)
        except KeyError:
            self._emit_error("unknown_channel")
            return 0
        channel = self._config.channels[index]
        if channel.mode != "down":
            self._emit_error("wrong_mode")
            return 0
        return self._transport.write(bytes(data))

    def recv(self, channel_name: str, size: int = -1) -> bytes:
        """从 up 通道读取并消费数据；size<0 读取全部可用。"""
        if not self._is_open:
            self._emit_error("session_not_open")
            return b""
        ring = self._rings.get(channel_name)
        if ring is None:
            self._emit_error("unknown_channel")
            return b""
        return ring.read(size)

    def available(self, channel_name: str) -> int:
        """返回指定 up 通道当前可读字节数。"""
        ring = self._rings.get(channel_name)
        return ring.available if ring is not None else 0

    def on_bytes_received(self, callback: BytesCallback) -> None:
        """订阅收向原始字节。"""
        self._byte_subs.append(callback)

    def on_error(self, callback: ErrorCallback) -> None:
        """订阅错误事件。"""
        self._error_subs.append(callback)

    def _on_transport_bytes(self, data: bytes) -> None:
        if self._active_up is None:
            self._emit_error("no_up_channel")
            return
        ring = self._rings.get(self._active_up)
        if ring is not None:
            ring.write(data)
        payload = bytes(data)
        for sub in list(self._byte_subs):
            sub(payload)

    def _on_transport_error(self, message: str) -> None:
        self._emit_error(message)

    def _emit_error(self, message: str) -> None:
        for sub in list(self._error_subs):
            sub(message)
