"""MQTT v3.1.1 控制包编解码（纯 Python）。"""

from __future__ import annotations

import struct

# 包类型（高 nibble）。
CONNECT = 1
CONNACK = 2
PUBLISH = 3
PUBACK = 4
SUBSCRIBE = 8
SUBACK = 9
UNSUBSCRIBE = 10
PINGREQ = 12
PINGRESP = 13
DISCONNECT = 14


def encode_remaining_length(length: int) -> bytes:
    """编码变长剩余长度。"""
    out = bytearray()
    while True:
        byte = length % 128
        length //= 128
        if length > 0:
            byte |= 0x80
        out.append(byte)
        if length == 0:
            break
    return bytes(out)


def decode_remaining_length(data: bytes, offset: int = 0) -> tuple[int, int]:
    """解码变长剩余长度，返回 (value, bytes_consumed)。"""
    multiplier = 1
    value = 0
    pos = offset
    while True:
        if pos >= len(data):
            raise ValueError("incomplete remaining length")
        byte = data[pos]
        value += (byte & 0x7F) * multiplier
        pos += 1
        if byte & 0x80 == 0:
            break
        multiplier *= 128
        if multiplier > 128 * 128 * 128:
            raise ValueError("remaining length too large")
    return value, pos - offset


def _encode_string(s: str) -> bytes:
    """UTF-8 编码字符串（2 字节长度前缀）。"""
    encoded = s.encode("utf-8")
    return struct.pack(">H", len(encoded)) + encoded


def encode_connect(config) -> bytes:
    """编码 CONNECT 包。"""
    payload = _encode_string(config.client_id or "embeddebug")
    var_header = (
        _encode_string("MQTT")
        + bytes([0x04])  # Protocol Level 3.1.1
        + bytes([0x02])  # Clean Session flag
        + struct.pack(">H", config.keepalive)
    )
    body = var_header + payload
    return bytes([CONNECT << 4]) + encode_remaining_length(len(body)) + body


def encode_connack(session_present: bool = False, return_code: int = 0) -> bytes:
    """编码 CONNACK 包。"""
    body = bytes([1 if session_present else 0, return_code])
    return bytes([CONNACK << 4]) + encode_remaining_length(len(body)) + body


def encode_publish(message) -> bytes:
    """编码 PUBLISH 包。"""
    topic_bytes = _encode_string(message.topic)
    header_flags = (PUBLISH << 4) | (message.qos << 1) | (1 if message.retain else 0)
    body = topic_bytes + message.payload
    return bytes([header_flags]) + encode_remaining_length(len(body)) + body


def encode_subscribe(packet_id: int, topic: str, qos: int = 0) -> bytes:
    """编码 SUBSCRIBE 包。"""
    body = struct.pack(">H", packet_id) + _encode_string(topic) + bytes([qos])
    return bytes([SUBSCRIBE << 4 | 0x02]) + encode_remaining_length(len(body)) + body


def encode_pingreq() -> bytes:
    return bytes([PINGREQ << 4, 0])


def encode_disconnect() -> bytes:
    return bytes([DISCONNECT << 4, 0])


class MqttFrameCodec:
    """MQTT 帧编解码器（纯协议，无 socket）。"""

    def encode_connect(self, config) -> bytes:
        return encode_connect(config)

    def encode_connack(self, session_present: bool = False, return_code: int = 0) -> bytes:
        return encode_connack(session_present, return_code)

    def encode_publish(self, message) -> bytes:
        return encode_publish(message)

    def encode_subscribe(self, packet_id: int, topic: str, qos: int = 0) -> bytes:
        return encode_subscribe(packet_id, topic, qos)

    def decode_packet_type(self, data: bytes) -> int:
        """从字节流解码包类型（高 nibble）。"""
        if not data:
            raise ValueError("empty data")
        return data[0] >> 4
