"""通知子系统测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.notifications import NotificationData, NotificationLevel, NotificationManager


def test_not_expired_within_timeout():
    data = NotificationData(NotificationLevel.INFO, "t", "m", 1_000_000_000, timeout_ms=3000)
    assert not data.is_expired(1_000_000_000 + 2_999_000_000)


def test_expired_after_timeout():
    data = NotificationData(NotificationLevel.WARNING, "t", "m", 0, timeout_ms=1000)
    assert data.is_expired(1_000_000_000)


def test_persistent_never_expires():
    data = NotificationData(NotificationLevel.ERROR, "t", "m", 0, timeout_ms=0)
    assert not data.is_expired(10_000_000_000_000)


def test_show_adds_to_queue(qtbot):
    mgr = NotificationManager()
    added: list = []
    mgr.notification_added.connect(added.append)
    data = mgr.show(NotificationLevel.SUCCESS, "ok", "done", timeout_ms=0)
    assert data in mgr.queue()
    assert len(added) == 1


def test_archives_to_history(qtbot):
    mgr = NotificationManager()
    mgr.show(NotificationLevel.INFO, "a", "m", timeout_ms=0)
    mgr.show(NotificationLevel.INFO, "b", "m", timeout_ms=0)
    assert len(mgr.history()) == 2


def test_max_visible_evicts_oldest(qtbot):
    mgr = NotificationManager(max_visible=3)
    for i in range(5):
        mgr.show(NotificationLevel.INFO, f"n{i}", "m", timeout_ms=0)
    assert len(mgr.queue()) == 3
    assert [d.title for d in mgr.queue()] == ["n2", "n3", "n4"]


def test_auto_dismiss_after_timeout(qtbot):
    mgr = NotificationManager()
    removed: list = []
    mgr.notification_removed.connect(removed.append)
    mgr.show(NotificationLevel.WARNING, "warn", "soon", timeout_ms=50)
    qtbot.waitUntil(lambda: not mgr.has_pending(), timeout=2000)
    assert len(removed) == 1


def test_dismiss_manual(qtbot):
    mgr = NotificationManager()
    data = mgr.show(NotificationLevel.INFO, "x", "m", timeout_ms=0)
    assert mgr.dismiss(data.uid)
    assert not mgr.has_pending()
    assert not mgr.dismiss(data.uid)


def test_clear_removes_all(qtbot):
    mgr = NotificationManager()
    for i in range(3):
        mgr.show(NotificationLevel.INFO, f"n{i}", "m", timeout_ms=0)
    mgr.clear()
    assert not mgr.has_pending()


def test_level_enum_values():
    assert len(NotificationLevel) == 4
    assert NotificationLevel.INFO.value == "info"
    assert NotificationLevel.ERROR.value == "error"
    for level in NotificationLevel:
        assert level.value == level.value.lower()


def test_notification_data_fields_and_defaults():
    data = NotificationData(
        level=NotificationLevel.INFO,
        title="Test",
        message="Body",
        timestamp_ns=1000,
    )

    assert data.level == NotificationLevel.INFO
    assert data.title == "Test"
    assert data.message == "Body"
    assert data.timestamp_ns == 1000
    assert data.timeout_ms == 3000
    assert data.uid == 0


def test_history_capped(qtbot):
    mgr = NotificationManager(max_visible=10, max_history=5)
    for i in range(8):
        mgr.show(NotificationLevel.INFO, f"n{i}", "m", timeout_ms=0)
    assert len(mgr.history()) == 5


# ---- Batch 139: NotificationManager 边界扩展 ----


def test_clear_history_empties_history_but_keeps_queue(qtbot):
    """clear_history 只清历史，不清当前队列。"""
    mgr = NotificationManager()
    mgr.show(NotificationLevel.INFO, "a", "m", timeout_ms=0)
    mgr.show(NotificationLevel.INFO, "b", "m", timeout_ms=0)
    assert len(mgr.history()) == 2
    mgr.clear_history()
    assert mgr.history() == []
    assert mgr.has_pending()  # 队列仍有 2 条


def test_dismiss_unknown_uid_returns_false(qtbot):
    """dismiss 一个不存在的 uid 返回 False（不抛异常）。"""
    mgr = NotificationManager()
    mgr.show(NotificationLevel.INFO, "x", "m", timeout_ms=0)
    assert mgr.dismiss(99999) is False
    assert mgr.has_pending()  # 原通知仍在


def test_max_visible_property_reflects_constructor(qtbot):
    """max_visible 属性暴露构造参数（clamp 到 ≥1）。"""
    assert NotificationManager(max_visible=5).max_visible == 5
    assert NotificationManager(max_visible=0).max_visible == 1  # clamp
    assert NotificationManager(max_visible=-3).max_visible == 1  # clamp


def test_queue_returns_defensive_copy(qtbot):
    """queue() 返回 list 副本，外部修改不影响内部。"""
    mgr = NotificationManager()
    mgr.show(NotificationLevel.INFO, "x", "m", timeout_ms=0)
    snapshot = mgr.queue()
    snapshot.clear()
    assert mgr.has_pending()  # 内部队列不受影响
    assert len(mgr.queue()) == 1


def test_history_returns_defensive_copy(qtbot):
    """history() 返回 list 副本，外部修改不影响内部。"""
    mgr = NotificationManager()
    mgr.show(NotificationLevel.INFO, "x", "m", timeout_ms=0)
    snapshot = mgr.history()
    snapshot.clear()
    assert len(mgr.history()) == 1  # 内部历史不受影响


def test_evict_prefers_timeout_enabled_over_persistent(qtbot):
    """超容量时优先驱逐有 timeout 的通知，持久通知（timeout=0）留到最后。"""
    mgr = NotificationManager(max_visible=2)
    # 1 持久 + 2 有 timeout → 持久应留到队列末尾
    mgr.show(NotificationLevel.ERROR, "persistent", "m", timeout_ms=0)
    mgr.show(NotificationLevel.INFO, "auto1", "m", timeout_ms=100)
    mgr.show(NotificationLevel.INFO, "auto2", "m", timeout_ms=100)
    titles = [d.title for d in mgr.queue()]
    # persistent 应保留（auto1 被驱逐）
    assert "persistent" in titles
    assert "auto1" not in titles


def test_notification_removed_signal_on_dismiss(qtbot):
    """dismiss 触发 notification_removed 信号。"""
    mgr = NotificationManager()
    removed: list = []
    mgr.notification_removed.connect(removed.append)
    data = mgr.show(NotificationLevel.INFO, "x", "m", timeout_ms=0)
    mgr.dismiss(data.uid)
    assert len(removed) == 1
    assert removed[0] is data


def test_notification_removed_signal_on_clear(qtbot):
    """clear 对每条通知触发 notification_removed。"""
    mgr = NotificationManager()
    removed: list = []
    mgr.notification_removed.connect(removed.append)
    for i in range(3):
        mgr.show(NotificationLevel.INFO, f"n{i}", "m", timeout_ms=0)
    mgr.clear()
    assert len(removed) == 3


def test_show_returns_data_with_unique_uid(qtbot):
    """每次 show 返回的 NotificationData 有唯一递增 uid。"""
    mgr = NotificationManager()
    uids = [
        mgr.show(NotificationLevel.INFO, f"n{i}", "m", timeout_ms=0).uid
        for i in range(3)
    ]
    assert len(set(uids)) == 3  # 全部唯一
    assert uids == sorted(uids)  # 递增
