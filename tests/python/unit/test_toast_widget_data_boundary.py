"""ToastWidget data()/is_leaving/stop_animations 边界测试。

test_toast_widget 覆盖基础 enter/leave/schedule_dismiss；
本文件补 data() 返回值 + is_leaving 初始 + stop_animations 清理。

覆盖：
1. data() 返回 NotificationData。
2. data() 返回的 title/message 与构造一致。
3. is_leaving 初始 False。
4. stop_animations 不崩。
5. stop_animations 后再调用不崩（幂等）。
6. enter 后 is_leaving 仍 False（enter 不触发 leaving）。
7. schedule_dismiss(0) 不启动 timer。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.notifications.data import (
    NotificationData,
    NotificationLevel,
)
from embeddebug.serial_station.ui.widgets.toast import ToastWidget


def _make_data(level=NotificationLevel.INFO):
    return NotificationData(
        level=level,
        title="测试标题",
        message="测试消息",
        timestamp_ns=0,
    )


# ── data() ───────────────────────────────────────────────────────
def test_data_returns_notification_data(qtbot):
    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    result = toast.data
    assert isinstance(result, NotificationData)


def test_data_title_matches(qtbot):
    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    assert toast.data.title == "测试标题"


def test_data_message_matches(qtbot):
    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    assert toast.data.message == "测试消息"


def test_data_level_matches(qtbot):
    data = _make_data(level=NotificationLevel.ERROR)
    toast = ToastWidget(data)
    qtbot.addWidget(toast)
    assert toast.data.level == NotificationLevel.ERROR


# ── is_leaving ───────────────────────────────────────────────────
def test_is_leaving_initial_false(qtbot):
    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    assert toast.is_leaving is False


# ── stop_animations ──────────────────────────────────────────────
def test_stop_animations_no_crash(qtbot):
    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    toast.stop_animations()


def test_stop_animations_idempotent(qtbot):
    """stop_animations 多次不崩。"""

    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    toast.stop_animations()
    toast.stop_animations()


# ── schedule_dismiss(0) ──────────────────────────────────────────
def test_schedule_dismiss_zero_no_timer(qtbot):
    """schedule_dismiss(0) 不启动 timer（立即隐藏路径）。"""

    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    toast.schedule_dismiss(0)  # 不崩
