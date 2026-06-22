"""NotificationData + NotificationLevel + profile_snapshot 边界测试。

补强 test_notifications / test_profile_snapshot 未直接断言的边角：
- NotificationLevel：4 成员 + value 小写字符串。
- NotificationData.create：factory 设置 timestamp_ns > 0 + uid > 0。
- NotificationData.is_expired：timeout_ms=0 永不过期 + 精确边界。
- NotificationData：非 frozen（可变 dataclass）+ action_label/action_callback 默认。
- build_profile_snapshot：commandHistory 非空 + protocol 字段。
- _transport_snapshot：config 全字段精确值。
"""

from __future__ import annotations


from embeddebug.serial_station.controllers.profile_snapshot import (
    _transport_snapshot,
    build_profile_snapshot,
)
from embeddebug.serial_station.drivers import SerialPortConfig
from embeddebug.serial_station.notifications import NotificationData, NotificationLevel


# ── NotificationLevel 枚举 ─────────────────────────────────────────────


def test_notification_level_has_four_members():
    """NotificationLevel 含 4 个级别。"""

    assert len(NotificationLevel) == 4


def test_notification_level_values_lowercase():
    """枚举 value 是小写字符串。"""

    assert NotificationLevel.INFO.value == "info"
    assert NotificationLevel.SUCCESS.value == "success"
    assert NotificationLevel.WARNING.value == "warning"
    assert NotificationLevel.ERROR.value == "error"


def test_notification_level_values_all_distinct():
    """4 个 value 互不相同。"""

    values = {level.value for level in NotificationLevel}
    assert len(values) == 4


# ── NotificationData.create factory ────────────────────────────────────


def test_create_sets_timestamp_positive():
    """create() 设置 timestamp_ns > 0。"""

    data = NotificationData.create(NotificationLevel.INFO, "title", "msg")
    assert data.timestamp_ns > 0


def test_create_default_timeout_3000():
    """create() 默认 timeout_ms=3000。"""

    data = NotificationData.create(NotificationLevel.INFO, "t", "m")
    assert data.timeout_ms == 3000


def test_create_custom_timeout():
    """create() 自定义 timeout_ms。"""

    data = NotificationData.create(NotificationLevel.INFO, "t", "m", timeout_ms=5000)
    assert data.timeout_ms == 5000


def test_create_default_action_label_empty():
    """create() 默认 action_label=""。"""

    data = NotificationData.create(NotificationLevel.INFO, "t", "m")
    assert data.action_label == ""


def test_create_default_action_callback_none():
    """create() 默认 action_callback=None。"""

    data = NotificationData.create(NotificationLevel.INFO, "t", "m")
    assert data.action_callback is None


def test_create_custom_action_label():
    """create() 自定义 action_label。"""

    data = NotificationData.create(
        NotificationLevel.WARNING, "t", "m", action_label="Retry",
    )
    assert data.action_label == "Retry"


# ── NotificationData.is_expired 边界 ──────────────────────────────────


def test_is_expired_exact_boundary():
    """timeout_ms=1000 → 在 1000ms 时恰好过期。"""

    data = NotificationData(NotificationLevel.INFO, "t", "m", 0, timeout_ms=1000)
    assert data.is_expired(1_000_000_000) is True  # 1000ms 后


def test_is_expired_just_before_boundary():
    """timeout_ms=1000 → 999ms 时未过期。"""

    data = NotificationData(NotificationLevel.INFO, "t", "m", 0, timeout_ms=1000)
    assert data.is_expired(999_000_000) is False


def test_is_expired_zero_timeout_never():
    """timeout_ms=0 → 永不过期。"""

    data = NotificationData(NotificationLevel.INFO, "t", "m", 0, timeout_ms=0)
    assert data.is_expired(10_000_000_000_000) is False


def test_is_expired_negative_timeout_never():
    """timeout_ms<0 → 永不过期（<=0 分支）。"""

    data = NotificationData(NotificationLevel.INFO, "t", "m", 0, timeout_ms=-1)
    assert data.is_expired(10_000_000_000_000) is False


# ── NotificationData 可变性 ───────────────────────────────────────────


def test_notification_data_is_mutable():
    """NotificationData 非 frozen（uid 可改）。"""

    data = NotificationData(NotificationLevel.INFO, "t", "m", 0)
    data.uid = 42  # 可变（不抛）
    assert data.uid == 42


# ── build_profile_snapshot 边界 ───────────────────────────────────────


def _config() -> SerialPortConfig:
    return SerialPortConfig(
        port_name="COM3", baud_rate=115200, data_bits=8,
        parity="none", stop_bits="1", flow_control="none",
    )


def test_build_snapshot_protocol_field():
    """snapshot 含 protocol 字段。"""

    snap = build_profile_snapshot(
        "test", "serial", _config(), True, "raw_data", ("AT",),
    )
    assert snap["protocol"] == "raw_data"


def test_build_snapshot_command_history_non_empty():
    """snapshot commandHistory 非空时保留全部。"""

    snap = build_profile_snapshot(
        "test", "serial", _config(), True, "raw_data",
        ("AT", "AT+RESET", "AT+VERSION"),
    )
    assert snap["commandHistory"] == ["AT", "AT+RESET", "AT+VERSION"]


def test_build_snapshot_name_field():
    """snapshot name 字段。"""

    snap = build_profile_snapshot("my-profile", "serial", None, False, "raw", ())
    assert snap["name"] == "my-profile"


# ── _transport_snapshot 全字段 ────────────────────────────────────────


def test_transport_snapshot_all_fields_with_config():
    """_transport_snapshot config 全字段精确值。"""

    ts = _transport_snapshot("serial", _config(), True)
    assert ts["mode"] == "serial"
    assert ts["portName"] == "COM3"
    assert ts["baudRate"] == 115200
    assert ts["dataBits"] == 8
    assert ts["parity"] == "none"
    assert ts["stopBits"] == "1"
    assert ts["flowControl"] == "none"
    assert ts["connected"] is True


def test_transport_snapshot_none_config_defaults():
    """config=None → 全字段默认值。"""

    ts = _transport_snapshot("fake", None, False)
    assert ts["portName"] == ""
    assert ts["baudRate"] == 0
    assert ts["dataBits"] == 8
    assert ts["parity"] == "none"
    assert ts["stopBits"] == "1"
    assert ts["flowControl"] == "none"
    assert ts["connected"] is False


def test_transport_snapshot_connected_false():
    """is_connected=False → connected=False。"""

    ts = _transport_snapshot("serial", _config(), False)
    assert ts["connected"] is False
