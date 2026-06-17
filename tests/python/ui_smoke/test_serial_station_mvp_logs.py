from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QComboBox, QLabel, QLineEdit, QPlainTextEdit, QPushButton

from embeddebug.app.main import build_main_window


def test_pyqt_mvp_filters_log_by_search_text(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    inject_edit = window.findChild(QLineEdit, "serialStationInjectEdit")
    inject_button = window.findChild(QPushButton, "serialStationInjectButton")
    log_filter_combo = window.findChild(QComboBox, "serialStationLogFilterCombo")
    log_search_edit = window.findChild(QLineEdit, "serialStationLogSearchEdit")
    log_view = window.findChild(QPlainTextEdit, "serialStationLogView")

    assert connect_button is not None
    assert send_edit is not None
    assert send_button is not None
    assert inject_edit is not None
    assert inject_button is not None
    assert log_filter_combo is not None
    assert log_search_edit is not None
    assert log_view is not None

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    send_edit.setText("alpha-command")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    inject_edit.setText("beta-reply")
    qtbot.mouseClick(inject_button, Qt.MouseButton.LeftButton)

    qtbot.waitUntil(
        lambda: "TX alpha-command" in log_view.toPlainText()
        and "RX beta-reply" in log_view.toPlainText(),
        timeout=1000,
    )

    log_search_edit.setText("beta")
    assert "TX alpha-command" not in log_view.toPlainText()
    assert "RX beta-reply" in log_view.toPlainText()

    log_filter_combo.setCurrentText("TX")
    assert log_view.toPlainText() == ""

    log_search_edit.clear()
    assert "TX alpha-command" in log_view.toPlainText()
    assert "RX beta-reply" not in log_view.toPlainText()


def test_pyqt_mvp_log_stats_follow_entries_and_filters(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)
    window.show()

    connect_button = window.findChild(QPushButton, "serialStationConnectButton")
    send_edit = window.findChild(QLineEdit, "serialStationSendEdit")
    send_button = window.findChild(QPushButton, "serialStationSendButton")
    inject_edit = window.findChild(QLineEdit, "serialStationInjectEdit")
    inject_button = window.findChild(QPushButton, "serialStationInjectButton")
    clear_button = window.findChild(QPushButton, "serialStationClearButton")
    log_filter_combo = window.findChild(QComboBox, "serialStationLogFilterCombo")
    log_stats_label = window.findChild(QLabel, "serialStationLogStatsLabel")

    assert connect_button is not None
    assert send_edit is not None
    assert send_button is not None
    assert inject_edit is not None
    assert inject_button is not None
    assert clear_button is not None
    assert log_filter_combo is not None
    assert log_stats_label is not None
    assert log_stats_label.text() == "Visible 0 / Total 0 | TX 0 | RX 0 | System 0 | Error 0"

    qtbot.mouseClick(connect_button, Qt.MouseButton.LeftButton)
    qtbot.waitUntil(lambda: "Visible 1 / Total 1" in log_stats_label.text(), timeout=1000)
    assert "System 1" in log_stats_label.text()

    send_edit.setText("stat-tx")
    qtbot.mouseClick(send_button, Qt.MouseButton.LeftButton)
    inject_edit.setText("stat-rx")
    qtbot.mouseClick(inject_button, Qt.MouseButton.LeftButton)

    qtbot.waitUntil(lambda: "Visible 3 / Total 3" in log_stats_label.text(), timeout=1000)
    assert "TX 1" in log_stats_label.text()
    assert "RX 1" in log_stats_label.text()
    assert "System 1" in log_stats_label.text()

    log_filter_combo.setCurrentText("TX")
    assert log_stats_label.text() == "Visible 1 / Total 3 | TX 1 | RX 1 | System 1 | Error 0"

    qtbot.mouseClick(clear_button, Qt.MouseButton.LeftButton)
    assert log_stats_label.text() == "Visible 0 / Total 0 | TX 0 | RX 0 | System 0 | Error 0"
