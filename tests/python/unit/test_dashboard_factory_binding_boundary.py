"""dashboard/factory apply_binding + read_binding + _apply_parsed 边界测试。

test_dashboard_binding 覆盖 create_bound_widget；本文件补 apply_binding 直接调用 +
read_binding 各 widget_type + invalid spec + binding_spec 属性。

覆盖：
1. apply_binding value_display → binding_channel 属性。
2. apply_binding led → binding_level 属性。
3. apply_binding button → command_template。
4. apply_binding invalid spec → False。
5. apply_binding 写 binding_spec raw 属性。
6. read_binding 无属性返回 None。
7. read_binding slider 读回。
8. read_binding button 读回。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.dashboard.factory import (
    apply_binding,
    create_widget,
    read_binding,
)


# ── apply_binding value_display ──────────────────────────────────
def test_apply_binding_value_display_sets_channel(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    w = create_widget("value_display", parent)
    qtbot.addWidget(w)
    result = apply_binding(w, "value_display", "value:ch3")
    assert result is True
    assert w.property("binding_channel") == 3  # 通道索引（int）


def test_apply_binding_led_sets_level(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    w = create_widget("led", parent)
    qtbot.addWidget(w)
    result = apply_binding(w, "led", "led:warning")
    assert result is True
    assert w.property("binding_level") is not None


def test_apply_binding_button_sets_command(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    w = create_widget("button", parent)
    qtbot.addWidget(w)
    result = apply_binding(w, "button", "button:RESET_NOW")
    assert result is True
    assert w.get_command() == "RESET_NOW"


# ── apply_binding invalid ────────────────────────────────────────
def test_apply_binding_invalid_spec_returns_false(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    w = create_widget("led", parent)
    qtbot.addWidget(w)
    result = apply_binding(w, "led", "garbage_no_match")
    assert result is False


# ── apply_binding binding_spec raw ───────────────────────────────
def test_apply_binding_writes_raw_spec(qtbot):
    """apply_binding 把 raw spec 写到 binding_spec 属性。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    w = create_widget("value_display", parent)
    qtbot.addWidget(w)
    apply_binding(w, "value_display", "value:ch5")
    assert w.property("binding_spec") == "value:ch5"


# ── read_binding ─────────────────────────────────────────────────
def test_read_binding_no_property_returns_none(qtbot):
    """read_binding 无属性返回 None。"""

    parent = QWidget()
    qtbot.addWidget(parent)
    w = create_widget("led", parent)
    qtbot.addWidget(w)
    assert read_binding(w) is None


def test_read_binding_slider_reads_back(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    w = create_widget("slider", parent)
    qtbot.addWidget(w)
    apply_binding(w, "slider", "slider:SET_VOLTAGE,0,200")
    result = read_binding(w)
    assert result is not None
    assert "SET_VOLTAGE" in result


def test_read_binding_button_reads_back(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    w = create_widget("button", parent)
    qtbot.addWidget(w)
    apply_binding(w, "button", "button:RESET")
    result = read_binding(w)
    assert result is not None
    assert "RESET" in result
