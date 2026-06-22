"""MQTT v3.1.1 编解码单元测试 — 变长长度 + 包结构。

覆盖：encode/decode_remaining_length round-trip、encode_connect 结构、
encode_connack/encode_subscribe/encode_publish/encode_disconnect 包类型。
"""

from __future__ import annotations


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


# ---- Batch 151: MQTT codec 边界扩展 ----


def test_encode_remaining_length_16384():
    """16384 是 3 字节编码的最小值。"""
    assert encode_remaining_length(16384) == b"\x80\x80\x01"


def test_encode_remaining_length_large_value():
    """大值（100000）正确编码为 3 字节。"""
    encoded = encode_remaining_length(100000)
    decoded, consumed = decode_remaining_length(encoded)
    assert decoded == 100000
    assert consumed == 3


def test_decode_remaining_length_with_offset():
    """decode 支持 offset 参数（从字节流中间解码）。"""
    encoded = b"\x00" + encode_remaining_length(300)  # 前导 1 字节
    value, consumed = decode_remaining_length(encoded, offset=1)
    assert value == 300
    assert consumed == 2


def test_encode_publish_qos1_flag():
    """QoS=1 时 PUBLISH 头 flags 含 qos<<1。"""
    msg = MqttMessage(topic="data/x", payload=b"hi", qos=1)
    packet = encode_publish(msg)
    # PUBLISH 高 nibble=3, 低 nibble: qos=1 → bit1=1, retain=0 → 0x02
    assert packet[0] == (PUBLISH << 4) | (1 << 1) | 0


def test_encode_publish_qos2_flag():
    """QoS=2 时 PUBLISH 头 flags 含 qos<<1=4。"""
    msg = MqttMessage(topic="data/x", payload=b"hi", qos=2)
    packet = encode_publish(msg)
    assert packet[0] == (PUBLISH << 4) | (2 << 1) | 0


def test_encode_publish_retain_flag():
    """retain=True 时 PUBLISH 头 bit0=1。"""
    msg = MqttMessage(topic="data/x", payload=b"hi", qos=0, retain=True)
    packet = encode_publish(msg)
    assert packet[0] == (PUBLISH << 4) | 1


def test_encode_connack_session_present_true():
    """session_present=True 时 CONNACK 第一字节=1。"""
    packet = encode_connack(session_present=True, return_code=0)
    # CONNACK body: [session_present, return_code]
    assert packet[2] == 1  # remaining_length=2，body 从 [3] 开始？ 不对
    # 实际：byte[0]=CONNACK<<4, byte[1]=remaining_length=2, byte[2]=session_present, byte[3]=return_code
    assert packet[2] == 1
    assert packet[3] == 0


def test_encode_connack_error_return_codes():
    """CONNACK 各种错误返回码（1-5）正确编码。"""
    for code in range(6):
        packet = encode_connack(return_code=code)
        assert packet[3] == code


def test_encode_connect_default_client_id():
    """client_id='' 时用默认 'embeddebug'。"""
    config = MqttConfig(host="localhost", port=1883, client_id="")
    packet = encode_connect(config)
    # 默认 client_id 应在 payload 中
    assert b"embeddebug" in packet


def test_encode_subscribe_qos_in_body():
    """SUBSCRIBE body 最后一字节是 QoS。"""
    packet = encode_subscribe(packet_id=42, topic="t", qos=2)
    # body: [packet_id 2B][topic_len 2B][topic][qos]
    # last byte = qos
    assert packet[-1] == 2


def test_encode_disconnect_is_two_bytes():
    """DISCONNECT 固定 2 字节（无 body）。"""
    assert encode_disconnect() == bytes([DISCONNECT << 4, 0])


def test_encode_pingreq_is_two_bytes():
    """PINGREQ 固定 2 字节（无 body）。"""
    assert encode_pingreq() == bytes([PINGREQ << 4, 0])


def test_decode_packet_type_empty_raises():
    """decode_packet_type(b'') 抛 ValueError。"""
    import pytest
    from embeddebug.serial_station.mqtt.codec import MqttFrameCodec
    with pytest.raises(ValueError, match="empty"):
        MqttFrameCodec().decode_packet_type(b"")


def test_decode_packet_type_extracts_high_nibble():
    """decode_packet_type 正确提取高 nibble。"""
    from embeddebug.serial_station.mqtt.codec import MqttFrameCodec
    codec = MqttFrameCodec()
    assert codec.decode_packet_type(encode_disconnect()) == DISCONNECT
    assert codec.decode_packet_type(encode_pingreq()) == PINGREQ
    assert codec.decode_packet_type(encode_connack()) == CONNACK


def test_mqtt_frame_codec_wraps_module_functions():
    """MqttFrameCodec 方法委托到模块级函数。"""
    from embeddebug.serial_station.mqtt.codec import MqttFrameCodec
    codec = MqttFrameCodec()
    config = MqttConfig(host="h", port=1, client_id="x")
    assert codec.encode_connect(config) == encode_connect(config)
    assert codec.encode_connack() == encode_connack()
    msg = MqttMessage(topic="t", payload=b"d")
    assert codec.encode_publish(msg) == encode_publish(msg)
    assert codec.encode_subscribe(1, "t") == encode_subscribe(1, "t")

