"""MQTT v3.1.1 编解码单元测试 — 变长长度 + 包结构。

覆盖：encode/decode_remaining_length round-trip、encode_connect 结构、
encode_connack/encode_subscribe/encode_publish/encode_disconnect 包类型。
"""

from __future__ import annotations


from embeddebug.serial_station.mqtt.codec import (
    CONNECT,
    CONNACK,
    DISCONNECT,
    PUBLISH,
    SUBSCRIBE,
    decode_remaining_length,
    encode_connect,
    encode_connack,
    encode_disconnect,
    encode_pingreq,
    encode_publish,
    encode_remaining_length,
    encode_subscribe,
)
from embeddebug.serial_station.mqtt.message import MqttConfig, MqttMessage


def test_encode_remaining_length_zero():
    assert encode_remaining_length(0) == b"\x00"


def test_encode_remaining_length_below_128():
    assert encode_remaining_length(64) == b"\x40"


def test_encode_remaining_length_128():
    assert encode_remaining_length(128) == b"\x80\x01"


def test_encode_remaining_length_16383():
    assert encode_remaining_length(16383) == b"\xFF\x7F"


def test_decode_remaining_length_round_trip():
    for value in (0, 1, 127, 128, 16383, 100000):
        encoded = encode_remaining_length(value)
        decoded, consumed = decode_remaining_length(encoded)
        assert decoded == value
        assert consumed == len(encoded)


def test_decode_remaining_length_incomplete_raises():
    import pytest
    with pytest.raises(ValueError):
        decode_remaining_length(b"\x80")


def test_decode_remaining_length_too_large_raises():
    import pytest
    with pytest.raises(ValueError):
        decode_remaining_length(b"\xFF\xFF\xFF\x80")


def test_encode_connect_structure():
    config = MqttConfig(host="localhost", port=1883, client_id="test_client")
    packet = encode_connect(config)
    assert packet[0] >> 4 == CONNECT
    assert len(packet) > 10


def test_encode_connack_structure():
    packet = encode_connack(session_present=False, return_code=0)
    assert packet[0] >> 4 == CONNACK


def test_encode_subscribe_structure():
    packet = encode_subscribe(packet_id=1, topic="sensor/temp", qos=0)
    assert packet[0] >> 4 == SUBSCRIBE


def test_encode_publish_structure():
    msg = MqttMessage(topic="data/x", payload=b"hello", qos=0)
    packet = encode_publish(msg)
    assert packet[0] >> 4 == PUBLISH


def test_encode_disconnect_structure():
    packet = encode_disconnect()
    assert packet[0] >> 4 == DISCONNECT


def test_encode_pingreq_structure():
    packet = encode_pingreq()
    assert len(packet) == 2  # PINGREQ 固定 2 字节

