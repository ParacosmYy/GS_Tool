"""ToastWidget 控件层测试。

覆盖单条 toast 控件：渲染 / objectName / 级别色条 / 入场(SlideAnimation) /
离场(FadeTransition) / 自动消失定时器 / closed 信号 / stop_animations /
源码接入断言（引用三动画模块）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock

from embeddebug.serial_station.notifications.data import (
    NotificationData,
    NotificationLevel,
)
from embeddebug.serial_station.ui.widgets.toast import ToastWidget


def _make_data(
    level=NotificationLevel.INFO,
    title="提示",
    message="内容",
    timeout_ms=3000,
) -> NotificationData:
    return NotificationData.create(level, title, message, timeout_ms=timeout_ms)


# ── 渲染 + objectName ──────────────────────────────────────────────
def test_toast_has_objectname(qtbot):
    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    assert toast.objectName() == "serialStationToast"


def test_toast_renders_title_and_message(qtbot):
    toast = ToastWidget(_make_data(title="连接成功", message="串口已就绪"))
    qtbot.addWidget(toast)
    assert toast._title_label.text() == "连接成功"
    assert toast._message_label.text() == "串口已就绪"
    assert toast.data.title == "连接成功"
    assert toast.data.message == "串口已就绪"


def test_toast_data_returns_notification_data(qtbot):
    toast = ToastWidget(_make_data(level=NotificationLevel.ERROR))
    qtbot.addWidget(toast)

    assert isinstance(toast.data, NotificationData)
    assert toast.data.level == NotificationLevel.ERROR


def test_toast_message_optional(qtbot):
    """message 为空时不创建 message label（无副标题 toast）。"""

    toast = ToastWidget(_make_data(title="提示", message=""))
    qtbot.addWidget(toast)
    assert toast._title_label.text() == "提示"
    assert not hasattr(toast, "_message_label")


def test_toast_accent_uses_level_color(qtbot):
    """色条 stylesheet 应含级别色（取 palette，无硬编码漂移）。"""

    from embeddebug.serial_station.ui.theme import palette as P

    toast = ToastWidget(_make_data(level=NotificationLevel.ERROR))
    qtbot.addWidget(toast)
    style = toast._accent.styleSheet()
    assert P.ERROR in style


def test_toast_is_leaving_initial_false(qtbot):
    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)

    assert toast.is_leaving is False


# ── 入场动画：SlideAnimation 激活 ───────────────────────────────────
def test_toast_enter_calls_slide_in(qtbot, monkeypatch):
    """enter() 应调用 SlideAnimation.slide_in（死代码激活）。"""

    import embeddebug.serial_station.ui.widgets.toast as toast_mod

    calls: list = []
    fake_anim = MagicMock()
    fake_anim.start = MagicMock()

    def _fake_slide_in(widget, direction=None, distance=0, **k):
        calls.append((widget, direction, distance))
        return fake_anim

    monkeypatch.setattr(toast_mod.SlideAnimation, "slide_in", staticmethod(_fake_slide_in))
    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    toast.enter()
    assert len(calls) == 1
    assert calls[0][0] is toast
    assert calls[0][1] is toast_mod.SlideDirection.RIGHT


# ── 离场动画：FadeTransition 激活 ───────────────────────────────────
def test_toast_leave_calls_fade_out(qtbot, monkeypatch):
    """leave() 应调用 FadeTransition.fade_out（死代码激活）。"""

    import embeddebug.serial_station.ui.widgets.toast as toast_mod

    calls: list = []
    fake_anim = MagicMock()
    fake_anim.start = MagicMock()
    # fade_out 完成回调链：finished 信号。
    fake_anim.finished = MagicMock()

    def _fake_fade_out(widget, **k):
        calls.append(widget)
        return fake_anim

    monkeypatch.setattr(toast_mod.FadeTransition, "fade_out", staticmethod(_fake_fade_out))
    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    toast.leave()
    assert len(calls) == 1
    assert calls[0] is toast
    assert toast.is_leaving is True


def test_toast_leave_is_idempotent(qtbot, monkeypatch):
    """重复 leave() 不应二次触发 fade_out（is_leaving 防重复）。"""

    import embeddebug.serial_station.ui.widgets.toast as toast_mod

    calls: list = []
    fake_anim = MagicMock()
    fake_anim.finished = MagicMock()

    monkeypatch.setattr(
        toast_mod.FadeTransition,
        "fade_out",
        staticmethod(lambda w, **k: (calls.append(w) or fake_anim)),
    )
    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    toast.leave()
    toast.leave()
    assert len(calls) == 1


# ── 自动消失定时器 ─────────────────────────────────────────────────
def test_toast_schedule_dismiss_starts_timer(qtbot):
    toast = ToastWidget(_make_data(timeout_ms=500))
    qtbot.addWidget(toast)
    toast.schedule_dismiss(500)
    assert toast._dismiss_timer is not None
    assert toast._dismiss_timer.isActive()


def test_toast_schedule_dismiss_zero_no_timer(qtbot):
    """timeout_ms<=0 表示手动关闭，不应启动定时器。"""

    toast = ToastWidget(_make_data(timeout_ms=0))
    qtbot.addWidget(toast)
    toast.schedule_dismiss(0)
    assert toast._dismiss_timer is None


# ── closed 信号 ────────────────────────────────────────────────────
def test_toast_closed_signal_on_leave(qtbot, monkeypatch):
    """fade_out 完成后应发 closed 信号。"""

    import embeddebug.serial_station.ui.widgets.toast as toast_mod

    fake_anim = MagicMock()
    # 捕获 finished 连接的回调，手动调用模拟动画完成。
    connected: list = []

    def _connect(slot):
        connected.append(slot)

    fake_anim.finished.connect = _connect
    fake_anim.start = MagicMock()
    monkeypatch.setattr(
        toast_mod.FadeTransition, "fade_out", staticmethod(lambda w, **k: fake_anim)
    )

    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)
    received: list = []
    toast.closed.connect(lambda d: received.append(d))
    toast.leave()
    # 模拟 fade_out 动画完成 → 触发 _emit_closed。
    assert len(connected) == 1
    connected[0]()
    assert received == [toast]


# ── stop_animations ────────────────────────────────────────────────
def test_toast_stop_animations_clears(qtbot):
    """stop_animations 应停止定时器并清理动画控制器。"""

    toast = ToastWidget(_make_data(timeout_ms=500))
    qtbot.addWidget(toast)
    toast.schedule_dismiss(500)
    assert toast._dismiss_timer is not None
    toast.stop_animations()
    assert toast._dismiss_timer is None


def test_toast_stop_animations_idempotent(qtbot):
    toast = ToastWidget(_make_data())
    qtbot.addWidget(toast)

    toast.stop_animations()
    toast.stop_animations()


# ── 死代码接入断言 ─────────────────────────────────────────────────
def test_toast_imports_three_dead_modules():
    """toast 源码应引用 SlideAnimation + FadeTransition + AnimationController。"""

    from embeddebug.serial_station.ui.widgets import toast

    src = inspect.getsource(toast)
    assert "SlideAnimation" in src and "slide_in" in src
    assert "FadeTransition" in src and "fade_out" in src
    assert "AnimationController" in src
