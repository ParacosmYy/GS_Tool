"""MQTT 子模块单元测试 — 合并 message/codec/client_stub。

覆盖：Config 验证、消息模型、topic_matches 通配符、帧编解码、ClientStub 生命周期。
"""

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
from embeddebug.serial_station.mqtt.codec import (
    CONNECT,
    CONNACK,
    DISCONNECT,
    PINGREQ,
    PUBLISH,
    SUBSCRIBE,
    decode_remaining_length,
    encode_connect,
    encode_connack,
    encode_disconnect,
    encode_pingreq,
    encode_publish,
    encode_remaining_length,
)
from embeddebug.serial_station.mqtt.message import (
    MqttSubscription,
    MqttTopic,
    topic_matches,
)


# ---- Config + 消息模型 ----


def test_config_defaults_and_validation():
    cfg = MqttConfig()
    assert cfg.host == "localhost" and cfg.port == 1883 and cfg.qos == 0
    with pytest.raises(ValueError):
        MqttConfig(port=99999)
    with pytest.raises(ValueError):
        MqttConfig(port=-1)
    with pytest.raises(ValueError):
        MqttConfig(qos=5)
    with pytest.raises(ValueError):
        MqttConfig(keepalive=-1)
    for qos in (0, 1, 2):
        assert MqttConfig(qos=qos).qos == qos


def test_message_and_topic_models():
    msg = MqttMessage(topic="t/c", payload=b"d")
    assert msg.topic == "t/c" and msg.payload == b"d" and msg.qos == 0 and not msg.retain
    assert MqttTopic(name="s/#", qos=1).name == "s/#"
    assert MqttSubscription(topic_filter="cmd/+", qos=1).qos == 1


# ---- topic_matches ----


def test_topic_matches_wildcards():
    # 精确
    assert topic_matches("a/b/c", "a/b/c") is True
    assert topic_matches("a/b/c", "a/b/d") is False
    # 单层 +
    assert topic_matches("sport/+", "sport/tennis") is True
    assert topic_matches("sport/+", "sport") is False
    assert topic_matches("a/+", "a/b/c") is False
    # 多层 #
    assert topic_matches("sport/#", "sport/tennis/player1") is True
    assert topic_matches("sport/#", "sport") is True
    assert topic_matches("#", "anything/at/all") is True
    assert topic_matches("a/#", "b/c") is False
    # # 在中间
    assert topic_matches("a/#/c", "a/b/c/d") is True
    # 长度不匹配
    assert topic_matches("a/b", "a/b/c") is False
    assert topic_matches("a/b/c", "a/b") is False


# ---- 帧编解码 ----


def test_encode_packet_types():
    codec = MqttFrameCodec()
    config = MqttConfig(client_id="test")
    assert codec.encode_connect(config)[0] >> 4 == CONNECT
    assert codec.encode_connack(return_code=0)[0] >> 4 == CONNACK
    msg = MqttMessage(topic="sensor/temp", payload=b"23.5")
    assert codec.encode_publish(msg)[0] >> 4 == PUBLISH
    assert codec.encode_subscribe(1, "cmd/#", qos=1)[0] >> 4 == SUBSCRIBE


def test_remaining_length_round_trip():
    assert encode_remaining_length(64) == bytes([64])
    encoded = encode_remaining_length(321)
    val, _ = decode_remaining_length(encoded)
    assert val == 321
    # 大值
    big = encode_remaining_length(100000)
    assert decode_remaining_length(big)[0] == 100000
    # 16384 编码 3 字节
    assert encode_remaining_length(16384) == b"\x80\x80\x01"


def test_encode_publish_qos_and_retain_flags():
    for qos in (1, 2):
        msg = MqttMessage(topic="d/x", payload=b"hi", qos=qos)
        assert encode_publish(msg)[0] == (PUBLISH << 4) | (qos << 1)
    msg_retain = MqttMessage(topic="d/x", payload=b"hi", retain=True)
    assert encode_publish(msg_retain)[0] == (PUBLISH << 4) | 1


def test_encode_connack_session_and_return_codes():
    pkt = encode_connack(session_present=True, return_code=0)
    assert pkt[2] == 1 and pkt[3] == 0
    for code in range(6):
        assert encode_connack(return_code=code)[3] == code


def test_encode_connect_default_client_id():
    config = MqttConfig(host="localhost", port=1883, client_id="")
    assert b"embeddebug" in encode_connect(config)


def test_encode_disconnect_and_pingreq_fixed():
    assert encode_disconnect() == bytes([DISCONNECT << 4, 0])
    assert encode_pingreq() == bytes([PINGREQ << 4, 0])


def test_decode_packet_type():
    codec = MqttFrameCodec()
    assert codec.decode_packet_type(encode_disconnect()) == DISCONNECT
    assert codec.decode_packet_type(encode_pingreq()) == PINGREQ
    with pytest.raises(ValueError, match="empty"):
        codec.decode_packet_type(b"")


# ---- ClientStub ----


def test_stub_lifecycle_and_subscribe_publish():
    stub = MqttClientStub()
    assert not stub.is_open and stub.subscription_count == 0
    stub.connect(MqttConfig())
    assert stub.is_open
    # 精确匹配
    stub.subscribe("sensor/temp")
    assert stub.subscription_count == 1
    assert stub.publish("sensor/temp", b"23.5") == 1
    # 单层通配
    stub.subscribe("sensor/+")
    assert stub.publish("sensor/humidity", b"x") == 1
    assert stub.publish("sensor/a/b", b"x") == 0
    # 多层通配
    stub.subscribe("data/#")
    assert stub.publish("data/a/b/c", b"x") == 1
    assert stub.publish("other/a", b"x") == 0
    # 无匹配
    assert stub.publish("ghost", b"x") == 0


def test_stub_string_payload_and_multiple_subscribers():
    stub = MqttClientStub()
    stub.connect(MqttConfig())
    stub.subscribe("t")
    assert stub.publish("t", "hello") == 1  # str payload 自动编码
    # 多订阅者
    stub.subscribe("a/b")
    stub.subscribe("a/+")
    stub.subscribe("#")
    assert stub.publish("a/b", b"x") == 3


def test_stub_closed_operations_error():
    errors: list[str] = []
    stub = MqttClientStub()
    stub.on_error(errors.append)
    assert stub.subscribe("test") is False
    assert "transport_not_open" in errors
    assert stub.publish("t", b"x") == 0
    assert stub.write(b"x") == 0


def test_stub_write_appends():
    stub = MqttClientStub()
    stub.connect(MqttConfig())
    stub.write(b"hello")
    stub.write(b"world")
    assert stub.written == [b"hello", b"world"]
