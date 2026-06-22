"""MqttClientStub 单元测试 — pub/sub 通配符匹配 + 生命周期。

覆盖：open/close 生命周期、subscribe/publish 订阅分发、
通配符匹配（topic_matches）、on_bytes_received/on_error 回调。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.mqtt.client_stub import MqttClientStub
from embeddebug.serial_station.mqtt.message import MqttConfig


def _config() -> MqttConfig:
    return MqttConfig(host="localhost", port=1883, client_id="test")


def test_stub_initial_state_closed():
    """新建 stub 初始未打开。"""
    stub = MqttClientStub()
    assert stub.is_open is False
    assert stub.config is None
    assert stub.subscription_count == 0


def test_open_close_lifecycle():
    """open/close 切换 is_open + config。"""
    stub = MqttClientStub()
    cfg = _config()
    assert stub.open(cfg) is True
    assert stub.is_open is True
    assert stub.config is not None
    stub.close()
    assert stub.is_open is False


def test_publish_when_closed_returns_zero():
    """未 open 时 publish 返回 0。"""
    stub = MqttClientStub()
    assert stub.publish("test/topic", b"data") == 0


def test_subscribe_publish_exact_match():
    """精确主题匹配：publish 分发到订阅者。"""
    stub = MqttClientStub()
    stub.open(_config())
    stub.subscribe("sensor/temp", qos=0)
    assert stub.subscription_count == 1
    count = stub.publish("sensor/temp", b"23.5")
    assert count == 1


def test_subscribe_wildcard_single_level():
    """单层通配符 + 匹配一级。"""
    stub = MqttClientStub()
    stub.open(_config())
    stub.subscribe("sensor/+")
    assert stub.publish("sensor/temp", b"x") == 1
    assert stub.publish("sensor/humidity", b"x") == 1
    assert stub.publish("sensor/a/b", b"x") == 0  # 两级不匹配


def test_subscribe_wildcard_multi_level():
    """多层通配符 # 匹配任意层级。"""
    stub = MqttClientStub()
    stub.open(_config())
    stub.subscribe("data/#")
    assert stub.publish("data/a", b"x") == 1
    assert stub.publish("data/a/b/c", b"x") == 1
    assert stub.publish("other/a", b"x") == 0


def test_publish_string_payload():
    """字符串 payload 自动编码。"""
    stub = MqttClientStub()
    stub.open(_config())
    stub.subscribe("t")
    assert stub.publish("t", "hello") == 1


def test_multiple_subscribers_match():
    """多个订阅者匹配同一主题都收到。"""
    stub = MqttClientStub()
    stub.open(_config())
    stub.subscribe("a/b")
    stub.subscribe("a/+")
    stub.subscribe("#")
    assert stub.subscription_count == 3
    assert stub.publish("a/b", b"x") == 3


def test_write_appends_to_written():
    """write 追加到 written 列表（供回放）。"""
    stub = MqttClientStub()
    stub.open(_config())
    stub.write(b"hello")
    stub.write(b"world")
    assert stub.written == [b"hello", b"world"]


def test_write_when_closed_returns_zero():
    """未 open write 返回 0 + emit error。"""
    stub = MqttClientStub()
    errors = []
    stub.on_error(lambda msg: errors.append(msg))
    assert stub.write(b"x") == 0
    assert len(errors) >= 1


def test_on_error_when_closed_publish():
    """未 open publish 触发 on_error 回调。"""
    stub = MqttClientStub()
    errors = []
    stub.on_error(lambda msg: errors.append(msg))
    stub.publish("t", b"x")
    assert len(errors) >= 1
