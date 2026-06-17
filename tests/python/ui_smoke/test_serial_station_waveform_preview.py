from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pyqtgraph as pg
from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QComboBox, QLabel, QLineEdit, QPushButton

from embeddebug.app.main import build_main_window


def test_fire_water_measurement_updates_waveform_preview(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    protocol_combo = window.findChild(QComboBox, "serialStationProtocolCombo")
    inject_edit = window.findChild(QLineEdit, "serialStationInjectEdit")
    inject_button = window.findChild(QPushButton, "serialStationInjectButton")
    plot_widget = window.findChild(pg.PlotWidget, "serialStationWaveformPlot")
    waveform_status = window.findChild(QLabel, "serialStationWaveformStatusLabel")

    assert connect_button is not None
    assert protocol_combo is not None
    assert inject_edit is not None
    assert inject_button is not None
    assert plot_widget is not None
    assert waveform_status is not None

    protocol_combo.setCurrentText("fire_water")
    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    inject_edit.setText("1.0,2.0\\n")
    qtbot.mouseClick(inject_button, Qt.MouseButton.LeftButton)

    qtbot.waitUntil(lambda: "2 channels" in waveform_status.text(), timeout=1000)
    assert plot_widget.objectName() == "serialStationWaveformPlot"
