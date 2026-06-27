"""B10 命令↔控件绑定层测试。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QPushButton, QSlider

from embeddebug.serial_station.controllers.command_history_state import (
    remember_command,
    restore_command_history,
)
from embeddebug.serial_station.ui.command_binding import CommandBinder


def _make_binder() -> tuple[CommandBinder, list[str]]:
    sent: list[str] = []
    binder = CommandBinder(send_callable=lambda cmd: sent.append(cmd))
    return binder, sent


def test_binder_bind_slider_emits_on_value_change(qtbot):
    binder, sent = _make_binder()
    slider = QSlider(Qt.Orientation.Horizontal)
    qtbot.addWidget(slider)
    binder.bind_slider(slider, "PWM={value}")
    slider.setValue(42)
    assert "PWM=42" in sent


def test_binder_bind_button_emits_on_click(qtbot):
    binder, sent = _make_binder()
    button = QPushButton("Fire")
    qtbot.addWidget(button)
    binder.bind_button(button, "FIRE 1")
    button.click()
    assert "FIRE 1" in sent


def test_binder_bind_custom_emits_on_signal(qtbot):
    binder, sent = _make_binder()
    button = QPushButton("X")
    qtbot.addWidget(button)
    binder.bind_custom(button, lambda: "CUSTOM", button.clicked, binding_id="custom1")
    button.click()
    assert "CUSTOM" in sent


def test_binder_remove_binding(qtbot):
    binder, sent = _make_binder()
    slider = QSlider(Qt.Orientation.Horizontal)
    qtbot.addWidget(slider)
    bid = binder.bind_slider(slider, "X={value}")
    assert binder.remove(bid) is True
    slider.setValue(99)
    assert sent == []


def test_binder_remove_unknown_returns_false(qtbot):
    binder, _ = _make_binder()
    assert binder.remove("nope") is False


def test_binder_disable_stops_sending(qtbot):
    binder, sent = _make_binder()
    slider = QSlider(Qt.Orientation.Horizontal)
    qtbot.addWidget(slider)
    bid = binder.bind_slider(slider, "X={value}")
    binder.disable(bid)
    slider.setValue(5)
    assert sent == []


def test_binder_enable_resumes_sending(qtbot):
    binder, sent = _make_binder()
    slider = QSlider(Qt.Orientation.Horizontal)
    qtbot.addWidget(slider)
    bid = binder.bind_slider(slider, "X={value}")
    binder.disable(bid)
    slider.setValue(5)
    sent.clear()
    binder.enable(bid)
    slider.setValue(10)
    assert "X=10" in sent


def test_binder_binding_ids(qtbot):
    binder, _ = _make_binder()
    slider = QSlider(Qt.Orientation.Horizontal)
    qtbot.addWidget(slider)
    button = QPushButton("B")
    qtbot.addWidget(button)
    binder.bind_slider(slider, "S={value}", binding_id="s1")
    binder.bind_button(button, "CMD", binding_id="b1")
    assert binder.binding_ids() == ["b1", "s1"]


def test_binder_binding_added_signal(qtbot):
    binder, _ = _make_binder()
    emitted: list[str] = []
    binder.binding_added.connect(lambda bid: emitted.append(bid))
    slider = QSlider(Qt.Orientation.Horizontal)
    qtbot.addWidget(slider)
    binder.bind_slider(slider, "X={value}", binding_id="s1")
    assert emitted == ["s1"]


def test_binder_binding_removed_signal(qtbot):
    binder, _ = _make_binder()
    slider = QSlider(Qt.Orientation.Horizontal)
    qtbot.addWidget(slider)
    bid = binder.bind_slider(slider, "X={value}", binding_id="s1")
    emitted: list[str] = []
    binder.binding_removed.connect(lambda bid: emitted.append(bid))
    binder.remove(bid)
    assert emitted == ["s1"]


def test_binder_multiple_sliders_independent(qtbot):
    binder, sent = _make_binder()
    s1 = QSlider(Qt.Orientation.Horizontal)
    s2 = QSlider(Qt.Orientation.Horizontal)
    qtbot.addWidget(s1)
    qtbot.addWidget(s2)
    binder.bind_slider(s1, "A={value}", binding_id="a")
    binder.bind_slider(s2, "B={value}", binding_id="b")
    s1.setValue(1)
    s2.setValue(2)
    assert "A=1" in sent
    assert "B=2" in sent


def test_remember_new_command():
    h = []
    remember_command(h, "AT")
    assert h == ["AT"]


def test_remember_moves_to_end():
    h = ["AT", "ATZ"]
    remember_command(h, "AT")
    assert h == ["ATZ", "AT"]


def test_remember_multiple_unique():
    h = []
    remember_command(h, "a")
    remember_command(h, "b")
    remember_command(h, "c")
    assert h == ["a", "b", "c"]


def test_remember_duplicate_only_once():
    h = []
    remember_command(h, "x")
    remember_command(h, "x")
    assert h == ["x"]


def test_restore_from_list():
    h = ["old"]
    restore_command_history(h, ["a", "b"])
    assert h == ["a", "b"]


def test_restore_clears_existing():
    h = ["old1", "old2"]
    restore_command_history(h, [])
    assert h == []


def test_restore_non_list_ignored():
    h = ["keep"]
    restore_command_history(h, "not a list")
    assert h == []


def test_restore_filters_non_string():
    h = []
    restore_command_history(h, ["valid", 123, "", "also_valid"])
    assert h == ["valid", "also_valid"]
