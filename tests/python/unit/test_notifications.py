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
    assert NotificationLevel.INFO.value == "info"
    assert NotificationLevel.ERROR.value == "error"


def test_history_capped(qtbot):
    mgr = NotificationManager(max_visible=10, max_history=5)
    for i in range(8):
        mgr.show(NotificationLevel.INFO, f"n{i}", "m", timeout_ms=0)
    assert len(mgr.history()) == 5
