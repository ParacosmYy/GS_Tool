"""mqtt/client_stub + spi_i2c/bridge_stub 边界单元测试。

补强：MqttClientStub（connect/open(None)/close 清空/publish 返回计数/subscribe 未 open）
+ SpiI2cBridgeStub（xfer_spi 空字节/read 不存在=0x00/write 覆盖/10-bit 高地址/常量）。
"""

from __future__ import annotations

from embeddebug.serial_station.mqtt.client_stub import MqttClientStub
from embeddebug.serial_station.mqtt.message import MqttConfig
from embeddebug.serial_station.spi_i2c.bridge_stub import (
    REG_WHO_AM_I,
    WHO_AM_I_VALUE,
    SpiI2cBridgeStub,
)
from embeddebug.serial_station.spi_i2c.config import I2cConfig, SpiConfig


# ── MqttClientStub.connect / open(None) / close ─────────────────────────


def test_connect_delegates_to_open():
    """connect() 委托 open()，设置 config + is_open。"""

    stub = MqttClientStub()
    cfg = MqttConfig(host="broker.local", port=8883)
    assert stub.connect(cfg) is True
    assert stub.is_open is True
    assert stub.config is cfg


def test_open_none_creates_default_config():
    """open(None) → 创建默认 MqttConfig。"""

    stub = MqttClientStub()
    stub.open(None)
    assert stub.is_open is True
    assert isinstance(stub.config, MqttConfig)
    assert stub.config.host == "localhost"


def test_open_non_mqttconfig_creates_default():
    """open(非 MqttConfig) → 创建默认 MqttConfig。"""

    stub = MqttClientStub()
    stub.open("not a config")  # type: ignore[arg-type]
    assert isinstance(stub.config, MqttConfig)


def test_close_clears_subscriptions():
    """close() 清空订阅列表。"""

    stub = MqttClientStub()
    stub.open(MqttConfig())
    stub.subscribe("topic/a")
    stub.subscribe("topic/b")
    assert stub.subscription_count == 2
    stub.close()
    assert stub.subscription_count == 0


def test_close_resets_is_open():
    """close() 后 is_open=False。"""

    stub = MqttClientStub()
    stub.open(MqttConfig())
    stub.close()
    assert stub.is_open is False


# ── MqttClientStub.subscribe / publish 边界 ─────────────────────────────


def test_subscribe_when_closed_returns_false():
    """未 open 时 subscribe 返回 False + 报错。"""

    stub = MqttClientStub()
    errors: list[str] = []
    stub.on_error(errors.append)
    assert stub.subscribe("topic") is False
    assert errors == ["transport_not_open"]


def test_subscribe_increments_count():
    """每次 subscribe 计数+1。"""

    stub = MqttClientStub()
    stub.open(MqttConfig())
    stub.subscribe("a")
    stub.subscribe("b")
    stub.subscribe("c")
    assert stub.subscription_count == 3


def test_publish_returns_matching_count():
    """publish 返回匹配的订阅数量。"""

    stub = MqttClientStub()
    stub.open(MqttConfig())
    stub.subscribe("sensors/#")
    stub.subscribe("sensors/temp")
    stub.subscribe("status/#")
    # "sensors/temp" 匹配 2 个（sensors/# + sensors/temp）
    assert stub.publish("sensors/temp", b"data") == 2


def test_publish_no_match_returns_zero():
    """无匹配订阅 → 0。"""

    stub = MqttClientStub()
    stub.open(MqttConfig())
    stub.subscribe("sensors/#")
    assert stub.publish("status/online", b"data") == 0


def test_publish_str_payload_does_not_crash():
    """publish 字符串 payload 不崩溃（count 不变）。"""

    stub = MqttClientStub()
    stub.open(MqttConfig())
    stub.subscribe("test")
    assert stub.publish("test", "hello") == 1


# ── MqttClientStub.write / on_bytes_received ────────────────────────────


