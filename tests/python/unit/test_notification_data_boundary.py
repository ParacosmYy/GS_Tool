"""NotificationLevel 枚举 + NotificationData 边界测试。

NotificationLevel 枚举值 + NotificationData 字段 + is_expired/is_persistent。

覆盖：
1. NotificationLevel 4 成员。
2. NotificationLevel 值小写。
3. NotificationData level/title/message/timestamp_ns。
4. NotificationData 默认 timeout_ms=3000。
5. NotificationData is_expired 未超时 False。
6. NotificationData is_expired 超时 True。
7. NotificationData is_persistent timeout_ms=0。
8. NotificationData uid 默认 0。
"""

from __future__ import annotations

from embeddebug.serial_station.notifications.data import (
    NotificationData,
    NotificationLevel,
)


def test_notification_level_has_four_members():
    assert len(NotificationLevel) == 4


def test_notification_level_values_lowercase():
    for level in NotificationLevel:
        assert level.value == level.value.lower()


def test_notification_data_fields():
    d = NotificationData(
        level=NotificationLevel.INFO,
        title="Test",
        message="Body",
        timestamp_ns=1000,
    )
    assert d.level == NotificationLevel.INFO
    assert d.title == "Test"
    assert d.message == "Body"
    assert d.timestamp_ns == 1000


def test_default_timeout_ms_3000():
    d = NotificationData(NotificationLevel.INFO, "t", "m", 0)
    assert d.timeout_ms == 3000


def test_is_expired_not_yet():
    d = NotificationData(NotificationLevel.INFO, "t", "m", 1_000_000_000, timeout_ms=3000)
    assert d.is_expired(1_000_000_000 + 2_000_000_000) is False


def test_is_expired_past():
    d = NotificationData(NotificationLevel.WARNING, "t", "m", 0, timeout_ms=1000)
    assert d.is_expired(2_000_000_000) is True


def test_is_persistent_timeout_zero():
    d = NotificationData(NotificationLevel.ERROR, "t", "m", 0, timeout_ms=0)
    assert d.is_expired(10_000_000_000_000) is False


def test_default_uid_zero():
    d = NotificationData(NotificationLevel.INFO, "t", "m", 0)
    assert d.uid == 0
