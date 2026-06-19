"""Batch 34 测试：WidgetPaletteButton 拖拽高亮（dragging 属性）。

覆盖：
1. _set_dragging(True) 设置 dragging 属性为 True。
2. _set_dragging(False) 清除 dragging 属性。
3. _start_drag 调用前后 dragging 属性切换（mock QDrag.exec 避免 GUI 阻塞）。
4. QSS 含 [dragging="true"] 选择器。
5. 源码接入断言。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


def _make_button(qtbot):
    from embeddebug.serial_station.ui.dashboard.palette import WidgetPaletteButton

    btn = WidgetPaletteButton("led", "LED", "circle")
    qtbot.addWidget(btn)
    return btn


def test_set_dragging_true(qtbot):
    """_set_dragging(True) 应把 dragging 属性设为 True。"""

    btn = _make_button(qtbot)
    btn._set_dragging(True)
    assert btn.property("dragging") is True


def test_set_dragging_false(qtbot):
    """_set_dragging(False) 应把 dragging 属性设为 False。"""

    btn = _make_button(qtbot)
    btn._set_dragging(True)
    btn._set_dragging(False)
    assert btn.property("dragging") is False


def test_set_dragging_does_not_crash_without_app(qtbot, monkeypatch):
    """无 QApplication 时 _set_dragging 不崩（polish 失败静默）。"""

    btn = _make_button(qtbot)
    # 模拟 app.style() 返回 None 触发 except 路径。
    import PyQt6.QtWidgets as QtWidgets

    monkeypatch.setattr(QtWidgets.QApplication, "instance", staticmethod(lambda: None))
    btn._set_dragging(True)  # 不应抛异常
    assert btn.property("dragging") is True


def test_start_drag_toggles_dragging(qtbot, monkeypatch):
    """_start_drag 执行期间 dragging=True，结束后 False（mock QDrag.exec）。"""

    btn = _make_button(qtbot)
    # 拦截 QDrag.exec 避免进入真正拖拽事件循环（会阻塞）。
    import embeddebug.serial_station.ui.dashboard.palette as palette_mod

    monkeypatch.setattr(palette_mod.QDrag, "exec", lambda *a, **k: None)
    monkeypatch.setattr(palette_mod.QDrag, "setMimeData", lambda *a, **k: None)
    monkeypatch.setattr(palette_mod.QDrag, "setPixmap", lambda *a, **k: None)
    from PyQt6.QtCore import QEvent, QPointF, Qt
    from PyQt6.QtGui import QMouseEvent

    event = QMouseEvent(
        QEvent.Type.MouseButtonPress, QPointF(5, 5), QPointF(5, 5),
        Qt.MouseButton.LeftButton, Qt.MouseButton.LeftButton, Qt.KeyboardModifier.NoModifier,
    )
    # _start_drag 内 exec 被拦截后立即返回，dragging 应已清回 False。
    btn._start_drag(event)
    assert btn.property("dragging") is False  # exec 返回后清除


# ── QSS 覆盖 ───────────────────────────────────────────────────────
def test_qss_has_dragging_selector():
    """build_qss 应含 [dragging="true"] 选择器（Batch 34）。"""

    from embeddebug.serial_station.ui.theme.qss_builder import build_qss

    qss = build_qss()
    assert '[dragging="true"]' in qss


# ── 源码接入断言 ───────────────────────────────────────────────────
def test_widget_palette_button_has_set_dragging():
    """WidgetPaletteButton 应有 _set_dragging 方法。"""

    from embeddebug.serial_station.ui.dashboard.palette import WidgetPaletteButton

    src = inspect.getsource(WidgetPaletteButton)
    assert "_set_dragging" in src
    assert 'setProperty("dragging"' in src
