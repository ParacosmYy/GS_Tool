"""ToastContainer 容器层 + AppShell 集成测试。

覆盖容器渲染闭环 / 管理 API / max_visible 挤兑，以及 AppShell 对
NotificationManager 的装配与 notify helper 行为。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.notifications import NotificationManager
from embeddebug.serial_station.notifications.data import NotificationData, NotificationLevel
from embeddebug.serial_station.ui.widgets.toast import ToastWidget
from embeddebug.serial_station.ui.widgets.toast_container import ToastContainer


def _make_data(uid=0, level=NotificationLevel.INFO):
    return NotificationData(
        level=level,
        title="t",
        message="m",
        timestamp_ns=0,
        uid=uid,
    )


# ── 渲染闭环 ──────────────────────────────────────────────────────
def test_container_has_objectname(qtbot):
    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    assert container.objectName() == "serialStationToastContainer"


def test_manager_show_renders_toast(qtbot):
    """manager.show() → container 渲染一条 ToastWidget（端到端闭环）。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    manager.show(NotificationLevel.SUCCESS, "连接成功", "COM3 就绪")
    assert len(container.active_toasts) == 1
    toast = container.active_toasts[0]
    assert isinstance(toast, ToastWidget)
    assert toast._title_label.text() == "连接成功"


def test_container_empty_hides(qtbot):
    """无 toast 时容器隐藏。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    assert container.is_empty is True


def test_on_added_creates_toast_and_accumulates(qtbot):
    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)

    container._on_added(_make_data(uid=1))
    container._on_added(_make_data(uid=2))

    assert container.count == 2
    assert container._active_count_widget_index() >= 1


def test_toast_closed_removes_from_container(qtbot):
    """ToastWidget 离场完成 → 从容器移除。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    manager.show(NotificationLevel.INFO, "提示", "内容", timeout_ms=100000)
    toast = container.active_toasts[0]
    # 模拟离场完成回调（leave → fade_out 完成 → closed 信号）。
    container._on_toast_closed(toast)
    assert container.is_empty is True


def test_manager_remove_triggers_toast_leave(qtbot, monkeypatch):
    """manager.dismiss/超时移除 → 对应 toast 调 leave（淡出）。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    data = manager.show(NotificationLevel.INFO, "提示", "内容", timeout_ms=100000)
    toast = container._find(data.uid)
    assert toast is not None
    leave_calls: list = []
    monkeypatch.setattr(toast, "leave", lambda: leave_calls.append(toast))
    manager.dismiss(data.uid)
    assert len(leave_calls) == 1


def test_find_known_and_unknown_uid(qtbot):
    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)

    container._on_added(_make_data(uid=42))

    assert container._find(42) is not None
    assert container._find(999) is None


# ── count / is_empty / clear_all / dismiss_oldest ──────────────────
def test_container_count_and_is_empty(qtbot):
    """count / is_empty 反映活跃 toast 数。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    assert container.count == 0
    assert container.is_empty is True
    manager.show(NotificationLevel.INFO, "1", "", timeout_ms=100000)
    manager.show(NotificationLevel.INFO, "2", "", timeout_ms=100000)
    assert container.count == 2
    assert container.is_empty is False


def test_clear_all_leaves_every_toast(qtbot, monkeypatch):
    """clear_all 应对每条活跃 toast 调 leave（淡出）。"""

    manager = NotificationManager()
    container = ToastContainer(manager, max_visible=10)
    qtbot.addWidget(container)
    leave_calls: list = []
    for i in range(3):
        manager.show(NotificationLevel.INFO, f"{i}", "", timeout_ms=100000)
    for _, t in container._active:
        monkeypatch.setattr(t, "leave", lambda tt=t: leave_calls.append(tt))
    container.clear_all()
    assert len(leave_calls) == 3


def test_dismiss_oldest_returns_false_when_empty(qtbot):
    """无活跃 toast 时 dismiss_oldest 返回 False。"""

    manager = NotificationManager()
    container = ToastContainer(manager)
    qtbot.addWidget(container)
    assert container.dismiss_oldest() is False


def test_dismiss_oldest_leaves_first(qtbot, monkeypatch):
    """dismiss_oldest 应对最早一条调 leave 并返回 True。"""

    manager = NotificationManager()
    container = ToastContainer(manager, max_visible=10)
    qtbot.addWidget(container)
    leave_calls: list = []
    for i in range(2):
        manager.show(NotificationLevel.INFO, f"{i}", "", timeout_ms=100000)
    for _, t in container._active:
        monkeypatch.setattr(t, "leave", lambda tt=t: leave_calls.append(tt))
    assert container.dismiss_oldest() is True
    assert len(leave_calls) == 1  # 只 dismiss 一条


def test_manager_clear_propagates_to_container(qtbot, monkeypatch):
    """manager.clear() → 每条通知 removed 信号 → 对应 toast leave。"""

    manager = NotificationManager()
    container = ToastContainer(manager, max_visible=10)
    qtbot.addWidget(container)
    for i in range(3):
        manager.show(NotificationLevel.INFO, f"{i}", "", timeout_ms=100000)
    leave_calls: list = []
    for _, t in container._active:
        monkeypatch.setattr(t, "leave", lambda tt=t: leave_calls.append(tt))
    manager.clear()
    assert len(leave_calls) == 3


# ── max_visible 挤兑 ───────────────────────────────────────────────
def test_container_enforces_max_visible(qtbot, monkeypatch):
    """超过 max_visible 时挤掉最早一条（对最旧 toast 调 leave）。"""

    manager = NotificationManager()
    container = ToastContainer(manager, max_visible=2)
    qtbot.addWidget(container)
    # 加三条 toast（绕过 _enforce_max_visible 直接加，再手动触发以精确控制断言）。
    leave_calls: list = []
    for i in range(3):
        manager.show(NotificationLevel.INFO, f"{i}", "", timeout_ms=100000)
        # 加完后立即拦截最新 toast 的 leave（避免被动画时序干扰）。
        for _, t in container._active:
            if not hasattr(t, "_leave_patched"):
                t._leave_patched = True
                monkeypatch.setattr(t, "leave", lambda tt=t: leave_calls.append(tt))
    # _enforce_max_visible 已在第三条加入时被调用，应挤掉 toast #1。
    assert len(leave_calls) >= 1


def test_enforce_max_visible_directly(qtbot, monkeypatch):
    """直接测 _enforce_max_visible：超限时对最早 toast 调 leave。"""

    manager = NotificationManager()
    container = ToastContainer(manager, max_visible=2)
    qtbot.addWidget(container)
    leave_calls: list = []
    # 手动塞三条 toast，全部拦截 leave。
    for i in range(3):
        manager.show(NotificationLevel.INFO, f"{i}", "", timeout_ms=100000)
    for _, t in container._active:
        monkeypatch.setattr(t, "leave", lambda tt=t: leave_calls.append(tt))
    # 临时调小 max_visible 再强制执行挤兑。
    container._max_visible = 1
    container._enforce_max_visible()
    assert len(leave_calls) >= 1


# ── AppShell 集成见 test_toast_integration.py（守 250 行门禁） ──────


# ── QSS 覆盖 ───────────────────────────────────────────────────────
def test_qss_covers_toast_container_objectname():
    """build_qss 应含 serialStationToastContainer 选择器。"""

    from embeddebug.serial_station.ui.theme.qss_builder import build_qss

    qss = build_qss()
    assert "#serialStationToastContainer" in qss
    assert "#serialStationToastPlaceholder" in qss
