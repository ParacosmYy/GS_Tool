"""MQTT 消息模型 + topic_matches 通配符匹配单元测试。

覆盖：MqttConfig 验证（port/qos/keepalive 边界）、MqttMessage/MqttTopic、
topic_matches 精确/+/ /# 匹配 + 不匹配 + 边界。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.mqtt.message import (
    MqttConfig,
    MqttMessage,
    MqttSubscription,
    MqttTopic,
    topic_matches,
)


def test_mqtt_config_defaults():
    cfg = MqttConfig()
    assert cfg.host == "localhost"
    assert cfg.port == 1883
    assert cfg.qos == 0


def test_mqtt_config_port_out_of_range():
    with pytest.raises(ValueError):
        MqttConfig(port=70000)


def test_mqtt_config_port_negative():
    with pytest.raises(ValueError):
        MqttConfig(port=-1)


def test_mqtt_config_invalid_qos():
    with pytest.raises(ValueError):
        MqttConfig(qos=3)


def test_mqtt_config_negative_keepalive():
    with pytest.raises(ValueError):
        MqttConfig(keepalive=-1)


def test_mqtt_config_valid_qos_values():
    for qos in (0, 1, 2):
        cfg = MqttConfig(qos=qos)
        assert cfg.qos == qos


def test_mqtt_message_basic():
    msg = MqttMessage(topic="test/topic", payload=b"data")
    assert msg.topic == "test/topic"
    assert msg.payload == b"data"
    assert msg.qos == 0
    assert msg.retain is False


def test_mqtt_topic():
    t = MqttTopic(name="sensor/#", qos=1)
    assert t.name == "sensor/#"
    assert t.qos == 1


def test_mqtt_subscription():
    sub = MqttSubscription(topic_filter="cmd/+", qos=1)
    assert sub.topic_filter == "cmd/+"
    assert sub.qos == 1


# ── topic_matches ──────────────────────────────────────────────────

def test_topic_matches_exact():
    assert topic_matches("a/b/c", "a/b/c") is True
    assert topic_matches("a/b/c", "a/b/d") is False


def test_topic_matches_single_level_wildcard():
    assert topic_matches("a/+/c", "a/b/c") is True
    assert topic_matches("a/+/c", "a/x/c") is True
    assert topic_matches("a/+/c", "a/b/d") is False  # c ≠ d
    assert topic_matches("a/+", "a/b") is True
    assert topic_matches("a/+", "a/b/c") is False  # 两层不匹配


def test_topic_matches_multi_level_wildcard():
    assert topic_matches("a/#", "a/b/c/d") is True
    assert topic_matches("a/#", "a") is True
    assert topic_matches("#", "anything/here") is True
    assert topic_matches("a/#", "b/c") is False  # a ≠ b


def test_topic_matches_hash_in_middle():
    """# 在中间位置匹配剩余全部。"""
    assert topic_matches("a/#/c", "a/b/c/d") is True  # # 在 i=1 匹配到末尾


def test_topic_matches_filter_shorter():
    """filter 比 topic 短且无 # → 不匹配。"""
    assert topic_matches("a/b", "a/b/c") is False


def test_topic_matches_filter_longer():
    """filter 比 topic 长 → 不匹配（除非 # 在 filter 末尾前已 return True）。"""
    assert topic_matches("a/b/c", "a/b") is False
