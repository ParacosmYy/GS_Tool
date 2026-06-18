"""MQTT 配置、消息与主题匹配。"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class MqttConfig:
    """MQTT 连接配置。"""

    host: str = "localhost"
    port: int = 1883
    client_id: str = ""
    username: str = ""
    password: str = ""
    keepalive: int = 60
    qos: int = 0

    def __post_init__(self) -> None:
        if not 0 <= self.port <= 65535:
            raise ValueError(f"port out of range: {self.port}")
        if self.qos not in (0, 1, 2):
            raise ValueError(f"qos must be 0/1/2: {self.qos}")
        if self.keepalive < 0:
            raise ValueError("keepalive must be non-negative")


@dataclass(frozen=True)
class MqttMessage:
    """一条 MQTT 消息。"""

    topic: str
    payload: bytes
    qos: int = 0
    retain: bool = False
    timestamp: float = 0.0


@dataclass(frozen=True)
class MqttTopic:
    """主题订阅配置。"""

    name: str
    qos: int = 0


@dataclass
class MqttSubscription:
    """主题订阅（含回调）。"""

    topic_filter: str
    qos: int = 0
    callback = None  # Callable[[MqttMessage], None]


def topic_matches(filter_str: str, topic: str) -> bool:
    """MQTT 主题通配符匹配。

    ``+`` 匹配单层，``#`` 匹配剩余全部（必须在末尾）。
    """
    if filter_str == topic:
        return True
    if filter_str == "#":
        return True
    f_parts = filter_str.split("/")
    t_parts = topic.split("/")
    for i, fp in enumerate(f_parts):
        if fp == "#":
            return True
        if i >= len(t_parts):
            return False
        if fp == "+":
            continue
        if fp != t_parts[i]:
            return False
    return len(f_parts) == len(t_parts)
