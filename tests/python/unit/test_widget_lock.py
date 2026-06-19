"""dashboard 控件锁定/解锁测试（防意外删除）。

覆盖：
1. _is_locked 默认 False。
2. _toggle_lock 切换锁定态（config[locked]）。
3. 锁定后 _safe_remove 跳过删除（防意外删）。
4. 解锁后 _safe_remove 正常删除。
5. _is_locked/_toggle_lock None/缺失 item 安全。
6. 右键菜单含「锁定」action + 锁定时删除禁用。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPoint

from embeddebug.serial_station.ui.panels import _dashboard_widget_menu as wm


def _make_canvas_with_widget(qtbot, widget_type="led"):
    from embeddebug.serial_station.ui.dashboard import DashboardCanvas

    canvas = DashboardCanvas()
    qtbot.addWidget(canvas)
    item_id = canvas.add_widget_at(widget_type, QPoint(20, 20))
    return canvas, item_id


# ── _is_locked / _toggle_lock ──────────────────────────────────────
def test_is_locked_default_false(qtbot):
    """默认未锁定。"""

    canvas, item_id = _make_canvas_with_widget(qtbot)
    assert wm._is_locked(canvas, item_id) is False


def test_toggle_lock_sets_locked(qtbot):
    """_toggle_lock 应翻转 config[locked]。"""

    canvas, item_id = _make_canvas_with_widget(qtbot)
    wm._toggle_lock(canvas, item_id)
    assert wm._is_locked(canvas, item_id) is True
    assert canvas.items[item_id].config["locked"] is True


def test_toggle_lock_unlock(qtbot):
    """再次 toggle 应解锁。"""

    canvas, item_id = _make_canvas_with_widget(qtbot)
    wm._toggle_lock(canvas, item_id)  # 锁
    wm._toggle_lock(canvas, item_id)  # 解
    assert wm._is_locked(canvas, item_id) is False


# ── 锁定防删除 ────────────────────────────────────────────────────
def test_locked_widget_not_removed(qtbot):
    """锁定控件 _safe_remove 应跳过（items 不变）。"""

    canvas, item_id = _make_canvas_with_widget(qtbot)
    wm._toggle_lock(canvas, item_id)  # 锁定
    assert item_id in canvas.items
    wm._safe_remove(canvas, item_id)  # 应跳过
    assert item_id in canvas.items  # 仍在


def test_unlocked_widget_removed(qtbot):
    """未锁定控件 _safe_remove 应正常删除。"""

    canvas, item_id = _make_canvas_with_widget(qtbot)
    wm._safe_remove(canvas, item_id)
    assert item_id not in canvas.items


# ── None/缺失安全 ──────────────────────────────────────────────────
def test_is_locked_missing_item_false(qtbot):
    """缺失 item 返回 False（不崩）。"""

    canvas, _ = _make_canvas_with_widget(qtbot)
    assert wm._is_locked(canvas, "nonexistent") is False


def test_is_locked_none_canvas_false():
    """None canvas 返回 False（不崩）。"""

    assert wm._is_locked(None, "x") is False


def test_toggle_lock_missing_item_no_crash(qtbot):
    """toggle 缺失 item 不崩。"""

    canvas, _ = _make_canvas_with_widget(qtbot)
    wm._toggle_lock(canvas, "nonexistent")  # 不应抛异常


# ── 菜单含锁定 + 锁定时删除禁用 ────────────────────────────────────
def test_menu_has_lock_action(qtbot, monkeypatch):
    """右键菜单应含「锁定」action（共 6 action）。"""

    canvas, item_id = _make_canvas_with_widget(qtbot)
    widget = canvas.items[item_id].widget
    wm.attach_widget_delete_menu(widget, canvas, item_id)
    import PyQt6.QtWidgets as QtWidgets

    built: list = []
    original_init = QtWidgets.QMenu.__init__

    def _init(self, *a, **k):
        original_init(self, *a, **k)
        built.append(self)

    monkeypatch.setattr(QtWidgets.QMenu, "__init__", _init)
    monkeypatch.setattr(QtWidgets.QMenu, "exec", lambda *a, **k: None)
    widget.customContextMenuRequested.emit(QPoint(5, 5))
    assert len(built) == 1
    texts = [a.text() for a in built[0].actions()]
    assert "锁定" in texts
    assert len(texts) == 6  # 复制/属性/置顶/置底/锁定/删除


def test_menu_delete_disabled_when_locked(qtbot, monkeypatch):
    """锁定控件的删除 action 应禁用。"""

    canvas, item_id = _make_canvas_with_widget(qtbot)
    widget = canvas.items[item_id].widget
    wm._toggle_lock(canvas, item_id)  # 锁定
    wm.attach_widget_delete_menu(widget, canvas, item_id)
    import PyQt6.QtWidgets as QtWidgets

    built: list = []
    original_init = QtWidgets.QMenu.__init__

    def _init(self, *a, **k):
        original_init(self, *a, **k)
        built.append(self)

    monkeypatch.setattr(QtWidgets.QMenu, "__init__", _init)
    monkeypatch.setattr(QtWidgets.QMenu, "exec", lambda *a, **k: None)
    widget.customContextMenuRequested.emit(QPoint(5, 5))
    delete_action = [a for a in built[0].actions() if a.text() == "删除控件"][0]
    assert delete_action.isEnabled() is False  # 锁定时禁用
