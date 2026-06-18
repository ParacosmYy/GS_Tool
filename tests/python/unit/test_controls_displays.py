"""B6 控件库测试（二）：可配置按钮 / 仪表盘 / 数值显示。"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QLabel, QPushButton

from embeddebug.serial_station.ui.controls import (
    ConfigurableButton,
    GaugeWidget,
    ValueDisplay,
)


# ── 可配置按钮 ────────────────────────────────────────────────────
def test_configurable_button_has_objectname(qtbot):
    btn = ConfigurableButton(text="Fire")
    qtbot.addWidget(btn)
    assert btn.objectName() == "serialStationConfigurableButton"


def test_configurable_button_emits_command_on_click(qtbot):
    btn = ConfigurableButton(text="Fire", command_template="FIRE 1")
    qtbot.addWidget(btn)
    commands: list[str] = []
    btn.command.connect(lambda cmd: commands.append(cmd))
    btn.click()
    assert commands == ["FIRE 1"]


def test_configurable_button_get_command(qtbot):
    btn = ConfigurableButton(text="Reset", command_template="RST")
    qtbot.addWidget(btn)
    assert btn.get_command() == "RST"


def test_configurable_button_custom_formatter(qtbot):
    btn = ConfigurableButton(text="Inc")
    qtbot.addWidget(btn)
    btn.set_formatter(lambda: "INCREMENT")
    assert btn.get_command() == "INCREMENT"
    commands: list[str] = []
    btn.command.connect(lambda cmd: commands.append(cmd))
    btn.click()
    assert commands == ["INCREMENT"]


def test_configurable_button_no_command_when_empty(qtbot):
    btn = ConfigurableButton(text="Noop")
    qtbot.addWidget(btn)
    commands: list[str] = []
    btn.command.connect(lambda cmd: commands.append(cmd))
    btn.click()
    assert commands == []


def test_configurable_button_is_qpushbutton(qtbot):
    btn = ConfigurableButton(text="X")
    qtbot.addWidget(btn)
    assert isinstance(btn, QPushButton)


# ── 仪表盘 ────────────────────────────────────────────────────────
def test_gauge_has_objectname(qtbot):
    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    assert gauge.objectName() == "serialStationGauge"


def test_gauge_default_value(qtbot):
    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    assert gauge.value() == 0.0


def test_gauge_set_value(qtbot):
    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_value(42.5)
    assert gauge.value() == 42.5


def test_gauge_set_range(qtbot):
    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_range(-50, 150)
    gauge.set_value(100)
    assert gauge.value() == 100.0


def test_gauge_set_label_and_unit(qtbot):
    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.set_label("RPM")
    gauge.set_unit(" rpm")
    gauge.set_value(3000)
    assert gauge.value() == 3000.0


def test_gauge_paint_does_not_crash(qtbot):
    """离屏环境下 paintEvent 不崩溃。"""

    gauge = GaugeWidget()
    qtbot.addWidget(gauge)
    gauge.resize(140, 140)
    gauge.set_range(0, 100)
    gauge.set_value(75)
    gauge.repaint()


# ── 数值显示 ──────────────────────────────────────────────────────
def test_value_display_has_objectname(qtbot):
    display = ValueDisplay()
    qtbot.addWidget(display)
    assert display.objectName() == "serialStationValueDisplay"


def test_value_display_set_value(qtbot):
    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(3.14)
    assert display.value() == 3.14


def test_value_display_trend_up(qtbot):
    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(10.0)
    display.set_value(15.0)
    trend = display.findChild(QLabel, "serialStationValueTrend")
    assert trend is not None
    assert "▲" in trend.text()


def test_value_display_trend_down(qtbot):
    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(20.0)
    display.set_value(5.0)
    trend = display.findChild(QLabel, "serialStationValueTrend")
    assert trend is not None
    assert "▼" in trend.text()


def test_value_display_trend_flat(qtbot):
    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_value(5.0)
    display.set_value(5.0)
    trend = display.findChild(QLabel, "serialStationValueTrend")
    assert trend is not None
    assert "◆" in trend.text()


def test_value_display_label_and_unit(qtbot):
    display = ValueDisplay()
    qtbot.addWidget(display)
    display.set_label("Voltage")
    display.set_unit("V")
    display.set_value(12.5)
    number = display.findChild(QLabel, "serialStationValueNumber")
    label = display.findChild(QLabel, "serialStationValueLabel")
    assert number is not None and "12.50V" in number.text()
    assert label is not None and label.text() == "Voltage"
