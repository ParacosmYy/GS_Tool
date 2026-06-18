"""MQTT 子模块单元测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.mqtt import (
    MqttClientStub,
    MqttConfig,
    MqttFrameCodec,
    MqttMessage,
)
from embeddebug.serial_station.mqtt.message import topic_matches


def test_config_defaults():
    c = MqttConfig()
    assert c.host == "localhost" and c.port == 1883 and c.qos == 0


def test_config_rejects_bad_port():
    with pytest.raises(ValueError):
        MqttConfig(port=99999)


def test_config_rejects_bad_qos():
    with pytest.raises(ValueError):
        MqttConfig(qos=5)


def test_topic_exact_match():
    assert topic_matches("a/b/c", "a/b/c") is True
    assert topic_matches("a/b/c", "a/b/d") is False


def test_topic_plus_wildcard():
    assert topic_matches("sport/+", "sport/tennis") is True
    assert topic_matches("sport/+", "sport") is False
    assert topic_matches("sport/+", "sport/tennis/player1") is False


def test_topic_hash_wildcard():
    assert topic_matches("sport/#", "sport/tennis/player1") is True
    assert topic_matches("sport/#", "sport") is True
    assert topic_matches("#", "anything/at/all") is True


def test_connect_encode():
    codec = MqttFrameCodec()
    config = MqttConfig(client_id="test")
    data = codec.encode_connect(config)
    assert data[0] >> 4 == 1  # CONNECT


def test_connack_encode():
    codec = MqttFrameCodec()
    data = codec.encode_connack(return_code=0)
    assert data[0] >> 4 == 2  # CONNACK


def test_publish_encode():
    codec = MqttFrameCodec()
    msg = MqttMessage(topic="sensor/temp", payload=b"23.5")
    data = codec.encode_publish(msg)
    assert data[0] >> 4 == 3  # PUBLISH


def test_subscribe_encode():
    codec = MqttFrameCodec()
    data = codec.encode_subscribe(1, "cmd/#", qos=1)
    assert data[0] >> 4 == 8  # SUBSCRIBE


def test_remaining_length_small():
    from embeddebug.serial_station.mqtt.codec import encode_remaining_length
    assert encode_remaining_length(64) == bytes([64])


def test_remaining_length_large():
    from embeddebug.serial_station.mqtt.codec import encode_remaining_length, decode_remaining_length
    encoded = encode_remaining_length(321)
    val, consumed = decode_remaining_length(encoded)
    assert val == 321


def test_client_connect_subscribe_publish():
    stub = MqttClientStub()
    stub.connect(MqttConfig())
    assert stub.is_open
    stub.subscribe("sensor/#")
    assert stub.subscription_count == 1
    count = stub.publish("sensor/temp", b"42")
    assert count == 1


def test_client_publish_no_match():
    stub = MqttClientStub()
    stub.connect(MqttConfig())
    stub.subscribe("actuator/#")
    assert stub.publish("sensor/temp", b"42") == 0


def test_client_closed_subscribe_errors():
    errors: list[str] = []
    stub = MqttClientStub()
    stub.on_error(errors.append)
    assert stub.subscribe("test") is False
    assert "transport_not_open" in errors
