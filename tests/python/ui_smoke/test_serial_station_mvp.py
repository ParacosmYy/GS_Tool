from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QComboBox, QLabel, QLineEdit, QPlainTextEdit, QPushButton

from embeddebug.app.main import build_main_window
from embeddebug.shared import OperationResult


def test_pyqt_mvp_fake_connect_send_receive_and_clear(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    history_combo = window.findChild(QComboBox, "serialStationCommandHistoryCombo")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    inject_edit = window.findChild(QLineEdit, "serialStationInjectEdit")
    inject_button = window.findChild(QPushButton, "serialStationInjectButton")
    clear_button = window.findChild(QPushButton, "serialStationClearButton")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    assert connect_button is not None
    assert send_edit is not None
    assert history_combo is not None
    assert send_button is not None
    assert inject_edit is not None
    assert inject_button is not None
    assert clear_button is not None
    assert log_view is not None
    assert status_label is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    assert "Connected" in status_label.text()

    send_edit.setText("ping")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    assert history_combo.currentText() == "ping"

    inject_edit.setText("pong")
    qtbot.mouseClick(inject_button, Qt.MouseButton.LeftButton)

    qtbot.waitUntil(
        lambda: "TX ping" in log_view.toPlainText()
        and "RX pong" in log_view.toPlainText(),
        timeout=1000,
    )

    qtbot.mouseClick(clear_button, Qt.MouseButton.LeftButton)
    assert log_view.toPlainText() == ""


def test_pyqt_mvp_command_history_refills_send_edit(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    history_combo = window.findChild(QComboBox, "serialStationCommandHistoryCombo")

    assert connect_button is not None
    assert send_edit is not None
    assert send_button is not None
    assert history_combo is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    send_edit.setText("first")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    send_edit.setText("second")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    send_edit.clear()

    history_combo.setCurrentText("first")

    assert send_edit.text() == "first"


def test_pyqt_mvp_send_failure_shows_result_message(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")
    log_stats_label = window.findChild(QLabel, "serialStationLogStatsLabel")

    assert send_edit is not None
    assert send_button is not None
    assert status_label is not None
    assert log_view is not None
    assert log_stats_label is not None

    send_edit.setText("before-open")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)

    assert "Send failed: Open a transport before sending" in status_label.text()
    assert "Error transport_not_open" in log_view.toPlainText()
    assert log_stats_label.text() == "Visible 1 / Total 1 | TX 0 | RX 0 | System 0 | Error 1"


def test_pyqt_mvp_fake_connection_failure_shows_result_message(qtbot, monkeypatch):
    monkeypatch.setattr(
        "embeddebug.serial_station.controllers.SerialWorkbenchController.connect_fake_result",
        lambda self: OperationResult.failure("transport_open_failed", "fake denied"),
    )
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    status_label = window.findChild(QLabel, "serialStationStatusLabel")

    assert connect_button is not None
    assert status_label is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)

    assert "Connection failed: fake denied" in status_label.text()


def test_pyqt_mvp_filters_log_by_direction(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    inject_edit = window.findChild(QLineEdit, "serialStationInjectEdit")
    inject_button = window.findChild(QPushButton, "serialStationInjectButton")
    log_filter_combo = window.findChild(QComboBox, "serialStationLogFilterCombo")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")

    assert connect_button is not None
    assert send_edit is not None
    assert send_button is not None
    assert inject_edit is not None
    assert inject_button is not None
    assert log_filter_combo is not None
    assert log_view is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    send_edit.setText("tx-only")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    inject_edit.setText("rx-only")
    qtbot.mouseClick(inject_button, Qt.MouseButton.LeftButton)

    qtbot.waitUntil(
        lambda: "TX tx-only" in log_view.toPlainText()
        and "RX rx-only" in log_view.toPlainText(),
        timeout=1000,
    )

    log_filter_combo.setCurrentText("TX")
    assert "TX tx-only" in log_view.toPlainText()
    assert "RX rx-only" not in log_view.toPlainText()

    log_filter_combo.setCurrentText("RX")
    assert "TX tx-only" not in log_view.toPlainText()
    assert "RX rx-only" in log_view.toPlainText()

    log_filter_combo.setCurrentText("All")
    assert "TX tx-only" in log_view.toPlainText()
    assert "RX rx-only" in log_view.toPlainText()


def test_pyqt_mvp_protocol_selection_is_system_log(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    protocol_combo = window.findChild(QComboBox, "serialStationProtocolCombo")
    log_filter_combo = window.findChild(QComboBox, "serialStationLogFilterCombo")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")
    log_stats_label = window.findChild(QLabel, "serialStationLogStatsLabel")

    assert protocol_combo is not None
    assert log_filter_combo is not None
    assert log_view is not None
    assert log_stats_label is not None

    protocol_combo.setCurrentText("fire_water")

    qtbot.waitUntil(lambda: "System protocol: fire_water" in log_view.toPlainText(), timeout=1000)
    assert log_stats_label.text() == "Visible 1 / Total 1 | TX 0 | RX 0 | System 1 | Error 0"

    log_filter_combo.setCurrentText("System")

    assert "System protocol: fire_water" in log_view.toPlainText()