def test_write_returns_byte_count():
    """write 返回写入字节数。"""

    stub = MqttClientStub()
    stub.open(MqttConfig())
    assert stub.write(b"\x01\x02\x03") == 3


def test_on_bytes_received_callback_registered():
    """on_bytes_received 注册回调（不立即调用）。"""

    stub = MqttClientStub()
    received: list[bytes] = []
    stub.on_bytes_received(received.append)
    # stub 不主动推送字节，仅注册
    assert len(received) == 0


def test_config_property_none_before_open():
    """open 前 config=None。"""

    stub = MqttClientStub()
    assert stub.config is None


# ── SpiI2cBridgeStub 常量 ───────────────────────────────────────────────


def test_reg_who_am_i_constant():
    """REG_WHO_AM_I = 0x75。"""

    assert REG_WHO_AM_I == 0x75


def test_who_am_i_value_constant():
    """WHO_AM_I_VALUE = 0x68（MPU-6050 标识）。"""

    assert WHO_AM_I_VALUE == 0x68


# ── SpiI2cBridgeStub.xfer_spi 边界 ──────────────────────────────────────


def test_xfer_spi_empty_returns_empty():
    """SPI 空字节回环 → 空。"""

    stub = SpiI2cBridgeStub()
    config = SpiConfig(mode=0, cs_active_low=False, word_size=8, max_speed_hz=1000000)
    assert stub.xfer_spi(config, b"") == b""


def test_xfer_spi_returns_input_unchanged():
    """SPI 全双工回环 → 返回输入原样。"""

    stub = SpiI2cBridgeStub()
    config = SpiConfig(mode=0, cs_active_low=False, word_size=8, max_speed_hz=1000000)
    data = b"\xDE\xAD\xBE\xEF"
    assert stub.xfer_spi(config, data) == data


# ── SpiI2cBridgeStub.read_i2c 寄存器不存在 ──────────────────────────────


def test_read_i2c_nonexistent_register_returns_zero():
    """读取不存在的寄存器 → 返回 0x00。"""

    stub = SpiI2cBridgeStub()
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    result = stub.read_i2c(config, 0xFF, length=1)
    assert result.success is True
    assert result.data == b"\x00"


def test_read_i2c_custom_registers():
    """自定义寄存器初始值。"""

    stub = SpiI2cBridgeStub(registers={0x10: 0xAA, 0x11: 0xBB})
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    result = stub.read_i2c(config, 0x10, length=2)
    assert result.data == b"\xAA\xBB"


# ── SpiI2cBridgeStub.write_i2c 覆盖 ─────────────────────────────────────


def test_write_i2c_overwrites_existing_register():
    """write 覆盖已有寄存器。"""

    stub = SpiI2cBridgeStub(registers={0x20: 0x00})
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    stub.write_i2c(config, 0x20, b"\xFF")
    result = stub.read_i2c(config, 0x20, length=1)
    assert result.data == b"\xFF"


def test_write_i2c_invalid_address_raises_on_config():
    """无效地址（>127 非 10-bit）→ I2cConfig 构造时抛 ValueError。"""

    import pytest

    with pytest.raises(ValueError):
        I2cConfig(address=200, is_ten_bit=False, speed_khz=400)


def test_read_i2c_ten_bit_address_allows_high():
    """10-bit 模式允许高地址（≤1023）。"""

    stub = SpiI2cBridgeStub()
    config = I2cConfig(address=200, is_ten_bit=True, speed_khz=400)
    result = stub.read_i2c(config, 0x20, length=1)
    assert result.success is True


def test_write_i2c_multi_byte_sequential():
    """多字节顺序写入连续寄存器。"""

    stub = SpiI2cBridgeStub()
    config = I2cConfig(address=0x68, is_ten_bit=False, speed_khz=400)
    stub.write_i2c(config, 0x30, b"\x01\x02\x03")
    result = stub.read_i2c(config, 0x30, length=3)
    assert result.data == b"\x01\x02\x03"
