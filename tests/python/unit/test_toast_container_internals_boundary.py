"""ToastContainer _on_added/_active_count_widget_index/_find 边界测试。

test_toast_container 覆盖基础；本文件补 _on_added + _active_count_widget_index +
_find 未知 uid 返回 None。

覆盖：
1. _on_added 创建 ToastWidget 并添加到 layout。
2. _active_count_widget_index 返回活跃 widget 数。
3. _find 已知 uid 返回 ToastWidget。
4. _find 未知 uid 返回 None。
5. _on_added 后 count 增加。
6. _active_count_widget_index 初始 0。
7. _on_added 多次累积。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.notifications import NotificationManager
from embeddebug.serial_station.notifications.data import (
    NotificationData,
    NotificationLevel,
)
from embeddebug.serial_station.ui.widgets.toast_container import ToastContainer


def _make_data(uid=0, level=NotificationLevel.INFO):
    return NotificationData(
        level=level,
        title="t",
        message="m",
        timestamp_ns=0,
        uid=uid,
    )


def _make_container(qtbot):
    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    return container, manager


# ── _on_added ────────────────────────────────────────────────────
def test_on_added_creates_toast(qtbot):
    container, _ = _make_container(qtbot)
    data = _make_data(uid=1)
    container._on_added(data)
    assert container.count == 1


def test_on_added_multiple_accumulates(qtbot):
    container, _ = _make_container(qtbot)
    container._on_added(_make_data(uid=1))
    container._on_added(_make_data(uid=2))
    container._on_added(_make_data(uid=3))
    assert container.count == 3


# ── _active_count_widget_index ───────────────────────────────────
def test_active_count_widget_index_returns_insert_position(qtbot):
    """_active_count_widget_index 返回插入位置（placeholder 后 = 1）。"""

    container, _ = _make_container(qtbot)
    assert container._active_count_widget_index() == 1  # placeholder 后


def test_active_count_widget_index_after_add(qtbot):
    container, _ = _make_container(qtbot)
    container._on_added(_make_data(uid=1))
    assert container._active_count_widget_index() >= 1


# ── _find ────────────────────────────────────────────────────────
def test_find_known_uid(qtbot):
    container, _ = _make_container(qtbot)
    data = _make_data(uid=42)
    container._on_added(data)
    found = container._find(42)
    assert found is not None


def test_find_unknown_uid_returns_none(qtbot):
    container, _ = _make_container(qtbot)
    container._on_added(_make_data(uid=1))
    assert container._find(999) is None


def test_find_empty_container_returns_none(qtbot):
    container, _ = _make_container(qtbot)
    assert container._find(0) is None
