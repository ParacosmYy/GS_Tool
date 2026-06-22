"""MQTT 内存 broker 桩（替身验证）。"""

from __future__ import annotations

from collections.abc import Callable

from embeddebug.serial_station.mqtt.message import (
    MqttConfig,
    MqttSubscription,
    topic_matches,
)

BytesCallback = Callable[[bytes], None]
ErrorCallback = Callable[[str], None]


class MqttClientStub:
    """内存 MQTT broker 模拟，匹配 SerialTransport 字节契约。"""

    def __init__(self) -> None:
        self._is_open = False
        self._config: MqttConfig | None = None
        self._subscriptions: list[MqttSubscription] = []
        self._bytes_callbacks: list[BytesCallback] = []
        self._error_callbacks: list[ErrorCallback] = []
        self.written: list[bytes] = []

    @property
    def is_open(self) -> bool:
        return self._is_open

    @property
    def config(self) -> MqttConfig | None:
        return self._config

    def open(self, config=None) -> bool:
        """连接 broker。"""
        self._config = config if isinstance(config, MqttConfig) else MqttConfig()
        self._is_open = True
        return True

    def close(self) -> None:
        self._is_open = False
        self._subscriptions.clear()

    def write(self, data: bytes) -> int:
        if not self._is_open:
            self._emit_error("transport_not_open")
            return 0
        self.written.append(bytes(data))
        return len(data)

    def on_bytes_received(self, callback: BytesCallback) -> None:
        self._bytes_callbacks.append(callback)

    def on_error(self, callback: ErrorCallback) -> None:
        self._error_callbacks.append(callback)

    def connect(self, config: MqttConfig) -> bool:
        """MQTT 语义连接。"""
        return self.open(config)

    def subscribe(self, topic_filter: str, qos: int = 0) -> bool:
        """订阅主题。"""
        if not self._is_open:
            self._emit_error("transport_not_open")
            return False
        self._subscriptions.append(MqttSubscription(topic_filter=topic_filter, qos=qos))
        return True

    def publish(self, topic: str, payload: bytes | str, qos: int = 0, retain: bool = False) -> int:
        """发布消息，按通配符分发给匹配的订阅者。"""
        if not self._is_open:
            self._emit_error("transport_not_open")
            return 0
        count = 0
        for sub in self._subscriptions:
            if topic_matches(sub.topic_filter, topic):
                count += 1
        return count

    @property
    def subscription_count(self) -> int:
        return len(self._subscriptions)

    def _emit_error(self, message: str) -> None:
        for cb in list(self._error_callbacks):
            cb(message)
