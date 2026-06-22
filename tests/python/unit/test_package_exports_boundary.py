"""serial_station 包 __init__ 导出契约测试。

各子包的 __init__.py __all__ 与实际可导入符号一致性 + 关键公共 API 可达性。
此前无直接导出契约测试（各模块单独测试但包级 __init__ 未验证）。

覆盖：
1. core 包导出 ChannelBatch + ChannelRingBuffer。
2. can 包导出 CanFrame + CanFrameCodec + CanId + CanFilter。
3. ble 包导出 BleFrameCodec + BleTransportStub。
4. mqtt 包导出 MqttConfig + MqttClientStub。
5. spi_i2c 包导出 SpiConfig + I2cConfig + SpiI2cFrameCodec + SpiI2cBridgeStub。
6. ota 包导出 TransferResult + make_protocol + OtaProtocolKind。
7. protocols 包导出 RawDataProtocol + FireWaterProtocol + JustFloatProtocol。
8. rtt 包导出 RttConfig + RttChannel + RttSession + RttTransportStub。
9. automation 包导出 AutomationEngine + AutomationRule + TriggerCondition。
10. 各包 __all__ 与实际属性一致（无幽灵导出）。
"""

from __future__ import annotations


def _verify_all(pkg, expected_extra: list[str] | None = None):
    """验证 pkg.__all__ 中每个名称在 pkg 上可访问。"""

    names = list(pkg.__all__)
    if expected_extra:
        names = expected_extra
    missing = [n for n in names if not hasattr(pkg, n)]
    assert not missing, f"{pkg.__name__}.__all__ 含不可访问符号: {missing}"


# ── core ──────────────────────────────────────────────────────────
def test_core_exports():
    from embeddebug.serial_station import core

    assert hasattr(core, "ChannelBatch")
    assert hasattr(core, "ChannelRingBuffer")


# ── can ───────────────────────────────────────────────────────────
def test_can_exports():
    from embeddebug.serial_station import can as can_pkg

    for name in ("CanFrame", "CanId", "CanFilter", "CanFrameCodec", "PROTOCOL_NAME"):
        assert hasattr(can_pkg, name), f"can 缺 {name}"
    _verify_all(can_pkg)


def test_can_all_accessible():
    from embeddebug.serial_station import can as can_pkg

    _verify_all(can_pkg)


# ── ble ───────────────────────────────────────────────────────────
def test_ble_exports():
    from embeddebug.serial_station import ble

    for name in ("BleFrameCodec", "BleFrameEvent", "BleTransportStub"):
        assert hasattr(ble, name), f"ble 缺 {name}"
    _verify_all(ble)


# ── mqtt ──────────────────────────────────────────────────────────
def test_mqtt_exports():
    from embeddebug.serial_station import mqtt

    for name in ("MqttConfig", "MqttMessage", "MqttClientStub"):
        assert hasattr(mqtt, name), f"mqtt 缺 {name}"
    _verify_all(mqtt)


# ── spi_i2c ───────────────────────────────────────────────────────
def test_spi_i2c_exports():
    from embeddebug.serial_station import spi_i2c

    for name in ("SpiConfig", "I2cConfig", "SpiI2cFrameCodec", "SpiI2cBridgeStub"):
        assert hasattr(spi_i2c, name), f"spi_i2c 缺 {name}"
    _verify_all(spi_i2c)


# ── ota ───────────────────────────────────────────────────────────
def test_ota_exports():
    from embeddebug import ota

    for name in ("TransferResult", "make_protocol", "OtaProtocolKind", "TransferEngine"):
        assert hasattr(ota, name), f"ota 缺 {name}"
    _verify_all(ota)


# ── protocols ─────────────────────────────────────────────────────
def test_protocols_exports():
    from embeddebug.serial_station import protocols

    for name in ("RawDataProtocol", "FireWaterProtocol", "JustFloatProtocol"):
        assert hasattr(protocols, name), f"protocols 缺 {name}"


# ── rtt ───────────────────────────────────────────────────────────
def test_rtt_exports():
    from embeddebug.serial_station import rtt

    for name in ("RttConfig", "RttChannel", "RttSession", "RttTransportStub"):
        assert hasattr(rtt, name), f"rtt 缺 {name}"
    _verify_all(rtt)


# ── automation ────────────────────────────────────────────────────
def test_automation_exports():
    from embeddebug.serial_station import automation

    for name in ("AutomationEngine", "AutomationRule", "TriggerCondition", "AutomationAction"):
        assert hasattr(automation, name), f"automation 缺 {name}"
    _verify_all(automation)


# ── drivers ───────────────────────────────────────────────────────
def test_drivers_exports():
    from embeddebug.serial_station import drivers

    for name in ("SerialTransport", "SerialPortConfig"):
        assert hasattr(drivers, name), f"drivers 缺 {name}"


# ── session ───────────────────────────────────────────────────────
def test_session_exports():
    from embeddebug.serial_station import session

    for name in ("SessionState", "SessionSerializer", "SessionManager"):
        assert hasattr(session, name), f"session 缺 {name}"


# ── notifications ─────────────────────────────────────────────────
def test_notifications_exports():
    from embeddebug.serial_station import notifications

    for name in ("NotificationLevel", "NotificationManager"):
        assert hasattr(notifications, name), f"notifications 缺 {name}"


# ── shared ────────────────────────────────────────────────────────
def test_shared_exports():
    from embeddebug import shared

    for name in ("OperationResult", "OperationError"):
        assert hasattr(shared, name), f"shared 缺 {name}"
